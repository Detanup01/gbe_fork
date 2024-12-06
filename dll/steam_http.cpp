/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "dll/steam_http.h"

Steam_HTTP::Steam_HTTP(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

Steam_Http_Request *Steam_HTTP::get_request(HTTPRequestHandle hRequest)
{
    auto conn = std::find_if(requests.begin(), requests.end(), [&hRequest](struct Steam_Http_Request const& conn) {
        return conn.handle == hRequest;
    });
    if (conn == requests.end()) return nullptr;

    return &(*conn);
}

void Steam_HTTP::create_http_request_web( struct Steam_Http_Request &request, unsigned url_index )
{
    request.protocol = Steam_Http_Request::Protocol_t::Web;

    std::string_view url_path(request.url.c_str() + url_index, request.url.size() - url_index);
    // strip last occurrences pf '/'
    auto last_non_slash = url_path.find_last_not_of('/');
    if (std::string_view::npos != last_non_slash) {
        url_path = url_path.substr(0, last_non_slash + 1);
    }

    PRINT_DEBUG("parsed url path '%.*s'", url_path.size(), url_path.data());

    if (!url_path.empty()) {
        request.target_filepath =
            Local_Storage::get_game_settings_path()
            + "http" + PATH_SEPARATOR
            + Local_Storage::sanitize_string(std::string(url_path));
        PRINT_DEBUG("parsed filepath (absolute) '%s'", request.target_filepath.c_str());
        
        auto file_size = file_size_(request.target_filepath);
        if (file_size > 0) {
            request.response.resize(file_size);
            auto read = Local_Storage::get_file_data(request.target_filepath, (char *)&request.response[0], file_size, 0);
            if (read < 0) read = 0;
            if (read != file_size) request.response.resize(static_cast<size_t>(read));
            PRINT_DEBUG("read file data size=%i/%u", read, file_size);
        }
    }
}

void Steam_HTTP::create_http_request_file( struct Steam_Http_Request &request, unsigned file_index )
{
    request.protocol = Steam_Http_Request::Protocol_t::File;

    std::string_view url_path(request.url.c_str() + file_index, request.url.size() - file_index);
    // strip last occurrences pf '/'
    auto last_non_slash = url_path.find_last_not_of('/');
    if (std::string_view::npos != last_non_slash) {
        url_path = url_path.substr(0, last_non_slash + 1);
    }

    // https://datatracker.ietf.org/doc/html/rfc8089
    // 7.2.  Informative References
    // Appendix B.
    enum class FilepathType {
        MinimalAbsolute,       // "file:/path/to/file"                   <<>> "file:/D:/path/to/file"
        NonLocalWithAuthority, // "file://host.example.com/path/to/file" <<>> "file://host.example.com/D:/path/to/file"
        NoAuthorityAbsolute,   // "file:///path/to/file"                 <<>> "file:///D:/path/to/file"
    };

    // detect type
    FilepathType file_type = FilepathType::MinimalAbsolute;
    if (!url_path.empty() && url_path.front() == '/') {
        file_type = FilepathType::NonLocalWithAuthority;
        url_path = url_path.substr(1);
    }

    if (FilepathType::NonLocalWithAuthority == file_type && !url_path.empty() && url_path.front() == '/') {
        file_type = FilepathType::NoAuthorityAbsolute;
        url_path = url_path.substr(1);
    }

    // fix non-local part with authority, by removing the authority part
    if (FilepathType::NonLocalWithAuthority == file_type) {
        auto authority_end = url_path.find('/');
        if (std::string_view::npos != authority_end) {
            url_path = url_path.substr(authority_end + 1);
        }
    }

    PRINT_DEBUG("parsed url path '%.*s'", url_path.size(), url_path.data());

    if (!url_path.empty()) {
        request.target_filepath = common_helpers::to_absolute(
            url_path,
            get_full_program_path()
        );
        PRINT_DEBUG("parsed filepath (absolute) '%s'", request.target_filepath.c_str());

        unsigned int file_size = file_size_(request.target_filepath);
        if (file_size > 0) {
            request.response.resize(file_size);
            auto read = Local_Storage::get_file_data(request.target_filepath, (char *)&request.response[0], file_size, 0);
            if (read < 0) read = 0;
            if (read != file_size) request.response.resize(static_cast<size_t>(read));
            PRINT_DEBUG("read file data size=%i/%u", read, file_size);
        }
    }
}



// Initializes a new HTTP request, returning a handle to use in further operations on it.  Requires
// the method (GET or POST) and the absolute URL for the request.  Both http and https are supported,
// so this string must start with http:// or https:// and should look like http://store.steampowered.com/app/250/ 
// or such.
HTTPRequestHandle Steam_HTTP::CreateHTTPRequest( EHTTPMethod eHTTPRequestMethod, const char *pchAbsoluteURL )
{
    PRINT_DEBUG("%i '%s'", eHTTPRequestMethod, pchAbsoluteURL);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchAbsoluteURL) return INVALID_HTTPREQUEST_HANDLE;

    std::string_view url = pchAbsoluteURL;
    
    unsigned url_index = 0;
    unsigned file_index = 0;
    if (url.rfind("https://", 0) == 0) {
        url_index = sizeof("https://") - 1;
    } else if (url.rfind("http://", 0) == 0) {
        url_index = sizeof("http://") - 1;
    } else if (url.rfind("file:/", 0) == 0) {
        file_index = sizeof("file:/") - 1;
    }

    static HTTPRequestHandle http_handle = 0;
    ++http_handle;
    if (!http_handle) ++http_handle;

    struct Steam_Http_Request request{};
    request.request_method = eHTTPRequestMethod;
    request.url = url;
    request.context_value = 0;
    request.protocol = Steam_Http_Request::Protocol_t::Web; // force web request anyway to keep compatibility
    request.handle = http_handle;

    if (url_index > 0) {
        PRINT_DEBUG("URL is a web link");
        create_http_request_web(request, url_index);
    } else if (file_index > 0) {
        PRINT_DEBUG("URL is a filepath");
        create_http_request_file(request, file_index);

    }

    requests.emplace_back(std::move(request)); // request object reference is invalidated after this move, be careful!
    return http_handle;
}


