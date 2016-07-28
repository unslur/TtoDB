//============================================================================
// Name        : TtoDB.cpp
// Author      : cy
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <mysql/mysql.h>
#include "xmysql.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <zlib.h>
#include <list>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>
#include <math.h>
#include <curl/curl.h>
//#include <libxml/parser.h>
//#include <libxml/xmlmemory.h>
#include "config.h"
#include "local.h"
#include "utils.h"
#include "md5.h"
#include "func.h"
//#define db_host "192.168.8.12"
//#define db_name "cheng"
//#define db_user "tcm"

//#define db_pass "tcm123"
using namespace std;
list<int> clients_list;
using EncapMysql::CEncapMysql;
using namespace Ttofun;
char gftuser[32]={0};
char gftpass[32]={0};
char db_host[32]={0};
char db_user[32]={0};
char db_pass[32]={0};
int serverport=0;
func f;
int getCheckCode(string codeStr);


//static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
//{
//    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
//    if( NULL == str || NULL == buffer )
//    {
//        return -1;
//    }
//
//    char* pData = (char*)buffer;
//    str->append(pData, size * nmemb);
//    return nmemb;
//}
//int post(const string &strUrl,const string &strPost,string &strResponse){
//	CURLcode res;
//	    CURL* curl = curl_easy_init();
//	    if(NULL == curl)
//	    {
//	        return CURLE_FAILED_INIT;
//	    }
////	    if(m_bDebug)
////	    {
////	        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
////	        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
////	    }
//	    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
//	    curl_easy_setopt(curl, CURLOPT_POST, 1);
//	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
//	    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
//	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
//	    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
//	    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
//	    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
//	    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
//	    res = curl_easy_perform(curl);
//	    curl_easy_cleanup(curl);
//	    return res;
//}
bool progress(int client,char* recv_buf)
{
		cout<<recv_buf<<endl;
		string sendstr;
		string srecv=recv_buf;
		string method=srecv.substr(0,srecv.find(","));
		srecv=srecv.substr(srecv.find(",")+1);
		if(!method.compare("login"))
		{
			string user=srecv.substr(0,srecv.find(","));
					string pass=srecv.substr(srecv.find(",")+1);

				CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					try {
						con->ModifyQuery("use tcmbpspdb");
					} catch (exception &e) {
						con->CloseConnect();
					}
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"select a.company_code,a.company_cpc,b.areacode ,a.user_code,b.company_name ,a.user_name from tcm_user a,tcm_company b where a.user_loginname='%s' and a.user_loginpass='%s' and a.user_state=1 and a.company_code=b.company_code",user.c_str(),pass.c_str());
					//cout<<sqlbuf<<endl;
					try{
						con->SelectQuery(sqlbuf);
					}
					catch(exception& e){
						printf("exception\n");
						CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
						con->CloseConnect();
						 return false;
					};
						char** r=NULL;
						if(  (r=con->FetchRow())){//right
//							char info[100]={0};
//							sprintf(info,"%s,%s,%s",r[0],r[1],user.c_str());
//							sendstr=info;

							stringstream out;
							out<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<user;
							sendstr=out.str();
							//cout<<sendstr<<endl;
							CHK(send(client, sendstr.c_str(),sendstr.length(), 0));
							con->CloseConnect();
							return true;
						}
						else{
							CHK(send(client, "NO", strlen("NO"), 0));
							con->CloseConnect();
							return true;
						}

		}
		else if(!method.compare("fenbao")){
			printf("\n");
			printf("================= sub start=========\n");
			CEncapMysql *con;
			con = new CEncapMysql;
			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			try{
			string totalnum=srecv.substr(0,srecv.find(","));// small box total number
			srecv=srecv.substr(srecv.find(",")+1);
			string boxitennum=srecv.substr(0,srecv.find(","));// small  box number per middle box
			srecv=srecv.substr(srecv.find(",")+1);
			string boxnum=srecv.substr(0,srecv.find(","));//middle box number
			srecv=srecv.substr(srecv.find(",")+1);
			string weight=srecv.substr(0,srecv.find(","));//small box weight
			srecv=srecv.substr(srecv.find(",")+1);
			string cpccorp=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string cpctable=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string loginname=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string tabletname=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string prodcode=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string areacode=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string spec=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string printtask_code=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string box_year=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string box_month=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string tablet_batchnumber=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string company_code=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string user_code=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string tablet_code=srecv;


			try {
				con->ModifyQuery("use tcmbpspdb");
			} catch (exception &e) {
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			}
			char sqlbuf[0x800]={0};
			//sprintf(sqlbuf,"SELECT	erpdb.erp_user.user_loginname,	erpdb.erp_user.user_loginpass,	erpdb.erp_boxtask.tabletboxitemnum,  erpdb.erp_boxtask.tabletboxnum,	erpdb.erp_boxtask.printstate FROM	erpdb.erp_boxtask INNER JOIN erpdb.erp_user ON erpdb.erp_boxtask.orgid = erpdb.erp_user.orgid WHERE	erpdb.erp_boxtask.boxtaskcode = '%s';",printtask.c_str());
			sprintf(sqlbuf,"call PROC_SAVETABLETSMALLBOX(%d,%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,%d,'%s','%s',NOW());",
					atoi(totalnum.c_str()),atoi(boxitennum.c_str()),atoi(boxnum.c_str()),
					weight.c_str(),cpccorp.c_str(),cpctable.c_str(),loginname.c_str(),
					tabletname.c_str(),prodcode.c_str(),areacode.c_str(),spec.c_str(),
					printtask_code.c_str(),atoi(box_year.c_str()),atoi(box_month.c_str()),
					tablet_code.c_str(),user_code.c_str());

				int rtn=0;
				try {
					rtn=con->ModifyQuery(sqlbuf);
				} catch (exception &e) {
					printf("exception\n");
					CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
					con->CloseConnect();
					return false;
				}
				if(rtn==0){
					sprintf(sqlbuf,"call ok");

				}else{
					sprintf(sqlbuf,"call failed %s\n",con->GetErrMsg());
					cout<<sqlbuf<<endl;
					CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
					con->CloseConnect();
					return false;
				}
				cout<<sqlbuf<<endl;
				memset(sqlbuf,0,strlen(sqlbuf));
				//cout<<"ss"<<endl;
				//printf("insert into tcm_printtask values('%s','%s','%s',%d,'%ld','%ld',%d,'%d','%d',%d,%d,%d,%d,%d,'%s','%s','%s','%s',%d,%d)",printtask_code.c_str(),tabletname.c_str(),tablet_batchnumber.c_str(),0,NULL,NULL,atoi(totalnum.c_str()),(atoi(totalnum.c_str())*atoi(weight.c_str())),atoi(weight.c_str()),0,0,2,1,2,company_code.c_str(),user_code.c_str(),cpctable.c_str(),atoi(box_year.c_str()),atoi(box_month.c_str()));
				sprintf(sqlbuf,"insert into tcm_printtask(printtask_code,printtask_name,printtask_batchnumber,printtask_tablet_totalnum,printtask_tablet_totalweight,printtask_tablet_numweight,printtask_type,printtask_state,printtask_packtype,company_code,user_code,tablet_cpc,printtask_year,printtask_month,addtime) values('%s','%s','%s',%d,'%d','%d',%d,%d,%d,'%s','%s','%s',%d,%d,NOW())",
						printtask_code.c_str(),tabletname.c_str(),tablet_batchnumber.c_str(),
						atoi(totalnum.c_str()),(atoi(totalnum.c_str())*atoi(weight.c_str())),
						atoi(weight.c_str()),2,1,2,company_code.c_str(),user_code.c_str(),
						cpctable.c_str(),atoi(box_year.c_str()),atoi(box_month.c_str()));
				cout<<sqlbuf<<endl;
				try {
					rtn=con->ModifyQuery(sqlbuf);
				} catch (exception &e) {
					printf("exception\n");
											CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
											con->CloseConnect();
											 return false;
				}

				if(rtn==0){
					sprintf(sqlbuf,"OK");
				}else{
					sprintf(sqlbuf,"call failed %s\n",con->GetErrMsg());
					cout<<sqlbuf<<endl;
					CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
					return false;
				}

				sprintf(sqlbuf,"insert into tcm_tabletmiddle(tablet_code,other_code,other_type) values('%s','%s','tcm_printcode')",tablet_code.c_str(),printtask_code.c_str());
				cout<<sqlbuf<<endl;

				try{
					rtn=con->ModifyQuery(sqlbuf);
				}

				catch(exception& e){
					printf("exception\n");
											CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
											con->CloseConnect();
											 return false;
				};
				int totalweight=0;
				try {
					totalweight=atoi(totalnum.c_str())*atoi(weight.c_str());
				} catch (exception &e) {
					printf("ss\n");
					totalweight=0;
				}
				sprintf(sqlbuf,"update tcm_tablet SET tablet_weight_present = CONVERT( CONVERT (	tablet_weight_present,	signed) - %d,CHAR)  where tablet_code='%s'",totalweight,tablet_code.c_str());
				cout<<sqlbuf<<endl;

				try{
					rtn=con->ModifyQuery(sqlbuf);
				}

				catch(exception& e){
					printf("exception\n");
											CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
											con->CloseConnect();
											 return false;
				};
				if(rtn==0){
				sprintf(sqlbuf,"OK");
				}else{
					sprintf(sqlbuf,"call failed %s\n",con->GetErrMsg());
					cout<<sqlbuf<<endl;
				}
				CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
				con->CloseConnect();
			}
			catch(exception& e){
				cout<<e.what()<<endl;
				CHK(send(client, "error:319", strlen("error:319"), 0));
				con->CloseConnect();
			}
			printf("================== sub end===========\n");

		}
		else if(!method.compare("fenbaomiddle")){
					printf("\n");
					printf("================= sub start=========\n");
					CEncapMysql *con;
					con = new CEncapMysql;
					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					try{
					string totalnum=srecv.substr(0,srecv.find(","));// small box total number
					srecv=srecv.substr(srecv.find(",")+1);
					string boxitennum=srecv.substr(0,srecv.find(","));// small  box number per middle box
					srecv=srecv.substr(srecv.find(",")+1);
					string boxnum=srecv.substr(0,srecv.find(","));//middle box number
					srecv=srecv.substr(srecv.find(",")+1);
					string boxweightall=srecv.substr(0,srecv.find(","));//middle weight
					srecv=srecv.substr(srecv.find(",")+1);
					string weight=srecv.substr(0,srecv.find(","));//small box weight
					srecv=srecv.substr(srecv.find(",")+1);
					string cpccorp=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string cpctable=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string loginname=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string tabletname=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string prodcode=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string areacode=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string spec=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string printtask_code=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string box_year=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string box_month=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string tablet_batchnumber=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string company_code=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string user_code=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string tablet_code=srecv;


					con->ModifyQuery("use tcmbpspdb");
					char sqlbuf[0x800]={0};
					//sprintf(sqlbuf,"SELECT	erpdb.erp_user.user_loginname,	erpdb.erp_user.user_loginpass,	erpdb.erp_boxtask.tabletboxitemnum,  erpdb.erp_boxtask.tabletboxnum,	erpdb.erp_boxtask.printstate FROM	erpdb.erp_boxtask INNER JOIN erpdb.erp_user ON erpdb.erp_boxtask.orgid = erpdb.erp_user.orgid WHERE	erpdb.erp_boxtask.boxtaskcode = '%s';",printtask.c_str());
					sprintf(sqlbuf,"call PROC_SAVETABLETMEDIUMBOX(%d,%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,%d,'%s','%s',NOW());",
							atoi(totalnum.c_str()),atoi(boxitennum.c_str()),atoi(boxnum.c_str()),boxweightall.c_str(),
							weight.c_str(),cpccorp.c_str(),cpctable.c_str(),loginname.c_str(),
							tabletname.c_str(),prodcode.c_str(),areacode.c_str(),spec.c_str(),
							printtask_code.c_str(),atoi(box_year.c_str()),atoi(box_month.c_str()),
							tablet_code.c_str(),user_code.c_str());

						int rtn=0;
						try {
							rtn=con->ModifyQuery(sqlbuf);
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						if(rtn==0){
							sprintf(sqlbuf,"call ok");

						}else{
							sprintf(sqlbuf,"call failed %s\n",con->GetErrMsg());
							cout<<sqlbuf<<endl;
							CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
							con->CloseConnect();
							return false;
						}
						cout<<sqlbuf<<endl;
						memset(sqlbuf,0,strlen(sqlbuf));
						//cout<<"ss"<<endl;
						//printf("insert into tcm_printtask values('%s','%s','%s',%d,'%ld','%ld',%d,'%d','%d',%d,%d,%d,%d,%d,'%s','%s','%s','%s',%d,%d)",printtask_code.c_str(),tabletname.c_str(),tablet_batchnumber.c_str(),0,NULL,NULL,atoi(totalnum.c_str()),(atoi(totalnum.c_str())*atoi(weight.c_str())),atoi(weight.c_str()),0,0,2,1,2,company_code.c_str(),user_code.c_str(),cpctable.c_str(),atoi(box_year.c_str()),atoi(box_month.c_str()));
						sprintf(sqlbuf,"insert into tcm_printtask(printtask_code,printtask_name,printtask_batchnumber,printtask_tablet_totalnum,printtask_tablet_totalweight,printtask_tablet_numweight,printtask_tablet_mediumnum,printtask_tablet_mediumsubnum,printtask_type,printtask_state,printtask_packtype,company_code,user_code,tablet_cpc,printtask_year,printtask_month,addtime) values('%s','%s','%s',%d,'%d','%d',%s,%s,%d,%d,%d,'%s','%s','%s',%d,%d,NOW())",
								printtask_code.c_str(),tabletname.c_str(),tablet_batchnumber.c_str(),
								atoi(totalnum.c_str()),(atoi(totalnum.c_str())*atoi(weight.c_str())),
								atoi(weight.c_str()),boxnum.c_str(),boxitennum.c_str(), 2,1,1,company_code.c_str(),user_code.c_str(),
								cpctable.c_str(),atoi(box_year.c_str()),atoi(box_month.c_str()));
						cout<<sqlbuf<<endl;
						try {
							rtn=con->ModifyQuery(sqlbuf);
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}

						if(rtn==0){
							sprintf(sqlbuf,"OK");
						}else{
							sprintf(sqlbuf,"call failed %s\n",con->GetErrMsg());
							cout<<sqlbuf<<endl;
							CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
							return false;
						}

						sprintf(sqlbuf,"insert into tcm_tabletmiddle(tablet_code,other_code,other_type) values('%s','%s','tcm_printcode')",tablet_code.c_str(),printtask_code.c_str());
						cout<<sqlbuf<<endl;

						try{
							rtn=con->ModifyQuery(sqlbuf);
						}

						catch(exception& e){
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						};
						int totalweight=0;
						try {
							totalweight=atoi(totalnum.c_str())*atoi(weight.c_str());
						} catch (exception &e) {
							printf("ss\n");
							totalweight=0;
						}
						sprintf(sqlbuf,"update tcm_tablet SET tablet_weight_present = CONVERT( CONVERT (	tablet_weight_present,	signed) - %d,CHAR)  where tablet_code='%s'",totalweight,tablet_code.c_str());
						cout<<sqlbuf<<endl;

						try{
							rtn=con->ModifyQuery(sqlbuf);
						}

						catch(exception& e){
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						};
						if(rtn==0){
											sprintf(sqlbuf,"OK");
						}else{
							sprintf(sqlbuf,"call failed %s\n",con->GetErrMsg());
							cout<<sqlbuf<<endl;
						}
						CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
						con->CloseConnect();
					}
					catch(exception& e){
						cout<<e.what()<<endl;
						CHK(send(client, "error:319", strlen("error:319"), 0));
						con->CloseConnect();
					}
					printf("================== sub end===========\n");

				}
		else if(!method.compare("downcode")){
				printf("\n");
				printf("================= send start=========\n");
				string md5_de=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string time=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string printtask=srecv.substr(0,srecv.find(","));
					cout<<"Requset printtask:"+printtask<<"\n";//2081457405516177426
					if(printtask.size()<18)//任务格式不对
					{
						CHK(send(client, "error:302", strlen("error:302"), 0));
						printf("================= send 302=========\n");
								 return false;
					}

					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 printf("================= send 303=========\n");
						 return false;
					}

					char sqlbuf[0x800]={0};
					//sprintf(sqlbuf,"SELECT	erpdb.erp_user.user_loginname,	erpdb.erp_user.user_loginpass,	erpdb.erp_boxtask.tabletboxitemnum,  erpdb.erp_boxtask.tabletboxnum,	erpdb.erp_boxtask.printstate FROM	erpdb.erp_boxtask INNER JOIN erpdb.erp_user ON erpdb.erp_boxtask.orgid = erpdb.erp_user.orgid WHERE	erpdb.erp_boxtask.boxtaskcode = '%s';",printtask.c_str());
					sprintf(sqlbuf,"select tcmbpspdb.tcm_user.user_loginname,tcmbpspdb.tcm_user.user_loginpass,tcmbpspdb.tcm_printtask.printtask_type ,tcmbpspdb.tcm_printtask.printtask_packtype,tcmbpspdb.tcm_printtask.printtask_state from tcmbpspdb.tcm_printtask  inner join tcmbpspdb.tcm_user  on tcmbpspdb.tcm_printtask.user_code=tcmbpspdb.tcm_user.user_code where tcmbpspdb.tcm_printtask.printtask_code='%s';",printtask.c_str());
					try{
						con->SelectQuery(sqlbuf);
					}
					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};

					char** r=NULL;
					if(!( r= con->FetchRow())){//mysql数据库无该任务
						CHK(send(client, "error:304", strlen("error:304"), 0));
						printf("================= send 304=========\n");
						return true;
					}
					if(!r[0]||!r[1]||!r[2]||!r[3]){
							CHK(send(client, "error:318", strlen("error:318"), 0));
							printf("================= send 318=========\n");
									return true;
						}
					if(!strstr(r[4],"1")){//数据已下载到本地sqlite
						CHK(send(client, "error:305", strlen("error:305"), 0));
						printf("================= send 305=========\n");
						return true;
					}
					char tmp[100]={0};
					sprintf(tmp,"%s%s%s",r[0],r[1],time.c_str());
					string md5_en=tmp;
					cout<<md5_en<<endl;
					unsigned char decrypt[17]={0};
					MD5_CTX md5;
					MD5Init(&md5);
					MD5Update(&md5,(unsigned char *)md5_en.c_str(),md5_en.length());
					MD5Final(&md5,decrypt);
					char decrypt_tmp[33]={0};
					char *x=decrypt_tmp;
					for(int i=0;i<16;i++)
					{
						sprintf(x,"%02x",decrypt[i]);x+=2;
					}
					cout<<decrypt_tmp<<endl;
					if(strcmp(decrypt_tmp,md5_de.c_str())!=0){//账号密码认证失败
					   CHK(send(client, "error:306", strlen("error:306"), 0));
					   printf("================= send 306=========\n");
					   return true;
					}

					list<string> vstr;
					unsigned char buf[0x400*0x400]={0};
					//unsigned char buf_tmp[0x400*0x400];
					unsigned char *p=buf;
					 int len;
					unsigned long len_src=0;
					//unsigned long len_tmp=0x400*0x400;
					if((strstr(r[2],"2")&&strstr(r[3],"1")))//middle
					{
						cout<<"mid"<<endl;
						sprintf(sqlbuf,"select distinct tcmbpspdb.tcm_box.box_code from tcmbpspdb.tcm_box  where tcmbpspdb.tcm_box.printtask_code='%s'",printtask.c_str());

					try {
						con->SelectQuery(sqlbuf);
					} catch (exception &e) {
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					}
					   while (char** r = con->FetchRow())
						{
							   vstr.push_back(r[0]);
							  // printf("%s\n",r[0]);
						}
					   if(vstr.size()==0){//任务中无中包溯源码
						   CHK(send(client, "error:307", strlen("error:307"), 0));
						   printf("================= send 307=========\n");
						   return true;
					   }
					   for ( list<string>::iterator iter = vstr.begin();iter != vstr.end(); ++iter )
					   {
						   sprintf(sqlbuf,"select tcmbpspdb.tcm_box.box_sourcecode from tcmbpspdb.tcm_box  where tcmbpspdb.tcm_box.box_code='%s'",iter->c_str());

						try {
							con->SelectQuery(sqlbuf);
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						   sprintf((char*)p,"%s",iter->c_str());
						   len=strlen((char*)p);
						   	p+=len;
							while (char** r = con->FetchRow())
							{
								sprintf((char*)p,",%s", r[0]);
								len=strlen((char*)p);
								p+=len;
							}
							sprintf((char*)p,"|");p+=1;
					   }
						len_src=strlen((char*)buf);
						//cout<<buf<<endl;
						//compress(buf_tmp,&len_tmp,buf,len_src);
						//memset(buf,0,0x400*0x400);
						CHK(send(client, buf, len_src, 0));
						//memset(buf_tmp,0,0x400*0x400);
					}
					else if((strstr(r[2],"2")&&strstr(r[3],"2"))||strstr(r[2],"1"))
					{//yaocai or xiaobao
						cout<<"other"<<endl;
					//
						 sprintf(sqlbuf,"select tcmbpspdb.tcm_box.box_sourcecode from tcmbpspdb.tcm_box  where tcmbpspdb.tcm_box.printtask_code='%s'",printtask.c_str());
						try {
							con->SelectQuery(sqlbuf);
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						sprintf((char*)p,"no");
					   len=strlen((char*)p);
						p+=len;
						while (char** r = con->FetchRow())
						{
							sprintf((char*)p,",%s", r[0]);
							len=strlen((char*)p);
							p+=len;
						}
						if(len<5){//任务中无溯源码
							CHK(send(client, "error:308", strlen("error:308"), 0));
							printf("================= send 308=========\n");
							return true;
						}
						sprintf((char*)p,"|");p+=1;

						len_src=strlen((char*)buf);
//						compress(buf_tmp,&len_tmp,buf,len_src);
//						memset(buf,0,0x400*0x400);
//						CHK(send(client, buf_tmp, len_tmp, 0));
						CHK(send(client, buf, len_src, 0));
						//memset(buf_tmp,0,0x400*0x400);
					
					}
					sprintf(sqlbuf,"update tcmbpspdb.tcm_printtask set printtask_state=2 where printtask_code=%s",printtask.c_str());
					con->ModifyQuery(sqlbuf);
					con->CloseConnect();
					delete(con);
					printf("================= send end=========\n");
					fflush(stdout);
		}
		else if(!method.compare("getyinpianinfo")){
			string user_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string page=srecv.substr(0,srecv.find(","));
			string condition=srecv.substr(srecv.find(",")+1);

			cout<<"==============Get Tablet info start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x200]={0};
//			if(condition.length()<1){
//				sprintf(sqlbuf,"select a.tablet_code,a.tablet_cpc,a.prodcode,a.tablet_name,a.tablet_batchnumber,a.tablet_spec,a.tablet_weight ,a.addtime  from tcm_tablet a where tablet_weight_present !='0' and a.user_code=%s order by addtime desc limit %d,10",user_code.c_str(),atoi(page.c_str()));
//			}else
			{
				sprintf(sqlbuf,"select a.tablet_code,a.tablet_cpc,a.prodcode,a.tablet_name,a.tablet_batchnumber,a.tablet_spec,a.tablet_weight ,a.addtime  from tcm_tablet a where tablet_name like '%%%s%%'  and a.user_code=%s order by addtime desc limit %d,10",condition.c_str(), user_code.c_str(),atoi(page.c_str()));
			}try{
				con->SelectQuery(sqlbuf);
			}
			catch(exception& e){
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			};
				char** r=NULL;
				cout<<sqlbuf<<endl;
				//unsigned char buf[0x400*0x400]={0};
				stringstream sendss;
				while((r=con->FetchRow())){
					sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<"|";
				}
				string buf=sendss.str();
				if(buf.length()<10){
					buf="get info error";
				}
				cout<<buf<<endl;
				CHK(send(client, buf.c_str(), buf.length(), 0));
				con->CloseConnect();
				cout<<"===============Get Tablet info end============="<<endl;
		}
		else if(!method.compare("getyinpianfenbao")){
					string user_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string page=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string condition=srecv;
					cout<<"==============Get Tablet info start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					con->ModifyQuery("use tcmbpspdb");
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"select a.tablet_code,a.tablet_cpc,a.prodcode,a.tablet_name,a.tablet_batchnumber,a.tablet_spec,a.tablet_weight ,a.tablet_weight_present,a.addtime  from tcm_tablet a JOIN tcm_tabletmiddle b ON a.tablet_code =b.tablet_code where tablet_weight_present !='0' and a.tablet_name like '%%%s%%' and b.other_type='tcm_check' and a.user_code=%s order by addtime desc limit %d,10",condition.c_str(), user_code.c_str(),atoi(page.c_str()));
					try{
						con->SelectQuery(sqlbuf);
					}
					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
						char** r=NULL;
						//unsigned char buf[0x400*0x400]={0};
						stringstream sendss;
						while((r=con->FetchRow())){
							sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<","<<r[8]<<"|";
						}
						string buf=sendss.str();
						if(buf.length()<10){
							buf="get info error";
						}
						cout<<buf<<endl;
						CHK(send(client, buf.c_str(), buf.length(), 0));
						con->CloseConnect();
						cout<<"===============Get Tablet info end============="<<endl;
				}
		else if(!method.compare("getaddressinfo")){

					string pcode=srecv;
					cout<<"==============Get address info start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					con->ModifyQuery("use tcmbpspdb");
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"select code ,name from tcm_codearea where pcode='%s'",pcode.c_str());
					try{
						con->SelectQuery(sqlbuf);
					}
					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
						char** r=NULL;
						//unsigned char buf[0x400*0x400]={0};
						stringstream sendss;
						while((r=con->FetchRow())){
							sendss<<r[0]<<","<<r[1]<<"|";
						}
						string buf=sendss.str();
						if(buf.length()<5){
							buf="get info error";
						}
						cout<<buf<<endl;
						CHK(send(client, buf.c_str(), buf.length(), 0));
						con->CloseConnect();
						cout<<"===============Get address info end============="<<endl;
				}
		else if(!method.compare("getaddress")){
			string info=srecv.substr(0,srecv.find(","));;
								srecv=srecv.substr(srecv.find(",")+1);
							string table=srecv;
							cout<<"==============Get address info start=================="<<endl;
							CEncapMysql *con;
							con = new CEncapMysql;

							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							con->ModifyQuery("use tcmbpspdb");
							char sqlbuf[0x200]={0};
							sprintf(sqlbuf,"SELECT	mi.province,	mi.city,	mi.area,	mi.address FROM	tcm_%sinfo mi WHERE	mi.%sinfo_code='%s'",table.c_str(),table.c_str(),info.c_str());
							try{
								con->SelectQuery(sqlbuf);
							}
							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
								char** r=NULL;
								//unsigned char buf[0x400*0x400]={0};
								stringstream sendss;
								if((r=con->FetchRow())!=NULL){

									sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3];
								}
								string buf=sendss.str();
								if(buf.length()<5){
									buf="get info error";
								}
								cout<<buf<<endl;
								CHK(send(client, buf.c_str(), buf.length(), 0));
								con->CloseConnect();
								cout<<"===============Get address info end============="<<endl;
						}
		else if(!method.compare("getaddressinfoname")){

							string pcode=srecv;
							cout<<"==============Get address info start=================="<<endl;
							CEncapMysql *con;
							con = new CEncapMysql;

							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							con->ModifyQuery("use tcmbpspdb");
							char sqlbuf[0x200]={0};
							sprintf(sqlbuf,"select pcode ,shortname from tcm_codearea where name='%s'",pcode.c_str());
							try{
								con->SelectQuery(sqlbuf);
							}
							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
								char** r=NULL;
								//unsigned char buf[0x400*0x400]={0};
								stringstream sendss;
								if((r=con->FetchRow())){
									sendss<<r[0]<<","<<r[1];
								}
								string buf=sendss.str();
								if(buf.length()<5){
									buf="get info error";
								}
								cout<<buf<<endl;
								CHK(send(client, buf.c_str(), buf.length(), 0));
								con->CloseConnect();
								cout<<"===============Get address info end============="<<endl;
						}
		else if(!method.compare("getaddressinfoshortname")){

									string pcode=srecv;
									cout<<"==============Get address info start=================="<<endl;
									CEncapMysql *con;
									con = new CEncapMysql;

									if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
									{
										 CHK(send(client, "error:303", strlen("error:303"), 0));
										 return false;
									}
									con->ModifyQuery("use tcmbpspdb");
									char sqlbuf[0x200]={0};
									sprintf(sqlbuf,"select code from tcm_codearea where name='%s'",pcode.c_str());
									try{
										con->SelectQuery(sqlbuf);
									}
									catch(exception& e){
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									};
										char** r=NULL;
										//unsigned char buf[0x400*0x400]={0};
										stringstream sendss;
										if((r=con->FetchRow())){
											sendss<<r[0];
										}
										string buf=sendss.str();
										if(buf.length()<0){
											buf="get info error";
										}
										cout<<buf<<endl;
										CHK(send(client, buf.c_str(), buf.length(), 0));
										con->CloseConnect();
										cout<<"===============Get address info end============="<<endl;
								}
		else if(!method.compare("updateyinpianbase")){
					string tablet_code=srecv.substr(0,srecv.find(","));
								srecv=srecv.substr(srecv.find(",")+1);
								string batnumber=srecv.substr(0,srecv.find(","));
								srecv=srecv.substr(srecv.find(",")+1);
								string spec=srecv;

					cout<<"==============Update yinpian info start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					con->ModifyQuery("use tcmbpspdb");
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"update tcm_tablet set tablet_batchnumber='%s',tablet_spec='%s' where tablet_code='%s'",batnumber.c_str(),spec.c_str(),tablet_code.c_str());
					cout<<sqlbuf<<endl;
					int rtn=0;
					try {
						rtn=con->ModifyQuery(sqlbuf);
					} catch (exception &e) {
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					}
					if(rtn==0){
						sprintf(sqlbuf,"update ok");

					}else{
						sprintf(sqlbuf,"update failed %s\n",con->GetErrMsg());
						cout<<sqlbuf<<endl;
					}

					CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
						con->CloseConnect();
						cout<<"===============Update yinpian info end============="<<endl;
				}
		else if(!method.compare("updateyinpianmore")){
							string tabletinfo_code=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string arg1=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string arg2=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string arg3=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string province=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string city=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string area=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string address=srecv;

							cout<<"==============Update yinpian info start=================="<<endl;
							CEncapMysql *con;
							con = new CEncapMysql;

							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							con->ModifyQuery("use tcmbpspdb");
							char sqlbuf[0x200]={0};
							sprintf(sqlbuf,"update tcm_tabletinfo set tabletinfo_execstandard='%s',tabletinfo_approval='%s',tabletinfo_validity='%s',province='%s',city='%s',area='%s',address='%s' where tabletinfo_code='%s'",arg1.c_str(),arg2.c_str(),arg3.c_str(),province.c_str(),city.c_str(),area.c_str(),address.c_str(), tabletinfo_code.c_str());
							cout<<sqlbuf<<endl;
							int rtn=0;
							try {
								rtn=con->ModifyQuery(sqlbuf);
							} catch (exception &e) {
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							}
							if(rtn==0){
								sprintf(sqlbuf,"update ok");

							}else{
								sprintf(sqlbuf,"update failed %s\n",con->GetErrMsg());
								cout<<sqlbuf<<endl;
							}

							CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
								con->CloseConnect();
								cout<<"===============Update yinpian info end============="<<endl;
						}
		else if(!method.compare("updateyinpiadetect")){
									string tabletinfo_code=srecv.substr(0,srecv.find(","));
												srecv=srecv.substr(srecv.find(",")+1);
												string arg1=srecv.substr(0,srecv.find(","));
												srecv=srecv.substr(srecv.find(",")+1);
												string arg2=srecv.substr(0,srecv.find(","));
												srecv=srecv.substr(srecv.find(",")+1);
												string arg3=srecv;

									cout<<"==============Update yinpian info start=================="<<endl;
									CEncapMysql *con;
									con = new CEncapMysql;

									if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
									{
										 CHK(send(client, "error:303", strlen("error:303"), 0));
										 return false;
									}
									con->ModifyQuery("use tcmbpspdb");
									char sqlbuf[0x200]={0};
									sprintf(sqlbuf,"update tcm_tabletinfo set tabletinfo_execstandard='%s',tabletinfo_approval='%s',tabletinfo_validity='%s' where tabletindf_code='%s'",arg1.c_str(),arg2.c_str(),arg3.c_str(),tabletinfo_code.c_str());
									cout<<sqlbuf<<endl;
									int rtn=0;
									try {
										rtn=con->ModifyQuery(sqlbuf);
									} catch (exception &e) {
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									}
									if(rtn==0){
										sprintf(sqlbuf,"update ok");

									}else{
										sprintf(sqlbuf,"update failed %s\n",con->GetErrMsg());
										cout<<sqlbuf<<endl;
									}

									CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
										con->CloseConnect();
										cout<<"===============Update yinpian info end============="<<endl;
								}
		else if(!method.compare("updateyinpianassist")){
							string tabletassist_code=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string name=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string number=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string weight=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string address=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string userate=srecv.substr(0,srecv.find(","));
										srecv=srecv.substr(srecv.find(",")+1);
										string med=srecv;


							cout<<"==============Update yinpian assist start=================="<<endl;
							CEncapMysql *con;
							con = new CEncapMysql;

							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							con->ModifyQuery("use tcmbpspdb");
							char sqlbuf[0x200]={0};
							sprintf(sqlbuf,"update tcm_tabletassist set tabletassist_name='%s',tabletassist_number='%s',tabletassist_weight='%s',tabletassist_address='%s',tabletassist_userate='%s',tabletassist_med='%s' where tabletassist_code='%s'",name.c_str(),number.c_str(),weight.c_str(),address.c_str(),userate.c_str(),med.c_str(),tabletassist_code.c_str());
							cout<<sqlbuf<<endl;
							int rtn=0;
							try {
								rtn=con->ModifyQuery(sqlbuf);
							} catch (exception &e) {
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							}
							if(rtn==0){
								sprintf(sqlbuf,"OK");

							}else{
								sprintf(sqlbuf,"update failed %s\n",con->GetErrMsg());
								cout<<sqlbuf<<endl;
							}

							CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
								con->CloseConnect();
								cout<<"===============Update yinpian assist info end============="<<endl;
						}
		else if(!method.compare("finishtask")){
									string printtask=srecv.substr(0,srecv.find(","));
									srecv=srecv.substr(srecv.find(",")+1);
									string state=srecv;



									cout<<"==============Update task start=================="<<endl;
									CEncapMysql *con;
									con = new CEncapMysql;

									if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
									{
										 CHK(send(client, "error:303", strlen("error:303"), 0));
										 return false;
									}
									con->ModifyQuery("use tcmbpspdb");
									char sqlbuf[0x200]={0};
									sprintf(sqlbuf,"update tcm_printtask set printtask_state=%s where printtask_code='%s'",state.c_str(),printtask.c_str());
									cout<<sqlbuf<<endl;
									int rtn=0;
									try {
										rtn=con->ModifyQuery(sqlbuf);
									} catch (exception &e) {
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									}
									if(rtn==0){
										sprintf(sqlbuf,"OK");

									}else{
										sprintf(sqlbuf,"update failed %s\n",con->GetErrMsg());
										cout<<sqlbuf<<endl;
									}


									con->CloseConnect();
										cout<<"===============Update task end============="<<endl;
								}
		else if(!method.compare("updateyaocaiinfo")){
			string med_code=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string batnumber=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string weight=srecv;

			cout<<"==============Update yaocai info start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x200]={0};
			sprintf(sqlbuf,"update tcm_med set med_batchnumber='%s',med_weight='%s' where med_code='%s'",batnumber.c_str(),weight.c_str(),med_code.c_str());
			cout<<sqlbuf<<endl;
			int rtn=0;
			try {
				rtn=con->ModifyQuery(sqlbuf);
			} catch (exception &e) {
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			}
			if(rtn==0){
				sprintf(sqlbuf,"update ok");

			}else{
				sprintf(sqlbuf,"update failed %d\n",rtn);
				cout<<sqlbuf<<endl;
			}

			CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
				con->CloseConnect();
				cout<<"===============Update yaocai info end============="<<endl;
		}else if(!method.compare("updateyaocaimore")){
			string medinfo_code=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string provider=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string standard=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string province=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string city=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string area=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string address=srecv.substr(0,srecv.find(","));
						srecv=srecv.substr(srecv.find(",")+1);
						string areacode=srecv;

			cout<<"==============Update yaocai more start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x200]={0};
			sprintf(sqlbuf,"update tcm_medinfo set medinfo_provider='%s',medinfo_standard='%s',province='%s', city='%s',area='%s',address='%s',areacode='%s' where medinfo_code='%s'",provider.c_str(),standard.c_str(),province.c_str(),city.c_str(),area.c_str(),address.c_str(),areacode.c_str(), medinfo_code.c_str());
			cout<<sqlbuf<<endl;
			int rtn=0;
			try {
				rtn=con->ModifyQuery(sqlbuf);
			} catch (exception &e) {
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			}
			if(rtn==0){
				sprintf(sqlbuf,"update ok");

			}else{
				sprintf(sqlbuf,"update failed %d\n",rtn);
				cout<<sqlbuf<<endl;
			}

			CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
				con->CloseConnect();
				cout<<"===============Update yaocai more end============="<<endl;
		}else if(!method.compare("updateyaocaidetect")){
			string check_code=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string standard=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string grade=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string condition=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string username=srecv;


			cout<<"==============Update yaocai check start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x200]={0};
			sprintf(sqlbuf,"update tcm_check set check_method='%s',check_grade='%s',check_condition='%s',check_username='%s' where check_code='%s'",standard.c_str(),grade.c_str(),condition.c_str(),username.c_str(),check_code.c_str());
			cout<<sqlbuf<<endl;
			int rtn=0;
			try {
				rtn=con->ModifyQuery(sqlbuf);
			} catch (exception &e) {
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			}
			if(rtn==0){
				sprintf(sqlbuf,"update ok");

			}else{
				sprintf(sqlbuf,"update failed %d\n",rtn);
				cout<<sqlbuf<<endl;
			}

			CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
				con->CloseConnect();
				cout<<"===============Update yaocai check end============="<<endl;
		}else if(!method.compare("updateyaocaidetect")){
			string check_code=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string standard=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string grade=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string condition=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);

			string username=srecv;


			cout<<"==============Update yinpian check start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x200]={0};
			sprintf(sqlbuf,"update tcm_check set check_method='%s',check_grade='%s',check_condition='%s',check_username='%s' where check_code='%s'",standard.c_str(),grade.c_str(),condition.c_str(),username.c_str(),check_code.c_str());
			cout<<sqlbuf<<endl;
			int rtn=0;
			try {
				rtn=con->ModifyQuery(sqlbuf);
			} catch (exception &e) {
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			}
			if(rtn==0){
				sprintf(sqlbuf,"OK");

			}else{
				sprintf(sqlbuf,"update failed %d\n",rtn);
				cout<<sqlbuf<<endl;
			}

			CHK(send(client, sqlbuf, strlen(sqlbuf), 0));
				con->CloseConnect();
				cout<<"===============Update yinpian check end============="<<endl;
		}
		else if(!method.compare("taskinfo")){
			string company_code=srecv;
			cout<<"==============Get Task info start=================="<<endl;
						CEncapMysql *con;
						con = new CEncapMysql;

						if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
						{
							 CHK(send(client, "error:303", strlen("error:303"), 0));
							 return false;
						}
						con->ModifyQuery("use tcmbpspdb");
						char sqlbuf[0x200]={0};
						sprintf(sqlbuf,"select distinct a.printtask_code,a.printtask_name,a.printtask_tablet_totalnum,a.printtask_tablet_numweight,a.addtime,b.tablet_spec,a.printtask_batchnumber from tcm_printtask a,tcm_tablet b where a.company_code='%s' and a.printtask_type=2 and a.printtask_state=1 and printtask_packtype=2 and a.tablet_cpc=b.tablet_cpc order by a.addtime",company_code.c_str());
						try{
							con->SelectQuery(sqlbuf);
						}
						catch(exception& e){
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						};
							char** r=NULL;
							//unsigned char buf[0x400*0x400]={0};
							stringstream sendss;
							while((r=con->FetchRow())){
								sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|";
								//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


							}
							string buf=sendss.str();
							if(buf.length()<10){
								buf="get info error";
							}
							cout<<buf<<endl;
							CHK(send(client, buf.c_str(), buf.length(), 0));
							con->CloseConnect();
							cout<<"===============Get Task info end============="<<endl;
		}
		else if(!method.compare("getyaocaiinfo")){
					string user_code=srecv.substr(0,srecv.find(","));;
										srecv=srecv.substr(srecv.find(",")+1);
										string page=srecv.substr(0,srecv.find(","));
										string condition=srecv.substr(srecv.find(",")+1);
					cout<<"==============Get yaocai info start=================="<<endl;
								CEncapMysql *con;
								con = new CEncapMysql;

								if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
								{
									 CHK(send(client, "error:303", strlen("error:303"), 0));
									 return false;
								}
								con->ModifyQuery("use tcmbpspdb");
								char sqlbuf[0x200]={0};
								sprintf(sqlbuf,"select med_code,med_name,IF(med_type=2,'非溯源','溯源'),med_batchnumber,med_weight,addtime from tcm_med where user_code='%s' and med_name like '%%%s%%' order by addtime desc limit %d ,10",user_code.c_str(),condition.c_str(),atoi(page.c_str()));
								//cout<<sqlbuf<<endl;
								try{
									con->SelectQuery(sqlbuf);
								}
								catch(exception& e){
									printf("exception\n");
															CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
															con->CloseConnect();
															 return false;
								};
									char** r=NULL;
									//unsigned char buf[0x400*0x400]={0};
									stringstream sendss;
									while((r=con->FetchRow())){
										sendss<<r[0]<<","<<r[1]<<","<<r[3]<<","<<r[2]<<","<<r[4]<<","<<r[5]<<"|";
										//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


									}
									string buf=sendss.str();
									if(buf.length()<1){
										buf="get yaocai info error or 没有相关信息，请添加！";
									}
									cout<<buf<<endl;
									CHK(send(client, buf.c_str(), buf.length(), 0));
									con->CloseConnect();
									cout<<"===============Get yaocai info end============="<<endl;
				}
		else if(!method.compare("gettaskinfo")){
							string user_code=srecv.substr(0,srecv.find(","));;
												srecv=srecv.substr(srecv.find(",")+1);
												string page=srecv.substr(0,srecv.find(","));
												srecv=srecv.substr(srecv.find(",")+1);
												string condition0=srecv.substr(0,srecv.find(","));
												srecv=srecv.substr(srecv.find(",")+1);
												string condition1=srecv.substr(0,srecv.find(","));
												srecv=srecv.substr(srecv.find(",")+1);
												string condition2=srecv;


							cout<<"==============Get task info start=================="<<endl;
										CEncapMysql *con;
										con = new CEncapMysql;

										if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
										{
											 CHK(send(client, "error:303", strlen("error:303"), 0));
											 return false;
										}
										con->ModifyQuery("use tcmbpspdb");
										char sqlbuf[0x300]={0};
										sprintf(sqlbuf,"select DISTINCT printtask_code,printtask_name,printtask_batchnumber,printtask_tablet_numweight,IF(printtask_tablet_mediumnum is NULL,'--',printtask_tablet_mediumnum),printtask_tablet_totalnum,IF(printtask_packtype=1,'中包','小包'),a.addtime,case printtask_state when  '1' then '未打印' when '2' then '打印中' ELSE '完成' end,b.tablet_spec from tcm_printtask a join tcm_tablet b on b.tablet_cpc=a.tablet_cpc  where a.user_code='%s'  and (printtask_packtype = %s) and (printtask_state =%s) and printtask_name like '%%%s%%' order by a.addtime desc limit %d ,10",user_code.c_str(),condition0.c_str(),condition1.c_str(),condition2.c_str(), atoi(page.c_str()));
										cout<<sqlbuf<<endl;
										try{
											con->SelectQuery(sqlbuf);
										}
										catch(exception& e){
											printf("exception\n");
																	CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																	con->CloseConnect();
																	 return false;
										};
											char** r=NULL;
											//unsigned char buf[0x400*0x400]={0};
											stringstream sendss;
											while((r=con->FetchRow())){
												sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<","<<r[8]<<","<<r[9]<<"|";
												//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


											}
											string buf=sendss.str();
											if(buf.length()<10){
												buf="Get task error or no data";
											}
											cout<<buf<<endl;
											CHK(send(client, buf.c_str(), buf.length(), 0));
											con->CloseConnect();
											cout<<"===============Get task info end============="<<endl;
						}
		else if(!method.compare("getyaocaiinfo_base")){
			string user_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string page=srecv;
			cout<<"==============Get yaocai info start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x200]={0};
			sprintf(sqlbuf,"select m.med_code,med_name,IF(med_type=2,'非溯源','溯源'),med_batchnumber,med_weight,addtime from tcm_med m INNER JOIN tcm_medmiddle mm on m.med_code=mm.med_code where user_code='%s' and mm.other_type='tcm_check' limit %d ,10",user_code.c_str(),atoi(page.c_str()));
			//cout<<sqlbuf<<endl;
			try{
				con->SelectQuery(sqlbuf);
			}
			catch(exception& e){
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			};
			char** r=NULL;
			//unsigned char buf[0x400*0x400]={0};
			stringstream sendss;
			while((r=con->FetchRow())){
				sendss<<r[0]<<","<<r[1]<<","<<r[3]<<","<<r[2]<<","<<r[4]<<","<<r[5]<<"|";
				//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


			}
			string buf=sendss.str();
			if(buf.length()<10){
				buf="get yaocai info error";
			}
			cout<<buf<<endl;
			CHK(send(client, buf.c_str(), buf.length(), 0));
			con->CloseConnect();
			cout<<"===============Get yaocai info end============="<<endl;
			}
		else if(!method.compare("getyaocaiinfo_more")){
					string user_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string page=srecv;
					cout<<"==============Get yaocai info start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					con->ModifyQuery("use tcmbpspdb");
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"select med_code,med_name,IF(med_type=2,'非溯源','溯源'),med_batchnumber,med_weight,addtime from tcm_med where user_code='%s' and med_code not in (select med_code FROM tcm_medmiddle WHERE other_type='tcm_medinfo') ORDER BY addtime DESC limit %d ,10",user_code.c_str(),atoi(page.c_str()));
					cout<<sqlbuf<<endl;
					try{
						con->SelectQuery(sqlbuf);
					}
					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
					char** r=NULL;
					//unsigned char buf[0x400*0x400]={0};
					stringstream sendss;
					while((r=con->FetchRow())){
						sendss<<r[0]<<","<<r[1]<<","<<r[3]<<","<<r[2]<<","<<r[4]<<","<<r[5]<<"|";
						//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


					}
					string buf=sendss.str();
					if(buf.length()<10){
						buf="get yaocai info error";
					}
					cout<<buf<<endl;
					CHK(send(client, buf.c_str(), buf.length(), 0));
					con->CloseConnect();
					cout<<"===============Get yaocai info end============="<<endl;
					}
		else if(!method.compare("getyinpianinfomore")){
							string user_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string page=srecv;
							cout<<"==============Get yinpian info start=================="<<endl;
							CEncapMysql *con;
							con = new CEncapMysql;

							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							con->ModifyQuery("use tcmbpspdb");
							char sqlbuf[0x200]={0};
							sprintf(sqlbuf,"select tablet_code,tablet_name,tablet_batchnumber,tablet_weight,addtime from tcm_tablet where user_code='%s' and tablet_code not in (select tablet_code FROM tcm_tabletmiddle WHERE other_type='tcm_tabletinfo') ORDER BY addtime DESC limit %d ,10",user_code.c_str(),atoi(page.c_str()));
							cout<<sqlbuf<<endl;
							try{
								con->SelectQuery(sqlbuf);
							}
							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
							char** r=NULL;
							//unsigned char buf[0x400*0x400]={0};
							stringstream sendss;
							while((r=con->FetchRow())){
								sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<"|";
								//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


							}
							string buf=sendss.str();
							if(buf.length()<10){
								buf="get yinpian  no more  error";
							}
							cout<<buf<<endl;
							CHK(send(client, buf.c_str(), buf.length(), 0));
							con->CloseConnect();
							cout<<"===============Get yaocai info end============="<<endl;
							}
		else if(!method.compare("getyinpianinfodetect")){
									string user_code=srecv.substr(0,srecv.find(","));;
									srecv=srecv.substr(srecv.find(",")+1);
									string page=srecv;
									cout<<"==============Get yinpian detect  info start=================="<<endl;
									CEncapMysql *con;
									con = new CEncapMysql;

									if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
									{
										 CHK(send(client, "error:303", strlen("error:303"), 0));
										 return false;
									}
									con->ModifyQuery("use tcmbpspdb");
									char sqlbuf[0x200]={0};
									sprintf(sqlbuf,"select tablet_code,tablet_name,tablet_batchnumber,tablet_weight,addtime from tcm_tablet where user_code='%s' and tablet_code not in (select tablet_code FROM tcm_tabletmiddle WHERE other_type='tcm_check') ORDER BY addtime DESC limit %d ,10",user_code.c_str(),atoi(page.c_str()));
									//cout<<sqlbuf<<endl;
									try{
										con->SelectQuery(sqlbuf);
									}
									catch(exception& e){
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									};
									char** r=NULL;
									//unsigned char buf[0x400*0x400]={0};
									stringstream sendss;
									while((r=con->FetchRow())){
										sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<"|";
										//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


									}
									string buf=sendss.str();
									if(buf.length()<10){
										buf="get yaocai info error";
									}
									cout<<buf<<endl;
									CHK(send(client, buf.c_str(), buf.length(), 0));
									con->CloseConnect();
									cout<<"===============Get yaocai detect info end============="<<endl;
									}
		else if(!method.compare("getyinpianinfoassist")){
									string user_code=srecv.substr(0,srecv.find(","));;
									srecv=srecv.substr(srecv.find(",")+1);
									string page=srecv;
									cout<<"==============Get yaocai assist info start=================="<<endl;
									CEncapMysql *con;
									con = new CEncapMysql;

									if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
									{
										 CHK(send(client, "error:303", strlen("error:303"), 0));
										 return false;
									}
									con->ModifyQuery("use tcmbpspdb");
									char sqlbuf[0x200]={0};
									sprintf(sqlbuf,"select tablet_code,tablet_name,tablet_batchnumber,tablet_weight,addtime from tcm_tablet where user_code='%s' and tablet_code not in (select tablet_code FROM tcm_tabletmiddle WHERE other_type='tcm_tabletassist') ORDER BY addtime DESC limit %d ,10",user_code.c_str(),atoi(page.c_str()));
									//cout<<sqlbuf<<endl;
									try{
										con->SelectQuery(sqlbuf);
									}
									catch(exception& e){
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									};
									char** r=NULL;
									//unsigned char buf[0x400*0x400]={0};
									stringstream sendss;
									while((r=con->FetchRow())){
										sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<"|";
										//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


									}
									string buf=sendss.str();
									if(buf.length()<10){
										buf="get yaocai info error";
									}
									cout<<buf<<endl;
									CHK(send(client, buf.c_str(), buf.length(), 0));
									con->CloseConnect();
									cout<<"===============Get yaocai assist info end============="<<endl;
									}
		else if(!method.compare("address")){

				string taskid=srecv;
				cout<<"==============Get address info start=================="<<endl;
				CEncapMysql *con;
				con = new CEncapMysql;

				if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
				{
					 CHK(send(client, "error:303", strlen("error:303"), 0));
					 return false;
				}
				con->ModifyQuery("use tcmbpspdb");
				char sqlbuf[0x200]={0};
				sprintf(sqlbuf,"select tt.province,tt.city from tcm_tabletinfo tt	inner join tcm_tabletmiddle tm on tt.tabletinfo_code = tm.other_code inner join tcm_tablet t on t.tablet_code = tm.tablet_code	where tm.other_type = 'tcm_tabletinfo'	and t.tablet_code =(SELECT  tablet_code from tcm_box where printtask_code='%s' LIMIT 0 ,1)",taskid.c_str());
				//cout<<sqlbuf<<endl;
				try{
					con->SelectQuery(sqlbuf);
				}
				catch(exception& e){
					printf("exception\n");
					CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
					con->CloseConnect();
					 return false;
				};
				char** r=NULL;
				//unsigned char buf[0x400*0x400]={0};
				stringstream sendss;
				r=con->FetchRow();
					sendss<<r[0]<<r[1];
					//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;



				string buf=sendss.str();
				if(buf.length()<1){
					buf="get yaocai info error";
				}
				cout<<buf<<endl;
				CHK(send(client, buf.c_str(), buf.length(), 0));
				con->CloseConnect();
				cout<<"===============Get address info end============="<<endl;
				}
		else if(!method.compare("getyinpianassist")){
			string user_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string page=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string condition0=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string condition1=srecv;
			cout<<"==============Get yinpian assist info start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x300]={0};
			sprintf(sqlbuf,"SELECT	mi.tabletassist_code,	m.tablet_name,	m.tablet_batchnumber,	mi.tabletassist_name,mi.tabletassist_number,mi.tabletassist_weight,mi.tabletassist_address,	mi.tabletassist_userate,mi.tabletassist_med	,LEFT (mi.addtime, 10) FROM	tcm_tabletassist mi INNER JOIN tcm_tabletmiddle mm ON mi.tabletassist_code = mm.other_code INNER JOIN tcm_tablet m ON m.tablet_code = mm.tablet_code WHERE 	mi.user_code = '%s' and m.tablet_name like '%%%s%%' and mi.tabletassist_name like '%%%s%%' ORDER BY	mi.addtime DESC LIMIT %d, 10",user_code.c_str(),condition0.c_str(),condition1.c_str(), atoi(page.c_str()));
			cout<<sqlbuf<<endl;
			try{
				con->SelectQuery(sqlbuf);
			}
			catch(exception& e){
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			};
			char** r=NULL;
			//unsigned char buf[0x400*0x400]={0};
			stringstream sendss;
			while((r=con->FetchRow())){
				sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<","<<r[8]<<","<<r[9]<<"|";
				//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


			}
			string buf=sendss.str();
			if(buf.length()<10){
				buf="get yinpian assist info error";
			}
			cout<<buf<<endl;
			CHK(send(client, buf.c_str(), buf.length(), 0));
			con->CloseConnect();
			cout<<"===============Get yinpian assist info end============="<<endl;
			}
		else if(!method.compare("getyinpiandetect")){

					string user_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string page=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string condition0=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string condition1=srecv;
					cout<<"==============Get yinpian detect info start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					con->ModifyQuery("use tcmbpspdb");
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"SELECT	mi.check_code,	m.tablet_name,	m.tablet_batchnumber,m.tablet_weight,	mi.check_username,mi.check_grade,mi.check_condition,mi.check_method	,LEFT (mi.check_time, 10),LEFT (mi.addtime, 10) FROM	tcm_check mi INNER JOIN tcm_tabletmiddle mm ON mi.check_code = mm.other_code INNER JOIN tcm_tablet m ON m.tablet_code = mm.tablet_code WHERE 	mi.check_username like '%%%s%%' and m.tablet_name like '%%%s%%' and mi.user_code = '%s' ORDER BY	mi.addtime DESC LIMIT %d, 10",condition1.c_str(),condition0.c_str(), user_code.c_str(),atoi(page.c_str()));
					cout<<sqlbuf<<endl;
					try{
						con->SelectQuery(sqlbuf);
					}
					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
					char** r=NULL;
					//unsigned char buf[0x400*0x400]={0};
					stringstream sendss;
					while((r=con->FetchRow())){
						sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<","<<r[8]<<","<<r[9]<<"|";
						//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


					}
					string buf=sendss.str();
					if(buf.length()<10){
						buf="get yinpian detect info error";
					}
					cout<<buf<<endl;
					CHK(send(client, buf.c_str(), buf.length(), 0));
					con->CloseConnect();
					cout<<"===============Get yinpian detect info end============="<<endl;
					}
		else if(!method.compare("getyaocaiinfo_check")){
			string user_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string page=srecv;
			cout<<"==============Get yaocai info start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			con->ModifyQuery("use tcmbpspdb");
			char sqlbuf[0x200]={0};
			sprintf(sqlbuf,"select med_code,med_name,IF(med_type=2,'非溯源','溯源'),med_batchnumber,med_weight,addtime from tcm_med where user_code='%s' and med_code not in (select med_code FROM tcm_medmiddle WHERE other_type='tcm_check') ORDER BY addtime DESC limit %d ,10",user_code.c_str(),atoi(page.c_str()));
			//cout<<sqlbuf<<endl;
			try{
				con->SelectQuery(sqlbuf);
			}
			catch(exception& e){
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			};
			char** r=NULL;
			//unsigned char buf[0x400*0x400]={0};
			stringstream sendss;
			while((r=con->FetchRow())){
				sendss<<r[0]<<","<<r[1]<<","<<r[3]<<","<<r[2]<<","<<r[4]<<","<<r[5]<<"|";
				//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


			}
			string buf=sendss.str();
			if(buf.length()<10){
				buf="get yaocai info error";
			}
			cout<<buf<<endl;
			CHK(send(client, buf.c_str(), buf.length(), 0));
			con->CloseConnect();
			cout<<"===============Get yaocai info end============="<<endl;
			}
		else if(!method.compare("getyaocaimore")){
			string user_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string page=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string condition0=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string condition1=srecv;
			cout<<"==============Get yaocai more info start=================="<<endl;
						CEncapMysql *con;
						con = new CEncapMysql;

						if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
						{
							 CHK(send(client, "error:303", strlen("error:303"), 0));
							 return false;
						}
						try {
							con->ModifyQuery("use tcmbpspdb");
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						char sqlbuf[0x200]={0};
						sprintf(sqlbuf,"select mi.medinfo_code, m.med_name,m.med_batchnumber,m.med_weight,mi.medinfo_provider,mi.medinfo_standard,mi.province,mi.city,mi.area,mi.address,mi.medinfo_thicktime,LEFT(mi.addtime ,10)from tcm_medinfo mi inner join tcm_medmiddle mm on mi.medinfo_code = mm.other_code inner join tcm_med m on m.med_code = mm.med_code where mi.user_code = '%s' and m.med_name like '%%%s%%' and mi.medinfo_provider like '%%%s%%' order by mi.addtime desc limit %d,10",user_code.c_str(),condition0.c_str(),condition1.c_str(),atoi(page.c_str()));
						cout<<sqlbuf<<endl;
						try{
							con->SelectQuery(sqlbuf);
						}
						catch(exception& e){
							printf("ss\n");
							con->CloseConnect();
						};
							char** r=NULL;
							//unsigned char buf[0x400*0x400]={0};
							stringstream sendss;
							while((r=con->FetchRow())){
								sendss<<r[0]<<","<<r[1]<<","<<r[3]<<","<<r[2]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<r[7]<<r[8]<<r[9]<<","<<r[10]<<","<<r[11]<<"|";
								//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


							}
							string buf=sendss.str();
							if(buf.length()<10){
								buf="get yaocai more info error";
							}
							cout<<buf<<endl;
							CHK(send(client, buf.c_str(), buf.length(), 0));
							con->CloseConnect();
							cout<<"===============Get yaocai more info end============="<<endl;
						}
		else if(!method.compare("getyinpianmore")){
					string user_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string page=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string condition0=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string condition1=srecv;
					cout<<"==============Get yinpian more info start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					try {
						con->ModifyQuery("use tcmbpspdb");
					} catch (exception &e) {
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					}
					char sqlbuf[0x300]={0};
					sprintf(sqlbuf,"SELECT	mi.tabletinfo_code,	m.tablet_name,	m.tablet_batchnumber,	m.tablet_weight,	mi.tabletinfo_execstandard,mi.tabletinfo_approval,mi.tabletinfo_validity,	mi.province,	mi.city,	mi.area,	mi.address,	mi.tabletinfo_productiontime,	LEFT (mi.addtime, 10) FROM	tcm_tabletinfo mi INNER JOIN tcm_tabletmiddle mm ON mi.tabletinfo_code = mm.other_code INNER JOIN tcm_tablet m ON m.tablet_code = mm.tablet_code WHERE 	mi.user_code = '%s' and m.tablet_name like '%%%s%%' and mi.tabletinfo_execstandard like '%%%s%%' ORDER BY	mi.addtime DESC LIMIT %d, 10",user_code.c_str(),condition0.c_str(),condition1.c_str(),atoi(page.c_str()));
					cout<<sqlbuf<<endl;
					try{
						con->SelectQuery(sqlbuf);
					}
					catch(exception& e){
						printf("ss\n");
						con->CloseConnect();
					};
						char** r=NULL;
						//unsigned char buf[0x400*0x400]={0};
						stringstream sendss;
						while((r=con->FetchRow())){
							sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<r[8]<<r[9]<<r[10]<<","<<r[11]<<","<<r[12]<<"|";
							//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;


						}
						string buf=sendss.str();
						if(buf.length()<10){
							buf="get yinpian more info error";
						}
						cout<<buf<<endl;
						CHK(send(client, buf.c_str(), buf.length(), 0));
						con->CloseConnect();
						cout<<"===============Get yinpian more info end============="<<endl;
					}

		else if(!method.compare("getyaocaidetect")){
			string user_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string page=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string condition0=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
			string condition1=srecv;
			cout<<"==============Get yaocai more info start=================="<<endl;
				CEncapMysql *con;
				con = new CEncapMysql;

				if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
				{
					 CHK(send(client, "error:303", strlen("error:303"), 0));
					 return false;
				}
				try {
					con->ModifyQuery("use tcmbpspdb");
				} catch (exception &e) {
					printf("exception\n");
											CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
											con->CloseConnect();
											 return false;
				}
				char sqlbuf[0x200]={0};
				sprintf(sqlbuf,"select c.check_code,m.med_name,m.med_batchnumber,m.med_weight,c.check_username,c.check_grade,c.check_condition,c.check_thickmethod,c.check_method,c.check_time from tcm_check c inner join tcm_medmiddle mm on c.check_code = mm.other_code	inner join tcm_med m on m.med_code = mm.med_code where c.check_type = 1 and c.user_code='%s' and m.med_name like '%%%s%%' and c.check_username like '%%%s%%' order by c.addtime desc limit %d,10",user_code.c_str(),condition0.c_str(),condition1.c_str(),atoi(page.c_str()));
				cout<<sqlbuf<<endl;
				try{
					con->SelectQuery(sqlbuf);
				}
				catch(exception& e){
					printf("exception\n");
											CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
											con->CloseConnect();
											 return false;
				};
					char** r=NULL;
					//unsigned char buf[0x400*0x400]={0};
					stringstream sendss;
					//printf("ss\n");
					while((r=con->FetchRow())){

						sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<","<<r[8]<<","<<r[9]<<"^";
						//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<","<<r[7]<<","<<r[8]<<"^"<<endl;


					}
					//printf("ss\n");
					string buf=sendss.str();
					if(buf.length()<10){
						buf="get yaocai more info error";
					}
					cout<<buf<<endl;
					CHK(send(client, buf.c_str(), buf.length(), 0));
					con->CloseConnect();
					cout<<"===============Get yaocai more info end============="<<endl;
				}
		else if(!method.compare("getyaocainamenum")){
						string name=srecv;
						cout<<"==============Get yaocai name num start=================="<<endl;
						CEncapMysql *con;
						con = new CEncapMysql;

						if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
						{
							 CHK(send(client, "error:303", strlen("error:303"), 0));
							 return false;
						}
						try {
							con->ModifyQuery("use tcmbpspdb");
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						//char sqlbuf[0x200]={0};
						char sqlbuf[0x200]={0};
						char** r=NULL;
						sprintf(sqlbuf,"select count(1) from tcm_medbase where enabled=1 and name like '%%%s%%'",name.c_str());
						try {
							con->SelectQuery(sqlbuf);
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}

						stringstream sendss;
						r=con->FetchRow();

						sendss<<r[0];
						string buf=sendss.str();
						cout<<buf<<endl;
						CHK(send(client, buf.c_str(), buf.length(), 0));
						con->CloseConnect();
						cout<<"===============Get yaocai name num end============="<<endl;
				}
		else if(!method.compare("getprinter")){
					string user_code=srecv;
					cout<<"==============Get print start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					try {
						con->ModifyQuery("use tcmbpspdb");
					} catch (exception &e) {
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					}
					char sqlbuf[0x200]={0};
					char** r=NULL;
					sprintf(sqlbuf,"select count(1) from tcm_print where user_code='%s'",user_code.c_str());
					try {
							cout<<sqlbuf<<endl;
							con->SelectQuery(sqlbuf);
						} catch (exception &e ) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
					}
						r=con->FetchRow();
						if(r[0][0]==48){
							CHK(send(client, "error no data", strlen("error no data"), 0));
							con->CloseConnect();
							cout<<"===============Get print end============="<<endl;
							return true;
						}
					sprintf(sqlbuf,"select tcmprint_name,tcmprint_ip,tcmprint_port,devicetype from tcm_print where  user_code='%s'",user_code.c_str());

					try {
						cout<<sqlbuf<<endl;
						con->SelectQuery(sqlbuf);
					} catch (exception &e ) {
						con->CloseConnect();
					}

					stringstream sendss;
					while((r=con->FetchRow())){
						sendss<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<"|";
					}
					string buf=sendss.str();
					if(buf.length()<10){
						buf="get print info error";
					}
					cout<<buf<<endl;
					CHK(send(client, buf.c_str(), buf.length(), 0));
					con->CloseConnect();
					cout<<"===============Get print end============="<<endl;
			}
		else if(!method.compare("getnum")){
			//printf("%s\n",srecv.c_str());
			string user_code=srecv.substr(0,srecv.find(","));
			srecv=srecv.substr(srecv.find(",")+1);
								string typeStr=srecv.substr(0,srecv.find(","));
								srecv=srecv.substr(srecv.find(",")+1);
								string condition=srecv;

			//int type=atoi(typeStr.c_str());
			cout<<"==============Get  num start=================="<<endl;
			printf("%s\n",condition.c_str());
			CEncapMysql *con;
			con = new CEncapMysql;
			char** r=NULL;
			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}

			try {
				con->ModifyQuery("use tcmbpspdb");
			} catch (exception &e) {
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			}
			char sqlbuf[0x200]={0};
			if	(typeStr=="0"){
				sprintf(sqlbuf,"select count(1) from tcm_med where user_code='%s' and med_name like '%%%s%%'",user_code.c_str(),condition.c_str());
			}else if(typeStr=="1"){
				string condition1=condition.substr(0,condition.find(','));
				string condition2=condition.substr(condition.find(',')+1);
				sprintf(sqlbuf,"select count(1) from tcm_medinfo mi inner join tcm_medmiddle mm on mi.medinfo_code = mm.other_code inner join tcm_med m on m.med_code = mm.med_code where mi.user_code = '%s' and m.med_name like '%%%s%%' and mi.medinfo_provider like '%%%s%%'",user_code.c_str(),condition1.c_str(),condition2.c_str());
			}
			else if(typeStr=="2"){
				string condition1=condition.substr(0,condition.find(','));
				string condition2=condition.substr(condition.find(',')+1);
				sprintf(sqlbuf,"select count(1) from tcm_check c inner join tcm_medmiddle mm on c.check_code = mm.other_code	inner join tcm_med m on m.med_code = mm.med_code where c.check_type = 1 and c.user_code='%s' and m.med_name like '%%%s%%' and c.check_username like '%%%s%%'",user_code.c_str(),condition1.c_str(),condition2.c_str());
			}
			else if(typeStr=="3"){
					sprintf(sqlbuf,"select count(1) from tcm_tablet where user_code='%s' and tablet_name like '%%%s%%' ",user_code.c_str(),condition.c_str());
			}
			else if(typeStr=="4"){
				string condition1=condition.substr(0,condition.find(','));
				string condition2=condition.substr(condition.find(',')+1);
				sprintf(sqlbuf,"SELECT	count(*) FROM	tcm_tabletinfo mi INNER JOIN tcm_tabletmiddle mm ON mi.tabletinfo_code = mm.other_code INNER JOIN tcm_tablet m ON m.tablet_code = mm.tablet_code WHERE 	 mi.user_code = '%s' and m.tablet_name like '%%%s%%' and mi.tabletinfo_execstandard like '%%%s%%' ",user_code.c_str(),condition1.c_str(),condition2.c_str());
			}
			else if(typeStr=="5"){
				string condition1=condition.substr(0,condition.find(','));
				string condition2=condition.substr(condition.find(',')+1);
				sprintf(sqlbuf,"SELECT	count(*) FROM	tcm_tabletassist mi INNER JOIN tcm_tabletmiddle mm ON mi.tabletassist_code = mm.other_code INNER JOIN tcm_tablet m ON m.tablet_code = mm.tablet_code WHERE 	mi.user_code = '%s' and m.tablet_name like '%%%s%%' and mi.tabletassist_name like '%%%s%%'",user_code.c_str(),condition1.c_str(),condition2.c_str());
			}
			else if(typeStr=="6"){
				string condition1=condition.substr(0,condition.find(','));
				string condition2=condition.substr(condition.find(',')+1);
				sprintf(sqlbuf,"SELECT	count(*) FROM	tcm_check mi INNER JOIN tcm_tabletmiddle mm ON mi.check_code = mm.other_code INNER JOIN tcm_tablet m ON m.tablet_code = mm.tablet_code WHERE 	mi.user_code = '%s' and m.tablet_name LIKE '%%%s%%' and mi.check_username like '%%%s%%' ",user_code.c_str(),condition1.c_str(),condition2.c_str());
			}
			else if(typeStr=="7"){

				sprintf(sqlbuf,"select count(1) from tcm_tablet a JOIN tcm_tabletmiddle b ON a.tablet_code =b.tablet_code where tablet_weight_present !='0' and user_code='%s' and tablet_name like '%%%s%%' and b.other_type='tcm_check'",user_code.c_str(),condition.c_str());
			}else if(typeStr=="8"){
				string condition1=condition.substr(0,condition.find(','));
				condition=condition.substr(condition.find(',')+1);
				string condition2=condition.substr(0,condition.find(','));
				string condition3=condition.substr(condition.find(',')+1);



				sprintf(sqlbuf,"select  count(DISTINCT printtask_code)  from tcm_printtask a join tcm_tablet b on b.tablet_cpc=a.tablet_cpc where a.user_code='%s'  and (printtask_packtype = %s) and (printtask_state =%s) and printtask_name like '%%%s%%'",user_code.c_str(),condition1.c_str(),condition2.c_str(),condition3.c_str());
			}
			else if(typeStr=="9"){
				sprintf(sqlbuf,"select count(*) from tcm_tablet where user_code='%s' and tablet_code not in (select tablet_code FROM tcm_tabletmiddle WHERE other_type='%s')",user_code.c_str(),condition.c_str());
			}else if(typeStr=="10"){
				sprintf(sqlbuf,"select count(*) from tcm_med where user_code='%s' and med_code not in (select med_code FROM tcm_medmiddle WHERE other_type='%s')",user_code.c_str(),condition.c_str());
			}
			else if(typeStr=="11"){
				sprintf(sqlbuf,"select count(*) from tcm_med m INNER JOIN tcm_medmiddle mm on m.med_code=mm.med_code where user_code='%s' and mm.other_type='tcm_check' ",user_code.c_str());
			}
			cout<<sqlbuf<<endl;

			try{
				con->SelectQuery(sqlbuf);
			}
			catch(exception& e){
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			};
			stringstream sendss;
			r=con->FetchRow();

			sendss<<r[0];
			string buf=sendss.str();
			//string buf=sendss.str();
			if(buf.length()<1){
				buf="get num error";
			}
			cout<<buf<<endl;
			CHK(send(client, buf.c_str(), buf.length(), 0));
			con->CloseConnect();
			cout<<"===============Get num end============="<<endl;
						}
		else if(!method.compare("getyinpiannamenum")){
						string name=srecv;
						cout<<"==============Get yinpian name num start=================="<<endl;
						CEncapMysql *con;
						con = new CEncapMysql;

						if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
						{
							 CHK(send(client, "error:303", strlen("error:303"), 0));
							 return false;
						}
						try {
							con->ModifyQuery("use tcmbpspdb");
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						char sqlbuf[0x200]={0};
						sprintf(sqlbuf,"select count(1) from tcm_tabletbase where enabled=1 and name like '%%%s%%'",name.c_str());
						char** r=NULL;
						try{
						con->SelectQuery(sqlbuf);
						}catch(exception &e){
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						stringstream sendss;
						r=con->FetchRow();

						sendss<<r[0];
						string buf=sendss.str();
						cout<<buf<<endl;
						CHK(send(client, buf.c_str(), buf.length(), 0));
						con->CloseConnect();
						cout<<"===============Get yinpian name num end============="<<endl;
				}
		else if(!method.compare("getyaocainame")){
				string page=srecv.substr(0,srecv.find(","));;
				srecv=srecv.substr(srecv.find(",")+1);
				string name=srecv;
				cout<<"==============Get yaocai name  start=================="<<endl;
				CEncapMysql *con;
				con = new CEncapMysql;

				if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
				{
					 CHK(send(client, "error:303", strlen("error:303"), 0));
					 return false;
				}
				try {
					con->ModifyQuery("use tcmbpspdb");
				} catch (exception &e) {
					printf("exception\n");
											CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
											con->CloseConnect();
											 return false;
				}
				char sqlbuf[0x200]={0};
				sprintf(sqlbuf,"select a.sourceid,a.name from tcm_medbase a where enabled=1 and a.name like '%%%s%%' order by lasttime desc limit %d,10 ",name.c_str(),(atoi(page.c_str())-1)*10);
				cout<<sqlbuf<<endl;
				try{

												con->SelectQuery(sqlbuf);


				}
				catch(exception& e){
					printf("exception\n");
					CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
					con->CloseConnect();
					return false;
				};
					char** r=NULL;
					stringstream sendss;
					while((r=con->FetchRow())){
						sendss<<r[0]<<","<<r[1]<<"|";
						//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;
					}
					string buf=sendss.str();
					if(buf.length()<10){
						buf="get yaocai name error";
					}
					cout<<buf<<endl;
					CHK(send(client, buf.c_str(), buf.length(), 0));
					con->CloseConnect();
					cout<<"===============Get yaocai name end============="<<endl;
			}
		else if(!method.compare("getyinpianname")){
						string page=srecv.substr(0,srecv.find(","));;
						srecv=srecv.substr(srecv.find(",")+1);
						string name=srecv;
						cout<<"==============Get yinpian name  start=================="<<endl;
						CEncapMysql *con;
						con = new CEncapMysql;

						if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
						{
							 CHK(send(client, "error:303", strlen("error:303"), 0));
							 return false;
						}
						try {
							con->ModifyQuery("use tcmbpspdb");
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						char sqlbuf[0x200]={0};
						sprintf(sqlbuf,"select a.sourceid,a.name from tcm_tabletbase a where enabled=1 and a.name like '%%%s%%' order by lasttime desc limit %d,10 ",name.c_str(),(atoi(page.c_str())-1)*10);
						cout<<sqlbuf<<endl;
						try{
							con->SelectQuery(sqlbuf);
						}
						catch(exception& e){
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						};
							char** r=NULL;
							stringstream sendss;
							while((r=con->FetchRow())){
								sendss<<r[0]<<","<<r[1]<<"|";
								//cout<<r[0]<<","<<r[1]<<","<<r[2]<<","<<r[3]<<","<<r[4]<<","<<r[5]<<","<<r[6]<<"|"<<endl;
							}
							string buf=sendss.str();
							if(buf.length()<10){
								buf="get yinpian name error";
							}
							cout<<buf<<endl;
							CHK(send(client, buf.c_str(), buf.length(), 0));
							con->CloseConnect();
							cout<<"===============Get yinpian name end============="<<endl;
					}
		else if(!method.compare("addyaocaibase")){
					string med_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string med_name=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string batchnumber=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string weight=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string company_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string user_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string sourceid=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string company_cpc=srecv;
					cout<<"==============add yaocai base start=================="<<endl;
						CEncapMysql *con;
						con = new CEncapMysql;

						if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
						{
							 CHK(send(client, "error:303", strlen("error:303"), 0));
							 return false;
						}
						try {
							con->ModifyQuery("use tcmbpspdb");
						} catch (exception &e) {
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						}
						char sqlbuf[0x200]={0};
						sprintf(sqlbuf,"select count(1),prodcode from tcm_goodscode where basecode='%s' and company_cpc='%s'",sourceid.c_str(),company_cpc.c_str());
						cout<<sqlbuf<<endl;
						try{
							con->SelectQuery(sqlbuf);
						}

						catch(exception& e){
							printf("exception\n");
													CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
													con->CloseConnect();
													 return false;
						};
							char** r=NULL;
							string prodcode="";
							printf("ss\n");
							r=con->FetchRow();
							printf("ss\n");
							if (atoi(r[0])==0){

								cout<<"no prodcode"<<endl;
								prodcode="273"+company_code.substr(6,7)+sourceid;
								cout<<prodcode<<endl;
								int checknum=getCheckCode(prodcode);
								char buf[2]={0};
								sprintf(buf,"%d",checknum);
								prodcode+=buf;
								cout<<prodcode<<endl;
								sprintf(sqlbuf,"insert into tcm_goodscode(type,basecode,prodcode,company_cpc) values('0','%s','%s','%s')",sourceid.c_str(),prodcode.c_str(),company_cpc.c_str());
								cout<<sqlbuf<<endl;
								try {
									con->ModifyQuery(sqlbuf);
								} catch (exception &e) {
									printf("exception\n");
															CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
															con->CloseConnect();
															 return false;
								}

							}else{
								prodcode=r[1];
							}
							sprintf(sqlbuf,"call PROC_FINDID('1','%s',@med_cpc)",company_cpc.c_str());
							cout<<sqlbuf<<endl;
							try{
								con->SelectQuery(sqlbuf);
							}
							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
							//sprintf(sqlbuf,"select @med_cpc");
							//con->SelectQuery(sqlbuf);
							r=con->FetchRow();
							string med_cpc=r[0];
							cout<<med_cpc<<endl;
							int checkcode=getCheckCode(med_cpc);
							char bufcode[2]={0};
							sprintf(bufcode,"%d",checkcode);
															med_cpc+=bufcode;
							cout<<med_cpc<<endl;

							con->CloseConnect();
							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							try {
								con->ModifyQuery("use tcmbpspdb");
							} catch (exception &e) {
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							}
							sprintf(sqlbuf,"insert into tcm_med values('%s','%s',2,'%s','%s','%s',"
									"'%s','%s','%s','%s',NOW())",med_code.c_str(),
									med_name.c_str(),batchnumber.c_str(),
									med_cpc.c_str(), weight.c_str(),
									weight.c_str(),company_code.c_str(),
									user_code.c_str(),prodcode.c_str());
							cout<<sqlbuf<<endl;
							int rtn=0;
							try {
								rtn=con->ModifyQuery(sqlbuf);
							} catch (exception &e) {
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							}
							cout<<con->GetErrMsg()<<endl;

							string buf="OK";
							if(rtn!=0){
								buf="add yaocai base info error"+rtn;
							}
							cout<<buf<<endl;
							CHK(send(client, buf.c_str(), buf.length(), 0));
							con->CloseConnect();
							cout<<"===============add yaocai base info end============="<<endl;
						}
		else if(!method.compare("addyaocaimore")){
			string med_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string provider=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string thicktime=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string standard=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string province=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string city=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string area=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string address=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string company_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string medinfo_code=srecv.substr(0,srecv.find(","));;
						srecv=srecv.substr(srecv.find(",")+1);
			string user_code=srecv.substr(0,srecv.find(","));;
			srecv=srecv.substr(srecv.find(",")+1);
			string areacode=srecv;
			string image="";
			cout<<"==============add yaocai more info start=================="<<endl;
			CEncapMysql *con;
			con = new CEncapMysql;

			if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
			{
				 CHK(send(client, "error:303", strlen("error:303"), 0));
				 return false;
			}
			int rtn=0;
			try {
				con->ModifyQuery("use tcmbpspdb");
			} catch (exception &e) {
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			}
			char sqlbuf[0x200]={0};
			sprintf(sqlbuf,"insert into tcm_medmiddle(med_code,other_code,other_type) values('%s','%s','tcm_medinfo')",med_code.c_str(),medinfo_code.c_str());
			cout<<sqlbuf<<endl;

			try{
				rtn=con->ModifyQuery(sqlbuf);
			}

			catch(exception& e){
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			};
			if(rtn!=0){
				printf("%s\n",con->GetErrMsg());
				CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
				return false;
			}
			sprintf(sqlbuf,"insert into tcm_medinfo values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',NOW())",medinfo_code.c_str(),
				provider.c_str(),thicktime.c_str(),standard.c_str(),image.c_str(),province.c_str(),city.c_str(),
				area.c_str(),address.c_str(),areacode.c_str(),company_code.c_str(),user_code.c_str()
				);
			cout<<sqlbuf<<endl;

			try{
				rtn=con->ModifyQuery(sqlbuf);
			}

			catch(exception& e){
				printf("exception\n");
										CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
										con->CloseConnect();
										 return false;
			};
			if(rtn!=0){
				printf("%s\n",con->GetErrMsg());
				CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
				return false;
			}

			string buf="OK";
			CHK(send(client, buf.c_str(), buf.length(), 0));
			con->CloseConnect();
			cout<<"===============add yaocai more info end============="<<endl;
			}
		else if(!method.compare("addyaocaidetect")){
					string med_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string check_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string standard=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string grade=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string condition=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string username=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string time=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);

					string company_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);

					string user_code=srecv;

					cout<<"==============add yaocai check start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					int rtn=0;
					try {
						con->ModifyQuery("use tcmbpspdb");
					} catch (exception &e) {
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					}
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"insert into tcm_medmiddle(med_code,other_code,other_type) values('%s','%s','tcm_check')",med_code.c_str(),check_code.c_str());
					cout<<sqlbuf<<endl;

					try{
						rtn=con->ModifyQuery(sqlbuf);
					}

					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
					if(rtn!=0){
						printf("%s\n",con->GetErrMsg());
						CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
						return false;
					}
					sprintf(sqlbuf,"insert into tcm_check values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',1,'%s','%s',NOW())",check_code.c_str(),
						standard.c_str(),grade.c_str(),condition.c_str(),"",username.c_str(),time.c_str(),"","","",company_code.c_str(),user_code.c_str()
						);
					cout<<sqlbuf<<endl;

					try{
						rtn=con->ModifyQuery(sqlbuf);
					}

					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
					if(rtn!=0){
						printf("%s\n",con->GetErrMsg());
						CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
						return false;
					}

					string buf="OK";
					CHK(send(client, buf.c_str(), buf.length(), 0));
					con->CloseConnect();
					cout<<"===============add yaocai check end============="<<endl;
					}
		else if(!method.compare("addyinpiandetect")){
							string med_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string check_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string standard=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string grade=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string condition=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string username=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string time=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);

							string company_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);

							string user_code=srecv;

							cout<<"==============add yaocai check start=================="<<endl;
							CEncapMysql *con;
							con = new CEncapMysql;

							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							int rtn=0;
							try {
								con->ModifyQuery("use tcmbpspdb");
							} catch (exception &e) {
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							}
							char sqlbuf[0x200]={0};
							sprintf(sqlbuf,"insert into tcm_tabletmiddle(tablet_code,other_code,other_type) values('%s','%s','tcm_check')",med_code.c_str(),check_code.c_str());
							cout<<sqlbuf<<endl;

							try{
								rtn=con->ModifyQuery(sqlbuf);
							}

							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
							if(rtn!=0){
								printf("%s\n",con->GetErrMsg());
								CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
								return false;
							}
							sprintf(sqlbuf,"insert into tcm_check values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',1,'%s','%s',NOW())",check_code.c_str(),
								standard.c_str(),grade.c_str(),condition.c_str(),"",username.c_str(),time.c_str(),"","","",company_code.c_str(),user_code.c_str()
								);
							cout<<sqlbuf<<endl;

							try{
								rtn=con->ModifyQuery(sqlbuf);
							}

							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
							if(rtn!=0){
								printf("%s\n",con->GetErrMsg());
								CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
								return false;
							}

							string buf="OK";
							CHK(send(client, buf.c_str(), buf.length(), 0));
							con->CloseConnect();
							cout<<"===============add yaocai check end============="<<endl;
							}
		else if(!method.compare("addyinpianbase")){
							string tablet_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string tablet_name=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string tablet_weight=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string batchnumber=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string spec=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string med_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string company_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string user_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string sourceid=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string company_cpc=srecv;
							cout<<"==============add yinpian base start=================="<<endl;
								CEncapMysql *con;
								con = new CEncapMysql;

								if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
								{
									 CHK(send(client, "error:303", strlen("error:303"), 0));
									 return false;
								}
								try {
									con->ModifyQuery("use tcmbpspdb");
								} catch (exception &e) {
									printf("exception\n");
															CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
															con->CloseConnect();
															 return false;
								}
								char sqlbuf[0x200]={0};
								sprintf(sqlbuf,"select count(1),prodcode from tcm_goodscode where basecode='%s' and company_cpc='%s'",tablet_name.c_str(),company_cpc.c_str());
								cout<<sqlbuf<<endl;
								try{
									con->SelectQuery(sqlbuf);
								}

								catch(exception& e){
									printf("exception\n");
															CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
															con->CloseConnect();
															 return false;
								};
									char** r=NULL;
									string prodcode="";
									printf("ss\n");
									r=con->FetchRow();
									printf("ss\n");
									if (atoi(r[0])==0){

										cout<<"no prodcode"<<endl;
										prodcode=f.prodcode("zyczs_cd","000000",company_cpc,tablet_name);
										cout<<prodcode<<endl;
										if(prodcode.length()<5){
											con->CloseConnect();
											 CHK(send(client, "get prodcode from central web", strlen("get prodcode from central web"), 0));
											 cout<<"===============add yaocai base info end============="<<endl;
											 return false;
										}
//										int checknum=getCheckCode(prodcode);
//										char buf[2]={0};
//										sprintf(buf,"%d",checknum);
//										prodcode+=buf;
//										cout<<prodcode<<endl;
										sprintf(sqlbuf,"insert into tcm_goodscode(type,basecode,prodcode,company_cpc) values('0','%s','%s','%s')",sourceid.c_str(),prodcode.c_str(),company_cpc.c_str());
										cout<<sqlbuf<<endl;
										try {
											con->ModifyQuery(sqlbuf);
										} catch (exception &e) {
											printf("exception\n");
																	CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																	con->CloseConnect();
																	 return false;
										}

									}else{
										prodcode=r[1];
									}
									sprintf(sqlbuf,"call PROC_FINDID('1','%s',@yinpian_cpc)",company_cpc.c_str());
									cout<<sqlbuf<<endl;
									try{
										con->SelectQuery(sqlbuf);
									}
									catch(exception& e){
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									};
									//sprintf(sqlbuf,"select @med_cpc");
									//con->SelectQuery(sqlbuf);
									r=con->FetchRow();
									string tablet_cpc=r[0];
									cout<<tablet_cpc<<endl;
									int checkcode=getCheckCode(tablet_cpc);
									char bufcode[2]={0};
									sprintf(bufcode,"%d",checkcode);
									tablet_cpc+=bufcode;
									cout<<tablet_cpc<<endl;

									con->CloseConnect();
									if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
									{
										 CHK(send(client, "error:303", strlen("error:303"), 0));
										 return false;
									}
									try {
										con->ModifyQuery("use tcmbpspdb");
									} catch (exception &e) {
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									}
									sprintf(sqlbuf,"insert into tcm_tablet values("
											"'%s','%s','%s','%s','%s','%s',"
											"'%s','%s','%s','%s','%s',NOW())",
											tablet_code.c_str(),tablet_cpc.c_str(),
											tablet_name.c_str(),tablet_weight.c_str(),
											tablet_weight.c_str(),batchnumber.c_str(),
											spec.c_str(),med_code.c_str(),
											company_code.c_str(),user_code.c_str(),
											prodcode.c_str());
									cout<<sqlbuf<<endl;
									int rtn=0;
									try {
										rtn=con->ModifyQuery(sqlbuf);
									} catch (exception &e) {
										printf("exception\n");
																CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
																con->CloseConnect();
																 return false;
									}


									string buf="OK";
									if(rtn!=0){
										buf="add yaocai base info error"+rtn;
										cout<<con->GetErrMsg()<<endl;
									}
									cout<<buf<<endl;
									CHK(send(client, buf.c_str(), buf.length(), 0));
									con->CloseConnect();
									cout<<"===============add yaocai base info end============="<<endl;
								}
		else if(!method.compare("addyinpianassist")){
					string tabletassist_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string tablet_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);
					string name=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string number=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string weight=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string address=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string userate=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string med=srecv.substr(0,srecv.find(","));
					srecv=srecv.substr(srecv.find(",")+1);
					string company_code=srecv.substr(0,srecv.find(","));;
					srecv=srecv.substr(srecv.find(",")+1);

					string user_code=srecv;


					cout<<"==============add yinpian assist info start=================="<<endl;
					CEncapMysql *con;
					con = new CEncapMysql;

					if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
					{
						 CHK(send(client, "error:303", strlen("error:303"), 0));
						 return false;
					}
					int rtn=0;
					try {
						con->ModifyQuery("use tcmbpspdb");
					} catch (exception &e) {
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					}
					char sqlbuf[0x200]={0};
					sprintf(sqlbuf,"insert into tcm_tabletmiddle(tablet_code,other_code,other_type) values('%s','%s','tcm_tabletassist')",tablet_code.c_str(),tabletassist_code.c_str());
					cout<<sqlbuf<<endl;

					try{
						rtn=con->ModifyQuery(sqlbuf);
					}

					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
					if(rtn!=0){
						printf("%s\n",con->GetErrMsg());
						CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
						return false;
					}
					sprintf(sqlbuf,
						"insert into tcm_tabletassist values('%s','%s','%s','%s','%s','%s','%s','%s','%s',NOW())",tabletassist_code.c_str(),
						name.c_str(),number.c_str(),weight.c_str(),
						address.c_str(),userate.c_str(),med.c_str(),
						company_code.c_str(),user_code.c_str()
						);
					cout<<sqlbuf<<endl;

					try{
						rtn=con->ModifyQuery(sqlbuf);
					}

					catch(exception& e){
						printf("exception\n");
												CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
												con->CloseConnect();
												 return false;
					};
					if(rtn!=0){
						printf("%s\n",con->GetErrMsg());
						CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
						return false;
					}

					string buf="OK";
					CHK(send(client, buf.c_str(), buf.length(), 0));
					con->CloseConnect();
					cout<<"===============add yaocai assistinfo end============="<<endl;
					}
		else if(!method.compare("addyinpianmore")){
							string tabletinfo_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string tablet_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string times=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string standard=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string approval=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string validity=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string province=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string city=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string area=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string address=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);
							string company_code=srecv.substr(0,srecv.find(","));;
							srecv=srecv.substr(srecv.find(",")+1);

							string user_code=srecv;

							string image="";
							cout<<"==============add yinpian more info start=================="<<endl;
							CEncapMysql *con;
							con = new CEncapMysql;

							if(con->Connect(db_host, db_user, db_pass)==-1)//mysql数据库连接失败
							{
								 CHK(send(client, "error:303", strlen("error:303"), 0));
								 return false;
							}
							int rtn=0;
							try {
								con->ModifyQuery("use tcmbpspdb");
							} catch (exception &e) {
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							}
							char sqlbuf[0x200]={0};
							sprintf(sqlbuf,"insert into tcm_tabletmiddle(tablet_code,other_code,other_type) values('%s','%s','tcm_tabletinfo')",tablet_code.c_str(),tabletinfo_code.c_str());
							cout<<sqlbuf<<endl;

							try{
								rtn=con->ModifyQuery(sqlbuf);
							}

							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
							if(rtn!=0){
								printf("%s\n",con->GetErrMsg());
								CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
								return false;
							}
							sprintf(sqlbuf,"insert into tcm_tabletinfo values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',NOW())",tabletinfo_code.c_str(),
								image.c_str(),times.c_str(),standard.c_str(),approval.c_str(),validity.c_str(),"","","",province.c_str(),city.c_str(),
								area.c_str(),address.c_str(),company_code.c_str(),user_code.c_str()
								);
							cout<<sqlbuf<<endl;

							try{
								rtn=con->ModifyQuery(sqlbuf);
							}

							catch(exception& e){
								printf("exception\n");
														CHK(send(client, "error:sql exception", strlen("error:sql exception"), 0));
														con->CloseConnect();
														 return false;
							};
							if(rtn!=0){
								printf("%s\n",con->GetErrMsg());
								CHK(send(client, con->GetErrMsg(), strlen(con->GetErrMsg()), 0));
								return false;
							}

							string buf="OK";
							CHK(send(client, buf.c_str(), buf.length(), 0));
							con->CloseConnect();
							cout<<"===============add yaocai more info end============="<<endl;
							}
		else {
			CHK(send(client, "API error", strlen("API error"), 0));
					return true;
		}

	return true;
}
int getCheckCode(string codeStr){
	char code[34]={0};
	strncpy(code ,codeStr.c_str(),34);
	char *p=code;
	int len=strlen(code);
	int w[34]={0};
	int i=0;
	for(;i<34;i++){
		w[i]=((int)pow(3,i+1))%10;
	}
	int f[34]={0};
	i=0;
	char buf[3]={0};
	for(;i<len-1;i++){
		memcpy(buf,p+17-i,2);
		buf[2]=0;
		f[i]=atoi(buf);
	}
	f[len-1]=2;
	int s=0;
	i=0;
	for(;i<len;i++){
		s=s+w[i]*f[i];
	}

	return s%10;

}
int handle_message(int client)
{
	char buf[BUF_SIZE], message[BUF_SIZE];
	bzero(buf, BUF_SIZE);
	bzero(message, BUF_SIZE);
	int len;
	CHK2(len,recv(client, buf, BUF_SIZE, 0));  //接受客户端信息
	if(len == 0)   //客户端关闭或出错，关闭socket，并从list移除socket
	{
		//printf("connect shut down\n");
		CHK(close(client));
		clients_list.remove(client);
	}
	else          //向客户端发送信息
	{
		progress(client,buf);
		 CHK(close(client));
		clients_list.remove(client);
	}
	return len;
}




