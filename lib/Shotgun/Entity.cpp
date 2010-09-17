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
#include <stdexcept>

#include <Shotgun/Type.h>
#include <Shotgun/FilterBy.h>
#include <Shotgun/Method.h>
#include <Shotgun/Entity.h>
#include <Shotgun/Shotgun.h>

namespace Shotgun {

// *****************************************************************************
Entity::Entity(Shotgun *sg)
    : m_sg(sg), m_invalidAttrMode(INVALID_ATTR_THROW_EXCEPTION)
    //: m_sg(sg), m_invalidAttrMode(INVALID_ATTR_USE_DEFAULT)
{
    m_attrs = NULL;

    // These are the default return fields for all entity types
    //m_returnFields.push_back(toXmlrpcValue("id"));
    //m_returnFields.push_back(toXmlrpcValue("project"));
    //m_returnFields.push_back(toXmlrpcValue("created_at"));
    //m_returnFields.push_back(toXmlrpcValue("updated_at"));
}

// *****************************************************************************
Entity::~Entity()
{
    delete m_attrs;
}

// *****************************************************************************
xmlrpc_c::value Entity::createSGEntity(Shotgun *sg, 
                                       const std::string &entityType, 
                                       const SgMap &data)
{
    Method *md = sg->method("create");

    SgMap createMap = buildCreateMap(entityType, data);

    xmlrpc_c::paramList params;
    params.add(toXmlrpcValue(sg->authMap()));
    params.add(toXmlrpcValue(createMap));

    xmlrpc_c::value rawResult = md->call(params); 
    xmlrpc_c::value results;

    if (rawResult.type() != xmlrpc_c::value::TYPE_NIL)
    {
        results = toXmlrpcValue(getAttrValueAsMap("results",
                                                  SgMap(xmlrpc_c::value_struct(rawResult))));
    }

    return results;
}

// *****************************************************************************
xmlrpc_c::value Entity::updateSGEntity(Shotgun *sg, 
                                       const std::string &entityType, 
                                       const int entityId,
                                       const SgArray &fieldsToUpdate)
{
    Method *md = sg->method("update");

    SgMap updateMap = buildUpdateMap(entityType, entityId, fieldsToUpdate);

    xmlrpc_c::paramList params;
    params.add(toXmlrpcValue(sg->authMap()));
    params.add(toXmlrpcValue(updateMap));

    xmlrpc_c::value rawResult = md->call(params); 
    xmlrpc_c::value results;

    if (rawResult.type() != xmlrpc_c::value::TYPE_NIL)
    {
        results = toXmlrpcValue(getAttrValueAsMap("results",
                                                  SgMap(xmlrpc_c::value_struct(rawResult))));
    }

    return results;
}

// *****************************************************************************
SgArray Entity::findSGEntities(Shotgun *sg, SgMap &findMap)
{
    Method *md = sg->method("read");

    SgArray entityArray;

    // Retrieve "limit" from the findMap because it is a field unrecognized
    // by Shotgun and we merely attach it to findMap to pass it over from
    // buildFindMap(..) to this function. So need to remove it from findMap
    // afterwards.
    int limit = getAttrValueAsInt("limit", findMap, 0, INVALID_ATTR_USE_DEFAULT);
    findMap.erase("limit");

    bool done = false;
    while (!done)
    {
        xmlrpc_c::paramList params;
        params.add(toXmlrpcValue(sg->authMap()));
        params.add(toXmlrpcValue(findMap));

        // api3 returns a struct - so convert it to an array of entities
        xmlrpc_c::value rawResult = md->call(params);

        SgArray entities = Entity::getFindResultEntityList(rawResult);

        if (entities.size() == 0)
        {
            done = true;
        }
        else
        {
            entityArray.insert(entityArray.end(), entities.begin(), entities.end());

            SgMap resultPaging = Entity::getResultPagingInfo(rawResult);
            int entityCount = getAttrValueAsInt("entity_count", resultPaging);

            if (limit > 0 && entityArray.size() >= limit)
            {
                entityArray.erase(entityArray.begin()+limit, entityArray.end());
                done = true;
            }
            else if (entityArray.size() == entityCount)
            {
                done = true;
            }
            else
            {
                // Erase the current "paging" field in the findMap
                SgMap findMapPaging = getAttrValueAsMap("paging", 
                                                        findMap,
                                                        SgMap(),
                                                        Entity::INVALID_ATTR_USE_DEFAULT);
                if (findMapPaging.size() > 0) findMap.erase("paging");


                // Increase the "current_page" for the findMap "paging".
                SgMap newFindMapPaging;
                newFindMapPaging["current_page"] = toXmlrpcValue(getAttrValueAsInt("current_page", findMapPaging)+1);
                newFindMapPaging["entities_per_page"] = toXmlrpcValue(getAttrValueAsInt("entities_per_page", findMapPaging));
                findMap["paging"] = toXmlrpcValue(newFindMapPaging);
            }
        }
    }

    return entityArray;
}

// *****************************************************************************
bool Entity::deleteSGEntity(Shotgun *sg,
                            const std::string &entityType,
                            const int id)
{
    Method *md = sg->method("delete");

    SgMap deleteMap;
    deleteMap["type"] = toXmlrpcValue(entityType);
    deleteMap["id"] = toXmlrpcValue(id);

    xmlrpc_c::paramList params;
    params.add(toXmlrpcValue(sg->authMap()));
    params.add(toXmlrpcValue(deleteMap));

    xmlrpc_c::value rawResult = md->call(params); 

    return getAttrValueAsBool("results", SgMap(xmlrpc_c::value_struct(rawResult)));
}

// *****************************************************************************
SgArray Entity::getFindResultEntityList(xmlrpc_c::value &rawResult)
{
    SgArray entityArray;

    if (rawResult.type() != xmlrpc_c::value::TYPE_NIL)
    {
        xmlrpc_c::value results = toXmlrpcValue(getAttrValueAsMap("results", 
                                                                  SgMap(xmlrpc_c::value_struct(rawResult))));
        xmlrpc_c::value entities = toXmlrpcValue(getAttrValueAsArray("entities", 
                                                                     SgMap(xmlrpc_c::value_struct(results))));
        entityArray = xmlrpc_c::value_array(entities).vectorValueValue();
    }

    return entityArray;
}

// *****************************************************************************
SgMap Entity::getResultPagingInfo(xmlrpc_c::value &rawResult)
{
    SgMap pagingInfoMap;

    if (rawResult.type() != xmlrpc_c::value::TYPE_NIL)
    {
        xmlrpc_c::value results = toXmlrpcValue(getAttrValueAsMap("results", 
                                                                  SgMap(xmlrpc_c::value_struct(rawResult))));
        xmlrpc_c::value pagingInfo = toXmlrpcValue(getAttrValueAsMap("paging_info", 
                                                                     SgMap(xmlrpc_c::value_struct(results))));
        pagingInfoMap = SgMap(xmlrpc_c::value_struct(pagingInfo));
    }

    return pagingInfoMap;
}

// *****************************************************************************
void Entity::validateLink(const SgMap &link)
{
    try
    {
        int id = getAttrValueAsInt("id", link);
        std::string type = getAttrValueAsString("type", link);
        std::string name = getAttrValueAsString("name", link);
    }
    catch (SgAttrError)
    {
        // TODO: is "name" field mandatory or can it be omitted?
        throw SgAttrLinkError(link);
    }
}

// *****************************************************************************
void Entity::validateLink(const xmlrpc_c::value &link)
{
    if (link.type() == xmlrpc_c::value::TYPE_STRUCT)
    {
        SgMap linkAsMap = SgMap(xmlrpc_c::value_struct(link));

        validateLink(linkAsMap);
    }
    else
    {
        throw SgAttrLinkError(link, link.type());
    }
}

// *****************************************************************************
const std::string Entity::getProjectName() const
{
    SgMap project = getAttrValueAsMap("project");
    std::string projectName = getAttrValueAsString("name", project);

    return projectName;
}

// *****************************************************************************
const std::string Entity::getProjectCode() const
{
    SgMap projectMap = getAttrValueAsMap("project");
    std::string projectName = getAttrValueAsString("name", projectMap);

    Project *project = m_sg->findProjectByName(projectName);
    return project->sgCode();
}

// *****************************************************************************
const SgMap Entity::asLink() const
{
    SgMap link;
    link["type"] = toXmlrpcValue(entityType());
    link["id"] = toXmlrpcValue(sgId());
    link["name"] = toXmlrpcValue(sgName());

    return link;
}

// *****************************************************************************
const std::string Entity::linkEntityType(const std::string &linkName) const
{
    SgMap entity = getAttrValueAsMap(linkName);

    if (!entity.empty())
    {
        std::string type = getAttrValueAsString("type", entity);
        return type;
    }
    else
    {
        throw SgEntityNotFoundError("\"" + linkName + "\"");
    }
}

// *****************************************************************************
#warning FIX THIS 
SgMap Entity::buildCreateMap(const std::string &entityType,
                             const SgMap &data,
                             const SgArray &extraReturnFields)
{
    SgMap createMap;

    // -------------------------------------------------------------------
    // "type"
    createMap["type"] = toXmlrpcValue(entityType);

    // -------------------------------------------------------------------
    // "fields"
    SgArray fields;

    for(SgMap::const_iterator dataIter = data.begin(); dataIter != data.end(); ++dataIter)
    {
        SgMap field;
        field["field_name"] = toXmlrpcValue((*dataIter).first);
        field["value"] = toXmlrpcValue((*dataIter).second);

        fields.push_back(toXmlrpcValue(field));
    }
    
    createMap["fields"] = toXmlrpcValue(fields);

//     // -------------------------------------------------------------------
//     // "return_fields"
//     //
//     // Add a few basic return fields for all entities
//     SgArray returnFields = extraReturnFields;
//     returnFields.push_back(toXmlrpcValue("id"));
//     returnFields.push_back(toXmlrpcValue("project"));
//     returnFields.push_back(toXmlrpcValue("created_at"));
//     returnFields.push_back(toXmlrpcValue("updated_at"));
// 
//     // Add entity-specific return fields here
//     if (entityType == "Project")
//     {
//         returnFields.push_back(toXmlrpcValue("name"));
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_status"));
//         returnFields.push_back(toXmlrpcValue("sg_archive_watcher"));
//         returnFields.push_back(toXmlrpcValue("sg_pub_stills_watcher"));
//         returnFields.push_back(toXmlrpcValue("sg_generate_shot_aliases"));
//         returnFields.push_back(toXmlrpcValue("sg_send_dailies_notices"));
//         returnFields.push_back(toXmlrpcValue("sg_polish_shot_notifications"));
//         returnFields.push_back(toXmlrpcValue("sg_report_storage_information"));
//         returnFields.push_back(toXmlrpcValue("sg_default_start_frame"));
//         returnFields.push_back(toXmlrpcValue("sg_ms_project_schedule"));
//     }
//     else if (entityType == "Sequence")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//     }
//     else if (entityType == "Shot")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_continuity"));
//         returnFields.push_back(toXmlrpcValue("description"));
//         returnFields.push_back(toXmlrpcValue("elements"));
//         returnFields.push_back(toXmlrpcValue("sg_estimated_frame_render_hours"));
//         returnFields.push_back(toXmlrpcValue("sg_final_daily"));
//         returnFields.push_back(toXmlrpcValue("sg_latest_daily"));
//         returnFields.push_back(toXmlrpcValue("sg_lens"));
//         returnFields.push_back(toXmlrpcValue("sg_prod_vfx__"));
//         returnFields.push_back(toXmlrpcValue("sg_sequence"));
//         returnFields.push_back(toXmlrpcValue("sg_shot_notifications"));
//         returnFields.push_back(toXmlrpcValue("project_names"));
//         returnFields.push_back(toXmlrpcValue("sg_status_list"));
//         returnFields.push_back(toXmlrpcValue("sg_turnover_"));
//         returnFields.push_back(toXmlrpcValue("sg_type"));
//         returnFields.push_back(toXmlrpcValue("sg_omit_"));
//         returnFields.push_back(toXmlrpcValue("sg_on_hold_"));
//         returnFields.push_back(toXmlrpcValue("sg_cbb_"));
//         returnFields.push_back(toXmlrpcValue("smart_cut_duration"));
//         returnFields.push_back(toXmlrpcValue("smart_cut_in"));
//         returnFields.push_back(toXmlrpcValue("smart_cut_out"));
//         returnFields.push_back(toXmlrpcValue("smart_cut_summary_display"));
//         returnFields.push_back(toXmlrpcValue("smart_duration_summary_display"));
//         returnFields.push_back(toXmlrpcValue("smart_head_duration"));
//         returnFields.push_back(toXmlrpcValue("smart_head_in"));
//         returnFields.push_back(toXmlrpcValue("smart_head_out"));
//         returnFields.push_back(toXmlrpcValue("smart_tail_duration"));
//         returnFields.push_back(toXmlrpcValue("smart_tail_in"));
//         returnFields.push_back(toXmlrpcValue("smart_tail_out"));
//         returnFields.push_back(toXmlrpcValue("smart_working_duration"));
//         returnFields.push_back(toXmlrpcValue("sg_tippett_working_length"));
//         returnFields.push_back(toXmlrpcValue("sg_actual_plate_resolution"));
//         returnFields.push_back(toXmlrpcValue("sg_storage___tier"));
//         returnFields.push_back(toXmlrpcValue("sg_storage___filesystem"));
//         returnFields.push_back(toXmlrpcValue("sg_storage___filesystem_used_percentage"));
//         returnFields.push_back(toXmlrpcValue("sg_storage___size_gb"));
//         returnFields.push_back(toXmlrpcValue("sg_slate_burnin_info"));
//         returnFields.push_back(toXmlrpcValue("sg_slate_header_info"));
//         returnFields.push_back(toXmlrpcValue("sg_pixel_aspect"));
//     }
//     else if (entityType == "Version") // Daily
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_department"));
//         returnFields.push_back(toXmlrpcValue("description"));
//         returnFields.push_back(toXmlrpcValue("sg_sequence"));
//         returnFields.push_back(toXmlrpcValue("entity"));
//         returnFields.push_back(toXmlrpcValue("sg_rev"));
//         returnFields.push_back(toXmlrpcValue("frame_count"));
//         returnFields.push_back(toXmlrpcValue("frame_range"));
//         returnFields.push_back(toXmlrpcValue("sg_source"));
//         returnFields.push_back(toXmlrpcValue("sg_source_2k"));
//         returnFields.push_back(toXmlrpcValue("sg_daily_hd"));
//         returnFields.push_back(toXmlrpcValue("image"));
//         returnFields.push_back(toXmlrpcValue("sg_status"));
//         returnFields.push_back(toXmlrpcValue("sg_status_list"));
//         returnFields.push_back(toXmlrpcValue("sg_epk_"));
//         returnFields.push_back(toXmlrpcValue("sg_dailies_date"));
//         returnFields.push_back(toXmlrpcValue("sg_view_order"));
//         returnFields.push_back(toXmlrpcValue("sg_preview_qt"));
//         returnFields.push_back(toXmlrpcValue("sg_preview_hd_qt"));
//         returnFields.push_back(toXmlrpcValue("user"));
//     }
//     else if (entityType == "HumanUser")
//     {
//         returnFields.push_back(toXmlrpcValue("name"));
//         returnFields.push_back(toXmlrpcValue("admin"));
//         returnFields.push_back(toXmlrpcValue("sg_department"));
//         returnFields.push_back(toXmlrpcValue("email"));
//         returnFields.push_back(toXmlrpcValue("login"));
//         returnFields.push_back(toXmlrpcValue("sg_role"));
//         returnFields.push_back(toXmlrpcValue("permission_rule_set"));
//     }
//     else if (entityType == "Element")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("assets"));
//         returnFields.push_back(toXmlrpcValue("shots"));
//         returnFields.push_back(toXmlrpcValue("tag_list"));
//         returnFields.push_back(toXmlrpcValue("sg_element_type"));
//     }
//     else if (entityType == "Asset")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_asset_type"));
//         returnFields.push_back(toXmlrpcValue("sg_status_list"));
//         returnFields.push_back(toXmlrpcValue("sg_asset_preview_qt"));
//         returnFields.push_back(toXmlrpcValue("sg_asset_source"));
//         returnFields.push_back(toXmlrpcValue("elements"));
//         returnFields.push_back(toXmlrpcValue("parents"));
//         returnFields.push_back(toXmlrpcValue("assets"));
//         returnFields.push_back(toXmlrpcValue("shots"));
//     }
//     else if (entityType == "Delivery")
//     {
//         returnFields.push_back(toXmlrpcValue("title"));
//         returnFields.push_back(toXmlrpcValue("sg_delivery_data_size"));
//         returnFields.push_back(toXmlrpcValue("sg_delivery_notes"));
//         returnFields.push_back(toXmlrpcValue("sg_delivery_path"));
//         returnFields.push_back(toXmlrpcValue("sg_delivery_staged_path"));
//         returnFields.push_back(toXmlrpcValue("sg_delivery_status"));
//         returnFields.push_back(toXmlrpcValue("sg_delivery_type"));
//         returnFields.push_back(toXmlrpcValue("sg_wrangler"));
//         returnFields.push_back(toXmlrpcValue("sg_wrangler_notes"));
//     }
//     else if (entityType == "CustomEntity01") // "DeliveryItem"
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_asset"));
//         returnFields.push_back(toXmlrpcValue("sg_delivery"));
//         returnFields.push_back(toXmlrpcValue("sg_staging_status"));
//         returnFields.push_back(toXmlrpcValue("sg_staging_qt"));
//         returnFields.push_back(toXmlrpcValue("sg_processing_status"));
//         returnFields.push_back(toXmlrpcValue("sg_quality_control_status"));
//         returnFields.push_back(toXmlrpcValue("sg_data_size"));
//         returnFields.push_back(toXmlrpcValue("sg_files"));
//         returnFields.push_back(toXmlrpcValue("sg_notes"));
//         returnFields.push_back(toXmlrpcValue("sg_priority"));
//         returnFields.push_back(toXmlrpcValue("sg_processing_type"));
//         returnFields.push_back(toXmlrpcValue("sg_publish_type"));
//         returnFields.push_back(toXmlrpcValue("sg_sequence"));
//         returnFields.push_back(toXmlrpcValue("sg_shot"));
//         returnFields.push_back(toXmlrpcValue("sg_tippett_name"));
//         returnFields.push_back(toXmlrpcValue("sg_tippett_path"));
//         returnFields.push_back(toXmlrpcValue("sg_tippett_start_frame"));
//         returnFields.push_back(toXmlrpcValue("sg_wrangler_notes"));
//     }
//     else if (entityType == "PublishEvent")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_file"));
//         //returnFields.push_back(toXmlrpcValue("sg_format"));
//         returnFields.push_back(toXmlrpcValue("sg_preview_hd_qt"));
//         returnFields.push_back(toXmlrpcValue("sg_preview_qt"));
//         returnFields.push_back(toXmlrpcValue("sg_rev"));
//         returnFields.push_back(toXmlrpcValue("sg_resolution"));
//         returnFields.push_back(toXmlrpcValue("sg_type"));
//     }
//     else if (entityType == "Review")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_review_type"));
//         returnFields.push_back(toXmlrpcValue("sg_review_media"));
//         returnFields.push_back(toXmlrpcValue("sg_review_date_sent"));
//         returnFields.push_back(toXmlrpcValue("sg_review_sent_to"));
//         returnFields.push_back(toXmlrpcValue("sg_review_date_reviewed"));
//         returnFields.push_back(toXmlrpcValue("sg_review_reviewed_by"));
//         returnFields.push_back(toXmlrpcValue("sg_review_disclaimers"));
//         returnFields.push_back(toXmlrpcValue("sg_review_tipsupe_notes"));
//         returnFields.push_back(toXmlrpcValue("sg_review_client_notes"));
//     }
//     else if (entityType == "ReviewItem")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_version"));
//         returnFields.push_back(toXmlrpcValue("sg_link"));
//         returnFields.push_back(toXmlrpcValue("sg_review"));
//         returnFields.push_back(toXmlrpcValue("sg_purpose"));
//         returnFields.push_back(toXmlrpcValue("sg_order"));
//         returnFields.push_back(toXmlrpcValue("sg_reviewed_by"));
//         returnFields.push_back(toXmlrpcValue("sg_date_reviewed"));
//         returnFields.push_back(toXmlrpcValue("sg_approved_"));
//     }
//     else if (entityType == "Task")
//     {
//         returnFields.push_back(toXmlrpcValue("content"));
//         returnFields.push_back(toXmlrpcValue("task_assignees"));
//         returnFields.push_back(toXmlrpcValue("color"));
//         returnFields.push_back(toXmlrpcValue("due_date"));
//         returnFields.push_back(toXmlrpcValue("duration"));
//         returnFields.push_back(toXmlrpcValue("entity"));
//         returnFields.push_back(toXmlrpcValue("milestone"));
//         returnFields.push_back(toXmlrpcValue("start_date"));
//         returnFields.push_back(toXmlrpcValue("sg_status_list"));
//         returnFields.push_back(toXmlrpcValue("sg_system_task_type"));
//         returnFields.push_back(toXmlrpcValue("sg_view_order"));
//     }
//     else if (entityType == "Group")
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//     }
//     else if (entityType == "Note")
//     {
//         returnFields.push_back(toXmlrpcValue("user"));
//         returnFields.push_back(toXmlrpcValue("content"));
//         returnFields.push_back(toXmlrpcValue("addressings_cc"));
//         returnFields.push_back(toXmlrpcValue("addressings_to"));
//         returnFields.push_back(toXmlrpcValue("sg_status_list"));
//         returnFields.push_back(toXmlrpcValue("subject"));
//         returnFields.push_back(toXmlrpcValue("sg_note_type"));
//         returnFields.push_back(toXmlrpcValue("note_links"));
//     }
//     else if (entityType == "Playlist") 
//     {
//         returnFields.push_back(toXmlrpcValue("code"));
//         returnFields.push_back(toXmlrpcValue("sg_date_and_time"));
//         returnFields.push_back(toXmlrpcValue("description"));
//         returnFields.push_back(toXmlrpcValue("notes"));
//         returnFields.push_back(toXmlrpcValue("tag_list"));
//         returnFields.push_back(toXmlrpcValue("image"));
//         returnFields.push_back(toXmlrpcValue("versions"));
//     }
// 
//     createMap["return_fields"] = toXmlrpcValue(returnFields);

    return createMap;
}

// *****************************************************************************
SgMap Entity::buildUpdateMap(const std::string &entityType,
                             const int entityId,
                             const SgArray &fieldsToUpdate)
{
    SgMap updateMap;

    updateMap["type"] = toXmlrpcValue(entityType);
    updateMap["id"] = toXmlrpcValue(entityId);
    updateMap["fields"] = toXmlrpcValue(fieldsToUpdate);

    return updateMap;
}

// *****************************************************************************
SgMap Entity::buildFindMap(const std::string &entityType,
                           const FilterBy &filterList,
                           const SgArray &filterReturnFields,
                           const bool retiredOnly,
                           const int limit,
                           const SgArray &order)
{
    SgMap findMap;

    // -------------------------------------------------------------------
    // "type"
    findMap["type"] = toXmlrpcValue(entityType);

    // -------------------------------------------------------------------
    // "filters"
    if (!filterList.empty())
    {
        findMap["filters"] = toXmlrpcValue(filterList.filters());
    }
    else
    {
        // The "logic_operator" is required, so give some default value       
        SgMap filters;
        filters["logical_operator"] = toXmlrpcValue("and");
        filters["conditions"] = toXmlrpcValue(SgArray());

        findMap["filters"] = toXmlrpcValue(filters);
    }

    // -------------------------------------------------------------------
    // "return_only"
    std::string returnOnly = retiredOnly ? "retired" : "active";
    findMap["return_only"] = toXmlrpcValue(returnOnly);

    // -------------------------------------------------------------------
    // "paging"
    //
    // "entities_per_page" is used to split up requests into batches 
    // when doing requests. This helps speed tremendously when getting 
    // lots of results back.  It doesn't affect the interface of the api 
    // at all (you always get the full set of results back as one array) 
    // but just how the client class communicates with the server.
    //
    // NOTE: It looks like there is a limit on how much data can be 
    // sent, i.e. 524288 characters. Otherwise you'll get "girerr::error"
    // such as:
    //
    //     Response XML from server is not valid XML-RPC response.  Unable 
    //     to find XML-RPC response in what server sent back.  XML-RPC response 
    //     too large.  Our limit is 524288 characters.  We got 758343 characters
    //
    // Therefore, we need to be careful about what number to put in
    // for "entities_per_page". Here I say 200 for Daily entities,
    // but to be safe use 100 for potential large entity data set.
    int maxEntitiesPerPage = 100;

    SgMap paging;
    paging["current_page"] = toXmlrpcValue(1);
    if (limit && limit > 0 && limit < maxEntitiesPerPage) 
    {
        paging["entities_per_page"] = toXmlrpcValue(limit);
    }
    else
    {
        paging["entities_per_page"] = toXmlrpcValue(maxEntitiesPerPage);
    }
    findMap["paging"] = toXmlrpcValue(paging);

     // -------------------------------------------------------------------
     // "return_fields" - will be populated in Shotgun class' entityFactoryFind(..)
     

    // -------------------------------------------------------------------
    // "sorts" - This part makes sure that the given order has valid 
    // list of ("field_name", "direction") pair
    if (order.size() > 0)
    {
        SgArray sorts;

        for (size_t i = 0; i < order.size(); i++)
        {
            SgMap orderMap = SgMap(xmlrpc_c::value_struct(order[i]));

            std::string fieldName;
            std::string fieldDir;

            //---------------
            // "field_name"
            fieldName = getAttrValueAsString("field_name", orderMap, "", INVALID_ATTR_USE_DEFAULT);

            //---------------
            // "direction"
            fieldDir = getAttrValueAsString("direction", orderMap, "", INVALID_ATTR_USE_DEFAULT);

            if (fieldName != "" && fieldDir != "")
            {
                SgMap orderMap2;
                orderMap2["field_name"] = toXmlrpcValue(fieldName);
                orderMap2["direction"] = toXmlrpcValue(fieldDir);

                sorts.push_back(toXmlrpcValue(orderMap2));
            }
        }

        if (sorts.size() > 0)
        {
            findMap["sorts"] = toXmlrpcValue(sorts);
        }
    }

    // -------------------------------------------------------------------
    // "limit" - This is a field that is not recognized by Shotgun. But we
    // add it to findMap so that it can be passed to findEntities(..) where
    // we need to remove it from the findMap.
    findMap["limit"] = toXmlrpcValue(limit);

    return findMap;
}

// *****************************************************************************
const xmlrpc_c::value Entity::getAttrValue(const std::string &attrName) const
{
    SgMap attrMap;

    // First check to see if the attrName exists in m_attrs, which contains a
    // list of default return fields for this entity type. 
    if (m_attrs->type() != xmlrpc_c::value::TYPE_NIL)
    {
        attrMap = SgMap(xmlrpc_c::value_struct(*m_attrs));

        SgMap::const_iterator foundIter = attrMap.find(attrName);

        if (foundIter != attrMap.end())
        {
            return (*foundIter).second;
        }
    }
    
    // If attrName is not in m_attrs, do a fresh search in Shotgun using the
    // entity's id because id is a required return field of all entity types
    // and it has already been retrieved at this point.
    // ------------------------------------------------------------------
    // "return fields" - add attrName to the list of return fields
    SgArray returnFields;
    returnFields.push_back(toXmlrpcValue(attrName));

    Entity *entity = m_sg->findEntity(m_type,
                                      FilterBy("id", "is", sgId()),
                                      returnFields);

    attrMap = SgMap(xmlrpc_c::value_struct(entity->attrs()));
    delete entity;

    SgMap::const_iterator foundIter = attrMap.find(attrName);
    if (foundIter != attrMap.end())
    {
        return (*foundIter).second;
    }
    else
    {
        throw SgAttrNotFoundError(attrName);
    }
}

// *****************************************************************************
// static
const xmlrpc_c::value Entity::getAttrValue(const std::string &attrName, 
                                           const SgMap &attrsMap)
{
    SgMap map;
    if (!attrsMap.empty())
    {
        map = attrsMap;
    }
    else
    {
        throw SgEmptyAttrMapError();
    }

    SgMap::const_iterator foundIter = map.find(attrName);

    if (foundIter != map.end())
    {
        return (*foundIter).second;
    }
    else
    {
        throw SgAttrNotFoundError(attrName);
    }
}

// *****************************************************************************
void Entity::setAttrValue(const std::string &attrName, 
                          const xmlrpc_c::value &attrValue,
                          const SgArray &multiValues)
{
    // This involves two steps:
    // (1) Update the Shotgun records
    // (2) Update the attribute that is already in m_attrs

    try
    {
        // -------------------------------------------------------------------------
        // Update the Shotgun records
        SgArray fields;

        SgMap field;
        field["field_name"] = toXmlrpcValue(attrName); 
        field["value"] = toXmlrpcValue(attrValue); 
        fields.push_back(toXmlrpcValue(field));

        // TODO: check if this is the correct way of doing it.
        for (size_t i = 0; i < multiValues.size(); i++)
        {
            field.clear();
            field["field_name"] = toXmlrpcValue(attrName);
            field["value"] = toXmlrpcValue(multiValues[i]);
            field["multi_entity_update_mode"] = toXmlrpcValue("add");
            fields.push_back(toXmlrpcValue(field));
        }
    
        // TODO: currently it omits the optional "parent_entity" field.
        // Don't know if it's needed ever.

        xmlrpc_c::value result = updateSGEntity(m_sg,
                                                m_type,
                                                sgId(),
                                                fields);

        // -------------------------------------------------------------------------
        // Update the attrbute if it is already in m_attrs. If it's not already
        // there, don't add it.
        if (m_attrs->type() != xmlrpc_c::value::TYPE_NIL)
        {
            SgMap attrMap = SgMap(xmlrpc_c::value_struct(*m_attrs));

            SgMap::iterator foundIter = attrMap.find(attrName);

            if (foundIter != attrMap.end())
            {
                // xmlrpc_c::value does not allow re-assignment except for its
                // pointer type. So erase this entry first, then add a new one.
                attrMap.erase(foundIter);
                attrMap[attrName] = toXmlrpcValue(attrValue);

                // Update the m_attrs as a whole
                delete m_attrs;
                m_attrs = new xmlrpc_c::value(xmlrpc_c::value_struct(attrMap));
            }
        }
    }
    catch (SgEntityXmlrpcError &error)
    {
        throw SgAttrSetValueError(attrName, error.what());
    }
}

// *****************************************************************************
void Entity::setAttrValue(const SgMap &attrPairs)
{
    // This involves two steps:
    // (1) Update the Shotgun records
    // (2) Update the attribute that is already in m_attrs

    try
    {
        // -------------------------------------------------------------------------
        // Update the Shotgun records
        SgArray fields;

        for (SgMap::const_iterator attrIter = attrPairs.begin(); attrIter != attrPairs.end(); ++attrIter)
        {
            SgMap field;
            field["field_name"] = toXmlrpcValue((*attrIter).first); 
            field["value"] = toXmlrpcValue((*attrIter).second); 
            fields.push_back(toXmlrpcValue(field));
        }

        // TODO: ignored multi_values and "parent_entity" field for now.

        // Update all the field in one call
        xmlrpc_c::value result = updateSGEntity(m_sg,
                                                m_type,
                                                sgId(),
                                                fields);

        // -------------------------------------------------------------------------
        // Update the attrbutes if they are already in m_attrs. If not there, don't 
        // add them to m_attrs.
        if (m_attrs->type() != xmlrpc_c::value::TYPE_NIL)
        {
            SgMap attrMap = SgMap(xmlrpc_c::value_struct(*m_attrs));
            bool updated = false;
        
            for (SgMap::const_iterator attrIter = attrPairs.begin(); attrIter != attrPairs.end(); ++attrIter)
            {
                SgMap::iterator foundIter = attrMap.find((*attrIter).first);

                if (foundIter != attrMap.end())
                {
                    // xmlrpc_c::value does not allow re-assignment except for its
                    // pointer type. So erase this entry first, then add a new one.
                    attrMap.erase(foundIter);
                    attrMap[(*attrIter).first] = toXmlrpcValue((*attrIter).second);

                    updated = true;
                }
            }

            // Update the m_attrs as a whole
            if (updated)
            {
                delete m_attrs;
                m_attrs = new xmlrpc_c::value(xmlrpc_c::value_struct(attrMap));
            }
        }
    }
    catch (SgEntityXmlrpcError &error)
    {
        throw SgAttrSetValueError(attrPairs, error.what());
    }
}

// *****************************************************************************
const int Entity::getAttrValueAsInt(const std::string &attrName, 
                                    const int defaultVal) const
{
    int result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_INT)
    {
        result = int(xmlrpc_c::value_int(genericResult));
    }
    else if (m_invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_INT, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
// static
const int Entity::getAttrValueAsInt(const std::string &attrName, 
                                    const SgMap &attrsMap,
                                    const int defaultVal,
                                    const InvalidAttrMode invalidAttrMode)
{
    int result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_INT)
    {
        result = int(xmlrpc_c::value_int(genericResult));
    }
    else if (invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_INT, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
const bool Entity::getAttrValueAsBool(const std::string &attrName, 
                                      const bool defaultVal) const
{
    bool result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_BOOLEAN)
    {
        result = bool(xmlrpc_c::value_boolean(genericResult));
    }
    else if (m_invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_BOOLEAN, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
// static
const bool Entity::getAttrValueAsBool(const std::string &attrName, 
                                      const SgMap &attrsMap,
                                      const bool defaultVal, 
                                      const InvalidAttrMode invalidAttrMode)
{
    bool result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_BOOLEAN)
    {
        result = bool(xmlrpc_c::value_boolean(genericResult));
    }
    else if (invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_BOOLEAN, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
const double Entity::getAttrValueAsDouble(const std::string &attrName, 
                                          const double defaultVal) const
{
    double result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_DOUBLE)
    {
        result = double(xmlrpc_c::value_double(genericResult));
    }
    else if (m_invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_DOUBLE, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
// static
const double Entity::getAttrValueAsDouble(const std::string &attrName, 
                                          const SgMap &attrsMap,
                                          const double defaultVal, 
                                          const InvalidAttrMode invalidAttrMode)
{
    double result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_DOUBLE)
    {
        result = double(xmlrpc_c::value_double(genericResult));
    }
    else if (invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_DOUBLE, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
const time_t Entity::getAttrValueAsDatetime(const std::string &attrName, 
                                           const time_t defaultVal) const
{
    time_t result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_DATETIME)
    {
        result = time_t(xmlrpc_c::value_datetime(genericResult));
    }
    else if (m_invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_DATETIME, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
// static
const time_t Entity::getAttrValueAsDatetime(const std::string &attrName, 
                                            const SgMap &attrsMap,
                                            const time_t defaultVal, 
                                            const InvalidAttrMode invalidAttrMode)                                          
{
    time_t result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_DATETIME)
    {
        result = time_t(xmlrpc_c::value_datetime(genericResult));
    }
    else if (invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_DATETIME, 
                                  genericResult.type());
        }
        else
        {
            throw SgAttrValueError(attrName);
        }
    }

    return result;
}

// *****************************************************************************
const std::string Entity::getAttrValueAsString(const std::string &attrName, 
                                               const std::string &defaultVal) const
{
    std::string result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_STRING)
    {
        result = std::string(xmlrpc_c::value_string(genericResult));
    }
    else if (m_invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_STRING, 
                                  genericResult.type());
        }
        else
        {
            //throw SgAttrValueError(attrName);

            // String type can have a default value (i.e. an empty string)
            // to represent an empty value instead of throwing an exception.
            result = "";
        }
    }

    return result;
}

// *****************************************************************************
// static
const std::string Entity::getAttrValueAsString(const std::string &attrName, 
                                               const SgMap &attrsMap,
                                               const std::string &defaultVal, 
                                               const InvalidAttrMode invalidAttrMode)
{
    std::string result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_STRING)
    {
        result = std::string(xmlrpc_c::value_string(genericResult));
    }
    else if (invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_STRING, 
                                  genericResult.type());
        }
        else
        {
            //throw SgAttrValueError(attrName);

            result = "";
        }
    }

    return result;
}

