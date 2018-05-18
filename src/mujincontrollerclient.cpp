// -*- coding: utf-8 -*-
// Copyright (C) 2012-2013 MUJIN Inc. <rosen.diankov@mujin.co.jp>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "common.h"
#include "controllerclientimpl.h"
#include <boost/thread.hpp> // for sleep
#include <boost/algorithm/string.hpp>

#include <mujincontrollerclient/binpickingtask.h>

#ifdef MUJIN_USEZMQ
#include "binpickingtaskzmq.h"
#endif

#include "logging.h"
#include "mujincontrollerclient/mujinjson.h"

MUJIN_LOGGER("mujin.controllerclientcpp");

namespace mujinclient {
using namespace mujinjson;

void ExtractEnvironmentStateFromPTree(const rapidjson::Value& envstatejson, EnvironmentState& envstate)
{
    // FIXME: is this a dict or array?
    envstate.clear();
    for (rapidjson::Document::ConstValueIterator it = envstatejson.Begin(); it != envstatejson.End(); ++it) {
        InstanceObjectState objstate;
        std::string name = GetJsonValueByKey<std::string>(*it, "name");
        std::vector<Real> quat = GetJsonValueByKey<std::vector<Real> >(*it, "quat_");
        BOOST_ASSERT(quat.size() == 4);
        Real dist2 = 0;
        for (int i = 0; i < 4; i ++ ) {
            Real f = quat[i] * quat[i];
            dist2 += f;
            objstate.transform.quaternion[i] = f;
        }
        // normalize the quaternion
        if( dist2 > 0 ) {
            Real fnorm =1/std::sqrt(dist2);
            objstate.transform.quaternion[0] *= fnorm;
            objstate.transform.quaternion[1] *= fnorm;
            objstate.transform.quaternion[2] *= fnorm;
            objstate.transform.quaternion[3] *= fnorm;
        }
        LoadJsonValueByKey(*it, "translation_", objstate.transform.translate);
        LoadJsonValueByKey(*it, "dofvalues", objstate.dofvalues);
        envstate[name] = objstate;
    }
}

void SerializeEnvironmentStateToJSON(const EnvironmentState& envstate, std::ostream& os)
{
    os << "[";
    bool bhaswritten = false;
    FOREACHC(itstate, envstate) {
        if( itstate->first.size() > 0 ) {
            if( bhaswritten ) {
                os << ",";
            }
            os << "{ \"name\":\"" << itstate->first << "\", \"translation_\":[";
            for(int i = 0; i < 3; ++i) {
                if( i > 0 ) {
                    os << ",";
                }
                os << itstate->second.transform.translate[i];
            }
            os << "], \"quat_\":[";
            for(int i = 0; i < 4; ++i) {
                if( i > 0 ) {
                    os << ",";
                }
                os << itstate->second.transform.quaternion[i];
            }
            os << "], \"dofvalues\":[";
            for(size_t i = 0; i < itstate->second.dofvalues.size(); ++i) {
                if( i > 0 ) {
                    os << ",";
                }
                os << itstate->second.dofvalues.at(i);
            }
            os << "] }";
            bhaswritten = true;
        }
    }
    os << "]";
}

WebResource::WebResource(ControllerClientPtr controller, const std::string& resourcename, const std::string& pk) : __controller(controller), __resourcename(resourcename), __pk(pk)
{
    BOOST_ASSERT(__pk.size()>0);
}

void WebResource::GetWrap(rapidjson::Document& pt, const std::string& field, double timeout)
{
    GETCONTROLLERIMPL();
    controller->CallGet(str(boost::format("%s/%s/?format=json&fields=%s")%GetResourceName()%GetPrimaryKey()%field), pt, 200, timeout);
}

void WebResource::Set(const std::string& field, const std::string& newvalue, double timeout)
{
    throw MujinException("not implemented");
}

void WebResource::SetJSON(const std::string& json, double timeout)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallPutJSON(str(boost::format("%s/%s/?format=json")%GetResourceName()%GetPrimaryKey()), json, pt, 202, timeout);
}

void WebResource::Delete(double timeout)
{
    GETCONTROLLERIMPL();
    controller->CallDelete(str(boost::format("%s/%s/")%GetResourceName()%GetPrimaryKey()), timeout);
}

void WebResource::Copy(const std::string& newname, int options, double timeout)
{
    throw MujinException("not implemented yet");
}

ObjectResource::ObjectResource(ControllerClientPtr controller, const std::string& pk) : WebResource(controller, "object", pk)
{
}

ObjectResource::ObjectResource(ControllerClientPtr controller, const std::string& resource, const std::string& pk) : WebResource(controller, resource, pk)
{
}

ObjectResource::LinkResource::LinkResource(ControllerClientPtr controller, const std::string& objectpk, const std::string& pk) : WebResource(controller, str(boost::format("object/%s/link")%objectpk), pk), objectpk(objectpk)
{
}

ObjectResource::GeometryResource::GeometryResource(ControllerClientPtr controller, const std::string& objectpk, const std::string& pk) : WebResource(controller, str(boost::format("object/%s/geometry")%objectpk), pk), objectpk(objectpk)
{
}

ObjectResource::IkParamResource::IkParamResource(ControllerClientPtr controller, const std::string& objectpk, const std::string& pk) : WebResource(controller, str(boost::format("object/%s/ikparam")%objectpk), pk)
{
}

void ObjectResource::GeometryResource::GetMesh(std::string& primitive, std::vector<std::vector<Real> >& indices, std::vector<std::vector<Real> >& vertices)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    const std::string relativeuri(str(boost::format("%s/%s/?format=json&mesh=true")%GetResourceName()%GetPrimaryKey()));
    controller->CallGet(relativeuri, pt);
    rapidjson::Value& objects = pt["mesh"];
    LoadJsonValueByKey(objects,"primitive",primitive);
    LoadJsonValueByKey(objects,"indices",indices);
    LoadJsonValueByKey(objects,"vertices",vertices);
}

void ObjectResource::GeometryResource::SetGeometryFromRawSTL(const std::vector<unsigned char>& rawstldata, const std::string& unit, double timeout)
{
    GETCONTROLLERIMPL();
    // todo fail if this->geomtype!="mesh"
    controller->SetObjectGeometryMesh(this->objectpk, this->pk, rawstldata, unit, timeout);
}

ObjectResource::GeometryResourcePtr ObjectResource::LinkResource::AddGeometryFromRawSTL(const std::vector<unsigned char>& rawstldata, const std::string& name, const std::string& unit, double timeout)
{
    GETCONTROLLERIMPL();
    const std::string& linkpk = GetPrimaryKey();
    const std::string geometryPk = controller->CreateObjectGeometry(this->objectpk, name, linkpk, timeout);

    ObjectResource::GeometryResourcePtr geometry(new GeometryResource(controller, this->objectpk, geometryPk));
    geometry->SetGeometryFromRawSTL(rawstldata, unit, timeout);
    return geometry;
}