// Set a context value for the request, which will be returned in the HTTPRequestCompleted_t callback after
// sending the request.  This is just so the caller can easily keep track of which callbacks go with which request data.
bool Steam_HTTP::SetHTTPRequestContextValue( HTTPRequestHandle hRequest, uint64 ulContextValue )
{
    PRINT_DEBUG("%llu", ulContextValue);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    request->context_value = ulContextValue;
    return true;
}


// Set a timeout in seconds for the HTTP request, must be called prior to sending the request.  Default
// timeout is 60 seconds if you don't call this.  Returns false if the handle is invalid, or the request
// has already been sent.
bool Steam_HTTP::SetHTTPRequestNetworkActivityTimeout( HTTPRequestHandle hRequest, uint32 unTimeoutSeconds )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    request->timeout_sec = unTimeoutSeconds;
    return true;
}


// Set a request header value for the request, must be called prior to sending the request.  Will 
// return false if the handle is invalid or the request is already sent.
bool Steam_HTTP::SetHTTPRequestHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, const char *pchHeaderValue )
{
    PRINT_DEBUG("'%s'='%s'", pchHeaderName, pchHeaderValue);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchHeaderName || !pchHeaderValue) return false;
    if (common_helpers::str_cmp_insensitive(pchHeaderName, "User-Agent")) return false;

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    // FIX: appid 1902490 adds the header "Cache-Control: only-if-cached, max-stale=2678400"
    // which means a response is returned back only if it was already cached, otherwise the server has to send a 504 "Gateway Timeout"
    // just bypass the known ones to be on the safe side
    if (common_helpers::str_cmp_insensitive(pchHeaderName, "Cache-Control")) return true;
    if (common_helpers::str_cmp_insensitive(pchHeaderName, "Accept")) return true;

    request->headers[pchHeaderName] = pchHeaderValue;
    return true;
}


