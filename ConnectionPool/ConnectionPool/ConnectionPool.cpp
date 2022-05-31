#include "ConnectionPool.h"
#include<json/json.h>
#include<fstream>
#include<thread>
using namespace Json;
ConnectionPoll* ConnectionPoll::getConnectionPoll()
{
	static ConnectionPoll pool;
	return &pool;
}
//����ָ�����
shared_ptr<Mysqlconn> ConnectionPoll::getConnection()
{
	//�����ӳ��Ƿ�Ϊ�յ� Ϊ������һ��
	unique_lock<mutex>locker(m_mutexQ);
	while (m_connectionQ.empty()) {
		if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout))){
			if(m_connectionQ.empty()) 
			{
				//return nullptr;
				continue;
			}
		}
	}
	//����ָ�� ���� ���� ���·Żص�����
	shared_ptr<Mysqlconn> connptr(m_connectionQ.front(), [this] (Mysqlconn* conn){
		lock_guard<mutex>locker(m_mutexQ);
		m_connectionQ.push(conn);
		conn->refreshAliveTime();
		});
	m_connectionQ.pop();
	m_cond.notify_all();
	return connptr;
}

ConnectionPoll::~ConnectionPoll()
{
	while (!m_connectionQ.empty()) {
		Mysqlconn* conn = m_connectionQ.front();
		m_connectionQ.pop();
		delete conn;
	}
}

bool ConnectionPoll::parseJsonFile()
{
	ifstream ifs("dbconf.json");
	Reader rd;
	Value root;
	rd.parse(ifs, root);
	//�Ƿ�δjson����
	//����json����
	if (root.isObject()) {
		m_ip = root["ip"].asString();
		m_port = root["port"].asInt();
		m_user = root["userName"].asString();
		m_password = root["password"].asString();
		m_dbName = root["dbName"].asString();
		m_minSize = root["minSize"].asInt();
		m_maxSize = root["maxSize"].asInt();
		m_maxIdleTime = root["maxIdleTime"].asInt();
		m_timeout = root["timeout"].asInt();
		return true;

	}
	return false;
}

void ConnectionPoll::produceConnection()
{
	while (true) {
		//�ж����ӳص������Ƿ���
		//�����Ӷ��������Ӹ���С����С��������ʱ��
		//�漰�����̷߳��� ����
		unique_lock<mutex>locker(m_mutexQ);
		while (m_connectionQ.size()>=m_minSize) {
			//������������
			m_cond.wait(locker);
		}
		//�����µ�����
		addConnection();
		//���������������Ѻ���
		m_cond.notify_all();

	}
}
void ConnectionPoll::recycleConnection()
{
	//�����Լ��
	while (true) {
		//ÿ��������һ��
		this_thread::sleep_for(chrono::milliseconds(500));
		//�漰�����̷߳��� ����
		lock_guard<mutex>locker(m_mutexQ);
		while (m_connectionQ.size() > m_minSize) {
			Mysqlconn* conn = m_connectionQ.front();
			//������������� ɾ����ͷ
			if (conn->getAliveTime()> m_maxIdleTime) {
				m_connectionQ.pop();
				delete conn;
			}
			else {
				break;
			}
		}
	}
}

void ConnectionPoll::addConnection()
{
	Mysqlconn* conn = new Mysqlconn;
	conn->connect(m_user, m_password, m_dbName, m_ip, m_port);
	//�ɹ����Ӻ� ���浽 ���ӳض�����
	//������ʱ���
	conn->refreshAliveTime();
	m_connectionQ.push(conn);
}


//���ӳصĹ��캯��
ConnectionPoll:: ConnectionPoll() {
	//���������ļ�
	if (!parseJsonFile()) {
		return;
	}
	for (int i = 0; i < m_minSize; i++) {
		//ʵ���� ���ݿ����Ӷ���
		//��ʵ������Сֵ  100��
		addConnection();
	}
	//�����߳̽��� ���� ������
	thread producer(&ConnectionPoll::produceConnection,this);
	thread recycler(&ConnectionPoll::recycleConnection,this);
	producer.detach();
	recycler.detach();
}