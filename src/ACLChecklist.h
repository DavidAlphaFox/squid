
/*
 * $Id: ACLChecklist.h,v 1.6 2003/02/21 12:02:30 robertc Exp $
 *
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#ifndef SQUID_ACLCHECKLIST_H
#define SQUID_ACLCHECKLIST_H

#include "typedefs.h"

class ACLChecklist {
  public:

    /* State class.
     * This abstract class defines the behaviour of
     * async lookups - which can vary for different ACL types.
     * Today, every state object must be a singleton.
     * See NULLState for an example.
     * Note that *no* state should be stored in the state object,
     * they are used to change the behaviour of the checklist, not
     * to hold information. If you need to store information in the
     * state object, consider subclassing ACLChecklist, converting it
     * to a composite, or changing the state objects from singletons to
     * refcounted objects.
     */

    class AsyncState {
      public:
	virtual void checkForAsync(ACLChecklist *) const = 0;
      protected:
	void changeState (ACLChecklist *, AsyncState *) const;
    };

    class NullState : public AsyncState {
      public:
	static NullState *Instance();
	virtual void checkForAsync(ACLChecklist *) const;
      private:
	static NullState _instance;
    };
    
    
    void *operator new(size_t);
    void operator delete(void *);
    void deleteSelf() const;

    ACLChecklist();
    ~ACLChecklist();
    /* To cause link failures if assignment attempted */
    ACLChecklist (ACLChecklist const &);
    ACLChecklist &operator=(ACLChecklist const &);

    void nonBlockingCheck(PF * callback, void *callback_data);
    void checkCallback(allow_t answer);
    bool matchAclList(const acl_list * list, bool const fast = false);
    ConnStateData *conn();
    void conn(ConnStateData *);
    int authenticated();

    bool asyncInProgress() const;
    void asyncInProgress(bool const);
    bool finished() const;
    void markFinished();
    void check();
    allow_t const & currentAnswer() const;
    void currentAnswer(allow_t const);
    void checkAccessList();
    void checkForAsync();
    void changeState (AsyncState *);
    AsyncState *asyncState() const;
    
    const acl_access *accessList;
    struct in_addr src_addr;
    struct in_addr dst_addr;
    struct in_addr my_addr;
    unsigned short my_port;
    request_t *request;
    /* for acls that look at reply data */
    HttpReply *reply;
    char rfc931[USER_IDENT_SZ];
    auth_user_request_t *auth_user_request;
    acl_lookup_state state[ACL_ENUM_MAX];
#if SQUID_SNMP
    char *snmp_community;
#endif
    PF *callback;
    void *callback_data;
    external_acl_entry *extacl_entry;
    bool destinationDomainChecked() const;
    void markDestinationDomainChecked();
    bool sourceDomainChecked() const;
    void markSourceDomainChecked();
private:
    CBDATA_CLASS(ACLChecklist);
    ConnStateData *conn_;	/* hack for ident and NTLM */
    bool async_;
    bool finished_;
    allow_t allow_;
    AsyncState *state_;
    bool destinationDomainChecked_;
    bool sourceDomainChecked_;
    bool checking_;
    bool checking() const;
    void checking (bool const);
};

SQUIDCEXTERN ACLChecklist *aclChecklistCreate(const acl_access *,
    request_t *,
    const char *ident);
SQUIDCEXTERN int aclCheckFast(const acl_access *A, ACLChecklist *);

#endif /* SQUID_ACLCHECKLIST_H */