ObjectResource::GeometryResourcePtr ObjectResource::LinkResource::GetGeometryFromName(const std::string& geometryName)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    const std::string relativeuri(str(boost::format("object/%s/geometry/?format=json&limit=0&fields=geometries")%this->objectpk));
    controller->CallGet(relativeuri, pt);
    if (pt.IsObject() && pt.HasMember("geometries") && pt["geometries"].IsArray()) {
        rapidjson::Value& objects = pt["geometries"];
        for (rapidjson::Document::ConstValueIterator it = objects.Begin(); it != objects.End(); ++it) {
            const std::string name = it->HasMember("name") ? (*it)["name"].GetString() : (*it)["pk"].GetString();
            if (name == geometryName && (*it)["linkpk"].GetString() == this->pk) {
                ObjectResource::GeometryResourcePtr geometry(new GeometryResource(controller, this->objectpk, GetJsonValueByKey<std::string>(*it, "pk")));
                geometry->name = name;
                LoadJsonValueByKey(*it,"pk",geometry->pk);
                LoadJsonValueByKey(*it,"linkpk",geometry->linkpk);
                LoadJsonValueByKey(*it,"visible",geometry->visible);
                LoadJsonValueByKey(*it,"geomtype",geometry->geomtype);
                LoadJsonValueByKey(*it,"transparency",geometry->transparency);
                LoadJsonValueByKey(*it,"quaternion",geometry->quaternion);
                LoadJsonValueByKey(*it,"translate",geometry->translate);
                LoadJsonValueByKey(*it,"diffusecolor",geometry->diffusecolor);
                return geometry;
            }
        }
    }
    throw MUJIN_EXCEPTION_FORMAT("link %s does not have geometry named %s", this->name%geometryName, MEC_InvalidArguments);
}

void ObjectResource::LinkResource::GetGeometries(std::vector<ObjectResource::GeometryResourcePtr>& geometries)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    const std::string relativeuri(str(boost::format("object/%s/geometry/?format=json&limit=0&fields=geometries")%this->objectpk));
    controller->CallGet(relativeuri, pt);
    if (pt.IsObject() && pt.HasMember("geometries") && pt["geometries"].IsArray()) {
        rapidjson::Value& objects = pt["geometries"];
        geometries.clear();
        for (rapidjson::Document::ConstValueIterator it = objects.Begin(); it != objects.End(); ++it) {
            if ((*it)["linkpk"].GetString() == this->pk) {
                ObjectResource::GeometryResourcePtr geometry(new GeometryResource(controller, this->objectpk, GetJsonValueByKey<std::string>(*it, "pk")));
                geometry->name = it->HasMember("name") ? (*it)["name"].GetString() : (*it)["pk"].GetString();
                LoadJsonValueByKey(*it,"pk",geometry->pk);
                LoadJsonValueByKey(*it,"linkpk",geometry->linkpk);
                LoadJsonValueByKey(*it,"visible",geometry->visible);
                LoadJsonValueByKey(*it,"geomtype",geometry->geomtype);
                LoadJsonValueByKey(*it,"transparency",geometry->transparency);
                LoadJsonValueByKey(*it,"quaternion",geometry->quaternion);
                LoadJsonValueByKey(*it,"translate",geometry->translate);
                LoadJsonValueByKey(*it,"diffusecolor",geometry->diffusecolor);
                geometries.push_back(geometry);
            }
        }
    }
}

void ObjectResource::GetLinks(std::vector<ObjectResource::LinkResourcePtr>& links)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("object/%s/link/?format=json&limit=0&fields=links")%GetPrimaryKey()), pt);
    rapidjson::Value& objects = pt["links"];
    links.resize(objects.Size());
    size_t i = 0;
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        LinkResourcePtr link(new LinkResource(controller, GetPrimaryKey(), GetJsonValueByKey<std::string>(*it, "pk")));
        LoadJsonValueByKey(*it,"name",link->name);
        LoadJsonValueByKey(*it,"pk",link->pk);
        LoadJsonValueByKey(*it,"collision",link->collision);
        LoadJsonValueByKey(*it,"attachmentpks",link->attachmentpks);
        LoadJsonValueByKey(*it,"quaternion",link->quaternion);
        LoadJsonValueByKey(*it,"translate",link->translate);
        links[i++] = link;
    }
}

ObjectResource::IkParamResourcePtr ObjectResource::AddIkParam(const std::string& name, const std::string& iktype, double timeout)
{
    GETCONTROLLERIMPL();
    const std::string ikparamPk = controller->CreateIkParam(this->pk, name, iktype, timeout);

    return ObjectResource::IkParamResourcePtr(new IkParamResource(controller, this->pk, ikparamPk));
}

void ObjectResource::GetIkParams(std::vector<ObjectResource::IkParamResourcePtr>& ikparams)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("object/%s/ikparam/?format=json&limit=0&fields=ikparams")%GetPrimaryKey()), pt);
    rapidjson::Value& objects = pt["ikparams"];
    ikparams.resize(objects.Size());
    size_t i = 0;
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        IkParamResourcePtr ikparam(new IkParamResource(controller, GetPrimaryKey(), GetJsonValueByKey<std::string>(*it, "pk")));
        LoadJsonValueByKey(*it,"name",ikparam->name);
        LoadJsonValueByKey(*it,"pk",ikparam->pk);
        LoadJsonValueByKey(*it,"iktype",ikparam->iktype);
        LoadJsonValueByKey(*it,"quaternion",ikparam->quaternion);
        LoadJsonValueByKey(*it,"direction",ikparam->direction);
        LoadJsonValueByKey(*it,"translation",ikparam->translation);
        LoadJsonValueByKey(*it,"angle",ikparam->angle);
        ikparams[i++] = ikparam;
    }
}

RobotResource::RobotResource(ControllerClientPtr controller, const std::string& pk) : ObjectResource(controller, "robot", pk)
{
}

RobotResource::ToolResource::ToolResource(ControllerClientPtr controller, const std::string& robotobjectpk, const std::string& pk) : WebResource(controller, str(boost::format("robot/%s/tool")%robotobjectpk), pk)
{
}

void RobotResource::GetTools(std::vector<RobotResource::ToolResourcePtr>& tools)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("robot/%s/tool/?format=json&limit=0&fields=tools")%GetPrimaryKey()), pt);
    rapidjson::Value& objects = pt["tools"];
    tools.resize(objects.Size());
    size_t i = 0;
//    FOREACH(v, objects) {
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        ToolResourcePtr tool(new ToolResource(controller, GetPrimaryKey(), GetJsonValueByKey<std::string>(*it, "pk")));

        LoadJsonValueByKey(*it, "name", tool->name);
        LoadJsonValueByKey(*it, "pk", tool->pk);
        LoadJsonValueByKey(*it, "frame_orgin", tool->frame_origin);
        LoadJsonValueByKey(*it, "frame_tip", tool->frame_tip);
        LoadJsonValueByKey(*it, "direction", tool->direction);
        LoadJsonValueByKey(*it, "quaternion", tool->quaternion);
        LoadJsonValueByKey(*it, "translate", tool->translate);

        tools[i++] = tool;
    }
}

RobotResource::AttachedSensorResource::AttachedSensorResource(ControllerClientPtr controller, const std::string& robotobjectpk, const std::string& pk) : WebResource(controller, str(boost::format("robot/%s/attachedsensor")%robotobjectpk), pk)
{
}