// Set a GET or POST parameter value on the request, which is set will depend on the EHTTPMethod specified
// when creating the request.  Must be called prior to sending the request.  Will return false if the 
// handle is invalid or the request is already sent.
bool Steam_HTTP::SetHTTPRequestGetOrPostParameter( HTTPRequestHandle hRequest, const char *pchParamName, const char *pchParamValue )
{
    PRINT_DEBUG("'%s' = '%s'", pchParamName, pchParamValue);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchParamName || !pchParamValue) return false;
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }
    if (request->request_method != EHTTPMethod::k_EHTTPMethodGET &&
        request->request_method != EHTTPMethod::k_EHTTPMethodHEAD &&
        request->request_method != EHTTPMethod::k_EHTTPMethodPOST) {
        return false;
    }
    if (request->post_raw.size()) return false;

    request->get_or_post_params[pchParamName] = pchParamValue;
    return true;
}

static int curl_debug_trace(
    CURL *handle, curl_infotype type,
    char *data, size_t size,
    void *clientp
)
{
    // https://curl.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html
    std::string text{};
    switch (type) {
        case CURLINFO_TEXT: text = "Info: " + std::string(data, size); break;
        case CURLINFO_HEADER_IN: text = "<= Recv header"; break;
        case CURLINFO_HEADER_OUT: text = "=> Send header"; break;
        case CURLINFO_DATA_IN: text = "<= Recv data"; break;
        case CURLINFO_DATA_OUT: text = "=> Send data"; break;
        case CURLINFO_SSL_DATA_OUT: text = "=> Send SSL data"; break;
        case CURLINFO_SSL_DATA_IN: text = "<= Recv SSL data"; break;

        default: text = "[X] ERROR: unknown callback type"; break;
    }

    PRINT_DEBUG("%s", text.c_str());
    return 0;
}


