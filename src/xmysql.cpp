#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <set>
#include <map>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include "xmysql.h"

using namespace std;
using namespace EncapMysql;
 
CEncapMysql::CEncapMysql() {
    SetConnected(false);
    //�ѽ����Ϊ��
    m_result = NULL;
    //��ʼ������
    mysql_init(&m_connection);

}
CEncapMysql::~CEncapMysql() {
    //�ͷ���һ�εĽ��
    FreePreResult();
    //�ر���ݿ�����
    CloseConnect();
}
 
int CEncapMysql::Connect(const char* szDbIp, const char* szUser,
        const char* szPassword) {
    SaveParam(szDbIp, szUser, szPassword);
    //���ж��Ƿ��Ѿ�������, ��ֹ�ظ�����
    if (IsConnected())
        return 0;
    //������ݿ�
    if (mysql_real_connect(&m_connection, szDbIp, szUser, szPassword, NULL, db_port,
            NULL, 0) == NULL) {
        ERRMSG2("%s", mysql_error(&m_connection));
        return -1;
    }
    mysql_set_character_set(&m_connection,"utf8");
   // printf("[mysql] conn to %s [user:%s] succ!\r\n", szDbIp, szUser);
    //�������ӱ�־Ϊ true
    SetConnected(true);
    return 0;
}
 
void CEncapMysql::CloseConnect() {
    //����m_connection���Ƿ����ӹ� ����رն�����������
    mysql_close(&m_connection);
    SetConnected(false);
}
 
int CEncapMysql::SelectQuery(const char* szSQL) {
    //����ѯ���ǿ�ָ��,�򷵻�
    if (szSQL == NULL) {
        ERRMSG2("%s", "szSQL==NULL");
        return -1;
    }
    //���û������,�򷵻�
    if (!IsConnected()) {
        ERRMSG2("%s", "��û�н�������");
        return -2;
    }
    try //��Щ����������йأ����쳣ʱ������
    {
        //��ѯ
        if (mysql_real_query(&m_connection, szSQL, strlen(szSQL)) != 0) {
            ERRMSG2("%s", mysql_error(&m_connection));
            printf("%s", mysql_error(&m_connection));
            printf("ReConnect()  is called, select111  !!!***\r\n");
            int nRet = ReConnect();
            if (nRet != 0)
                return -3;
            //
            if (mysql_real_query(&m_connection, szSQL, strlen(szSQL)) != 0)
                return -33;
            //
        }
        //�ͷ���һ�εĽ��
        FreePreResult();
        //ȡ���
        m_result = mysql_store_result(&m_connection);
        if (m_result == NULL) {
            ERRMSG2("%s", mysql_error(&m_connection));
            return -4;
        }

    } catch (...) {
        printf("ReConnect()  is called, select  !!!***\r\n");
        ReConnect();
        return -5;
    }
    //ȡ�ֶεĸ���
    m_iFields = mysql_num_fields(m_result);
    m_mapFieldNameIndex.clear();
    //ȡ�����ֶε�������Ϣ
    MYSQL_FIELD *fields;
    fields = mysql_fetch_fields(m_result);
    //���ֶ����ֺ�����浽һ��map��
    for (unsigned int i = 0; i < m_iFields; i++) {
        m_mapFieldNameIndex[fields[i].name] = i;
    }
    return 0;
}
 
int CEncapMysql::ModifyQuery(const char* szSQL) {
    //����ѯ���ǿ�ָ��,�򷵻�
    if (szSQL == NULL) {
        ERRMSG2("%s", "szSQL==NULL");
        return -1;
    }
    //���û������,�򷵻�
    if (!IsConnected()) {
        ERRMSG2("%s", "��û�н�������");
        return -2;
    }
    try //��Щ����������йأ����쳣ʱ������
    {
        //��ѯ, ʵ���Ͽ�ʼ������޸���ݿ�
        if (mysql_real_query(&m_connection, szSQL, strlen(szSQL)) != 0) {
            ERRMSG2("%s", mysql_error(&m_connection));
            return -3;
        }
    } catch (...) {
        printf("ReConnect()  is called  ,modify!!!***\r\n");
        ReConnect();
        return -5;
    }
    return 0;
}
 
char** CEncapMysql::FetchRow() {
    //�����Ϊ��,��ֱ�ӷ��ؿ�; ����FetchRow֮ǰ, �����ȵ��� SelectQuery(...)
    if (m_result == NULL)
        return NULL;
    //�ӽ����ȡ��һ��
    m_row = mysql_fetch_row(m_result);
    return m_row;
}
 
char* CEncapMysql::GetField(const char* szFieldName) {
    return GetField(m_mapFieldNameIndex[szFieldName]);
}
 
char* CEncapMysql::GetField(unsigned int iFieldIndex) {
    //��ֹ�������Χ
    if (iFieldIndex >= m_iFields)
        return NULL;
    return m_row[iFieldIndex];
}
 
void CEncapMysql::FreePreResult() {
 
    if (m_result != NULL) {
        mysql_free_result(m_result);
        m_result = NULL;
    }
}
 
const char* CEncapMysql::GetErrMsg() {
    return m_szErrMsg;
}
 
bool CEncapMysql::IsConnected() {
    return m_bConnected;
}
 
void CEncapMysql::SetConnected(bool bTrueFalse) {
    m_bConnected = bTrueFalse;
}
 
void CEncapMysql::SaveParam(const char* szDbIp, const char* szUser,
        const char* szPassword) {
    m_sDbIp = szDbIp; //��ݿ������IP
    m_sUser = szUser; //�û���
    m_sPassword = szPassword; //����
}
 
int CEncapMysql::ReConnect() {
    CloseConnect();
    //������ݿ�
    if (mysql_real_connect(&m_connection, m_sDbIp.c_str(), m_sUser.c_str(),
            m_sPassword.c_str(), NULL, 0, NULL, 0) == NULL) {
        ERRMSG2("%s", mysql_error(&m_connection));
        return -1;
    }
    //�������ӱ�־Ϊ true
    SetConnected(true);
    return 0;
}
/////////////////////////  ���ӳ��Ǹ�����Ҫ�õ���3������
void CEncapMysql::SetUsed() {
    m_bUseIdle = true;
}
void CEncapMysql::SetIdle() {
    m_bUseIdle = false;
}
//�����У�����true
bool CEncapMysql::IsIdle() {
    return !m_bUseIdle;
}
