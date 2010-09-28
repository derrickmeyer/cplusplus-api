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
#include <Shotgun/Note.h>

namespace Shotgun {

// *****************************************************************************
Note::Note(Shotgun *sg, const xmlrpc_c::value &attrs)
    : Entity(sg)
{
    m_type = "Note";
    m_attrs = new xmlrpc_c::value(attrs);
}

// *****************************************************************************
Note::Note(const Note &ref)
    : Entity(ref.m_sg)
{
    m_type = "Note";
    m_attrs = new xmlrpc_c::value(*ref.m_attrs);
}

// *****************************************************************************
Note::~Note()
{
    // Nothing
}

// *****************************************************************************
Note *Note::create(Shotgun *sg, 
                   const std::string &projectCode,
                   const std::string &noteFromUserName,
                   const Strings &noteToUserNames,
                   const Strings &noteCcUserNames,
                   const std::string &noteSubject,
                   const std::string &noteBody,
                   const std::string &noteType,
                   const List &noteLinks,
                   const std::string &noteOrigin)
{
    HumanUser *user = sg->findHumanUserByLogin(noteFromUserName);

    Dict attrsMap = Dict("project", sg->getProjectLink(projectCode))
                    .add("user", user->asLink())
                    .add("subject", noteSubject)
                    .add("content", noteBody)
                    .add("sg_note_type", noteType)
                    .add("sg_note_origin", noteOrigin);
    
    delete user;

    if (noteLinks.size() > 0)
    {
        attrsMap.add("note_links", noteLinks);
    }

    // "addressings_to"
    List addressingsTo;
    for (size_t i = 0; i < noteToUserNames.size(); i++)
    {
        try
        {
            HumanUser *toUser = sg->findHumanUserByLogin(noteToUserNames[i]);
            addressingsTo.append(toUser->asLink());

            delete toUser;
        }
        catch (SgEntityNotFoundError)
        {
            // Do nothing
        }
    }
    attrsMap.add("addressings_to", addressingsTo);

    // "addressings_cc"
    List addressingsCc;
    for (size_t i = 0; i < noteCcUserNames.size(); i++)
    {
        try
        {
            HumanUser *toUser = sg->findHumanUserByLogin(noteCcUserNames[i]);
            addressingsCc.append(toUser->asLink());

            delete toUser;
        }
        catch (SgEntityNotFoundError)
        {
            // Do nothing
        }
    }
    attrsMap.add("addressings_cc", addressingsCc);

    return sg->createEntity<Note>(attrsMap);
}

// *****************************************************************************
List Note::populateReturnFields()
{
    return List("id")
           .append("project")
           .append("created_at")
           .append("updated_at")
           .append("user")
           .append("content")
           .append("addressings_cc")
           .append("addressings_to")
           .append("sg_status_list")
           .append("subject")
           .append("sg_note_type")
           .append("note_links");
}

// *****************************************************************************
Review *Note::getLinkedReview()
{
    List links = sgLinks();

    for (size_t i = 0; i < links.size(); i++)
    {
        Dict linkAsDict;
        fromXmlrpcValue(links[i], linkAsDict);

        int id = Entity::getAttrValueAsInt("id", linkAsDict);

        return m_sg->findEntity<Review>(FilterBy("id", "is", id));
    }

    throw SgEntityNotFoundError("Review");
}

// *****************************************************************************
Shot *Note::getLinkedShot()
{
    List links = sgLinks();

    for (size_t i = 0; i < links.size(); i++)
    {
        Dict linkAsDict;
        fromXmlrpcValue(links[i], linkAsDict);

        int id = Entity::getAttrValueAsInt("id", linkAsDict);

        return m_sg->findEntity<Shot>(FilterBy("id", "is", id));
    }

    throw SgEntityNotFoundError("Shot");
}

// *****************************************************************************
Version *Note::getLinkedVersion() 
{
    List links = sgLinks();

    for (size_t i = 0; i < links.size(); i++)
    {
        Dict linkAsDict;
        fromXmlrpcValue(links[i], linkAsDict);

        int id = Entity::getAttrValueAsInt("id", linkAsDict);

        return m_sg->findEntity<Version>(FilterBy("id", "is", id));
    }

    throw SgEntityNotFoundError("Version");
}

} // End namespace Shotgun

// *****************************************************************************
// *****************************************************************************
std::string toStdString(const Shotgun::Note &note)
{
    return toStdString(note.attrs());
}

// *****************************************************************************
std::string toStdString(const Shotgun::Notes &notes)
{
    Shotgun::List list;
    for (size_t i = 0; i < notes.size(); i++)
    {
        list.append(notes[i].attrs());
    }
    
    return toStdString(list);
}

// *****************************************************************************
std::ostream& operator<<(std::ostream &output, const Shotgun::Note &note)
{
    output << toStdString(note);
    return output;
}

// *****************************************************************************
std::ostream& operator<<(std::ostream &output, const Shotgun::Notes &notes)
{
    output << toStdString(notes);
    return output;
}