void Steam_HTTP::online_http_request(Steam_Http_Request *request, SteamAPICall_t call_res_id)
{
    PRINT_DEBUG("attempting to download from url: '%s', target filepath: '%s'",
        request->url.c_str(), request->target_filepath.c_str());

    const auto send_callresult = [=]() -> void {
        struct HTTPRequestCompleted_t data{};
        data.m_hRequest = request->handle;
        data.m_ulContextValue = request->context_value;
        data.m_unBodySize = static_cast<uint32>(request->response.size());
        if (request->response.empty() && !settings->force_steamhttp_success) {
            data.m_bRequestSuccessful = false;
            data.m_eStatusCode = k_EHTTPStatusCode404NotFound;
        } else {
            data.m_bRequestSuccessful = true;
            data.m_eStatusCode = k_EHTTPStatusCode200OK;
        }

        callback_results->addCallResult(call_res_id, data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    };

    std::size_t filename_part = request->target_filepath.find_last_of("\\/");
    std::string directory_path{};
    std::string file_name{};
    if (filename_part != std::string::npos) {
        filename_part += 1; // point at filename, not the '/' or '\'
        directory_path = request->target_filepath.substr(0, filename_part);
        file_name = request->target_filepath.substr(filename_part);
    } else {
        directory_path = ".";
        file_name = request->target_filepath;
    }
    PRINT_DEBUG("directory: '%s', filename '%s'", directory_path.c_str(), file_name.c_str());
    Local_Storage::store_file_data(directory_path, file_name, (char *)"", sizeof("")); // create empty file

    FILE *hfile = nullptr;

    {
        const auto fsp = std::filesystem::u8path(request->target_filepath);
#if defined(__WINDOWS__)
        // TODO use "\\?\" to solve max path problem
        // https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
        // note that this bypasses the NT object manager, note from above link:
        // "File I/O functions in the Windows API convert "/" to "\" as part of converting the name to an NT-style name, except when using the "\\?\" prefix"
        hfile = _wfopen(fsp.c_str(), L"wb");
#else
        hfile = std::fopen(fsp.c_str(), "wb");
#endif
    }

    if (!hfile) {
        PRINT_DEBUG("failed to open file for writing");
        send_callresult();
        return;
    }
    CURL *chttp = curl_easy_init();
    if (!chttp) {
        fclose(hfile);
        PRINT_DEBUG("curl_easy_init() failed");
        send_callresult();
        return;
    }

#ifndef EMU_RELEASE_BUILD
    curl_easy_setopt(chttp, CURLOPT_DEBUGFUNCTION, curl_debug_trace);
    curl_easy_setopt(chttp, CURLOPT_VERBOSE, 1L);
#endif
    
    // headers
    std::vector<std::string> headers{};
    for (const auto &hdr : request->headers) {
        std::string new_header = hdr.first + ": " + hdr.second;
        PRINT_DEBUG("CURL header: '%s'", new_header.c_str());
        headers.push_back(new_header);
    }

    struct curl_slist *headers_list = nullptr;
    for (const auto &hrd : headers) {
        headers_list = curl_slist_append(headers_list, hrd.c_str());
    }
    curl_easy_setopt(chttp, CURLOPT_HTTPHEADER, headers_list);
    
    // request method
    switch (request->request_method)
    {
    case EHTTPMethod::k_EHTTPMethodGET:
        PRINT_DEBUG("CURL method type: GET");
        curl_easy_setopt(chttp, CURLOPT_HTTPGET, 1L);
    break;
    
    case EHTTPMethod::k_EHTTPMethodHEAD:
        PRINT_DEBUG("CURL method type: HEAD");
        curl_easy_setopt(chttp, CURLOPT_NOBODY, 1L);
    break;
    
    case EHTTPMethod::k_EHTTPMethodPOST:
        PRINT_DEBUG("CURL method type: POST");
        curl_easy_setopt(chttp, CURLOPT_POST, 1L);
    break;
    
    case EHTTPMethod::k_EHTTPMethodPUT:
        PRINT_DEBUG("TODO CURL method type: PUT");
        curl_easy_setopt(chttp, CURLOPT_UPLOAD, 1L); // CURLOPT_PUT "This option is deprecated since version 7.12.1. Use CURLOPT_UPLOAD."
    break;
    
    case EHTTPMethod::k_EHTTPMethodDELETE:
        PRINT_DEBUG("TODO CURL method type: DELETE");
        headers_list = curl_slist_append(headers_list, "Content-Type: application/x-www-form-urlencoded");
        headers_list = curl_slist_append(headers_list, "Accept: application/json,application/x-www-form-urlencoded,text/html,application/xhtml+xml,application/xml");
        curl_easy_setopt(chttp, CURLOPT_CUSTOMREQUEST, "DELETE"); // https://stackoverflow.com/a/34751940
    break;
    
    case EHTTPMethod::k_EHTTPMethodOPTIONS:
        PRINT_DEBUG("TODO CURL method type: OPTIONS");
        curl_easy_setopt(chttp, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    break;
    
    case EHTTPMethod::k_EHTTPMethodPATCH:
        PRINT_DEBUG("TODO CURL method type: PATCH");
        headers_list = curl_slist_append(headers_list, "Content-Type: application/x-www-form-urlencoded");
        headers_list = curl_slist_append(headers_list, "Accept: application/json,application/x-www-form-urlencoded,text/html,application/xhtml+xml,application/xml");
        curl_easy_setopt(chttp, CURLOPT_CUSTOMREQUEST, "PATCH");
    break;
    
    default:
        break;
    }
    
    curl_easy_setopt(chttp, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(chttp, CURLOPT_WRITEDATA, (void *)hfile);
    curl_easy_setopt(chttp, CURLOPT_TIMEOUT, request->timeout_sec);
    curl_easy_setopt(chttp, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(chttp, CURLOPT_USE_SSL, request->requires_valid_ssl ? CURLUSESSL_TRY : CURLUSESSL_NONE);
    curl_easy_setopt(chttp, CURLOPT_SSL_VERIFYPEER, 0L);

    // post data, or get params
    std::string post_data{};
    if (request->get_or_post_params.size()) {
        for (const auto &pdata : request->get_or_post_params) {
            char *form_encoded_key = curl_easy_escape(chttp, pdata.first.c_str(), (int)pdata.first.size());
            char *form_encoded_val = curl_easy_escape(chttp, pdata.second.c_str(), (int)pdata.second.size());
            if (form_encoded_key && form_encoded_val) {
                post_data += form_encoded_key + std::string("=") + form_encoded_val + "&";
            }
            if (form_encoded_key) curl_free(form_encoded_key);
            if (form_encoded_val) curl_free(form_encoded_val);
        }
        if (post_data.size()) post_data = post_data.substr(0, post_data.size() - 1); // remove the last "&"
        if (request->request_method == EHTTPMethod::k_EHTTPMethodGET) {
            request->url += "?" + post_data;
            PRINT_DEBUG("GET URL with params (url-encoded): '%s'", request->url.c_str());
        } else {
            PRINT_DEBUG("POST form data (url-encoded): '%s'", post_data.c_str());
            curl_easy_setopt(chttp, CURLOPT_POSTFIELDS, post_data.c_str());
        }
    } else if (request->post_raw.size()) {
        PRINT_DEBUG("POST form data (raw): '%s'", request->post_raw.c_str());
        curl_easy_setopt(chttp, CURLOPT_POSTFIELDS, request->post_raw.c_str());
    }

    curl_easy_setopt(chttp, CURLOPT_URL, request->url.c_str());
    
    CURLcode res_curl = curl_easy_perform(chttp);
    curl_slist_free_all(headers_list);
    curl_easy_cleanup(chttp);

    fclose(hfile);
    headers.clear();

    PRINT_DEBUG("CURL error code for '%s' [%i = '%s'] (OK == 0)", request->url.c_str(), (int)res_curl, curl_easy_strerror(res_curl));
    
    unsigned int file_size = file_size_(request->target_filepath);
    if (file_size) {
        request->response.resize(static_cast<size_t>(file_size));
        long long read = Local_Storage::get_file_data(request->target_filepath, (char *)&request->response[0], file_size);
        if (read < 0) read = 0;
        request->response.resize(static_cast<size_t>(read));
    }
    
    send_callresult();
}

// Sends the HTTP request, will return false on a bad handle, otherwise use SteamCallHandle to wait on
// asynchronous response via callback.
//
// Note: If the user is in offline mode in Steam, then this will add a only-if-cached cache-control 
// header and only do a local cache lookup rather than sending any actual remote request.
bool Steam_HTTP::SendHTTPRequest( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    PRINT_DEBUG("%u %p", hRequest, pCallHandle);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    switch (request->protocol)
    {
    case Steam_Http_Request::Protocol_t::Web: {
        bool bad_local_file = request->response.empty() && !request->target_filepath.empty(); // we need the filepath since we want to save the response to that file
        bool internet_allowed = !settings->disable_networking && settings->download_steamhttp_requests;
        if (bad_local_file && internet_allowed) {
            auto call_res_id = callback_results->reserveCallResult();
            if (pCallHandle) *pCallHandle = call_res_id;

            std::thread(&Steam_HTTP::online_http_request, this, request, call_res_id).detach();
        } else {
            struct HTTPRequestCompleted_t data{};
            data.m_hRequest = request->handle;
            data.m_ulContextValue = request->context_value;
            data.m_unBodySize = static_cast<uint32>(request->response.size());
            if (request->response.empty() && !settings->force_steamhttp_success) {
                data.m_bRequestSuccessful = false;
                data.m_eStatusCode = k_EHTTPStatusCode404NotFound;
            } else {
                data.m_bRequestSuccessful = true;
                data.m_eStatusCode = k_EHTTPStatusCode200OK;
            }

            auto callres = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1);
            if (pCallHandle) *pCallHandle = callres;
            
            callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
        }
    }
    break;
    
    case Steam_Http_Request::Protocol_t::File: {
        struct HTTPRequestCompleted_t data{};
        data.m_hRequest = request->handle;
        data.m_ulContextValue = request->context_value;
        data.m_unBodySize = static_cast<uint32>(request->response.size());
        if (request->target_filepath.empty()) { // malformed filepath
            data.m_bRequestSuccessful = false;
            data.m_eStatusCode = k_EHTTPStatusCode403Forbidden;
        } else if (request->response.empty() && !settings->force_steamhttp_success) {
            data.m_bRequestSuccessful = false;
            data.m_eStatusCode = k_EHTTPStatusCode404NotFound;
        } else {
            data.m_bRequestSuccessful = true;
            data.m_eStatusCode = k_EHTTPStatusCode200OK;
        }

        auto callres = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.03);
        if (pCallHandle) *pCallHandle = callres;
        
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.03);
    }
    break;
    
    default: {
        PRINT_DEBUG("[X] unhandled protocol type <%i>", (int)request->protocol);
        struct HTTPRequestCompleted_t data{};
        data.m_hRequest = request->handle;
        data.m_ulContextValue = request->context_value;
        data.m_unBodySize = 0;
        data.m_bRequestSuccessful = false;
        data.m_eStatusCode = k_EHTTPStatusCode501NotImplemented;

        auto callres = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.05);
        if (pCallHandle) *pCallHandle = callres;
        
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.05);
    }
    break;
    }

    return true;
}


