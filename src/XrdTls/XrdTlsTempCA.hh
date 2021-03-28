/******************************************************************************/
/*                                                                            */
/*               X r d T l s T e m p C A . h h                                */
/*                                                                            */
/* (c) 2021 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*   Produced by Brian Bockelman                                              */
/*                                                                            */
/* This file is part of the XRootD software suite.                            */
/*                                                                            */
/* XRootD is free software: you can redistribute it and/or modify it under    */
/* the terms of the GNU Lesser General Public License as published by the     */
/* Free Software Foundation, either version 3 of the License, or (at your     */
/* option) any later version.                                                 */
/*                                                                            */
/* XRootD is distributed in the hope that it will be useful, but WITHOUT      */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      */
/* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public       */
/* License for more details.                                                  */
/*                                                                            */
/* You should have received a copy of the GNU Lesser General Public License   */
/* along with XRootD in a file called COPYING.LESSER (LGPL license) and file  */
/* COPYING (GPL license).  If not, see <http://www.gnu.org/licenses/>.        */
/*                                                                            */
/* The copyright holder's institutional names and contributor's names may not */
/* be used to endorse or promote products derived from this software without  */
/* specific prior written permission of the institution or contributor.       */
/******************************************************************************/

#include <atomic>
#include <string>
#include <memory>

// Forward dec'ls.
class XrdSysError;

namespace XrdTls {

/**
 * This class provides manages a "CA file" that is a concatenation of all the
 * CAs in a given CA directory.  This is useful in TLS contexts where, instead
 * of loading all CAs for each connection, we only want to load a single file.
 *
 * This will hand out the CA file directly, allowing external libraries (such
 * as libcurl) do the loading of CAs directly.
 */
class XrdTlsTempCA {
public:
    class TempCAGuard;

    XrdTlsTempCA(XrdSysError *log, std::string ca_dir);

    /**
     * Return a handle to the current CA file.  The shared_ptr
     * *must* be kept alive while the CA file is in use; if it
     * goes out of scope, the corresponding temporary file may
     * be garbage collected.
     */
    std::shared_ptr<TempCAGuard> getHandle();

    /**
     * Returns true if object is valid.
     */
    bool IsValid() const {return m_ca_file.get();}

    /**
     * Manages the temporary file associated with the curl handle
     */
    class TempCAGuard {
    public:
        static std::unique_ptr<TempCAGuard> create(XrdSysError &);

    int getFD() const {return m_fd;}
    std::string getFilename() const {return m_fname;}

    TempCAGuard(const TempCAGuard &) = delete;

    ~TempCAGuard();

    private:
        TempCAGuard(int fd, const std::string &fname);

        int m_fd;
        std::string m_fname;
    };


private:
    /** 
     * Run the CA maintenance routines.
     * This will go through the CA directory, concatenate the
     * CA contents into a single PEM file, and delete the prior
     * copy of the concatenated CA certs.
     */
    bool Maintenance();

    /**
     * Returns true if we need to run the CA maintenance
     * routine.
     */
    bool NeedsMaintenance();

    std::atomic<time_t> m_next_update{0};
    XrdSysError &m_log;
    const std::string m_ca_dir;
    std::shared_ptr<TempCAGuard> m_ca_file;
};

}
