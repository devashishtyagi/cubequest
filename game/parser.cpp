/*
 * parser.cpp
 *
 *  Created on: Nov 9, 2011
 *      Author: devashish
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxml++/libxml++.h>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "structure.h"

using namespace std;
using namespace boost;




void getData(const xmlpp::Node* node, vector<string>& data1, vector<string>& data2, vector<string>& data3, vector<string>& data4, vector<string>& data5){
	const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
	const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);

	if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
	  	return;

	string nodename = node->get_name();

	if (const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node)){
		if (nodename == "wall"){

			const xmlpp::Attribute* attribute = nodeElement->get_attribute("min");
			data1.push_back(attribute->get_value());
			attribute = nodeElement->get_attribute("max");
			data1.push_back(attribute->get_value());
		}
		else if(nodename == "obstacle"){
			const xmlpp::Attribute* attribute = nodeElement->get_attribute("min");
			data2.push_back(attribute->get_value());
			attribute = nodeElement->get_attribute("max");
			data2.push_back(attribute->get_value());
			attribute = nodeElement->get_attribute("end");
			data2.push_back(attribute->get_value());
			attribute = nodeElement->get_attribute("d");
			data2.push_back(attribute->get_value());
		}
		else if(nodename == "hole"){
			const xmlpp::Attribute* attribute = nodeElement->get_attribute("pos");
			data3.push_back(attribute->get_value());
			attribute = nodeElement->get_attribute("radius");
			data3.push_back(attribute->get_value());
		}
		else if(nodename == "flame"){
			const xmlpp::Attribute* attribute = nodeElement->get_attribute("pos");
			data4.push_back(attribute->get_value());
		}
		else if(nodename == "particle"){
			const xmlpp::Attribute* attribute = nodeElement->get_attribute("pos");
			data5.push_back(attribute->get_value());
		}
	}

	if(!nodeContent)
  	{
    	//Recurse through child nodes:
    	xmlpp::Node::NodeList list = node->get_children();
    	for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
    	{
    	  getData(*iter, data1, data2, data3, data4, data5); //recursive
    	}
  	}

}

void readXML(char* filepath, vector<string>& data1, vector<string>& data2, vector<string>& data3, vector<string>& data4, vector<string>& data5){

	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
	try
	{
  		#endif //LIBXMLCPP_EXCEPTIONS_ENABLED
    	xmlpp::DomParser parser;
    	//parser.set_validate();
    	parser.set_substitute_entities();
    	parser.parse_file(filepath);
    	if(parser)
    	{
      		//Walk the tree:
      		const xmlpp::Node* pNode = parser.get_document()->get_root_node(); //deleted by DomParser.
      		getData(pNode, data1, data2, data3, data4, data5);
    	}
  	#ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  	}
  	catch(const std::exception& ex)
  	{
    	std::cout << "Exception caught: " << ex.what() << std::endl;
  	}
  	#endif //LIBXMLCPP_EXCEPTIONS_ENABLED

}

void getWallData(vector<string> data1, vector<wall>& wallData){
	int len = (int) data1.size();
	for(int i=0; i<len; i = i+2){
		location min;
		location max;
		vector<string> tokens, tokens2;
		split(tokens, data1.at(i), is_any_of("\t "));
		split(tokens2, data1.at(i+1), is_any_of("\t "));

		for(int j=0; j<3; j++){
			min.v[j] = strtod((tokens.at(j)).c_str(), NULL);
			max.v[j] = strtod((tokens2.at(j)).c_str(), NULL);
		}
		wall w(min, max);
		wallData.push_back(w);
	}
}

void getObsData(vector<string> data2, vector<obstacle>& obsData){
	int len = (int) data2.size();
	for(int i=0; i<len; i=i+4){
		location min, max, end;
		float d;
		vector<string> tokens, tokens2, tokens3;
		split(tokens, data2.at(i), is_any_of("\t "));
		split(tokens2, data2.at(i+1), is_any_of("\t "));
		split(tokens3, data2.at(i+2), is_any_of("\t "));

		d = strtod((data2.at(i+3)).c_str(), NULL);
		for(int j=0; j<3; j++){
			min.v[j] = strtod((tokens.at(j)).c_str(), NULL);
			max.v[j] = strtod((tokens2.at(j)).c_str(), NULL);
			end.v[j] = strtod((tokens3.at(j)).c_str(), NULL);
		}

		obstacle o(min, max, end, d);
		obsData.push_back(o);
	}
}

void getHoleData(vector<string> data, vector<holes>& holesData){
	int len = (int) data.size();
	for(int i=0; i<len; i=i+2){
		location pos;
		float radius;
		vector<string> tokens;
		split(tokens, data.at(i), is_any_of("\t "));

		radius = strtod((data.at(i+1)).c_str(), NULL);

		for(int j=0; j<3; j++)
			pos.v[j] = strtod((tokens.at(j)).c_str(), NULL);

		holes h(pos, radius);
		holesData.push_back(h);
	}
}

void getPowerData(vector<string> data, vector<location>& powerData){
	int len = (int) data.size();
	for(int i=0; i<len; i=i++){
		location pos;
		vector<string> tokens;
		split(tokens, data.at(i), is_any_of("\t "));

		for(int j=0; j<3; j++)
			pos.v[j] = strtod((tokens.at(j)).c_str(), NULL);

		powerData.push_back(pos);
	}
}

void getFlameData(vector<string> data, vector<location>& flameData){
	int len = (int) data.size();
	for(int i=0; i<len; i=i++){
		location pos;
		vector<string> tokens;
		split(tokens, data.at(i), is_any_of("\t "));

		for(int j=0; j<3; j++)
			pos.v[j] = strtod((tokens.at(j)).c_str(), NULL);

		flameData.push_back(pos);
	}
}



