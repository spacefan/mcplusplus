#include "xmlparser.h"

#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <stdlib.h>

using namespace boost::math::constants;

XMLParser::XMLParser()
{
    showTrajectory = false;
    sim = NULL;
    src = NULL;
    _sample = NULL;
}

void XMLParser::parseFile(const string &filename)
{
    read_xml(filename, pt, xml_parser::no_comments);
    parse();
}

void XMLParser::parseString(const string &xmlContent)
{
    if(xmlContent.empty())
        return;
    stringstream ss;
    ss << xmlContent;
    read_xml(ss, pt, xml_parser::no_comments);
    parse();
}

void XMLParser::parse() {
    //materials
    map<string,Material>::iterator it = materialMap.begin();
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("MCPlusPlus.materials")) {
        if(v.first != "material")
            continue;
        string name = v.second.get_child("<xmlattr>.name").data();
        Material mat;
        mat.ls = v.second.get<MCfloat>("<xmlattr>.ls");
        mat.g = v.second.get<MCfloat>("<xmlattr>.g");
        mat.n = v.second.get<MCfloat>("<xmlattr>.n");
        materialMap.insert(it, pair<string,Material>(name,mat));
    }


    //sample
    if(_sample != NULL)
        delete _sample;
    _sample = new Sample();
    string left = pt.get<string>("MCPlusPlus.MLSample.<xmlattr>.left");
    string right = pt.get<string>("MCPlusPlus.MLSample.<xmlattr>.right");
    _sample->setSurroundingEnvironment(material(left),material(right));

    vector<Material> preLayerMaterial;
    vector<MCfloat> preLayerThickness;
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("MCPlusPlus.MLSample")) {
        if(v.first == "layer" || v.first == "prelayer") {
            Material mat = material(v.second.get_child("<xmlattr>.material").data());
            if(v.first == "layer")
                _sample->addLayer(mat,v.second.get<MCfloat>("<xmlattr>.thickness"));
            else if(v.first == "prelayer") {
                preLayerMaterial.push_back(mat);
                preLayerThickness.push_back(v.second.get<MCfloat>("<xmlattr>.thickness"));
            }
        }
    }

    for (int i = preLayerMaterial.size() - 1; i >= 0; --i) {
        _sample->addPreLayer(preLayerMaterial[i],preLayerThickness[i]);
    }


    //source
    string srcType = pt.get<string>("MCPlusPlus.MLSample.source.type", "__default__");
    if(src != NULL)
        delete src;
    if(srcType == "__default__") {
        src = new Source();
        AbstractDistribution *rDistr[3];
        rDistr[0] = distribution(pt.get<string>("MCPlusPlus.source.<xmlattr>.rx"));
        rDistr[1] = distribution(pt.get<string>("MCPlusPlus.source.<xmlattr>.ry"));
        rDistr[2] = distribution(pt.get<string>("MCPlusPlus.source.<xmlattr>.rz"));

        src->setr0Distribution(rDistr);

        AbstractDistribution *cosTheta = distribution(pt.get<string>("MCPlusPlus.source.<xmlattr>.cosTheta"));
        AbstractDistribution *psi = distribution(pt.get<string>("MCPlusPlus.source.<xmlattr>.psi"));

        src->setk0Distribution(cosTheta,psi);

        AbstractDistribution *walkTime = distribution(pt.get<string>("MCPlusPlus.source.<xmlattr>.walkTime"));
        src->setWalkTimeDistribution(walkTime);
    }

    //simulation
    if(sim!=NULL)
        delete sim;
    sim = new Simulation();
    sim->setSample(_sample);
    sim->setSource(src);

    try {
        ptree v = pt.get_child("MCPlusPlus.simulation");
        string str = v.get<string>("<xmlattr>.showTrajectory", "__default__");
        showTrajectory = boolean(str);
        sim->setSaveTrajectoryEnabled(boolean(str));
    } catch (ptree_bad_path) {
    }
}

bool XMLParser::showTrajectoryEnabled() const
{
    return showTrajectory;
}

Simulation *XMLParser::simulation() const
{
    return sim;
}

Sample *XMLParser::sample() const
{
    return _sample;
}

Material XMLParser::material(const string name) const
{
    if(name == "Air")
       return Air();
    else if(name == "Vacuum")
        return Vacuum();
    else if(name == "GlassSlide")
        return GlassSlide();
    else if(name == "NorlandOpticalAdhesive65")
        return NorlandOpticalAdhesive65();
    else
        return materialMap.at(name);
}

AbstractDistribution *XMLParser::distribution(const string name) const
{
    if(name == "uniform_0_2pi") {
        return new UniformDistribution(0,two_pi<MCfloat>());
    }
    else if (name == "uniform_0_pi") {
        return new UniformDistribution(0,pi<MCfloat>());
    }
    else
        return new DeltaDistribution(atof(name.c_str()));
}

bool XMLParser::boolean(const string str) const
{
    if(str == "true")
        return true;
    else if(str == "false")
        return false;
    else {
        struct InvalidBool e;
        e.str = str;
        throw e;
    };
}
