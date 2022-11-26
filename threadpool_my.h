#ifndef THREADPOOL_MY_H
#define THREADPOOL_MY_H

#include <cstdio>
#include <exception>
#include <list>
#include <exception>
#include<iostream>
#include <pthread.h>

/* 线程同步机制的包装类 */

#include "locker_my.h"



template<typename T>
class threadpool
{
	public:
		threadpool(int thread_number=8,int max_requests=10000);

		bool append(T*requests);
		~threadpool();

	private:
		static void*worker(void*arg);
		void run();


	private:
		int m_thread_nubmer;
		int m_max_requests;
		std::list<T*>m_workqueue;
		pthread_t*m_threads;
		sem m_queuestat;  //是否有任务需要被处理
		locker m_queuelocker; //保护请求队列的互斥锁
		bool m_stop; //是否结束线程



};

template<typename T>
threadpool<T>::threadpool(int thread_number,int max_requests):
	m_thread_nubmer(thread_number),m_max_requests(max_requests),
	m_stop(false),m_threads(NULL)
{

	if(thread_number<=0 || max_requests<=0)
	{
		throw std::exception();

	}

	m_threads=new pthread_t[m_thread_nubmer];
	if (!m_threads) {
		throw std::exception();
	}

	for (int i = 0; i < thread_number; i++) 
	{
		std::cout << "create the" <<i<<"th thread" << std::endl;
		if ((pthread_create(&m_threads[i],NULL,worker,this))!=0) {
			delete [] m_threads;
			throw std::exception();			
		}

		if (pthread_detach(m_threads[i])) {

			delete [] m_threads;
			throw std::exception();			

		}
	}


}

	template<typename T>
threadpool<T>::~threadpool<T>()
{
	delete []m_threads;
	m_stop=true;


}

	template<typename T>
bool threadpool<T>::append(T*requests)
{
	m_queuelocker.lock();
	if (m_workqueue.size()>m_max_requests) {
		m_queuelocker.unlock();
		return false;
	}
	m_workqueue.push_back(requests);
	m_queuelocker.unlock();
	m_queuestat.post();
	return true;

}
	template<typename T>
void*threadpool<T>::worker(void*arg)
{
	threadpool<T>*pool=(threadpool<T>*)arg;
	pool->run();
	return pool;

}

	template<typename T>
void threadpool<T>::run()
{
	while (!m_stop) 
	{	

		m_queuestat.wait();
		m_queuelocker.lock();
		if(m_workqueue.empty())
		{
			m_queuelocker.unlock();
			continue;
		}
		T*request= m_workqueue.front();
		m_workqueue.pop_front();
		m_queuelocker.unlock();
		if(!request)
		{
			continue;

		}
		request->process();

	}

}


#endif /* THREADPOOL_MY_H */