void RobotResource::GetAttachedSensors(std::vector<AttachedSensorResourcePtr>& attachedsensors)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("robot/%s/attachedsensor/?format=json&limit=0&fields=attachedsensors")%GetPrimaryKey()), pt);
    //boost::property_tree::ptree& objects = pt.get_child("attachedsensors");
    rapidjson::Value& objects = pt["attachedsensors"];
    attachedsensors.resize(objects.Size());
    size_t i = 0;
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        AttachedSensorResourcePtr attachedsensor(new AttachedSensorResource(controller, GetPrimaryKey(), GetJsonValueByKey<std::string>(*it, "pk")));

        LoadJsonValueByKey(*it, "name", attachedsensor->name);
        LoadJsonValueByKey(*it, "pk", attachedsensor->pk);
        LoadJsonValueByKey(*it, "frame_origin", attachedsensor->frame_origin);
        LoadJsonValueByKey(*it, "sensortype", attachedsensor->sensortype);
        LoadJsonValueByKey(*it, "quaternion", attachedsensor->quaternion);
        LoadJsonValueByKey(*it, "translate", attachedsensor->translate);
        std::vector<double> distortionCoeffs = GetJsonValueByPath<std::vector<double> > (*it, "/sensordata/distortion_coeffs");
        BOOST_ASSERT(distortionCoeffs.size() <= 5);
        for (size_t i = 0; i < distortionCoeffs.size(); i ++) {
            attachedsensor->sensordata.distortion_coeffs[i] = distortionCoeffs[i];
        }
        attachedsensor->sensordata.distortion_model = GetJsonValueByPath<std::string>(*it, "/sensordata/distortion_model");
        attachedsensor->sensordata.focal_length = GetJsonValueByPath<Real>(*it, "/sensordata/focal_length");
        attachedsensor->sensordata.measurement_time= GetJsonValueByPath<Real>(*it, "/sensordata/measurement_time");
        std::vector<double> intrinsics = GetJsonValueByPath<std::vector<double> >(*it, "/sensordata/intrinsic");
        BOOST_ASSERT(intrinsics.size() <= 6);
        for (size_t i = 0; i < intrinsics.size(); i++) {
            attachedsensor->sensordata.intrinsic[i] = intrinsics[i];
        }
        std::vector<int> imgdim = GetJsonValueByPath<std::vector<int> >(*it, "/sensordata/image_dimensions");
        BOOST_ASSERT(imgdim.size() <= 3);
        for (size_t i = 0; i < imgdim.size(); i++) {
            attachedsensor->sensordata.image_dimensions[i] = imgdim[i];
        }

        if (rapidjson::Pointer("/sensordata/extra_parameters").Get(*it)) {
            std::string parameters_string = GetJsonValueByPath<std::string>(*it, "/sensordata/extra_parameters");
            //std::cout << "extra param " << parameters_string << std::endl;
            std::list<std::string> results;
            boost::split(results, parameters_string, boost::is_any_of(" "));
            results.remove("");
            attachedsensor->sensordata.extra_parameters.resize(results.size());
            size_t iparam = 0;
            BOOST_FOREACH(std::string p, results) {
                //std::cout << "'"<< p << "'"<< std::endl;
                try {
                    attachedsensor->sensordata.extra_parameters[iparam++] = boost::lexical_cast<Real>(p);
                } catch (...) {
                    //lexical_cast fails...
                }
            }
        } else {
            //std::cout << "no asus param" << std::endl;
        }

        attachedsensors[i++] = attachedsensor;
    }
}

SceneResource::InstObject::InstObject(ControllerClientPtr controller, const std::string& scenepk, const std::string& pk) : WebResource(controller, str(boost::format("scene/%s/instobject")%scenepk), pk)
{
}

void SceneResource::InstObject::SetTransform(const Transform& t)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt;
    std::string data = str(boost::format("{\"quaternion\":[%.15f, %.15f, %.15f, %.15f], \"translate\":[%.15f, %.15f, %.15f]}")%t.quaternion[0]%t.quaternion[1]%t.quaternion[2]%t.quaternion[3]%t.translate[0]%t.translate[1]%t.translate[2]);
    controller->CallPutJSON(str(boost::format("%s/%s/?format=json")%GetResourceName()%GetPrimaryKey()), data, pt);
}

void SceneResource::InstObject::SetDOFValues()
{
    GETCONTROLLERIMPL();
    std::stringstream ss;
    ss << "{\"dofvalues\":";
    ss << "[";
    if( this->dofvalues.size() > 0 ) {
        for (unsigned int i = 0; i < this->dofvalues.size(); i++) {
            ss << this->dofvalues[i];
            if( i != this->dofvalues.size()-1) {
                ss << ", ";
            }
        }
    }
    ss << "]}";
    rapidjson::Document pt;
    controller->CallPutJSON(str(boost::format("%s/%s/?format=json")%GetResourceName()%GetPrimaryKey()), ss.str(), pt);
}


void SceneResource::InstObject::GrabObject(InstObjectPtr grabbedobject, std::string& grabbedobjectlinkpk, std::string& grabbinglinkpk)
{
    SceneResource::InstObject::Grab grab;
    grab.instobjectpk = grabbedobject->pk;
    grab.grabbed_linkpk = grabbedobjectlinkpk;
    grab.grabbing_linkpk = grabbinglinkpk;
    //TODO do not use this->grabs. this is the cached information
    for (size_t igrab = 0; igrab < this->grabs.size(); igrab++) {
        if (this->grabs[igrab] == grab) {
            std::stringstream ss;
            ss << grabbedobject->name << "is already grabbed";
            MUJIN_LOG_ERROR(ss.str());
            return;
        }
    }
    std::stringstream ss;
    ss << "{\"grabs\":";
    ss << "[";
    if( this->grabs.size() > 0 ) {
        for (unsigned int i = 0; i < this->grabs.size(); i++) {
            ss << this->grabs[i].Serialize() << ", ";
        }
    }
    ss << grab.Serialize();
    ss << "]}";
    GETCONTROLLERIMPL();
    rapidjson::Document pt;
    controller->CallPutJSON(str(boost::format("%s/%s/?format=json")%GetResourceName()%GetPrimaryKey()), ss.str(), pt);
}

void SceneResource::InstObject::ReleaseObject(InstObjectPtr grabbedobject, std::string& grabbedobjectlinkpk, std::string& grabbinglinkpk)
{
    SceneResource::InstObject::Grab grab;
    grab.instobjectpk = grabbedobject->pk;
    grab.grabbed_linkpk = grabbedobjectlinkpk;
    grab.grabbing_linkpk = grabbinglinkpk;
    for (size_t igrab = 0; igrab < this->grabs.size(); igrab++) {
        if (this->grabs[igrab] == grab) {
            this->grabs.erase(std::remove(this->grabs.begin(), this->grabs.end(), this->grabs[igrab]), this->grabs.end());
            std::stringstream ss;
            ss << "{\"grabs\":";
            ss << "[";
            if( this->grabs.size() > 0 ) {
                for (unsigned int i = 0; i < this->grabs.size(); i++) {
                    ss << this->grabs[i].Serialize() << ", ";
                }
                if( igrab != this->grabs.size()-1) {
                    ss << ", ";
                }
            }
            ss << "]}";
            GETCONTROLLERIMPL();
            rapidjson::Document pt;
            controller->CallPutJSON(str(boost::format("%s/%s/?format=json")%GetResourceName()%GetPrimaryKey()), ss.str(), pt);
        }
    }
    std::stringstream ss;
    ss << grabbedobject->name << "is not grabbed";
    MUJIN_LOG_ERROR(ss.str());

}

