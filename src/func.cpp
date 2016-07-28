/*
 * func.cpp
 *
 *  Created on: Jun 1, 2016
 *      Author: unslur
 */

#include "func.h"

namespace Ttofun {


 char* func::getXmlValue(xmlDocPtr tmpdoc,string name){
	xmlNodePtr curNode;
	xmlChar *szKey;
	curNode=xmlDocGetRootElement(tmpdoc);
	if(NULL==curNode){
		xmlFreeDoc(tmpdoc);
		return NULL;
	}

	if(xmlStrcmp(curNode->name,BAD_CAST"cpc")){
		xmlFreeDoc(tmpdoc);
				return NULL;
	}
	curNode=curNode->xmlChildrenNode;
	//xmlNodePtr propnpdePtr=curNode;
	while(curNode!=NULL){
			if(!xmlStrcmp(curNode->name,(const xmlChar*)"operadata")){
				curNode=curNode->xmlChildrenNode;
				while(curNode!=NULL){
					if(!xmlStrcmp(curNode->name,(const xmlChar*)"prodcpccode")){
						szKey=xmlNodeGetContent(curNode);
						return (char*)szKey;
					}
					curNode=curNode->next;
				}
			}

			curNode=curNode->next;
	}
	xmlFreeDoc(tmpdoc);
	return NULL;
};
 size_t func::OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
 {
     std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
     if( NULL == str || NULL == buffer )
     {
         return -1;
     }

     char* pData = (char*)buffer;
     str->append(pData, size * nmemb);
     return nmemb;
 }
 int func::post(const string &strUrl,const string &strPost,string &strResponse){
 	CURLcode res;
 	    CURL* curl = curl_easy_init();
 	    if(NULL == curl)
 	    {
 	        return CURLE_FAILED_INIT;
 	    }
 //	    if(m_bDebug)
 //	    {
 //	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
 //	        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
 //	    }
 	    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
 	    curl_easy_setopt(curl, CURLOPT_POST, 1);
 	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
 	    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
 	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
 	    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
 	    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
 	    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
 	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
 	    res = curl_easy_perform(curl);
 	    curl_easy_cleanup(curl);
 	    return res;
 }
 string func::prodcode(string username,string userpwd,string company_cpc,string tablet_name){
 	string url="http://platform.ecpc.org.cn/CPCNEW/trans/prodinfo.htm";
 	string rtn="";
 		stringstream sendss;

 			sendss<<"xmlname="<<"<?xml version=\"1.0\" encoding=\"utf-8\"?>"<<
 					"<cpc>"<<
 					"<operatetype>prodinfo</operatetype>"<<
 					"<operatecode>insert</operatecode>"<<
 				"<operatename>"<<username<<"</operatename>"<<
 				"<operatepwd>"<<userpwd<<"</operatepwd>"<<
 				"<operadata>"<<
 					"<corpcpccode>"<<company_cpc<<"</corpcpccode>"<<
 					"<prodname>"<<tablet_name<<"</prodname>"<<
 					"<prodenname></prodenname>"<<
 					"<prodintro></prodintro>"<<
 					"<prodsize></prodsize>"<<
 					"<prodcountry></prodcountry>"<<
 					"<submitdate></submitdate>"<<
 					"<updatedate></updatedate>"<<
 				"</operadata>"<<
 				"</cpc>"
 				;
 			string Post=sendss.str();
 				int n=0;

 				n=post(url,Post,rtn);

 				//string testrtn="<?xml version=\"1.0\" encoding=\"UTF-8\"?><cpc><respcode>0000</respcode><respmsg>成功</respmsg><operatetype>prodinfo</operatetype><operatecode>insert</operatecode><operadata><corpcpccode>10127300628871</corpcpccode><prodcpccode>10127300628871000095</prodcpccode><prodcountry></prodcountry><prodintro></prodintro><prodsize></prodsize><prodenname></prodenname><prodname>川芎</prodname><submitdate>2016-06-01 15:01:59</submitdate><updatedate>2016-06-01 15:01:59</updatedate></operadata></cpc>";
 				//cout<<testrtn<<endl;
 				//rtn=testrtn;

 				if(rtn==""){
 					rtn="get goodcode failed\n";
 					cout<<rtn<<endl;
 					return rtn;
 				}else{
 					xmlDocPtr doc =xmlParseMemory(rtn.c_str(),rtn.length()+1);
 					char*key=NULL;

 					key=getXmlValue(doc,"corpcpccode");
 					rtn=key;
 					xmlFree(key);
 					return rtn;
 				}
 }
}/* namespace TtoDB */
