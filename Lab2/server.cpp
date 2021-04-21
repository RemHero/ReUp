#include <iostream>
#include <csignal>
#include <ctime>
#include "global.h"
#include "queue_pool.h"
#include "TCPServer.h"
using namespace std;
#define B 1
struct T{
};
T* t=new T;

int R=8;
int R1=1;
int W=1;
int O1=1;
int O2=1;

sem_t ROsem;
pthread_mutex_t mutex1;
TCPServer tcp;
pthread_t msg1[MAX_CLIENT];
int num_message = 0;
int time_send   = 1;
vector<descript_socket*> descALL;
ThreadPool po2;

void close_app(int s) {
    tcp.closed();
    exit(0);
}

struct argT{
    string message;
	struct descript_socket *desc;
};
argT* T=new argT[B];

struct Message{
    string type;
    string path;
    string name,id;
};

void parse(string s,Message &m){//这里的解析是否会成为性能瓶颈？？
    //Message m;
    int beg=s.find_first_of(' ',2);
    int end=s.find(' ',beg+1);
    m.type=s.substr(0,beg);
    //cout << "type--------------\n" << m.type << endl;
    m.path=s.substr(beg,end-beg);
    if(m.type=="GET"){
        
        //cout << "path--------------\n" << m.path << endl;
    }
    else if(m.type=="POST"){
        int f1=s.find("\r\n",end);
        //cout << "f1--------------\n" << f1 << endl;
        int f2=s.find("Name=",end);
        //cout << "f2--------------\n" << f2 << endl;
        int f3=s.find("&",f1);
        //cout << "f3--------------\n" << f3 << endl;
        int f4=s.find("ID=",f2);
        //cout << "f4--------------\n" << f4 << endl;
        if(f1!=-1 && f2!=-1 && f4!=-1){
            m.name=s.substr(f2+5,f3-f2-5);
            m.id=s.substr(f4+3);
            //cout << "name--------------\n" << m.name << endl;
            //cout << "id--------------\n" << m.id << endl;
        }
    }
}

string sendCM(struct descript_socket *desc){
    Message m;//每次创建消耗时间
    string date;
    parse(desc->message,m);
    if(m.type=="GET" && m.path!="/index.html"){
        date = "HTTP/1.1 200 OK\r\n";

        date += "Server: ReUp Server\n";
        date += "Content-type: text/html\n";
        date += "Content-length: 107\n";

        date += "\r\n";

        date += "<html><title>Get the ReUp</title><body bgcolor=ffffff>\n";
        date += " GOOD! \n";
        date += "<hr><em>HTTP Web server</em>\n";
        date += "</body></html>\n";
    }else if(m.type=="POST" && !m.name.empty() && !m.id.empty() && m.path=="/Post_show"){
        date = "HTTP/1.1 200 OK\r\n";

        date += "Server: ReUp Server\n";
        date += "Content-type: text/html\n";
        date += "Content-length: 129\n";

        date += "\r\n";

        date += "<html><title>POST method</title><body bgcolor=ffffff>\n";
        date += " Your Name:  "+m.name+"\n";
        date += " ID:  "+m.id+"\n";
        date += "<hr><em>HTTP Web server</em>\n";
        date += "</body></html>\n";
    }
    else if(m.type=="DELETE"){
        date = "HTTP/1.1 501 Not Implemented\r\n";

        date += "Server: ReUp Server\n";
        date += "Content-type: text/html\n";
        date += "Content-length: 165\n";

        date += "\r\n";

        date += "<html><title>501 Not Implemented</title><body bgcolor=ffffff>\n";
        date += " Not Implemented\n";
        date += "<p>Dose not implement this mrthod: "+m.type+"\n";
        date += "<hr><em>HTTP Web server</em>\n";
        date += "</body></html>\n";
    }
    else if(m.type=="PUT"){
        date = "HTTP/1.1 501 Not Implemented\r\n";

        date += "Server: ReUp Server\n";
        date += "Content-type: text/html\n";
        date += "Content-length: 162\n";

        date += "\r\n";

        date += "<html><title>501 Not Implemented</title><body bgcolor=ffffff>\n";
        date += " Not Implemented\n";
        date += "<p>Dose not implement this mrthod: "+m.type+"\n";
        date += "<hr><em>HTTP Web server</em>\n";
        date += "</body></html>\n";
    }
    else if(m.type=="HEAD"){
        date = "HTTP/1.1 501 Not Implemented\r\n";

        date += "Server: ReUp Server\n";
        date += "Content-type: text/html\n";
        date += "Content-length: 163\n";

        date += "\r\n";

        date += "<html><title>501 Not Implemented</title><body bgcolor=ffffff>\n";
        date += " Not Implemented\n";
        date += "<p>Dose not implement this mrthod: "+m.type+"\n";
        date += "<hr><em>HTTP Web server</em>\n";
        date += "</body></html>\n";
    }
    else{
        date = "HTTP/1.1 404 Not Found\r\n";

        date += "Server: Lab Web Server\n";
        date += "Content-type: text/html\n";
        date += "Content-length: 112\n";

        date += "\r\n";

        date += "<html><title>404 Not Found</title><body bgcolor=ffffff>\n";
        date += " Not Found \n";
        date += "<hr><em>HTTP Web server</em>\n";
        date += "</body></html>\n";
    }
    return date;
}


