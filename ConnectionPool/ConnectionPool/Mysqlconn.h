#pragma once
using namespace std;
#include<iostream>
#include<mysql.h>
#include<chrono>
using namespace chrono;
class Mysqlconn
{
public:
	//��ʼ�����ݿ�����
	Mysqlconn();
	//�ͷ����ݿ�����
	~Mysqlconn();
	//�������ݿ�
	bool connect(string user, string passwd, string dbName, string ip, unsigned short port = 3306);
	//�������ݿ� insert, update,dalete
	bool update(string sql);
	//��ѯ���ݿ�
	bool query(string sql);

	//������ѯ�õ��Ľ����
	bool next();//ȡ
	//�õ�������е��ֶ�
	string value(int index);
	//�������  Ĭ���ύ
	bool transaction();
	//�ύ����
	bool commit();
	//����ع�
	bool rollback();
	//ˢ����ʼ�Ŀ���ʱ��
	void refreshAliveTime();
	//�������Ӵ�����ʱ��
	long long getAliveTime();

private:
	void freeResult();
	MYSQL* m_conn = nullptr;
	MYSQL_RES* m_result = nullptr;
	MYSQL_ROW m_row = nullptr;
	//ʱ�ӱ��� ����ʱ��
	steady_clock::time_point m_alivetime;
};

