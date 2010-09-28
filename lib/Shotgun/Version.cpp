/*
-----------------------------------------------------------------------------
Copyright (c) 2010 Tippett Studio
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/

#include <Shotgun/Method.h>
#include <Shotgun/Entity.h>
#include <Shotgun/Shotgun.h>
#include <Shotgun/Version.h>
#include <Shotgun/Shot.h>
#include <Shotgun/Sequence.h>
#include <Shotgun/Project.h>

namespace Shotgun {

// *****************************************************************************
Version::Version(Shotgun *sg, const xmlrpc_c::value &attrs)
    : Entity(sg), NoteMixin()
{
    m_type = "Version";
    m_attrs = new xmlrpc_c::value(attrs);
}

// *****************************************************************************
Version::Version(const Version &ref)
    : Entity(ref.m_sg)
{
    m_type = "Version";
    m_attrs = new xmlrpc_c::value(*ref.m_attrs);
}

// *****************************************************************************
Version::~Version()
{
    // Nothing
}

// *****************************************************************************
Version *Version::create(Shotgun *sg, const std::string &versionName)
{
    // Check if the version already exists
    try
    {
        Version *version = sg->findVersionByName(versionName);
        delete version;

        std::string err = "Version \"" + versionName + "\" already exists.";
        throw SgEntityCreateError(err);
    }
    catch (SgEntityNotFoundError)
    {
        throw SgEntityCreateError("[Not implemented yet] Version::create(..)");
#warning Implement in non-Tippett way
//         TipUtil::VersionName dn = TipUtil::VersionName(versionName);
// 
//         // Create a very basic Version entity with most of its attributes 
//         // be filled later from the Python side.
//         Dict attrsMap = Dict("project", sg->getProjectLink(projectCode))
//                         .add("code", versionName)
//                         .add("sg_sequence", dn.sequence())
//                         .add("sg_rev", dn.versionStr());
// 
//         return sg->createEntity<Version>(attrsMap);
    }
}

// *****************************************************************************
List Version::populateReturnFields()
{
    return List("id")
           .append("project")
           .append("created_at")
           .append("updated_at")
           .append("code")
           .append("sg_department")
           .append("description")
           .append("sg_sequence")
           .append("entity")
           .append("sg_rev")
           .append("frame_count")
           .append("frame_range")
           .append("sg_source")
           .append("sg_source_2k")
           .append("sg_daily_hd")
           .append("image")
           .append("sg_status")
           .append("sg_status_list")
           .append("sg_epk_")
           .append("sg_dailies_date")
           .append("sg_view_order")
           .append("sg_preview_qt")
           .append("sg_preview_hd_qt")
           .append("user");
}

// *****************************************************************************
const Shot *Version::sgShot() const
{
#warning Implement in non-Tippett way
//     std::string theShotName = TipUtil::ShotName(sgProjectCode(), sgShotName()).shot(true, true);
// 
//     return m_sg->findShotByName(theShotName);
}

} // End namespace Shotgun

// *****************************************************************************
// *****************************************************************************
std::string toStdString(const Shotgun::Version &version)
{
    return toStdString(version.attrs());
}

// *****************************************************************************
std::string toStdString(const Shotgun::Versions &versions)
{
    Shotgun::List list;
    for (size_t i = 0; i < versions.size(); i++)
    {
        list.append(versions[i].attrs());
    }
    
    return toStdString(list);
}

// *****************************************************************************
std::ostream& operator<<(std::ostream &output, const Shotgun::Version &version)
{
    output << toStdString(version);
    return output;
}

// *****************************************************************************
std::ostream& operator<<(std::ostream &output, const Shotgun::Versions &versions)
{
    output << toStdString(versions);
    return output;
}
