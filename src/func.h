/*
 * func.h
 *
 *  Created on: Jun 1, 2016
 *      Author: unslur
 */

#ifndef FUNC_H_
#define FUNC_H_
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
using namespace std;
#include <iostream>
#include <string>
#include <sstream>
#include <curl/curl.h>
namespace Ttofun{

class func {


public:
	static char*  getXmlValue(xmlDocPtr tmpdoc,string name);
	static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int post(const string &strUrl,const string &strPost,string &strResponse);
	static string prodcode(string username,string userpwd,string company_cpc,string tablet_name);
};
}
 /* namespace TtoDB */

#endif /* FUNC_H_ */