// *****************************************************************************
const SgArray Entity::getAttrValueAsArray(const std::string &attrName, 
                                          const SgArray &defaultVal) const
{
    SgArray result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_ARRAY)
    {
        result = (xmlrpc_c::value_array(genericResult)).vectorValueValue();
    }
    else if (m_invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_ARRAY, 
                                  genericResult.type());
        }
        else
        {
            //throw SgAttrValueError(attrName);

            // SgArray type can have a default value (i.e. an empty array)
            // to represent an empty value instead of throwing an exception.
            result = SgArray();
        }
    }

    return result;
}

// *****************************************************************************
// static
const SgArray Entity::getAttrValueAsArray(const std::string &attrName, 
                                          const SgMap &attrsMap,
                                          const SgArray &defaultVal, 
                                          const InvalidAttrMode invalidAttrMode)
{
    SgArray result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_ARRAY)
    {
        result = (xmlrpc_c::value_array(genericResult)).vectorValueValue();
    }
    else if (invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_ARRAY, 
                                  genericResult.type());
        }
        else
        {
            //throw SgAttrValueError(attrName);

            result = SgArray();
        }
    }

    return result;
}

// *****************************************************************************
const SgMap Entity::getAttrValueAsMap(const std::string &attrName) const
{
    //SgMap result = defaultVal;
    SgMap result;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_STRUCT)
    {
        result = SgMap(xmlrpc_c::value_struct(genericResult));
    }
    else if (m_invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_STRUCT, 
                                  genericResult.type());
        }
        else
        {
            //throw SgAttrValueError(attrName);

            // SgMap type can have a default value (i.e. an empty map)
            // to represent an empty value instead of throwing an exception.
            result = SgMap();
        }
    }

    return result;
}