// Sends the HTTP request, will return false on a bad handle, otherwise use SteamCallHandle to wait on
// asynchronous response via callback for completion, and listen for HTTPRequestHeadersReceived_t and 
// HTTPRequestDataReceived_t callbacks while streaming.
bool Steam_HTTP::SendHTTPRequestAndStreamResponse( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    // TODO this function triggers 3 callbacks:
    // https://partner.steamgames.com/doc/api/ISteamHTTP#SendHTTPRequestAndStreamResponse
    // Triggers a HTTPRequestDataReceived_t callback.
    // Triggers a HTTPRequestHeadersReceived_t callback.
    // Triggers a HTTPRequestCompleted_t callback. 
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return SendHTTPRequest(hRequest, pCallHandle);
}


// Defers a request you have sent, the actual HTTP client code may have many requests queued, and this will move
// the specified request to the tail of the queue.  Returns false on invalid handle, or if the request is not yet sent.
bool Steam_HTTP::DeferHTTPRequest( HTTPRequestHandle hRequest )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Prioritizes a request you have sent, the actual HTTP client code may have many requests queued, and this will move
// the specified request to the head of the queue.  Returns false on invalid handle, or if the request is not yet sent.
bool Steam_HTTP::PrioritizeHTTPRequest( HTTPRequestHandle hRequest )
{
    PRINT_DEBUG("%u", hRequest);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Checks if a response header is present in a HTTP response given a handle from HTTPRequestCompleted_t, also 
// returns the size of the header value if present so the caller and allocate a correctly sized buffer for
// GetHTTPResponseHeaderValue.
bool Steam_HTTP::GetHTTPResponseHeaderSize( HTTPRequestHandle hRequest, const char *pchHeaderName, uint32 *unResponseHeaderSize )
{
    PRINT_DEBUG("'%s'", pchHeaderName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (unResponseHeaderSize) *unResponseHeaderSize = 0;

    if (!pchHeaderName) return false;

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }
    const auto hdr = request->headers.find(pchHeaderName);
    if (request->headers.end() == hdr) return false;

    if (unResponseHeaderSize) *unResponseHeaderSize = (uint32)hdr->second.size();
    return true;
}


// Gets header values from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the
// header is not present or if your buffer is too small to contain it's value.  You should first call 
// BGetHTTPResponseHeaderSize to check for the presence of the header and to find out the size buffer needed.
bool Steam_HTTP::GetHTTPResponseHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, uint8 *pHeaderValueBuffer, uint32 unBufferSize )
{
    PRINT_DEBUG("'%s' [%u]", pchHeaderName, unBufferSize);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchHeaderName) return false;

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }
    const auto hdr = request->headers.find(pchHeaderName);
    if (request->headers.end() == hdr) return false;
    PRINT_DEBUG("  required header buffer size = %zu", hdr->second.size());

    if (unBufferSize < hdr->second.size()) return false;
    if (pHeaderValueBuffer) {
        memset(pHeaderValueBuffer, 0, unBufferSize);
        hdr->second.copy((char *)pHeaderValueBuffer, unBufferSize);
    }
    return true;
}