void * send_client(void * m) {
	struct argT *arg=(struct argT*) m;
    struct descript_socket *desc =  (struct descript_socket*)(arg->desc);
    // while(1) {
        // if(!tcp.is_online() && tcp.get_last_closed_sockets() == desc->id) {
        //     cerr << "Connessione chiusa: stop send_clients( id:" << desc->id << " ip:" << desc->ip << " )"<< endl;
        //     break;
        // }//这部分是对连接的持久性进行测试，过于高端，实验用不上，咱直接发送，鲁棒性永远滴神
        
        string date=sendCM(desc);
        tcp.Send(date, desc->id);
        //sem_post(&ROsem);
        //sleep(time_send);
    // }
    free(desc);
    cerr << "-------------------2" << endl;
    //pthread_exit(NULL);
    return 0;
}

void * received(void * m)//之后的优化可以考虑一次分配几个任务!!!
{
    //pthread_detach(pthread_self());//这里不需要，我们用的是线程池
    while(1){
        sem_wait(&ROsem);
        queue<descript_socket*> descT;
        pthread_mutex_lock(&mutex1);
        descT = tcp.getMessage();
        pthread_mutex_unlock(&mutex1);
        while(!descT.empty()) {
            descript_socket* desc=descT.front();
            descT.pop();
                if(!desc->enable_message_runtime)
                {
                    desc->enable_message_runtime = true;
					T->desc=desc;
					T->message=desc->message;
                    //descALL.push_back(desc[i]);
                    po2.dispatch(send_client,(void *) T);
                    // if( pthread_create(&msg1[num_message], NULL, send_client, (void *) T) == 0) {
                    //     cerr << "ATTIVA THREAD INVIO MESSAGGI" << endl;
                    // }
                    // start message background thread
                }

                cout << "id:      " << desc->id      << endl
                     << "ip:      " << desc->ip      << endl
                     << "message: " << desc->message << endl
                     << "socket:  " << desc->socket  << endl
                     << "enable:  " << desc->enable_message_runtime << endl;
                //sem_wait(&ROsem);
                //tcp.clean(i);
        }
    }
    return 0;
}

void* LHZFUN(void *arg){
    while(1){//accepted()如果没有接受到连接请求，则会自己休眠，所以直接while(1)即可
    cerr << "-------------------1" << endl;
        tcp.accepted();
    }
}

int main(int argc, char **argv)
{
    if(argc < 7) {
        cerr << "Usage: ./server port (opt)time-send" << endl;
        return 0;
    }
    if (sem_init(&ROsem,0,0)) {
        printf("Semaphore initialization failed!!\n");
        exit(EXIT_FAILURE);
    }
    

    std::signal(SIGINT, close_app);
    pthread_mutex_init(&mutex1,NULL);
    O2=atoi(argv[6]);
    ThreadPool po1(O1);
    ThreadPool pw(W);
    ThreadPool pr(R);
    po2.setNum(O2);
	pr.run();po1.run();po2.run();pw.run();
    pthread_t msg;
    vector<int> opts = { SO_REUSEPORT, SO_REUSEADDR };


    if( tcp.setup(atoi(argv[4]),opts) == 0) {
        for(int i=0;i<O1;i++){
            po1.dispatch(received,(void *)0);
        }  
        for(int i=0;i<R;i++){
            pr.dispatch(LHZFUN,(void*)0);
        }
        cerr << "Accepted" << endl;
        string cmd;
        cin >> cmd;//cin阻塞
        if(cmd=="quit") return 0;
    }
    else
        cerr << "Errore apertura socket" << endl;
    cerr << "-------------------" << endl;
    return 0;
}
