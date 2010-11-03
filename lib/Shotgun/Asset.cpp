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

#include <Shotgun/Entity.h>
#include <Shotgun/Shotgun.h>
#include <Shotgun/Asset.h>

namespace SG {

// *****************************************************************************
Asset::Asset(Shotgun *sg, const xmlrpc_c::value &attrs)
    : Entity(sg), TaskMixin(), NoteMixin()
{
    m_entityType = m_classType = "Asset";
    m_attrs = new xmlrpc_c::value(attrs);
}

// *****************************************************************************
Asset::Asset(const Asset &ref)
    : Entity(ref.m_sg)
{
    m_entityType = m_classType = "Asset";
    m_attrs = new xmlrpc_c::value(*ref.m_attrs);
}

// *****************************************************************************
Asset::~Asset()
{
    // Nothing
}

// *****************************************************************************
List Asset::defaultReturnFields()
{
    return List("id")
           .append("project")
           .append("created_at")
           .append("updated_at")
           .append("code")
           .append("sg_asset_type")
           .append("sg_status_list")
           .append("sg_asset_preview_qt")
           .append("sg_asset_source")
           .append("elements")
           .append("parents")
           .append("assets")
           .append("shots");
}

} // End namespace SG