// Gets the size of the body data from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the 
// handle is invalid.
bool Steam_HTTP::GetHTTPResponseBodySize( HTTPRequestHandle hRequest, uint32 *unBodySize )
{
    PRINT_DEBUG("%u", hRequest);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (unBodySize) *unBodySize = 0;

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (unBodySize) *unBodySize = (uint32)request->response.size();
    return true;
}


// Gets the body data from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the 
// handle is invalid or is to a streaming response, or if the provided buffer is not the correct size.  Use BGetHTTPResponseBodySize first to find out
// the correct buffer size to use.
bool Steam_HTTP::GetHTTPResponseBodyData( HTTPRequestHandle hRequest, uint8 *pBodyDataBuffer, uint32 unBufferSize )
{
    PRINT_DEBUG("%p [%u]", pBodyDataBuffer, unBufferSize);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }
    PRINT_DEBUG("  required buffer size = %zu", request->response.size());
    if (unBufferSize < request->response.size()) return false;
    if (pBodyDataBuffer) {
        memset(pBodyDataBuffer, 0, unBufferSize);
        request->response.copy((char *)pBodyDataBuffer, unBufferSize);
    }
    return true;
}


// Gets the body data from a streaming HTTP response given a handle from HTTPRequestDataReceived_t. Will return false if the 
// handle is invalid or is to a non-streaming response (meaning it wasn't sent with SendHTTPRequestAndStreamResponse), or if the buffer size and offset 
// do not match the size and offset sent in HTTPRequestDataReceived_t.
bool Steam_HTTP::GetHTTPStreamingResponseBodyData( HTTPRequestHandle hRequest, uint32 cOffset, uint8 *pBodyDataBuffer, uint32 unBufferSize )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (pBodyDataBuffer && cOffset <= request->response.size()) {
        memset(pBodyDataBuffer, 0, unBufferSize);
        request->response.copy((char *)pBodyDataBuffer, unBufferSize, cOffset);
        return true;
    }
    return false;
}