SceneResource::SceneResource(ControllerClientPtr controller, const std::string& pk) : WebResource(controller, "scene", pk)
{
    // get something from the scene?
    //this->Get("");
}

TaskResourcePtr SceneResource::GetOrCreateTaskFromName_UTF8(const std::string& taskname, const std::string& tasktype, int options)
{
    GETCONTROLLERIMPL();
    boost::shared_ptr<char> pescapedtaskname = controller->GetURLEscapedString(taskname);
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("scene/%s/task/?format=json&limit=1&name=%s&fields=pk,tasktype")%GetPrimaryKey()%pescapedtaskname), pt);
    // task exists
    std::string pk;

    if (pt.IsObject() && pt.HasMember("objects") && pt["objects"].IsArray() && pt["objects"].Size() > 0) {
        rapidjson::Value& objects = pt["objects"];
        pk = GetJsonValueByKey<std::string>(objects[0], "pk");
        std::string currenttasktype = GetJsonValueByKey<std::string>(objects[0], "tasktype");
        if( currenttasktype != tasktype ) {
            throw MUJIN_EXCEPTION_FORMAT("task pk %s exists and has type %s, expected is %s", pk%currenttasktype%tasktype, MEC_InvalidState);
        }
    }
    else {
        pt.SetObject();
        controller->CallPost(str(boost::format("scene/%s/task/?format=json&fields=pk")%GetPrimaryKey()), str(boost::format("{\"name\":\"%s\", \"tasktype\":\"%s\", \"scenepk\":\"%s\"}")%taskname%tasktype%GetPrimaryKey()), pt);
        LoadJsonValueByKey(pt, "pk", pk);
    }

    if( pk.size() == 0 ) {
        return TaskResourcePtr();
    }

    if( tasktype == "binpicking" || tasktype == "realtimeitlplanning") {
        BinPickingTaskResourcePtr task;
        if( options & 1 ) {
#ifdef MUJIN_USEZMQ
            task.reset(new BinPickingTaskZmqResource(GetController(), pk, GetPrimaryKey(), tasktype));
#else
            throw MujinException("cannot create binpicking zmq task since not compiled with zeromq library", MEC_Failed);
#endif
        }
        else {
            task.reset(new BinPickingTaskResource(GetController(), pk, GetPrimaryKey()));
        }
        return task;
    }
    else if( tasktype == "cablepicking" ) { // TODO create CablePickingTaskResource
        BinPickingTaskResourcePtr task;
        if( options & 1 ) {
#ifdef MUJIN_USEZMQ
            task.reset(new BinPickingTaskZmqResource(GetController(), pk, GetPrimaryKey()));
#else
            throw MujinException("cannot create binpicking zmq task since not compiled with zeromq library", MEC_Failed);
#endif
        }
        else {
            task.reset(new BinPickingTaskResource(GetController(), pk, GetPrimaryKey()));
        }
        return task;
    }
    else {
        TaskResourcePtr task(new TaskResource(GetController(), pk));
        return task;
    }
}

void SceneResource::SetInstObjectsState(const std::vector<SceneResource::InstObjectPtr>& instobjects, const std::vector<InstanceObjectState>& states)
{
    GETCONTROLLERIMPL();
    MUJIN_ASSERT_OP_FORMAT(instobjects.size(), ==, states.size(), "the size of instobjects (%d) and the one of states (%d) must be the same",instobjects.size()%states.size(),MEC_InvalidArguments);
    boost::format transformtemplate("{\"pk\":\"%s\",\"quaternion\":[%.15f, %.15f, %.15f, %.15f], \"translate\":[%.15f, %.15f, %.15f] %s}");
    std::string datastring, sdofvalues;
    for(size_t i = 0; i < instobjects.size(); ++i) {
        const Transform& transform = states[i].transform;
        if( states[i].dofvalues.size() > 0 ) {
            sdofvalues = str(boost::format(", \"dofvalues\":[%.15f")%states[i].dofvalues.at(0));
            for(size_t j = 1; j < states[i].dofvalues.size(); ++j) {
                sdofvalues += str(boost::format(", %.15f")%states[i].dofvalues.at(j));
            }
            sdofvalues += "]";
        }
        else {
            sdofvalues = "";
        }
        datastring += str(transformtemplate%instobjects[i]->pk%transform.quaternion[0]%transform.quaternion[1]%transform.quaternion[2]%transform.quaternion[3]%transform.translate[0]%transform.translate[1]%transform.translate[2]%sdofvalues);
        if ( i != instobjects.size()-1) {
            datastring += ",";
        }
    }
    std::string data = str(boost::format("{\"objects\": [%s]}")%datastring);
    rapidjson::Document pt;
    controller->CallPutJSON(str(boost::format("%s/%s/instobject/?format=json")%GetResourceName()%GetPrimaryKey()), data, pt);
}

TaskResourcePtr SceneResource::GetTaskFromName_UTF8(const std::string& taskname, int options)
{
    GETCONTROLLERIMPL();
    boost::shared_ptr<char> pescapedtaskname = controller->GetURLEscapedString(taskname);
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("scene/%s/task/?format=json&limit=1&name=%s&fields=pk,tasktype")%GetPrimaryKey()%pescapedtaskname), pt);
    // task exists

    if (!(pt.IsObject() && pt.HasMember("objects") && pt["objects"].IsArray() && pt["objects"].Size() > 0)) {
        throw MUJIN_EXCEPTION_FORMAT("could not find task with name %s", taskname, MEC_InvalidState);
    }

    std::string pk = GetJsonValueByKey<std::string>(pt["objects"][0], "pk");
    TaskResourcePtr task(new TaskResource(GetController(), pk));
    return task;
}

TaskResourcePtr SceneResource::GetOrCreateTaskFromName_UTF16(const std::wstring& taskname, const std::string& tasktype, int options)
{
    std::string taskname_utf8;
    utf8::utf16to8(taskname.begin(), taskname.end(), std::back_inserter(taskname_utf8));
    return GetOrCreateTaskFromName_UTF8(taskname_utf8, tasktype, options);
}

TaskResourcePtr SceneResource::GetTaskFromName_UTF16(const std::wstring& taskname, int options)
{
    std::string taskname_utf8;
    utf8::utf16to8(taskname.begin(), taskname.end(), std::back_inserter(taskname_utf8));
    return GetTaskFromName_UTF8(taskname_utf8, options);
}

BinPickingTaskResourcePtr SceneResource::GetOrCreateBinPickingTaskFromName_UTF8(const std::string& taskname, const std::string& tasktype, int options)
{
    return boost::dynamic_pointer_cast<BinPickingTaskResource>(GetOrCreateTaskFromName_UTF8(taskname, tasktype, options));
}

BinPickingTaskResourcePtr SceneResource::GetOrCreateBinPickingTaskFromName_UTF16(const std::wstring& taskname, const std::string& tasktype, int options)
{
    return boost::dynamic_pointer_cast<BinPickingTaskResource>(GetOrCreateTaskFromName_UTF16(taskname, tasktype, options));
}

void SceneResource::GetTaskPrimaryKeys(std::vector<std::string>& taskkeys)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("scene/%s/task/?format=json&limit=0&fields=pk")%GetPrimaryKey()), pt);
    rapidjson::Value& objects = pt["objects"];
    taskkeys.resize(objects.Size());
    size_t i = 0;
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        taskkeys[i++] = GetJsonValueByKey<std::string>(*it, "pk");
    }
}