int main() {

		if(getconfigstr("mysql","ip",db_host,32,"config.ini")){
				printf("get mysql ip error\n");
				return -1;
			}
		if(getconfigstr("mysql","user",db_user,32,"config.ini")){
				printf("get mysql user error\n");
				return -1;
			}
		if(getconfigstr("mysql","pass",db_pass,32,"config.ini")){
				printf("get pass error\n");
				return -1;
			}
		if(getconfigint("server","localport",&serverport,"config.ini")){
				printf("get server port error\n");
				return -1;
			}
		if(getconfigstr("gft","user",gftuser,32,"config.ini")){
						printf("get gft user error\n");
						return -1;
					}
		if(getconfigstr("gft","pass",gftpass,32,"config.ini")){
								printf("get gft pass error\n");
								return -1;
							}
	int listener;   //监听socket
	struct sockaddr_in addr, their_addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(serverport);
	//addr.sin_addr.s_addr = inet_addr("192.168.9.217");
	addr.sin_addr.s_addr = INADDR_ANY;
	socklen_t socklen;
	socklen = sizeof(struct sockaddr_in);

	static struct epoll_event ev, events[EPOLL_SIZE];
	ev.events = EPOLLIN | EPOLLET;     //对读感兴趣，边沿触发
	int epfd;  //epoll描述符
	int client, res, epoll_events_count;
	CHK2(listener, socket(PF_INET, SOCK_STREAM, 0));             //初始化监听socket
	setnonblocking(listener);                                    //设置监听socket为不阻塞
	CHK(bind(listener, (struct sockaddr *)&addr, sizeof(addr))); //绑定监听socket
//		int rtn=0;
//		while(1){
//			rtn=bind(listener,(struct sockaddr*)&addr,sizeof(addr));
//			if(rtn>0){
//				break;
//			}
//			perror("eva");
//			sleep(1000);
//		}
	CHK(listen(listener, 1));                                    //设置监听
	CHK2(epfd,epoll_create(EPOLL_SIZE));                         //创建一个epoll描述符，并将监听socket加入epoll
	ev.data.fd = listener;
	CHK(epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev));
	while(1)
	{
		CHK2(epoll_events_count,epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_RUN_TIMEOUT));
		for(int i = 0; i < epoll_events_count ; i++)
		{
			if(events[i].data.fd == listener)                    //新的连接到来，将连接添加到epoll中，
			{
				CHK2(client,accept(listener, (struct sockaddr *) &their_addr, &socklen));
				setnonblocking(client);
				ev.data.fd = client;
				CHK(epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev));
				clients_list.push_back(client);                  // 添加新的客户端到list
			}else
			{
				CHK2(res,handle_message(events[i].data.fd)); //注意：这里并没有调用epoll_ctl重新设置socket的事件类型，但还是可以继续收到客户端发送过来的信息
			}
		}
	}
	close(listener);
	close(epfd);
	return 0;
}