// Releases an HTTP response handle, should always be called to free resources after receiving a HTTPRequestCompleted_t
// callback and finishing using the response.
bool Steam_HTTP::ReleaseHTTPRequest( HTTPRequestHandle hRequest )
{
    PRINT_DEBUG("%u", hRequest);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto c = std::begin(requests);
    while (c != std::end(requests)) {
        if (c->handle == hRequest) {
            c = requests.erase(c);
            return true;
        } else {
            ++c;
        }
    }

    return false;
}


// Gets progress on downloading the body for the request.  This will be zero unless a response header has already been
// received which included a content-length field.  For responses that contain no content-length it will report
// zero for the duration of the request as the size is unknown until the connection closes.
bool Steam_HTTP::GetHTTPDownloadProgressPct( HTTPRequestHandle hRequest, float *pflPercentOut )
{
    PRINT_DEBUG("%u", hRequest);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (pflPercentOut) *pflPercentOut = 0;

    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }
    if (pflPercentOut) *pflPercentOut = 100.0f;
    return true;
}


// Sets the body for an HTTP Post request.  Will fail and return false on a GET request, and will fail if POST params
// have already been set for the request.  Setting this raw body makes it the only contents for the post, the pchContentType
// parameter will set the content-type header for the request so the server may know how to interpret the body.
bool Steam_HTTP::SetHTTPRequestRawPostBody( HTTPRequestHandle hRequest, const char *pchContentType, uint8 *pubBody, uint32 unBodyLen )
{
    PRINT_DEBUG("%u '%s'", hRequest, pchContentType);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (request->request_method != EHTTPMethod::k_EHTTPMethodPOST &&
        request->request_method != EHTTPMethod::k_EHTTPMethodPUT &&
        request->request_method != EHTTPMethod::k_EHTTPMethodPATCH) {
        return false;
    }
    if (request->get_or_post_params.size()) return false;

    request->post_raw = std::string((char *)pubBody, unBodyLen);
    return true;
}


