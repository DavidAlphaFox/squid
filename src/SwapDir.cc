
/*
 * $Id: SwapDir.cc,v 1.5 2003/08/31 21:20:08 robertc Exp $
 *
 * DEBUG: section ??    Swap Dir base object
 * AUTHOR: Robert Collins
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

#include "squid.h"
#include "SwapDir.h"
#include "Store.h"
#include "StoreFileSystem.h"

SwapDir::~SwapDir()
{
    xfree(path);
}

void
SwapDir::newFileSystem(){}

void
SwapDir::dump(StoreEntry &)const{}

bool
SwapDir::doubleCheck(StoreEntry &)
{
    return false;
}

void
SwapDir::unlink(StoreEntry &){}

void
SwapDir::statfs(StoreEntry &)const {}

void
SwapDir::maintainfs(){}

void
SwapDir::reference(StoreEntry &){}

void
SwapDir::dereference(StoreEntry &){}

int
SwapDir::callback()
{
    return 0;
}

void
SwapDir::sync(){}

/* Move to StoreEntry ? */
bool
SwapDir::canLog(StoreEntry const &e)const
{
    if (e.swap_filen < 0)
        return false;

    if (e.swap_status != SWAPOUT_DONE)
        return false;

    if (e.swap_file_sz <= 0)
        return false;

    if (EBIT_TEST(e.flags, RELEASE_REQUEST))
        return false;

    if (EBIT_TEST(e.flags, KEY_PRIVATE))
        return false;

    if (EBIT_TEST(e.flags, ENTRY_SPECIAL))
        return false;

    return true;
}

void
SwapDir::openLog(){}

void
SwapDir::closeLog(){}

int
SwapDir::writeCleanStart()
{
    return 0;
}

void
SwapDir::writeCleanDone(){}

void
SwapDir::logEntry(const StoreEntry & e, int op) const{}

char const *
SwapDir::type() const
{
    return theType;
}

/* NOT performance critical. Really. Don't bother optimising for speed
 * - RBC 20030718 
 */
SwapDirOption *
SwapDir::getOptionTree() const
{
    SwapDirOptionVector *result = new SwapDirOptionVector;
    result->options.push_back(new SwapDirOptionAdapter<SwapDir>(*const_cast<SwapDir *>(this), &SwapDir::optionReadOnlyParse, &SwapDir::optionReadOnlyDump));
    result->options.push_back(new SwapDirOptionAdapter<SwapDir>(*const_cast<SwapDir *>(this), &SwapDir::optionMaxSizeParse, &SwapDir::optionMaxSizeDump));
    return result;
}

void
SwapDir::parseOptions(int reconfiguring)
{
    unsigned int old_read_only = flags.read_only;
    char *name, *value;

    SwapDirOption *newOption = getOptionTree();

    while ((name = strtok(NULL, w_space)) != NULL) {
        value = strchr(name, '=');

        if (value)
            *value++ = '\0';	/* cut on = */

        debugs(3,2, "SwapDir::parseOptions: parsing store option '" << name << "'='" << (value ? value : "") << "'");

        if (newOption)
            if (!newOption->parse(name, value, reconfiguring))
                self_destruct();
    }

    delete newOption;

    /*
     * Handle notifications about reconfigured single-options with no value
     * where the removal of the option cannot be easily detected in the
     * parsing...
     */

    if (reconfiguring) {
        if (old_read_only != flags.read_only) {
            debug(3, 1) ("Cache dir '%s' now %s\n",
                         path, flags.read_only ? "Read-Only" : "Read-Write");
        }
    }
}

void
SwapDir::dumpOptions(StoreEntry * entry) const
{
    SwapDirOption *newOption = getOptionTree();

    if (newOption)
        newOption->dump(entry);

    delete newOption;
}

bool
SwapDir::optionReadOnlyParse(char const *option, const char *value, int reconfiguring)
{
    if (strcmp(option, "read-only") != 0)
        return false;

    int read_only = 0;

    if (value)
        read_only = xatoi(value);
    else
        read_only = 1;

    flags.read_only = read_only;

    return true;
}

void
SwapDir::optionReadOnlyDump(StoreEntry * e) const
{
    if (flags.read_only)
        storeAppendPrintf(e, " read-only");
}

bool
SwapDir::optionMaxSizeParse(char const *option, const char *value, int reconfiguring)
{
    if (strcmp(option, "max-size") != 0)
        return false;

    if (!value)
        self_destruct();

    ssize_t size = xatoi(value);

    if (reconfiguring && max_objsize != size)
        debug(3, 1) ("Cache dir '%s' max object size now %ld\n", path, (long int) size);

    max_objsize = size;

    return true;
}

void
SwapDir::optionMaxSizeDump(StoreEntry * e) const
{
    if (max_objsize != -1)
        storeAppendPrintf(e, " max-size=%ld", (long int) max_objsize);
}

SwapDirOptionVector::~SwapDirOptionVector()
{
    while (options.size()) {
        delete options.back();
        options.pop_back();
    }
}

bool
SwapDirOptionVector::parse(char const *option, const char *value, int reconfiguring)
{
    Vector<SwapDirOption *>::iterator i = options.begin();

    while (i != options.end()) {
        if ((*i)->parse(option,value, reconfiguring))
            return true;

        ++i;
    }

    return false;
}

void
SwapDirOptionVector::dump(StoreEntry * e) const
{
    for(Vector<SwapDirOption *>::const_iterator i = options.begin();
            i != options.end(); ++i)
        (*i)->dump(e);
}