// *****************************************************************************
// static
const SgMap Entity::getAttrValueAsMap(const std::string &attrName, 
                                      const SgMap &attrsMap,
                                      const SgMap &defaultVal, 
                                      const InvalidAttrMode invalidAttrMode)
{
    SgMap result = defaultVal;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_STRUCT)
    {
        result = SgMap(xmlrpc_c::value_struct(genericResult));
    }
    else if (invalidAttrMode == INVALID_ATTR_THROW_EXCEPTION)
    {
        if (genericResult.type() != xmlrpc_c::value::TYPE_NIL)
        {
            throw SgAttrTypeError(attrName, 
                                  xmlrpc_c::value::TYPE_STRUCT, 
                                  genericResult.type());
        }
        else
        {
            //throw SgAttrValueError(attrName);
            result = SgMap();
        }
    }

    return result;
}

// *****************************************************************************
const Strings Entity::getAttrValueAsTags(const std::string &attrName) const 
{
    Strings tags;

    xmlrpc_c::value genericResult = getAttrValue(attrName);

    if (genericResult.type() == xmlrpc_c::value::TYPE_ARRAY)
    {
        SgArray array = (xmlrpc_c::value_array(genericResult)).vectorValueValue();
    
        for (size_t i = 0; i < array.size(); i++)
        {
            if (array[i].type() == xmlrpc_c::value::TYPE_STRUCT)
            {
                SgMap map = SgMap(xmlrpc_c::value_struct(array[i]));

                SgMap::const_iterator foundIter = map.find("name");

                if (foundIter != map.end())
                {
                    tags.push_back(std::string(xmlrpc_c::value_string((*foundIter).second)));
                }
            }
            else if (array[i].type() == xmlrpc_c::value::TYPE_STRING)
            {
                tags.push_back(std::string(xmlrpc_c::value_string(array[i])));
            }
        }
    }

    return tags;
}