void SceneResource::GetSensorMapping(std::map<std::string, std::string>& sensormapping)
{
    GETCONTROLLERIMPL();
    sensormapping.clear();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("scene/%s/instobject/?format=json&limit=0")%GetPrimaryKey()), pt);
    rapidjson::Value& objects = pt["objects"];
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        if ( it->HasMember("attachedsensors") ) {
            rapidjson::Value& jsonattachedsensors = (*it)["attachedsensors"];
            if (jsonattachedsensors.IsArray() && jsonattachedsensors.Size() > 0) {
                std::string object_pk = GetJsonValueByKey<std::string>(*it, "object_pk");
                std::string cameracontainername = GetJsonValueByKey<std::string>(*it, "name");
                rapidjson::Document pt_robot(rapidjson::kObjectType);
                controller->CallGet(str(boost::format("robot/%s/attachedsensor/?format=json")%object_pk), pt_robot);
                rapidjson::Value& pt_attachedsensors = pt_robot["attachedsensors"];
                for (rapidjson::Document::ValueIterator itsensor = pt_attachedsensors.Begin();
                        itsensor != pt_attachedsensors.End(); ++itsensor) {
                    std::string sensorname = GetJsonValueByKey<std::string>(*itsensor, "name");
                    std::string camerafullname = str(boost::format("%s/%s")%cameracontainername%sensorname);
                    std::string cameraid = GetJsonValueByPath<std::string>(*itsensor, "/sensordata/hardware_id");
                    sensormapping[camerafullname] = cameraid;
                }
            }
        }
    }
}

void SceneResource::GetInstObjects(std::vector<SceneResource::InstObjectPtr>& instobjects)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("scene/%s/instobject/?format=json&limit=0")%GetPrimaryKey()), pt);
    rapidjson::Value& objects = pt["objects"];

    instobjects.resize(objects.Size());
    size_t iobj = 0;
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        InstObjectPtr instobject(new InstObject(controller, GetPrimaryKey(), GetJsonValueByKey<std::string>(*it, "pk")));

        LoadJsonValueByKey(*it, "name", instobject->name);
        LoadJsonValueByKey(*it, "pk", instobject->pk);
        LoadJsonValueByKey(*it, "object_pk", instobject->object_pk);
        LoadJsonValueByKey(*it, "reference_uri", instobject->reference_uri);
        LoadJsonValueByKey(*it, "dofvalues", instobject->dofvalues);
        LoadJsonValueByKey(*it, "quaternion", instobject->quaternion);
        LoadJsonValueByKey(*it, "translate", instobject->translate);

        if (it->HasMember("links")) {
            rapidjson::Value& jsonlinks = (*it)["links"];
            instobject->links.resize(jsonlinks.Size());
            size_t ilink = 0;
            for (rapidjson::Document::ValueIterator itlink = jsonlinks.Begin(); itlink != jsonlinks.End(); ++itlink) {
                InstObject::Link& link = instobject->links[ilink];
                LoadJsonValueByKey(*itlink, "name", link.name);
                LoadJsonValueByKey(*itlink, "quaternion", link.quaternion);
                LoadJsonValueByKey(*itlink, "translate", link.translate);
                ilink++;
            }
        }

        if (it->HasMember("tools")) {
            rapidjson::Value& jsontools = (*it)["tools"];
            instobject->tools.resize(jsontools.Size());
            size_t itool = 0;
            for (rapidjson::Document::ValueIterator ittool = jsontools.Begin(); ittool != jsontools.End(); ++ittool) {
                InstObject::Tool &tool = instobject->tools[itool];
                LoadJsonValueByKey(*ittool, "name", tool.name);
                LoadJsonValueByKey(*ittool, "quaternion", tool.quaternion);
                LoadJsonValueByKey(*ittool, "translate", tool.translate);
                LoadJsonValueByKey(*ittool, "direction", tool.direction);
                itool++;
            }
        }

        if (it->HasMember("grabs")) {
            rapidjson::Value& jsongrabs = (*it)["grabs"];
            instobject->grabs.resize(jsongrabs.Size());
            size_t igrab = 0;
            for (rapidjson::Document::ValueIterator itgrab = jsongrabs.Begin(); itgrab != jsongrabs.End(); ++itgrab) {
                InstObject::Grab &grab = instobject->grabs[igrab];
                LoadJsonValueByKey(*itgrab, "instobjectpk", grab.instobjectpk);
                LoadJsonValueByKey(*itgrab, "grabbed_linkpk", grab.grabbed_linkpk);
                LoadJsonValueByKey(*itgrab, "grabbing_linkpk", grab.grabbing_linkpk);
                igrab++;
            }
        }

        if (it->HasMember("attachedsensors")) {
            rapidjson::Value& jsonattachedsensors = (*it)["attachedsensors"];
            instobject->attachedsensors.resize(jsonattachedsensors.Size());
            size_t iattchedsensor = 0;
            for (rapidjson::Document::ValueIterator itsensor = jsonattachedsensors.Begin();
                    itsensor != jsonattachedsensors.End(); ++itsensor) {
                InstObject::AttachedSensor& sensor  = instobject->attachedsensors[iattchedsensor];
                LoadJsonValueByKey(*itsensor, "name", sensor.name);
                LoadJsonValueByKey(*itsensor, "quaternion", sensor.quaternion);
                LoadJsonValueByKey(*itsensor, "translate", sensor.translate);
                iattchedsensor++;
            }
        }
        instobjects.at(iobj++) = instobject;
    }
}

bool SceneResource::FindInstObject(const std::string& name, SceneResource::InstObjectPtr& instobject)
{

    std::vector<SceneResource::InstObjectPtr> instobjects;
    this->GetInstObjects(instobjects);
    for(size_t i = 0; i < instobjects.size(); ++i) {
        if (instobjects[i]->name == name) {
            instobject = instobjects[i];
            return true;
        }
    }
    return false;
}

SceneResource::InstObjectPtr SceneResource::CreateInstObject(const std::string& name, const std::string& referenceUri, const Real quaternion[4], const Real translation[3], double timeout)
{
    GETCONTROLLERIMPL();
    const std::string uri(str(boost::format("scene/%s/instobject/?format=json&fields=pk")%GetPrimaryKey()));
    std::string data(str(boost::format("{\"name\":\"%s\", \"quaternion\":[%.15f,%.15f,%.15f,%.15f], \"translate\":[%.15f,%.15f,%.15f]")%name%quaternion[0]%quaternion[1]%quaternion[2]%quaternion[3]%translation[0]%translation[1]%translation[2]));
    if (!referenceUri.empty()) {
        data += ", \"reference_uri\": \"" + referenceUri + "\"";
    }
    data += "}";

    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallPost(uri, data, pt, 201, timeout);
    std::string inst_pk = GetJsonValueByKey<std::string>(pt, "pk");
    SceneResource::InstObjectPtr instobject(new SceneResource::InstObject(GetController(), GetPrimaryKey(),  inst_pk));
    return instobject;
}

void SceneResource::DeleteInstObject(const std::string& pk)
{
    GETCONTROLLERIMPL();
    controller->CallDelete(str(boost::format("scene/%s/instobject/%s/")%GetPrimaryKey()%pk));
}

