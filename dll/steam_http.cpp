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

#include "steam_http.h"

Steam_HTTP::Steam_HTTP(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

Steam_Http_Request *Steam_HTTP::get_request(HTTPRequestHandle hRequest)
{
    auto conn = std::find_if(requests.begin(), requests.end(), [&hRequest](struct Steam_Http_Request const& conn) { return conn.handle == hRequest;});
    if (conn == requests.end()) return NULL;
    return &(*conn);
}

// Initializes a new HTTP request, returning a handle to use in further operations on it.  Requires
// the method (GET or POST) and the absolute URL for the request.  Both http and https are supported,
// so this string must start with http:// or https:// and should look like http://store.steampowered.com/app/250/ 
// or such.
HTTPRequestHandle Steam_HTTP::CreateHTTPRequest( EHTTPMethod eHTTPRequestMethod, const char *pchAbsoluteURL )
{
    PRINT_DEBUG("CreateHTTPRequest %i %s\n", eHTTPRequestMethod, pchAbsoluteURL);
    if (!pchAbsoluteURL) return INVALID_HTTPREQUEST_HANDLE;
    std::string url = pchAbsoluteURL;
    unsigned url_index = 0;
    if (url.rfind("https://", 0) == 0) {
        url_index = sizeof("https://") - 1;
    } else if (url.rfind("http://", 0) == 0) {
        url_index = sizeof("http://") - 1;
    }

    struct Steam_Http_Request request;
    if (url_index) {
        if (url[url.size() - 1] == '/') url += "index.html";
        std::string file_path = Local_Storage::get_game_settings_path() + "http/" + Local_Storage::sanitize_string(url.substr(url_index));
        unsigned long long file_size = file_size_(file_path);
        if (file_size) {
            request.response.resize(file_size);
            long long read = Local_Storage::get_file_data(file_path, (char *)request.response.data(), file_size, 0);
            if (read < 0) read = 0;
            if (read != file_size) request.response.resize(read);
        } else if (!settings->disable_networking && settings->http_online) {
            std::lock_guard<std::recursive_mutex> lock(global_mutex);

            std::size_t url_directory = file_path.rfind("/");
            std::string directory_path;
            std::string file_name;
            if (url_directory != std::string::npos) {
                directory_path = file_path.substr(0, url_directory);
                file_name = file_path.substr(url_directory);
            }
            Local_Storage::store_file_data(directory_path, file_name, (char *)"", sizeof(""));

#if defined(STEAM_WIN32)
            FILE *hfile = fopen(file_path.c_str(), "wb");
#else
            FILE *hfile = fopen(file_path.c_str(), "w");
#endif
            CURL *chttp = curl_easy_init();
            curl_easy_setopt(chttp, CURLOPT_URL, url.c_str());
            curl_easy_setopt(chttp, CURLOPT_WRITEDATA, (void *)hfile);
            curl_easy_setopt(chttp, CURLOPT_TIMEOUT, 60L);
            curl_easy_setopt(chttp, CURLOPT_NOSIGNAL, 1L);
            curl_easy_setopt(chttp, CURLOPT_USE_SSL, CURLUSESSL_TRY);
            curl_easy_perform(chttp);
            curl_easy_cleanup(chttp);
            fclose(hfile);

            file_size = file_size_(file_path);
            if (file_size) {
                request.response.resize(file_size);
                long long read = Local_Storage::get_file_data(file_path, (char *)request.response.data(), file_size, 0);
                if (read < 0) read = 0;
                if (read != file_size) request.response.resize(read);
            }
        }
    }

    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    static HTTPRequestHandle h;
    ++h;

    request.handle = h;
    request.context_value = 0;

    requests.push_back(request);
    return request.handle;
}


// Set a context value for the request, which will be returned in the HTTPRequestCompleted_t callback after
// sending the request.  This is just so the caller can easily keep track of which callbacks go with which request data.
bool Steam_HTTP::SetHTTPRequestContextValue( HTTPRequestHandle hRequest, uint64 ulContextValue )
{
    PRINT_DEBUG("SetHTTPRequestContextValue\n");
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
    PRINT_DEBUG("SetHTTPRequestNetworkActivityTimeout\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Set a request header value for the request, must be called prior to sending the request.  Will 
// return false if the handle is invalid or the request is already sent.
bool Steam_HTTP::SetHTTPRequestHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, const char *pchHeaderValue )
{
    PRINT_DEBUG("SetHTTPRequestHeaderValue %s %s\n", pchHeaderName, pchHeaderValue);
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Set a GET or POST parameter value on the request, which is set will depend on the EHTTPMethod specified
// when creating the request.  Must be called prior to sending the request.  Will return false if the 
// handle is invalid or the request is already sent.
bool Steam_HTTP::SetHTTPRequestGetOrPostParameter( HTTPRequestHandle hRequest, const char *pchParamName, const char *pchParamValue )
{
    PRINT_DEBUG("SetHTTPRequestGetOrPostParameter\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Sends the HTTP request, will return false on a bad handle, otherwise use SteamCallHandle to wait on
// asynchronous response via callback.
//
// Note: If the user is in offline mode in Steam, then this will add a only-if-cached cache-control 
// header and only do a local cache lookup rather than sending any actual remote request.
bool Steam_HTTP::SendHTTPRequest( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    PRINT_DEBUG("SendHTTPRequest %u %p\n", hRequest, pCallHandle);
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    struct HTTPRequestCompleted_t data = {};
    data.m_hRequest = request->handle;
    data.m_ulContextValue = request->context_value;
    if (request->response.size() == 0) {
        data.m_bRequestSuccessful = false;
        data.m_eStatusCode = k_EHTTPStatusCode404NotFound;
        data.m_unBodySize = request->response.size();
    } else {
        data.m_bRequestSuccessful = true;
        data.m_eStatusCode = k_EHTTPStatusCode200OK;
        data.m_unBodySize = request->response.size();
    }

    if (pCallHandle) {
        *pCallHandle = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1);
    }

    return true;
}


// Sends the HTTP request, will return false on a bad handle, otherwise use SteamCallHandle to wait on
// asynchronous response via callback for completion, and listen for HTTPRequestHeadersReceived_t and 
// HTTPRequestDataReceived_t callbacks while streaming.
bool Steam_HTTP::SendHTTPRequestAndStreamResponse( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle )
{
    PRINT_DEBUG("SendHTTPRequestAndStreamResponse\n");
    return false;
}


// Defers a request you have sent, the actual HTTP client code may have many requests queued, and this will move
// the specified request to the tail of the queue.  Returns false on invalid handle, or if the request is not yet sent.
bool Steam_HTTP::DeferHTTPRequest( HTTPRequestHandle hRequest )
{
    PRINT_DEBUG("DeferHTTPRequest\n");
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
    PRINT_DEBUG("PrioritizeHTTPRequest\n");
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
    PRINT_DEBUG("GetHTTPResponseHeaderSize\n");
    return false;
}


// Gets header values from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the
// header is not present or if your buffer is too small to contain it's value.  You should first call 
// BGetHTTPResponseHeaderSize to check for the presence of the header and to find out the size buffer needed.
bool Steam_HTTP::GetHTTPResponseHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, uint8 *pHeaderValueBuffer, uint32 unBufferSize )
{
    PRINT_DEBUG("GetHTTPResponseHeaderValue\n");
    return false;
}


// Gets the size of the body data from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the 
// handle is invalid.
bool Steam_HTTP::GetHTTPResponseBodySize( HTTPRequestHandle hRequest, uint32 *unBodySize )
{
    PRINT_DEBUG("GetHTTPResponseBodySize\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (unBodySize) *unBodySize = request->response.size();
    return true;
}


// Gets the body data from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the 
// handle is invalid or is to a streaming response, or if the provided buffer is not the correct size.  Use BGetHTTPResponseBodySize first to find out
// the correct buffer size to use.
bool Steam_HTTP::GetHTTPResponseBodyData( HTTPRequestHandle hRequest, uint8 *pBodyDataBuffer, uint32 unBufferSize )
{
    PRINT_DEBUG("GetHTTPResponseBodyData\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (unBufferSize < request->response.size()) {
        return false;
    }

    if (pBodyDataBuffer) memcpy(pBodyDataBuffer, request->response.data(), request->response.size());
    return true;
}


// Gets the body data from a streaming HTTP response given a handle from HTTPRequestDataReceived_t. Will return false if the 
// handle is invalid or is to a non-streaming response (meaning it wasn't sent with SendHTTPRequestAndStreamResponse), or if the buffer size and offset 
// do not match the size and offset sent in HTTPRequestDataReceived_t.
bool Steam_HTTP::GetHTTPStreamingResponseBodyData( HTTPRequestHandle hRequest, uint32 cOffset, uint8 *pBodyDataBuffer, uint32 unBufferSize )
{
    PRINT_DEBUG("GetHTTPStreamingResponseBodyData\n");
    return false;
}


// Releases an HTTP response handle, should always be called to free resources after receiving a HTTPRequestCompleted_t
// callback and finishing using the response.
bool Steam_HTTP::ReleaseHTTPRequest( HTTPRequestHandle hRequest )
{
    PRINT_DEBUG("ReleaseHTTPRequest\n");
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
    PRINT_DEBUG("GetHTTPDownloadProgressPct\n");
    return false;
}


// Sets the body for an HTTP Post request.  Will fail and return false on a GET request, and will fail if POST params
// have already been set for the request.  Setting this raw body makes it the only contents for the post, the pchContentType
// parameter will set the content-type header for the request so the server may know how to interpret the body.
bool Steam_HTTP::SetHTTPRequestRawPostBody( HTTPRequestHandle hRequest, const char *pchContentType, uint8 *pubBody, uint32 unBodyLen )
{
    PRINT_DEBUG("SetHTTPRequestRawPostBody %s\n", pchContentType);
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Creates a cookie container handle which you must later free with ReleaseCookieContainer().  If bAllowResponsesToModify=true
// than any response to your requests using this cookie container may add new cookies which may be transmitted with
// future requests.  If bAllowResponsesToModify=false than only cookies you explicitly set will be sent.  This API is just for
// during process lifetime, after steam restarts no cookies are persisted and you have no way to access the cookie container across
// repeat executions of your process.
HTTPCookieContainerHandle Steam_HTTP::CreateCookieContainer( bool bAllowResponsesToModify )
{
    PRINT_DEBUG("CreateCookieContainer\n");
    return false;
}


// Release a cookie container you are finished using, freeing it's memory
bool Steam_HTTP::ReleaseCookieContainer( HTTPCookieContainerHandle hCookieContainer )
{
    PRINT_DEBUG("ReleaseCookieContainer\n");
    return false;
}


// Adds a cookie to the specified cookie container that will be used with future requests.
bool Steam_HTTP::SetCookie( HTTPCookieContainerHandle hCookieContainer, const char *pchHost, const char *pchUrl, const char *pchCookie )
{
    PRINT_DEBUG("SetCookie\n");
    return false;
}


// Set the cookie container to use for a HTTP request
bool Steam_HTTP::SetHTTPRequestCookieContainer( HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer )
{
    PRINT_DEBUG("SetHTTPRequestCookieContainer\n");
    return false;
}


// Set the extra user agent info for a request, this doesn't clobber the normal user agent, it just adds the extra info on the end
bool Steam_HTTP::SetHTTPRequestUserAgentInfo( HTTPRequestHandle hRequest, const char *pchUserAgentInfo )
{
    PRINT_DEBUG("SetHTTPRequestUserAgentInfo\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Set that https request should require verified SSL certificate via machines certificate trust store
bool Steam_HTTP::SetHTTPRequestRequiresVerifiedCertificate( HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate )
{
    PRINT_DEBUG("SetHTTPRequestRequiresVerifiedCertificate\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Set an absolute timeout on the HTTP request, this is just a total time timeout different than the network activity timeout
// which can bump everytime we get more data
bool Steam_HTTP::SetHTTPRequestAbsoluteTimeoutMS( HTTPRequestHandle hRequest, uint32 unMilliseconds )
{
    PRINT_DEBUG("SetHTTPRequestAbsoluteTimeoutMS\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    return true;
}


// Check if the reason the request failed was because we timed it out (rather than some harder failure)
bool Steam_HTTP::GetHTTPRequestWasTimedOut( HTTPRequestHandle hRequest, bool *pbWasTimedOut )
{
    PRINT_DEBUG("GetHTTPRequestWasTimedOut\n");
    Steam_Http_Request *request = get_request(hRequest);
    if (!request) {
        return false;
    }

    if (pbWasTimedOut) *pbWasTimedOut = false;
    return true;
}