// *****************************************************************************
// static
const Strings Entity::getAttrValueAsTags(const std::string &attrName,
                                         const SgMap &attrsMap)
{
    Strings tags;

    xmlrpc_c::value genericResult = getAttrValue(attrName, attrsMap);

    if (genericResult.type() == xmlrpc_c::value::TYPE_ARRAY)
    {
        SgArray array = (xmlrpc_c::value_array(genericResult)).vectorValueValue();
    
        for (size_t i = 0; i < array.size(); i++)
        {
            if (array[i].type() == xmlrpc_c::value::TYPE_STRUCT)
            {
                SgMap map = SgMap(xmlrpc_c::value_struct(array[i]));

                SgMap::const_iterator foundIter = map.find("name");

                if (foundIter != map.end())
                {
                    tags.push_back(std::string(xmlrpc_c::value_string((*foundIter).second)));
                }
            }
            else if (array[i].type() == xmlrpc_c::value::TYPE_STRING)
            {
                tags.push_back(std::string(xmlrpc_c::value_string(array[i])));
            }
        }
    }

    return tags;
}

// *****************************************************************************
const Entity *Entity::getAttrValueAsEntityPtr(const std::string &attrName) const
{
//    xmlrpc_c::value entity = getAttrValueAsEntityAttrMap(attrName);
//    return entityAttrMapToEntityPtr(m_sg, entity);

    SgMap entity = getAttrValueAsMap(attrName);

    if (!entity.empty())
    {
        int id = getAttrValueAsInt("id", entity);
        std::string type = getAttrValueAsString("type", entity);

        return m_sg->findEntity(type,
                                FilterBy("id", "is", id));
    }
    else
    {
        throw SgEntityNotFoundError("\"" + attrName + "\"");
    }
}

