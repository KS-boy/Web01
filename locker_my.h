#pragma once

#ifndef LOCKER_MY_H
#define LOCKER_MY_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/*封装信号量类*/
class sem
{
public:
	sem()
	{
		if (sem_init(&m_sem,0,0)!=0) {
			throw std::exception();
		};
	}
	 ~sem()
	 {
		 sem_destroy(&m_sem);
	 };

/* 等待信号量 */
bool wait()
{
	return sem_wait(&m_sem)==0;
}
/* 增加信号量 */
bool post()
{
	return sem_post(&m_sem);
}

private:
	sem_t m_sem;
	

};

/*! \class locker
 *  \brief 封装互斥锁的类
 *
 *  Detailed description
 */
class locker
{
public:
	/* 创建并初始化互斥锁 */	
	locker()
	{
		if (pthread_mutex_init(&m_mutex,NULL)!=0) {
			throw std::exception();
		}
	}
	/* destroy mutex */
	
	~locker()
	{
		pthread_mutex_destroy(&m_mutex);
	}


	/* fetch mutex */
	bool lock()
	{
		return pthread_mutex_lock(&m_mutex)==0;
	}


	/* release lock */
	bool unlock()
	{
		return pthread_mutex_unlock(&m_mutex)==0;
	}

private:
	pthread_mutex_t m_mutex; /*!< Member description */
};


/*! \class cond
 *  \brief 封装条件变量的类
 *
 *  Detailed description
 */
class cond
{
public:
	/* create and init cond */
	
	cond()
	{
		if (pthread_mutex_init(&m_mutex,NULL)!=0) {
			throw std::exception();
		}
		if (pthread_cond_init(&m_cond,NULL)!=0) {
			pthread_mutex_destroy(&m_mutex);
			throw std::exception();

		}
	}
	 ~cond()
	 {

		 pthread_mutex_destroy(&m_mutex);
		 pthread_cond_destroy(&m_cond);

	 }
	 /* wait m_cond */
	 bool wait()
	 {
		 int ret=0;
		 pthread_mutex_lock(&m_mutex);
		 ret=pthread_cond_wait(&m_cond,&m_mutex);
		 pthread_mutex_unlock(&m_mutex);
		 return ret==0;
	 }

	 /* awake the pthread who waits m_cond*/
	 bool signal()
	 {
		 return pthread_cond_signal(&m_cond)==1;

	 }

private:
	pthread_cond_t m_cond; /*!< Member description */
	pthread_mutex_t m_mutex;

};

#endif /* LOCKER_MY_H */
