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

#ifndef __INCLUDED_STEAM_HTTP_H__
#define __INCLUDED_STEAM_HTTP_H__

#include "base.h"
#include <curl/curl.h>


struct Steam_Http_Request {
    enum class Protocol_t {
        Web,
        File,
    };

	HTTPRequestHandle handle{};
	EHTTPMethod request_method{};
	std::string url{};
	uint64 timeout_sec = 60;
	bool requires_valid_ssl = false;
	Protocol_t protocol = Protocol_t::Web;

	constexpr const static char STEAM_DEFAULT_USER_AGENT[] = "Valve/Steam HTTP Client 1.0";
	// check Steam_HTTP::SetHTTPRequestHeaderValue() and make sure to bypass the ones that should be reserved
	std::map<std::string, std::string> headers{
		{ "User-Agent", STEAM_DEFAULT_USER_AGENT },
		{ "Cache-Control", "max-age=0" },
		{ "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7" },
		{ "Upgrade-Insecure-Requests", "1" },
	};

	// GET or POST parameter value of the request
	std::map<std::string, std::string> get_or_post_params{};
	std::string post_raw{};

	uint64 context_value{};

	// target local filepath to save
	std::string target_filepath{};

	// TODO
	HTTPCookieContainerHandle cookie_container_handle = INVALID_HTTPCOOKIE_HANDLE;

	std::string response{};
};


class Steam_HTTP :
public ISteamHTTP001,
public ISteamHTTP002,
public ISteamHTTP
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};

	std::vector<Steam_Http_Request> requests{};

	Steam_Http_Request *get_request(HTTPRequestHandle hRequest);
	void online_http_request(Steam_Http_Request *request, SteamAPICall_t call_res_id);

	void create_http_request_web( struct Steam_Http_Request &request, unsigned url_index );
	void create_http_request_file( struct Steam_Http_Request &request, unsigned file_index  );