// *****************************************************************************
// static
const Entity *Entity::getAttrValueAsEntityPtr(Shotgun *sg, 
                                              const std::string &attrName,
                                              const SgMap &attrsMap)
{
//     xmlrpc_c::value entity = getAttrValueAsEntityAttrMap(sg, attrName, attrsMap);
//     return entityAttrMapToEntityPtr(sg, entity);

    SgMap entity = getAttrValueAsMap(attrName, attrsMap);

    if (!entity.empty())
    {
        int id = getAttrValueAsInt("id", entity);
        std::string type = getAttrValueAsString("type", entity);

        return sg->findEntity(type,
                              FilterBy("id", "is", id));
    }
    else
    {
        throw SgEntityNotFoundError("\"" + attrName + "\"");
    }
}

// *****************************************************************************
const EntityPtrs Entity::getAttrValueAsMultiEntityPtr(const std::string &attrName) const
{
//     EntityPtrs entityPtrs;
// 
//     SgArray entities = getAttrValueAsMultiEntityAttrMap(attrName);
// 
//     for (size_t i = 0; i < entities.size(); i++)
//     {
//         entityPtrs.push_back(entityAttrMapToEntityPtr(m_sg, entities[i]));
//     }
// 
//     return entityPtrs;

    EntityPtrs entities;

    SgArray entityList = getAttrValueAsArray(attrName);
    for (size_t i = 0; i < entityList.size(); i++)
    {
        SgMap entity = SgMap(xmlrpc_c::value_struct(entityList[i]));

        if (!entity.empty())
        {
            int id = getAttrValueAsInt("id", entity);
            std::string type = getAttrValueAsString("type", entity);

            entities.push_back(m_sg->findEntity(type,
                                                FilterBy("id", "is", id)));
        }
    }

    return entities;
}

