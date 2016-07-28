#ifndef ENCAPSULATION_MYSQL_H_
#define ENCAPSULATION_MYSQL_H_
 
#include <iostream>
#include <cassert>
#include <set>
#include <sys/shm.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <memory>
#include <iconv.h>
#include <dlfcn.h>
#include <mysql/mysql.h>
 
using namespace std;
#define db_port 0
#define ERRMSG1(fmt,...)  ; sprintf(m_szErrMsg, fmt, __VA_ARGS__);
#define ERRMSG2(fmt,args...)  ; sprintf(m_szErrMsg, "[%s �� %d �� ]; "fmt"\r\n" , __FILE__, __LINE__, ##args);
namespace EncapMysql {
 
class CEncapMysql {
    typedef map<string, int> MapFieldNameIndex;
public:
    CEncapMysql();
    ~CEncapMysql();
public:
 
    int Connect(const char* szDbIp, const char* szUser, const char* szPassword);
 
    void CloseConnect();
 
    int SelectQuery(const char* szSQL);
 
    int ModifyQuery(const char* szSQL);
 
    const char* GetErrMsg();
 
    char** FetchRow();
 
    char* GetField(const char* szFieldName);
 
////////���ӳ��Ǹ�����Ҫ�õ���3������  2011-01-20
public:
    void SetUsed();
    void SetIdle();
    bool IsIdle(); //���� true ��ʶ Idle
private:
    bool m_bUseIdle;    // true: use;   false:idle
 
private:
 
    bool IsConnected();
 
    void SetConnected(bool bTrueFalse);
 
    char* GetField(unsigned int iFieldIndex);
 
    void FreePreResult();
 
    int ReConnect();
 
    void SaveParam(const char* szDbIp, const char* szUser,
            const char* szPassword);
 
public:
    bool m_bConnected;    //��ݿ���������?   true--�Ѿ�����;  false--��û������
    char m_szErrMsg[1024]; //��������, ������Ϣ���ڴ˴�
    int m_iFields; //�ֶθ���
    MapFieldNameIndex m_mapFieldNameIndex; //��һ��map,  key���ֶ���,  value���ֶ�����
public:
    MYSQL m_connection; //����
    MYSQL_RES* m_result; //���ָ��
    MYSQL_ROW m_row; //һ��,  typedef char **MYSQL_ROW;
 
private:
    string m_sDbIp; //��ݿ������IP
    string m_sUser; //�û���
    string m_sPassword; //����
};
} //end of namespace  EncapMysql
 
#endif /* ENCAPSULATION_MYSQL_H_ */