SceneResourcePtr SceneResource::Copy(const std::string& name)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallPost("scene/?format=json", str(boost::format("{\"name\":\"%s\", \"reference_pk\":\"%s\", \"overwrite\": \"1\"}")%name%GetPrimaryKey()), pt);
    std::string pk = GetJsonValueByKey<std::string>(pt, "pk");
    SceneResourcePtr scene(new SceneResource(GetController(), pk));
    return scene;
}

TaskResource::TaskResource(ControllerClientPtr controller, const std::string& pk) : WebResource(controller,"task",pk)
{
}

bool TaskResource::Execute()
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallPost("job/", str(boost::format("{\"resource_type\":\"task\", \"target_pk\":%s}")%GetPrimaryKey()), pt, 200);
    _jobpk = GetJsonValueByKey<std::string>(pt, "jobpk");
    return true;
}

void TaskResource::Cancel()
{
    // have to look through all jobs for the task
    BOOST_ASSERT(0);
}

JobStatusCode GetStatusCode(const std::string& str)
{
    MUJIN_LOG_INFO(str);
    if (str == "pending") return JSC_Pending;
    if (str == "active") return JSC_Active;
    if (str == "preempted") return JSC_Preempted;
    if (str == "succeeded") return JSC_Succeeded;
    if (str == "aborted") return JSC_Aborted;
    if (str == "rejected") return JSC_Rejected;
    if (str == "preempting") return JSC_Preempting;
    if (str == "recalling") return JSC_Recalling;
    if (str == "recalled") return JSC_Recalled;
    if (str == "lost") return JSC_Lost;
    if (str == "unknown") return JSC_Unknown;
    throw MUJIN_EXCEPTION_FORMAT("unknown staus %s", str, MEC_InvalidArguments);
}

void TaskResource::GetRunTimeStatus(JobStatus& status, int options)
{
    status.code = JSC_Unknown;
    if( _jobpk.size() > 0 ) {
        GETCONTROLLERIMPL();
        rapidjson::Document pt(rapidjson::kObjectType);
        std::string url = str(boost::format("job/%s/?format=json&fields=pk,status,fnname,elapsedtime")%_jobpk);
        if( options & 1 ) {
            url += std::string(",status_text");
        }
        controller->CallGet(url, pt);
        //pt.get("error_message")
        LoadJsonValueByKey(pt, "pk", status.pk);
        //LoadJsonValueByKey(pt, "status", status.code);
        status.code  = GetStatusCode(GetJsonValueByKey<std::string>(pt, "status"));
        LoadJsonValueByKey(pt, "elapsedtime", status.elapsedtime);
        LoadJsonValueByKey(pt, "fname", status.type);
        if( options & 1 ) {
            LoadJsonValueByKey(pt, "status_text", status.message);
        }
    }
}

OptimizationResourcePtr TaskResource::GetOrCreateOptimizationFromName_UTF8(const std::string& optimizationname, const std::string& optimizationtype)
{
    GETCONTROLLERIMPL();
    boost::shared_ptr<char> pescapedoptimizationname = controller->GetURLEscapedString(optimizationname);
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("task/%s/optimization/?format=json&limit=1&name=%s&fields=pk,optimizationtype")%GetPrimaryKey()%pescapedoptimizationname), pt);
    // optimization exists
    if (pt.IsObject() && pt.HasMember("objects") && pt["objects"].IsArray() && pt["objects"].Size() > 0) {
        rapidjson::Value& object = pt["objects"][0];
        std::string pk = GetJsonValueByKey<std::string>(object, "pk");
        std::string currentoptimizationtype = GetJsonValueByKey<std::string>(object, "optimizationtype");
        if( currentoptimizationtype != optimizationtype ) {
            throw MUJIN_EXCEPTION_FORMAT("optimization pk %s exists and has type %s, expected is %s", pk%currentoptimizationtype%optimizationtype, MEC_InvalidState);
        }
        OptimizationResourcePtr optimization(new OptimizationResource(GetController(), pk));
        return optimization;
    }

    pt.SetObject();
    controller->CallPost(str(boost::format("task/%s/optimization/?format=json&fields=pk")%GetPrimaryKey()), str(boost::format("{\"name\":\"%s\", \"optimizationtype\":\"%s\", \"taskpk\":\"%s\"}")%optimizationname%optimizationtype%GetPrimaryKey()), pt);
    std::string pk = GetJsonValueByKey<std::string>(pt, "pk");
    OptimizationResourcePtr optimization(new OptimizationResource(GetController(), pk));
    return optimization;
}

OptimizationResourcePtr TaskResource::GetOrCreateOptimizationFromName_UTF16(const std::wstring& optimizationname, const std::string& optimizationtype)
{
    std::string optimizationname_utf8;
    utf8::utf16to8(optimizationname.begin(), optimizationname.end(), std::back_inserter(optimizationname_utf8));
    return GetOrCreateOptimizationFromName_UTF8(optimizationname_utf8, optimizationtype);
}

void TaskResource::GetOptimizationPrimaryKeys(std::vector<std::string>& optimizationkeys)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("task/%s/optimization/?format=json&limit=0&fields=pk")%GetPrimaryKey()), pt);
    rapidjson::Value& objects = pt["objects"];

    optimizationkeys.resize(objects.Size());
    size_t i = 0;
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        LoadJsonValueByKey(*it, "pk", optimizationkeys[i++]);
    }
}

void TaskResource::GetTaskParameters(ITLPlanningTaskParameters& taskparameters)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("task/%s/?format=json&fields=taskparameters,tasktype")%GetPrimaryKey()), pt);
    std::string tasktype = GetJsonValueByKey<std::string>(pt, "tasktype");
    if( tasktype != "itlplanning" ) {
        throw MUJIN_EXCEPTION_FORMAT("task %s is type %s, expected itlplanning", GetPrimaryKey()%tasktype, MEC_InvalidArguments);
    }
    rapidjson::Value& taskparametersjson = pt["taskparameters"];
    taskparameters.SetDefaults();
    bool bhasreturnmode = false, bhasreturntostart = false, breturntostart = false;
    for (rapidjson::Document::MemberIterator v = taskparametersjson.MemberBegin(); v != taskparametersjson.MemberEnd(); ++v) {
        if( std::string(v->name.GetString()) == "startfromcurrent" ) {
            taskparameters.startfromcurrent = std::string("True") == v->value.GetString();
        }
        else if(std::string(v->name.GetString()) == "returntostart" ) {
            bhasreturntostart = true;
            breturntostart = std::string("True") == v->value.GetString();
        }
        else if( std::string(v->name.GetString()) == "returnmode" ) {
            taskparameters.returnmode = v->value.GetString();
            bhasreturnmode = true;
        }
        else if( std::string(v->name.GetString()) == "ignorefigure" ) {
            taskparameters.ignorefigure = std::string("True") == v->value.GetString();
        }
        else if( std::string(v->name.GetString()) == "vrcruns" ) {
            //taskparameters.vrcruns = boost::lexical_cast<int>(v->value);
            LoadJsonValueByKey(taskparametersjson, "vrcruns", taskparameters.vrcruns);
        }
        else if( std::string(v->name.GetString()) == "unit" ) {
            taskparameters.unit = v->value.GetString();
        }
        else if( std::string(v->name.GetString()) == "optimizationvalue" ) {
            LoadJsonValueByKey(taskparametersjson, "optimizationvalue", taskparameters.optimizationvalue);
            //taskparameters.optimizationvalue = boost::lexical_cast<Real>(v->second.data());
        }
        else if( std::string(v->name.GetString()) == "program" ) {
            taskparameters.program = v->value.GetString();
        }
        else if( std::string(v->name.GetString()) == "initial_envstate" ) {
            ExtractEnvironmentStateFromPTree(v->value, taskparameters.initial_envstate);
        }
        else if( std::string(v->name.GetString()) == "final_envstate" ) {
            ExtractEnvironmentStateFromPTree(v->value, taskparameters.final_envstate);
        }
        else {
            std::stringstream ss;
            ss << "unsupported ITL task parameter " << v->name.GetString();
            MUJIN_LOG_ERROR(ss.str());
        }
    }
    // for back compat
    if( !bhasreturnmode && bhasreturntostart ) {
        taskparameters.returnmode = breturntostart ? "start" : "";
    }
}