// *****************************************************************************
// static
const EntityPtrs Entity::getAttrValueAsMultiEntityPtr(Shotgun *sg, 
                                                      const std::string &attrName,
                                                      const SgMap &attrsMap)
{
//     EntityPtrs entityPtrs;
// 
//     SgArray entities = getAttrValueAsMultiEntityAttrMap(sg, attrName, attrsMap);
// 
//     for (size_t i = 0; i < entities.size(); i++)
//     {
//         entityPtrs.push_back(entityAttrMapToEntityPtr(sg, entities[i]));
//     }
// 
//     return entityPtrs;

    EntityPtrs entities;

    SgArray entityList = getAttrValueAsArray(attrName, attrsMap);
    for (size_t i = 0; i < entityList.size(); i++)
    {
        SgMap entity = SgMap(xmlrpc_c::value_struct(entityList[i]));

        if (!entity.empty())
        {
            int id = getAttrValueAsInt("id", entity);
            std::string type = getAttrValueAsString("type", entity);

            entities.push_back(sg->findEntity(type,
                                              FilterBy("id", "is", id)));
        }
    }

    return entities;
}

// *****************************************************************************
const std::string Entity::getAttrValueAsUserLogin(const std::string &attrName) const 
{
//    HumanUser user(m_sg, getAttrValueAsEntityAttrMap(attrName));
//    return user.sgLogin();

    const Entity *entity = getAttrValueAsEntityPtr(attrName);
    if (const HumanUser *user = dynamic_cast<const HumanUser *>(entity))
    {
        std::string sgLogin = user->sgLogin();
        delete entity;

        return sgLogin;
    }
    else
    {
        delete entity;
        throw SgEntityDynamicCastError("HumanUser");
    }
}

// *****************************************************************************
// static
const std::string Entity::getAttrValueAsUserLogin(Shotgun *sg,
                                                  const std::string &attrName,
                                                  const SgMap &attrsMap) 
{
//     HumanUser user(sg, getAttrValueAsEntityAttrMap(sg, attrName, attrsMap));
//     return user.sgLogin();

    const Entity *entity = getAttrValueAsEntityPtr(sg, attrName, attrsMap);
    if (const HumanUser *user = dynamic_cast<const HumanUser *>(entity))
    {
        std::string sgLogin = user->sgLogin();
        delete entity;

        return sgLogin;
    }
    else
    {
        delete entity;
        throw SgEntityDynamicCastError("HumanUser");
    }
}


// *****************************************************************************
void Entity::printAttrs() const
{
    std::cout << *m_attrs << std::endl;
}

} // End namespace Shotgun
