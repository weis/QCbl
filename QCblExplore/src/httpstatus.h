#ifndef HTTPSTATUS_H
#define HTTPSTATUS_H

enum HttpStatus
{
    HttpError = -1,		/* An error response from httpXxxx() */

    HttpContinue = 100,		/* Everything OK, keep going... */
    HttpSwitchingProtocols,	/* Httpupgrade to TLS/SSL */

    HttpOk = 200,		/* OPTIONS/GET/HEAD/POST/TRACE command was successful */
    HttpCreated,			/* PUT command was successful */
    HttpAccepted,		/* DELETE command was successful */
    HttpNotAuthoritative,	/* Information isn't authoritative */
    HttpNoContent,		/* Successful command, no new data */
    HttpResetContent,		/* Content was reset/recreated */
    HttpPartialContent,		/* Only a partial file was recieved/sent */

    HttpMultipleChoices = 300,	/* Multiple files match request */
    HttpMovedPermanently,	/* Document has moved permanently */
    HttpMovedTemporarily,	/* Document has moved temporarily */
    HttpSeeOther,		/* See this other link... */
    HttpNotModified,		/* File not modified */
    HttpUseProxy,		/* Must use a proxy to access this URI */

    HttpBadRequest = 400,	/* Bad request */
    HttpUnauthorized,		/* Unauthorized to access host */
    HttpPaymentRequired,	/* Payment required */
    HttpForbidden,		/* Forbidden to access this URI */
    HttpNotFound,		/* URI was not found */
    HttpMethodNotAllowed,	/* Method is not allowed */
    HttpNotAcceptable,		/* Not Acceptable */
    HttpProxyAuthentication,	/* Proxy Authentication is Required */
    HttpRequestTimeout,		/* Request timed out */
    HttpConflict,		/* Request is self-conflicting */
    HttpGone,			/* Server has gone away */
    HttpLengthRequired,		/* A content length or encoding is required */
    HttpPrecondition,		/* Precondition failed */
    HttpRequestTooLarge,	/* Request entity too large */
    HttpUriTooLong,		/* URI too long */
    HttpUnsupportedMediatype,	/* The requested media type is unsupported */
    HttpUpgradeRequired = 426,	/* Upgrade to SSL/TLS required */

    HttpServerError = 500,	/* Internal server error */
    HttpNotImplemented,		/* Feature not implemented */
    HttpBadGateway,		/* Bad gateway */
    HttpServiceUnavailable,	/* Service is unavailable */
    HttpGatewayTimeout,		/* Gateway connection timed out */
    HttpNotSupported		/* Httpversion not supported */
};


#endif // HTTPSTATUS_H