void TaskResource::SetTaskParameters(const ITLPlanningTaskParameters& taskparameters)
{
    GETCONTROLLERIMPL();
    std::string startfromcurrent = taskparameters.startfromcurrent ? "True" : "False";
    std::string ignorefigure = taskparameters.ignorefigure ? "True" : "False";
    std::string vrcruns = boost::lexical_cast<std::string>(taskparameters.vrcruns);

    std::stringstream ssinitial_envstate;
    if( taskparameters.initial_envstate.size() > 0 ) {
        ssinitial_envstate << std::setprecision(std::numeric_limits<Real>::digits10+1);
        ssinitial_envstate << ", \"initial_envstate\":";
        SerializeEnvironmentStateToJSON(taskparameters.initial_envstate, ssinitial_envstate);
    }
    std::stringstream ssfinal_envstate;
    if( taskparameters.final_envstate.size() > 0 ) {
        ssfinal_envstate << std::setprecision(std::numeric_limits<Real>::digits10+1);
        ssfinal_envstate << ", \"final_envstate\":";
        SerializeEnvironmentStateToJSON(taskparameters.final_envstate, ssfinal_envstate);
    }

    // because program will inside string, encode newlines
    std::string program;
    std::vector< std::pair<std::string, std::string> > serachpairs(3);
    serachpairs[0].first = "\""; serachpairs[0].second = "\\\"";
    serachpairs[1].first = "\n"; serachpairs[1].second = "\\n";
    serachpairs[2].first = "\r\n"; serachpairs[2].second = "\\n";
    SearchAndReplace(program, taskparameters.program, serachpairs);
    std::string taskgoalput = str(boost::format("{\"tasktype\": \"itlplanning\", \"taskparameters\":{\"optimizationvalue\":%f, \"program\":\"%s\", \"unit\":\"%s\", \"returnmode\":\"%s\", \"startfromcurrent\":\"%s\", \"ignorefigure\":\"%s\", \"vrcruns\":%d %s %s } }")%taskparameters.optimizationvalue%program%taskparameters.unit%taskparameters.returnmode%startfromcurrent%ignorefigure%vrcruns%ssinitial_envstate.str()%ssfinal_envstate.str());
    rapidjson::Document pt;
    controller->CallPutJSON(str(boost::format("task/%s/?format=json&fields=")%GetPrimaryKey()), taskgoalput, pt);
}

PlanningResultResourcePtr TaskResource::GetResult()
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("task/%s/result/?format=json&limit=1&optimization=None&fields=pk")%GetPrimaryKey()), pt);
    if (!(pt.IsObject() && pt.HasMember("objects") && pt["objects"].IsArray() && pt["objects"].Size() > 0) ) {
        return PlanningResultResourcePtr();
    }
    std::string pk = GetJsonValueByKey<std::string>(pt["objects"][0], "pk");
    PlanningResultResourcePtr result(new PlanningResultResource(GetController(), pk));
    return result;
}

OptimizationResource::OptimizationResource(ControllerClientPtr controller, const std::string& pk) : WebResource(controller,"optimization",pk)
{
}

void OptimizationResource::Execute(bool bClearOldResults)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallPost(str(boost::format("optimization/%s/")%GetPrimaryKey()), str(boost::format("{\"clear\":%d}")%bClearOldResults), pt, 200);
    _jobpk = GetJsonValueByKey<std::string>(pt, "jobpk");
}

void OptimizationResource::Cancel()
{
    BOOST_ASSERT(0);
}

void OptimizationResource::GetRunTimeStatus(JobStatus& status, int options)
{
    status.code = JSC_Unknown;
    if( _jobpk.size() > 0 ) {
        GETCONTROLLERIMPL();
        rapidjson::Document pt(rapidjson::kObjectType);
        std::string url = str(boost::format("job/%s/?format=json&fields=pk,status,fnname,elapsedtime")%_jobpk);
        if( options & 1 ) {
            url += std::string(",status_text");
        }
        controller->CallGet(url, pt);
        //pt.get("error_message")
        LoadJsonValueByKey(pt, "pk", status.pk);
        // LoadJsonValueByKey(pt, "status", status.code);
        status.code = GetStatusCode(GetJsonValueByKey<std::string>(pt, "status"));
        LoadJsonValueByKey(pt, "fname", status.type);
        LoadJsonValueByKey(pt, "elpasedtime", status.elapsedtime);
        if( options & 1 ) {
            LoadJsonValueByKey(pt, "status-text", status.message);
        }
    }
}

void OptimizationResource::SetOptimizationParameters(const RobotPlacementOptimizationParameters& optparams)
{
    GETCONTROLLERIMPL();
    std::string ignorebasecollision = optparams.ignorebasecollision ? "True" : "False";
    std::string optimizationgoalput = str(boost::format("{\"optimizationtype\":\"robotplacement\", \"optimizationparameters\":{\"targetname\":\"%s\", \"frame\":\"%s\", \"ignorebasecollision\":\"%s\", \"unit\":\"%s\", \"maxrange_\":[ %.15f, %.15f, %.15f, %.15f],  \"minrange_\":[ %.15f, %.15f, %.15f, %.15f], \"stepsize_\":[ %.15f, %.15f, %.15f, %.15f], \"topstorecandidates\":%d} }")%optparams.targetname%optparams.framename%ignorebasecollision%optparams.unit%optparams.maxrange[0]%optparams.maxrange[1]%optparams.maxrange[2]%optparams.maxrange[3]%optparams.minrange[0]%optparams.minrange[1]%optparams.minrange[2]%optparams.minrange[3]%optparams.stepsize[0]%optparams.stepsize[1]%optparams.stepsize[2]%optparams.stepsize[3]%optparams.topstorecandidates);
    rapidjson::Document pt;
    controller->CallPutJSON(str(boost::format("optimization/%s/?format=json&fields=")%GetPrimaryKey()), optimizationgoalput, pt);
}

