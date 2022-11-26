#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <fcntl.h>
#include <cstdlib>
#include <cassert>
#include <sys/epoll.h>
#include<libgen.h>


#include"locker_my.h"
#include"threadpool_my.h"
#include"http_conn_my.h"


#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000

extern int addfd(int epollfd,int fd,bool one_shoot);
extern int removefd(int epollfd,int fd);   //extern 的作用？？？


void addsig(int sig,void(*handler)(int),bool restart=true)
{
	struct sigaction sa;
	memset(&sa,'\0',sizeof(sa));
	sa.sa_handler=handler;
	if(restart)
	{
		sa.sa_flags|=SA_RESTART;
	}
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig,&sa,NULL)!=-1);
}


void show_error(int connfd,const char*info)
{
	printf("%s",info);
	send(connfd,info,strlen(info),0);
	close(connfd);
}


int main(int argc, char *argv[])
{
	if(argc<=2)
	{
		printf("usage: %s ip_address port_number\n",basename(argv[0]));
		return 1;
	}
	const char*ip=argv[1];
	int port=atoi(argv[2]);
	
	//忽略SIGPIPE信号
	addsig(SIGPIPE,SIG_IGN);

	//创建线程池
	threadpool<http_conn>*pool=NULL;
	try {
		pool=new threadpool<http_conn>;
	
	}catch(...) {
		return 1;
	}

	//预先为每个可能的客户连接分配一个http_conn对象
	http_conn*users=new http_conn[MAX_FD];
	assert(users);

	int listenfd=socket(PF_INET,SOCK_STREAM,0);
	assert(listenfd!=-1);
	struct linger tmp={1,0};
	setsockopt(listenfd,SOL_SOCKET,SO_LINGER,&tmp,sizeof(tmp));
	
	int ret=0;
	struct sockaddr_in address;
	bzero(&address,sizeof(address));
	address.sin_family=AF_INET;
	inet_pton(AF_INET,ip,&address.sin_addr);
	address.sin_port=htons(port);

	ret=bind(listenfd,(struct sockaddr*)&address,sizeof(address));
	assert(ret!=-1);

	ret=listen(listenfd,5);
	assert(ret!=-1);

	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd=epoll_create(5);
	assert(epollfd!=-1);
	addfd(epollfd,listenfd,false);
	http_conn::m_epollfd=epollfd;

	while(true)
	{
		int number=epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
		if((number<0)&&(errno!=EINTR))
		{
			printf("epoll failure\n");
			break;
		}
		for (int i = 0;i < number; i++) 
		{
			int sockfd=events[i].data.fd;
			if(sockfd==listenfd)
			{
				struct sockaddr_in client_address;
				socklen_t client_addrlength=sizeof(client_address);
				int connfd=accept(listenfd,(struct sockaddr*)&client_address,
						&client_addrlength);
				if(connfd<0)
				{
					printf("errno is :%d\n",errno);
					continue;
				}
				if(http_conn::m_user_count>=MAX_FD)
				{
					show_error(connfd,"Internal server busy");
				}
				//初始化客户连接
				users[connfd].init(connfd,address);
			}
			else if(events[i].events&(EPOLLRDHUP|EPOLLERR|EPOLLHUP))
			{
				//如果有异常，直接关闭客户端连接
				users[sockfd].close_conn();
				
			}
			else if(events[i].events&EPOLLIN)
			{
				//根据读的结果，决定是将任务添加到线程池还是关闭连接
				if(users[sockfd].m_read())
				{
					pool->append(users+sockfd);
				}
				else 
				{
					users[sockfd].close_conn();
				}

			}
			else if(events[i].events&EPOLLOUT)
			{
				if(!users[sockfd].m_write())
				{
					users[sockfd].close_conn();
				}

			}
			else
			{}
		}
	}

	close(epollfd);
	close(listenfd);
	delete[]users;
	delete pool;

	return 0;
}

