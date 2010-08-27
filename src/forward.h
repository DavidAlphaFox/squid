#ifndef SQUID_FORWARD_H
#define SQUID_FORWARD_H

/* forward decls */

class ErrorState;
class HttpRequest;

#include "comm.h"
#include "comm/Connection.h"
#include "fde.h"
#include "ip/Address.h"
#include "Array.h"

class FwdState : public RefCountable
{
public:
    typedef RefCount<FwdState> Pointer;
    ~FwdState();
    static void initModule();

    static void fwdStart(Comm::ConnectionPointer &client, StoreEntry *, HttpRequest *);
    void startConnectionOrFail();
    void fail(ErrorState *err);
    void unregister(Comm::ConnectionPointer &conn);
    void unregister(int fd);
    void complete();
    void handleUnregisteredServerEnd();
    int reforward();
    bool reforwardableStatus(http_status s);
    void serverClosed(int fd);
    void connectStart();
    void connectDone(Comm::ConnectionPointer & conn, comm_err_t status, int xerrno);
    void connectTimeout(int fd);
    void initiateSSL();
    void negotiateSSL(int fd);
    bool checkRetry();
    bool checkRetriable();
    void dispatch();
    void pconnPush(Comm::ConnectionPointer & conn, const peer *_peer, const HttpRequest *req, const char *domain, Ip::Address &client_addr);

    bool dontRetry() { return flags.dont_retry; }

    void dontRetry(bool val) { flags.dont_retry = val; }

    /** return a ConnectionPointer to the current server connection (may or may not be open) */
    Comm::ConnectionPointer const & serverConnection() const { return serverConn; };

private:
    // hidden for safer management of self; use static fwdStart
    FwdState(Comm::ConnectionPointer &client, StoreEntry *, HttpRequest *);
    void start(Pointer aSelf);

    static void logReplyStatus(int tries, http_status status);
    void updateHierarchyInfo();
    void completed();
    void retryOrBail();
    static void RegisterWithCacheManager(void);

#if WIP_FWD_LOG

    void uninit                /**DOCS_NOSEMI*/
    static void logRotate      /**DOCS_NOSEMI*/
    void status()              /**DOCS_NOSEMI*/
#endif

public:
    StoreEntry *entry;
    HttpRequest *request;
    static void abort(void*);

private:
    Pointer self;
    ErrorState *err;
    Comm::ConnectionPointer clientConn;        ///< a possibly open connection to the client.
    time_t start_t;
    int n_tries;
    int origin_tries;
#if WIP_FWD_LOG

    http_status last_status;
#endif

    struct {
        unsigned int dont_retry:1;
        unsigned int forward_completed:1;
    } flags;

    /** connections to open, in order, until successful */
    Comm::ConnectionList serverDestinations;

    Comm::ConnectionPointer serverConn; ///< a successfully opened connection to a server.

    // NP: keep this last. It plays with private/public
    CBDATA_CLASS2(FwdState);
};

#endif /* SQUID_FORWARD_H */
