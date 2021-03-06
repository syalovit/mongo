/**
*    Copyright (C) 2008 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "../stdafx.h"
#include "../util/message.h"
#include "../util/top.h"
#include "boost/version.hpp"
#include "concurrency.h"
#include "pdfile.h"

namespace mongo {

    void jniCallback(Message& m, Message& out);

// tempish...move to TLS or pass all the way down as a parm
    extern map<string,Database*> databases;
    extern Database *database;
    extern const char *curNs;
    extern bool master;

    inline string getKey( const char *ns, const string& path ) {
        char cl[256];
        nsToClient(ns, cl);
        return string( cl ) + ":" + path;
    }

    /* returns true if the database ("database") did not exist, and it was created on this call */
    inline bool setClient(const char *ns, const string& path=dbpath) {
        /* we must be in critical section at this point as these are global
           variables.
        */
        requireInWriteLock();
        
        log( 5 ) << "setClient: " << ns << endl;
        
        Top::clientStart( ns );
        
        curNs = ns;
        string key = getKey( ns, path );
        map<string,Database*>::iterator it = databases.find(key);
        if ( it != databases.end() ) {
            database = it->second;
            return false;
        }

        // when master for replication, we advertise all the db's, and that
        // looks like a 'first operation'. so that breaks this log message's
        // meaningfulness.  instead of fixing (which would be better), we just
        // stop showing for now.
        // 2008-12-22 We now open every database on startup, so this log is
        // no longer helpful.  Commenting.
//    if( !master )
//        log() << "first operation for database " << key << endl;

        char cl[256];
        nsToClient(ns, cl);
        bool justCreated;
        Database *c = new Database(cl, justCreated, path);
        databases[key] = c;
        database = c;
        database->finishInit();

        return justCreated;
    }

// shared functionality for removing references to a database from this program instance
// does not delete the files on disk
    void closeClient( const char *cl, const string& path = dbpath );

    /* remove database from the databases map */
    inline void eraseDatabase( const char *ns, const string& path=dbpath ) {
        string key = getKey( ns, path );
        databases.erase( key );
    }

    /* We normally keep around a curNs ptr -- if this ns is temporary,
       use this instead so we don't have a bad ptr.  we could have made a copy,
       but trying to be fast as we call setClient this for every single operation.
    */
    inline bool setClientTempNs(const char *ns) {
        bool jc = setClient(ns);
        curNs = "";
        return jc;
    }

    inline bool clientIsEmpty() {
        return !database->namespaceIndex.allocated();
    }
    
    struct dbtemprelease {
        string clientname;
        string clientpath;
        dbtemprelease() {
            if ( database ) {
                clientname = database->name;
                clientpath = database->path;
            }
            Top::clientStop();
            dbMutexInfo.leaving();
#if BOOST_VERSION >= 103500
            dbMutex.unlock();
#else
            boost::detail::thread::lock_ops<boost::recursive_mutex>::unlock(dbMutex);
#endif
        }
        ~dbtemprelease() {
#if BOOST_VERSION >= 103500
            dbMutex.lock();
#else
            boost::detail::thread::lock_ops<boost::recursive_mutex>::lock(dbMutex);
#endif
            dbMutexInfo.entered();
            if ( clientname.empty() )
                database = 0;
            else
                setClient(clientname.c_str(), clientpath.c_str());
        }
    };

} // namespace mongo

#include "dbinfo.h"
#include "concurrency.h"
