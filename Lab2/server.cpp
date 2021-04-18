#include <iostream>
#include <csignal>
#include <ctime>
#include "TCPServer.h"
#include "TCPServer.h"
using namespace std;
#define B 1
sem_t ROsem;
TCPServer tcp;
pthread_t msg1[MAX_CLIENT];
int num_message = 0;
int time_send   = 1;

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
    while(1) {
        if(!tcp.is_online() && tcp.get_last_closed_sockets() == desc->id) {
            cerr << "Connessione chiusa: stop send_clients( id:" << desc->id << " ip:" << desc->ip << " )"<< endl;
            break;
        }
        
        string date=sendCM(desc);
        tcp.Send(date, desc->id);
        sem_post(&ROsem);
        sleep(time_send);
    }
    pthread_exit(NULL);
    return 0;
}

void * received(void * m)
{
    pthread_detach(pthread_self());
    vector<descript_socket*> desc;
    while(1)
    {
        desc = tcp.getMessage();

        for(unsigned int i = 0; i < desc.size(); i++) {
            if( desc[i]->message != "" )
            {
            	
                if(!desc[i]->enable_message_runtime)
                {
                    desc[i]->enable_message_runtime = true;
					T->desc=desc[i];
					T->message=desc[i]->message;
                    if( pthread_create(&msg1[num_message], NULL, send_client, (void *) T) == 0) {
                        cerr << "ATTIVA THREAD INVIO MESSAGGI" << endl;
                    }
                    num_message++;
                    // start message background thread
                }

                //cout << "-----------------------------------\n"
                
                cout << "id:      " << desc[i]->id      << endl
                     << "ip:      " << desc[i]->ip      << endl
                     << "message: " << desc[i]->message << endl
                     << "socket:  " << desc[i]->socket  << endl
                     << "enable:  " << desc[i]->enable_message_runtime << endl;
                sem_wait(&ROsem);
                tcp.clean(i);
            }
        }
        usleep(1000);
    }
    return 0;
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
    /*
    if(argc == 3)
        time_send = atoi(argv[2]);
    */
    std::signal(SIGINT, close_app);

    pthread_t msg;
    vector<int> opts = { SO_REUSEPORT, SO_REUSEADDR };
    //cout << argv[4] << endl;
    if( tcp.setup(atoi(argv[4]),opts) == 0) {
        if( pthread_create(&msg, NULL, received, (void *)0) == 0)
        {
            while(1) {
                tcp.accepted();
                cerr << "Accepted" << endl;
            }
        }
    }
    else
        cerr << "Errore apertura socket" << endl;
    return 0;
}
