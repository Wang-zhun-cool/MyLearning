#pragma once
#include<queue>
#include<mutex>
#include<condition_variable>
#include"Mysqlconn.h"
using namespace std;
class ConnectionPoll
{
public:
	//ͨ����̬���� �õ�Ψһ�ĵ�������
	static ConnectionPoll* getConnectionPoll();
	ConnectionPoll(const ConnectionPoll& obj) = delete;
	ConnectionPoll& operation(const ConnectionPoll& obj) = delete;
	shared_ptr<Mysqlconn> getConnection();
	~ConnectionPoll();
private:
	//���캯��
	ConnectionPoll();

	//����json �����ļ�
	bool parseJsonFile();
	//�̵߳�������
	void produceConnection();
	void recycleConnection();
	//�������
	void addConnection();
	//д��json �����ļ���
	string m_ip;
	string m_user;
	string m_password;
	string m_dbName;
	unsigned short m_port;
	//���ݿ����������Сֵ
	int m_minSize;
	int m_maxSize;
	//��ʱʱ��
	int m_timeout;
	//������ʱ��
	int m_maxIdleTime;
	//��
	queue<Mysqlconn*>m_connectionQ;
	mutex m_mutexQ;

	condition_variable m_cond;
};

