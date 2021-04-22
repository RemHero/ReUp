#include "TCPServer.h"
#include "global.h"
#include "queue_pool.h"


char TCPServer::msg[MAXPACKETSIZE];
int TCPServer::num_client;
int TCPServer::last_closed;
bool TCPServer::isonline;
queue<descript_socket*> TCPServer::Message;
vector<descript_socket*> TCPServer::newsockfd;
std::mutex TCPServer::mt;
std::mutex TCPServer::mut[10];
long long recNum=0;

void* TCPServer::Task(void *arg)
{
	char msgT[MAXT];//???MAXT
	int n;
	struct descript_socket *desc = (struct descript_socket*) arg;
	pthread_detach(pthread_self());

        cerr << "open client[ id:"<< desc->id <<" ip:"<< desc->ip <<" socket:"<< desc->socket<<" send:"<< desc->enable_message_runtime <<" ]" << endl;
	while(1) 
	{
		//cout << n <<endl;
		//sleep(1);
		n = recv(desc->socket, msgT, MAXT, 0);//???MAXT
		if(true) 
		{
			if(n==-1) 
			{
				isonline = false;//!!!
				cerr << "close client[ id:"<< desc->id <<" ip:"<< desc->ip <<" socket:"<< desc->socket<<" ]" << endl;
				last_closed = desc->id;
				close(desc->socket);

				int id = desc->id;

				mut[1].lock();//LZH
				auto new_end = std::remove_if(newsockfd.begin(), newsockfd.end(),
													[id](descript_socket *device)
														{ return device->id == id; });
			   	newsockfd.erase(new_end, newsockfd.end());
			   	if(num_client>0) num_client--;
			   	mut[1].unlock();//LZH
			   	break;
			}
			msg[n]=0;
			desc->message = string(msgT);
	        //std::lock_guard<std::mutex> guard(mt);//LZH
			mut[2].lock();//LZH
			Message.push( desc );
			// recNum++;
			// if(recNum%R1==0)
				sem_post(&ROsem);
			mut[2].unlock();//LZH
		}
		//usleep(600);
    }
	// if(desc != NULL)//这里怎么可能能释放呢？肯定要用完再释放。
	// 	free(desc);
	cerr << "exit thread: " << this_thread::get_id() << endl;
	cerr << "-------------------3" << endl;
	pthread_exit(NULL);
	return 0;
}

int TCPServer::setup(int port, vector<int> opts)
{
	int opt = 1;
	isonline = false;
	last_closed = -1;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
 	memset(&serverAddress,0,sizeof(serverAddress));

	for(unsigned int i = 0; i < opts.size(); i++) {
		if( (setsockopt(sockfd, SOL_SOCKET, opts.size(), (char *)&opt, sizeof(opt))) < 0 ) {
			cerr << "Errore setsockopt" << endl; 
      			return -1;
	      	}
	}

	serverAddress.sin_family      = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port        = htons(port);

	if((::bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress))) < 0){
		cerr << "Errore bind" << endl;
		return -1;
	}
	
 	if(listen(sockfd,5) < 0){
		cerr << "Errore listen" << endl;
		return -1;
	}
	num_client = 0;
	isonline = true;
	return 0;
}

void TCPServer::accepted()//这里需要对队列进行加锁访问
{		
		socklen_t sosize    = sizeof(clientAddress);
		descript_socket *so = new descript_socket;
		so->socket          = accept(sockfd,(struct sockaddr*)&clientAddress,&sosize);
		
		//cout <<"--------------\n";

		mut[0].lock();//LZH
		so->id              = num_client;
		so->ip              = inet_ntoa(clientAddress.sin_addr);
		newsockfd.push_back( so );
		cerr << "accept client[ id:" << newsockfd[num_client]->id << 
							" ip:" << newsockfd[num_client]->ip << 
						" handle:" << newsockfd[num_client]->socket << " ]" << endl;
		pthread_create(&serverThread[num_client], NULL, &Task, (void *)newsockfd[num_client]);
		isonline=true;
		num_client++;
		mut[0].unlock();//LZH
}

// vector<descript_socket*> TCPServer::getMessage()//Version 1.0只返回一个值进行处理
// {
// 	std::lock_guard<std::mutex> guard(mt);
// 	return Message;
// }

queue<descript_socket*> TCPServer::getMessage()//Version 1.0只返回一个值进行处理,这样也不需要clean函数
{
	std::lock_guard<std::mutex> guard(mt);
	// return Message;
	queue<descript_socket*> MT;
	int k=0;
	while(!Message.empty()){
		k++;
		MT.push(Message.front());
		Message.pop();
		if(k>=R1) break;
	}
	return MT;
}

void TCPServer::Send(string msg, int id)
{
	send(newsockfd[id]->socket,msg.c_str(),msg.length(),0);
}

int TCPServer::get_last_closed_sockets()
{
	return last_closed;
}

// void TCPServer::clean(int id)
// {
// 	Message[id]->message = "";
// 	memset(msg, 0, MAXPACKETSIZE);
// }

string TCPServer::get_ip_addr(int id)
{
	return newsockfd[id]->ip;
}

bool TCPServer::is_online() 
{
	return isonline;
}

void TCPServer::detach(int id)
{
	close(newsockfd[id]->socket);
	newsockfd[id]->ip = "";
	newsockfd[id]->id = -1;
	newsockfd[id]->message = "";
} 

void TCPServer::closed() 
{
	close(sockfd);
}
