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

#include <iostream>

#include <Shotgun/Method.h>
#include <Shotgun/Entity.h>
#include <Shotgun/Shotgun.h>
#include <Shotgun/Review.h>
#include <Shotgun/Project.h>

namespace Shotgun {

// *****************************************************************************
Review::Review(Shotgun *sg, const xmlrpc_c::value &attrs)
    : Entity(sg)
{
    m_type = "Review";
    m_attrs = new xmlrpc_c::value(attrs);
}

// *****************************************************************************
Review::Review(const Review &ref)
    : Entity(ref.m_sg)
{
    m_type = "Review";
    m_attrs = new xmlrpc_c::value(*ref.m_attrs);
}

// *****************************************************************************
Review::~Review()
{
    // Nothing
}

// *****************************************************************************
Review *Review::create(Shotgun *sg, 
                       const std::string &projectCode,
                       const std::string &reviewName)
{
    // Check if the review already exists
    try
    {
        Review *review = sg->findReviewByName(projectCode, reviewName);
        delete review;

        std::string err = "Review \"" + reviewName + "\" already exists.";
        throw SgEntityCreateError(err);
    }
    catch (SgEntityNotFoundError)
    {
        Dict attrsMap = Dict("project", sg->getProjectLink(projectCode))
                        .add("code", reviewName);

        return sg->createEntity<Review>(attrsMap);
    }
}

// *****************************************************************************
List Review::populateReturnFields()
{
    return List("id")
           .append("project")
           .append("created_at")
           .append("updated_at")
           .append("code")
           .append("sg_review_type")
           .append("sg_review_media")
           .append("sg_review_date_sent")
           .append("sg_review_sent_to")
           .append("sg_review_date_reviewed")
           .append("sg_review_reviewed_by")
           .append("sg_review_disclaimers")
           .append("sg_review_tipsupe_notes")
           .append("sg_review_client_notes");
}

} // End namespace Shotgun

// *****************************************************************************
std::string toStdString(const Shotgun::Review &review)
{
    return toStdString(review.attrs());
}

// *****************************************************************************
std::string toStdString(const Shotgun::Reviews &reviews)
{
    Shotgun::List list;
    for (size_t i = 0; i < reviews.size(); i++)
    {
        list.append(reviews[i].attrs());
    }
    
    return toStdString(list);
}

// *****************************************************************************
std::ostream& operator<<(std::ostream &output, const Shotgun::Review &review)
{
    output << toStdString(review);
    return output;
}

// *****************************************************************************
std::ostream& operator<<(std::ostream &output, const Shotgun::Reviews &reviews)
{
    output << toStdString(reviews);
    return output;
}
