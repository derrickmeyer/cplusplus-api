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

#ifndef __SG_PROJECT_H__
#define __SG_PROJECT_H__

#include <Shotgun/Entity.h>

namespace SG {

// *****************************************************************************
/// \class Project
class Project : public Entity
{
    friend class Shotgun;
 
public:
    // -------------------------------------------------------------------------
    /// A copy constructor.
    Project(const Project &ref);

    /// A destructor that does nothing.
    virtual ~Project();

    // -------------------------------------------------------------------------
    /// Overrides for base Entity class methods that don't work correctly 
    /// for Project entity type.
    virtual const std::string sgProjectName() const { return getAttrValueAsString("name"); }

    /// Overrides for base Entity class methods that don't work correctly 
    /// for Project entity type.
    virtual const std::string sgProjectCode() const { return getAttrValueAsString("code"); }

    // -------------------------------------------------------------------------
    /// The string representation of Project entity type.
    static std::string entityType() { return std::string("Project"); }

    /// The string representation of Project class type.
    static std::string classType() { return entityType(); }

    // -------------------------------------------------------------------------
    Project &operator=(const Project &that)
    {
        if (this != &that)
        {
            Entity::operator=(that);
        }

        return *this;
    }

    // -------------------------------------------------------------------------
    friend std::ostream& operator<<(std::ostream &output, const Project &project)
    {
        output << project.str();
        return output;
    }

protected:
    // -------------------------------------------------------------------------
    /// A constructor.
    ///
    /// \param sg - instantiated Shotgun object pointer
    /// \param attrs - raw attribute map for a Shotgun entity
    Project(Shotgun *sg, const xmlrpc_c::value &attrs);

    // -------------------------------------------------------------------------
    /// A Project entity factory function.
    ///
    /// \param sg - instantiated Shotgun object pointer
    /// \param attrs - raw attribute map for a Shotgun entity
    /// \return a newly-created Project * as its base Entity * type
    static Entity *factory(Shotgun *sg, const xmlrpc_c::value &attrs) { return new Project(sg, attrs); }

    // -------------------------------------------------------------------------
    /// Builds a list of default "return_fields" which are the attributes
    /// exposed to the users when a Project entity is created or searched.
    ///
    /// \return a list of default "return_fields" name strings
    static List defaultReturnFields();
};

// *****************************************************************************
typedef std::vector<Project *> ProjectPtrs;

} // End namespace SG

#endif    // End #ifdef __SG_PROJECT_H__