void OptimizationResource::SetOptimizationParameters(const PlacementsOptimizationParameters& optparams)
{
    GETCONTROLLERIMPL();
    std::stringstream optimizationgoalput;
    optimizationgoalput << str(boost::format("{\"optimizationtype\":\"placements\", \"optimizationparameters\":{ \"unit\":\"%s\", \"topstorecandidates\":%d")%optparams.unit%optparams.topstorecandidates);
    for(size_t itarget = 0; itarget < optparams.targetnames.size(); ++itarget) {
        std::string ignorebasecollision = optparams.ignorebasecollisions[itarget] ? "True" : "False";
        std::string suffix;
        if( itarget > 0 ) {
            suffix = boost::lexical_cast<std::string>(itarget+1);
        }
        optimizationgoalput << str(boost::format(", \"targetname%s\":\"%s\", \"frame%s\":\"%s\", \"ignorebasecollision%s\":\"%s\", , \"maxrange%s_\":[ %.15f, %.15f, %.15f, %.15f],  \"minrange%s_\":[ %.15f, %.15f, %.15f, %.15f], \"stepsize%s_\":[ %.15f, %.15f, %.15f, %.15f]")%suffix%optparams.targetnames[itarget]%suffix%optparams.framenames[itarget]%suffix%ignorebasecollision%suffix%optparams.maxranges[itarget][0]%optparams.maxranges[itarget][1]%optparams.maxranges[itarget][2]%optparams.maxranges[itarget][3]%suffix%optparams.minranges[itarget][0]%optparams.minranges[itarget][1]%optparams.minranges[itarget][2]%optparams.minranges[itarget][3]%suffix%optparams.stepsizes[itarget][0]%optparams.stepsizes[itarget][1]%optparams.stepsizes[itarget][2]%optparams.stepsizes[itarget][3]);
    }
    optimizationgoalput << "} }";
    rapidjson::Document pt;
    controller->CallPutJSON(str(boost::format("optimization/%s/?format=json&fields=")%GetPrimaryKey()), optimizationgoalput.str(), pt);
}

void OptimizationResource::GetResults(std::vector<PlanningResultResourcePtr>& results, int startoffset, int num)
{
    GETCONTROLLERIMPL();
    std::string querystring = str(boost::format("optimization/%s/result/?format=json&fields=pk&order_by=task_time&offset=%d&limit=%d")%GetPrimaryKey()%startoffset%num);
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(querystring, pt);
    if (!(pt.IsObject() && pt.HasMember("objects") && pt["objects"].IsArray() && pt["objects"].Size() > 0)) {
        return;
    }
    rapidjson::Value& objects = pt["objects"];
    results.resize(objects.Size());
    size_t i = 0;
    for (rapidjson::Document::ValueIterator it = objects.Begin(); it != objects.End(); ++it) {
        results[i++].reset(new PlanningResultResource(controller, GetJsonValueByKey<std::string>(*it, "pk")));
    }
}

PlanningResultResource::PlanningResultResource(ControllerClientPtr controller, const std::string& resulttype, const std::string& pk) : WebResource(controller,resulttype,pk)
{
}

PlanningResultResource::PlanningResultResource(ControllerClientPtr controller, const std::string& pk) : WebResource(controller,"planningresult",pk)
{
}

void PlanningResultResource::GetEnvironmentState(EnvironmentState& envstate)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    controller->CallGet(str(boost::format("%s/%s/?format=json&fields=envstate")%GetResourceName()%GetPrimaryKey()), pt);
    ExtractEnvironmentStateFromPTree(pt["envstate"], envstate);
}

void PlanningResultResource::GetAllRawProgramData(std::string& outputdata, const std::string& programtype)
{
    GETCONTROLLERIMPL();
    controller->CallGet(str(boost::format("%s/%s/program/?type=%s")%GetResourceName()%GetPrimaryKey()%programtype), outputdata);
}

void PlanningResultResource::GetRobotRawProgramData(std::string& outputdata, const std::string& robotpk, const std::string& programtype)
{
    GETCONTROLLERIMPL();
    controller->CallGet(str(boost::format("%s/%s/program/%s/?type=%s")%GetResourceName()%GetPrimaryKey()%robotpk%programtype), outputdata);
}

void PlanningResultResource::GetPrograms(RobotControllerPrograms& programs, const std::string& programtype)
{
    GETCONTROLLERIMPL();
    rapidjson::Document pt(rapidjson::kObjectType);
    programs.programs.clear();
    controller->CallGet(str(boost::format("%s/%s/program/?format=json&type=%s")%GetResourceName()%GetPrimaryKey()%programtype), pt);
    for (rapidjson::Document::MemberIterator it = pt.MemberBegin(); it != pt.MemberEnd(); ++it) {
        std::string robotpk = it->name.GetString();
        std::string program = GetJsonValueByKey<std::string>(it->value, "program");
        std::string currenttype = GetJsonValueByKey<std::string>(it->value, "type");
        programs.programs[robotpk] = RobotProgramData(program, currenttype);
    }
}

ControllerClientPtr CreateControllerClient(const std::string& usernamepassword, const std::string& baseurl, const std::string& proxyserverport, const std::string& proxyuserpw, int options, double timeout)
{
    return ControllerClientPtr(new ControllerClientImpl(usernamepassword, baseurl, proxyserverport, proxyuserpw, options, timeout));
}

void ControllerClientDestroy()
{
    DestroyControllerClient();
}

void DestroyControllerClient()
{
}

void ComputeMatrixFromTransform(Real matrix[12], const Transform &transform)
{
    throw MujinException("not implemented yet");
//    length2 = numpy.sum(quat**2)
//    ilength2 = 2.0/length2
//    qq1 = ilength2*quat[1]*quat[1]
//    qq2 = ilength2*quat[2]*quat[2]
//    qq3 = ilength2*quat[3]*quat[3]
//    T = numpy.eye(4)
//    T[0,0] = 1 - qq2 - qq3
//    T[0,1] = ilength2*(quat[1]*quat[2] - quat[0]*quat[3])
//    T[0,2] = ilength2*(quat[1]*quat[3] + quat[0]*quat[2])
//    T[1,0] = ilength2*(quat[1]*quat[2] + quat[0]*quat[3])
//    T[1,1] = 1 - qq1 - qq3
//    T[1,2] = ilength2*(quat[2]*quat[3] - quat[0]*quat[1])
//    T[2,0] = ilength2*(quat[1]*quat[3] - quat[0]*quat[2])
//    T[2,1] = ilength2*(quat[2]*quat[3] + quat[0]*quat[1])
//    T[2,2] = 1 - qq1 - qq2
}

void ComputeZXYFromMatrix(Real ZXY[3], Real matrix[12])
{
    throw MujinException("not implemented yet");
//    if abs(T[2][0]) < 1e-10 and abs(T[2][2]) < 1e-10:
//        sinx = T[2][1]
//        x = numpy.pi/2 if sinx > 0 else -numpy.pi/2
//        z = 0.0
//        y = numpy.arctan2(sinx*T[1][0],T[0][0])
//    else:
//        y = numpy.arctan2(-T[2][0],T[2][2])
//        siny = numpy.sin(y)
//        cosy = numpy.cos(y)
//        Ryinv = numpy.array([[cosy,0,-siny],[0,1,0],[siny,0,cosy]])
//        Rzx = numpy.dot(T[0:3,0:3],Ryinv)
//        x = numpy.arctan2(Rzx[2][1],Rzx[2][2])
//        z = numpy.arctan2(Rzx[1][0],Rzx[0][0])
//    return numpy.array([x,y,z])
}

void ComputeZXYFromTransform(Real ZXY[3], const Transform& transform)
{
    throw MujinException("not implemented yet");
    //zxyFromMatrix(matrixFromTransform())
}

}