public:
	Steam_HTTP(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks);

	// Initializes a new HTTP request, returning a handle to use in further operations on it.  Requires
	// the method (GET or POST) and the absolute URL for the request.  Both http and https are supported,
	// so this string must start with http:// or https:// and should look like http://store.steampowered.com/app/250/ 
	// or such.
	HTTPRequestHandle CreateHTTPRequest( EHTTPMethod eHTTPRequestMethod, const char *pchAbsoluteURL );

	// Set a context value for the request, which will be returned in the HTTPRequestCompleted_t callback after
	// sending the request.  This is just so the caller can easily keep track of which callbacks go with which request data.
	bool SetHTTPRequestContextValue( HTTPRequestHandle hRequest, uint64 ulContextValue );

	// Set a timeout in seconds for the HTTP request, must be called prior to sending the request.  Default
	// timeout is 60 seconds if you don't call this.  Returns false if the handle is invalid, or the request
	// has already been sent.
	bool SetHTTPRequestNetworkActivityTimeout( HTTPRequestHandle hRequest, uint32 unTimeoutSeconds );

	// Set a request header value for the request, must be called prior to sending the request.  Will 
	// return false if the handle is invalid or the request is already sent.
	bool SetHTTPRequestHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, const char *pchHeaderValue );

	// Set a GET or POST parameter value on the request, which is set will depend on the EHTTPMethod specified
	// when creating the request.  Must be called prior to sending the request.  Will return false if the 
	// handle is invalid or the request is already sent.
	bool SetHTTPRequestGetOrPostParameter( HTTPRequestHandle hRequest, const char *pchParamName, const char *pchParamValue );

	// Sends the HTTP request, will return false on a bad handle, otherwise use SteamCallHandle to wait on
	// asynchronous response via callback.
	//
	// Note: If the user is in offline mode in Steam, then this will add a only-if-cached cache-control 
	// header and only do a local cache lookup rather than sending any actual remote request.
	bool SendHTTPRequest( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle );

	// Sends the HTTP request, will return false on a bad handle, otherwise use SteamCallHandle to wait on
	// asynchronous response via callback for completion, and listen for HTTPRequestHeadersReceived_t and 
	// HTTPRequestDataReceived_t callbacks while streaming.
	bool SendHTTPRequestAndStreamResponse( HTTPRequestHandle hRequest, SteamAPICall_t *pCallHandle );

	// Defers a request you have sent, the actual HTTP client code may have many requests queued, and this will move
	// the specified request to the tail of the queue.  Returns false on invalid handle, or if the request is not yet sent.
	bool DeferHTTPRequest( HTTPRequestHandle hRequest );

	// Prioritizes a request you have sent, the actual HTTP client code may have many requests queued, and this will move
	// the specified request to the head of the queue.  Returns false on invalid handle, or if the request is not yet sent.
	bool PrioritizeHTTPRequest( HTTPRequestHandle hRequest );

	// Checks if a response header is present in a HTTP response given a handle from HTTPRequestCompleted_t, also 
	// returns the size of the header value if present so the caller and allocate a correctly sized buffer for
	// GetHTTPResponseHeaderValue.
	bool GetHTTPResponseHeaderSize( HTTPRequestHandle hRequest, const char *pchHeaderName, uint32 *unResponseHeaderSize );

	// Gets header values from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the
	// header is not present or if your buffer is too small to contain it's value.  You should first call 
	// BGetHTTPResponseHeaderSize to check for the presence of the header and to find out the size buffer needed.
	bool GetHTTPResponseHeaderValue( HTTPRequestHandle hRequest, const char *pchHeaderName, uint8 *pHeaderValueBuffer, uint32 unBufferSize );

	// Gets the size of the body data from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the 
	// handle is invalid.
	bool GetHTTPResponseBodySize( HTTPRequestHandle hRequest, uint32 *unBodySize );

	// Gets the body data from a HTTP response given a handle from HTTPRequestCompleted_t, will return false if the 
	// handle is invalid or is to a streaming response, or if the provided buffer is not the correct size.  Use BGetHTTPResponseBodySize first to find out
	// the correct buffer size to use.
	bool GetHTTPResponseBodyData( HTTPRequestHandle hRequest, uint8 *pBodyDataBuffer, uint32 unBufferSize );

	// Gets the body data from a streaming HTTP response given a handle from HTTPRequestDataReceived_t. Will return false if the 
	// handle is invalid or is to a non-streaming response (meaning it wasn't sent with SendHTTPRequestAndStreamResponse), or if the buffer size and offset 
	// do not match the size and offset sent in HTTPRequestDataReceived_t.
	bool GetHTTPStreamingResponseBodyData( HTTPRequestHandle hRequest, uint32 cOffset, uint8 *pBodyDataBuffer, uint32 unBufferSize );

	// Releases an HTTP response handle, should always be called to free resources after receiving a HTTPRequestCompleted_t
	// callback and finishing using the response.
	bool ReleaseHTTPRequest( HTTPRequestHandle hRequest );

	// Gets progress on downloading the body for the request.  This will be zero unless a response header has already been
	// received which included a content-length field.  For responses that contain no content-length it will report
	// zero for the duration of the request as the size is unknown until the connection closes.
	bool GetHTTPDownloadProgressPct( HTTPRequestHandle hRequest, float *pflPercentOut );

	// Sets the body for an HTTP Post request.  Will fail and return false on a GET request, and will fail if POST params
	// have already been set for the request.  Setting this raw body makes it the only contents for the post, the pchContentType
	// parameter will set the content-type header for the request so the server may know how to interpret the body.
	bool SetHTTPRequestRawPostBody( HTTPRequestHandle hRequest, const char *pchContentType, uint8 *pubBody, uint32 unBodyLen );

	// Creates a cookie container handle which you must later free with ReleaseCookieContainer().  If bAllowResponsesToModify=true
	// than any response to your requests using this cookie container may add new cookies which may be transmitted with
	// future requests.  If bAllowResponsesToModify=false than only cookies you explicitly set will be sent.  This API is just for
	// during process lifetime, after steam restarts no cookies are persisted and you have no way to access the cookie container across
	// repeat executions of your process.
	HTTPCookieContainerHandle CreateCookieContainer( bool bAllowResponsesToModify );

	// Release a cookie container you are finished using, freeing it's memory
	bool ReleaseCookieContainer( HTTPCookieContainerHandle hCookieContainer );

	// Adds a cookie to the specified cookie container that will be used with future requests.
	bool SetCookie( HTTPCookieContainerHandle hCookieContainer, const char *pchHost, const char *pchUrl, const char *pchCookie );

	// Set the cookie container to use for a HTTP request
	bool SetHTTPRequestCookieContainer( HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer );

	// Set the extra user agent info for a request, this doesn't clobber the normal user agent, it just adds the extra info on the end
	bool SetHTTPRequestUserAgentInfo( HTTPRequestHandle hRequest, const char *pchUserAgentInfo );

	// Set that https request should require verified SSL certificate via machines certificate trust store
	bool SetHTTPRequestRequiresVerifiedCertificate( HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate );

	// Set an absolute timeout on the HTTP request, this is just a total time timeout different than the network activity timeout
	// which can bump everytime we get more data
	bool SetHTTPRequestAbsoluteTimeoutMS( HTTPRequestHandle hRequest, uint32 unMilliseconds );

	// Check if the reason the request failed was because we timed it out (rather than some harder failure)
	bool GetHTTPRequestWasTimedOut( HTTPRequestHandle hRequest, bool *pbWasTimedOut );
};

#endif // __INCLUDED_STEAM_HTTP_H__