// Creates a cookie container handle which you must later free with ReleaseCookieContainer().  If bAllowResponsesToModify=true
// than any response to your requests using this cookie container may add new cookies which may be transmitted with
// future requests.  If bAllowResponsesToModify=false than only cookies you explicitly set will be sent.  This API is just for
// during process lifetime, after steam restarts no cookies are persisted and you have no way to access the cookie container across
// repeat executions of your process.
HTTPCookieContainerHandle Steam_HTTP::CreateCookieContainer( bool bAllowResponsesToModify )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    static HTTPCookieContainerHandle handle = 0;
    ++handle;
    if (!handle) ++handle;
    
    return INVALID_HTTPCOOKIE_HANDLE;
}


// Release a cookie container you are finished using, freeing it's memory
bool Steam_HTTP::ReleaseCookieContainer( HTTPCookieContainerHandle hCookieContainer )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return false;
}


// Adds a cookie to the specified cookie container that will be used with future requests.
bool Steam_HTTP::SetCookie( HTTPCookieContainerHandle hCookieContainer, const char *pchHost, const char *pchUrl, const char *pchCookie )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}


// Set the cookie container to use for a HTTP request
bool Steam_HTTP::SetHTTPRequestCookieContainer( HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}


// Set the extra user agent info for a request, this doesn't clobber the normal user agent, it just adds the extra info on the end
bool Steam_HTTP::SetHTTPRequestUserAgentInfo( HTTPRequestHandle hRequest, const char *pchUserAgentInfo )
{
    PRINT_DEBUG("%u '%s'", hRequest, pchUserAgentInfo);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (!pchUserAgentInfo || !pchUserAgentInfo[0]) {
        request->headers["User-Agent"] = Steam_Http_Request::STEAM_DEFAULT_USER_AGENT;
    } else {
        request->headers["User-Agent"] += std::string(" ") + pchUserAgentInfo;
    }
    return true;
}


// Set that https request should require verified SSL certificate via machines certificate trust store
bool Steam_HTTP::SetHTTPRequestRequiresVerifiedCertificate( HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    request->requires_valid_ssl = bRequireVerifiedCertificate;
    return true;
}


// Set an absolute timeout on the HTTP request, this is just a total time timeout different than the network activity timeout
// which can bump everytime we get more data
bool Steam_HTTP::SetHTTPRequestAbsoluteTimeoutMS( HTTPRequestHandle hRequest, uint32 unMilliseconds )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    request->timeout_sec = (uint64)(unMilliseconds / 1000.0);
    return true;
}


// Check if the reason the request failed was because we timed it out (rather than some harder failure)
bool Steam_HTTP::GetHTTPRequestWasTimedOut( HTTPRequestHandle hRequest, bool *pbWasTimedOut )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (pbWasTimedOut) *pbWasTimedOut = false;
    return true;
}
