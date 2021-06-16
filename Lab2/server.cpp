#include <iostream>
#include <csignal>
#include <ctime>
#include "global.h"
#include "queue_pool.h"
#include "TCPServer.h"
#include "TCPClient.h"
#include <map>

using namespace std;
#define B 1

int R=1;   //接受请求accepted()
int R1=1;  //receive每次处理的最大线程数量
int W=30;    //目前没用到
int O1=1;  //取出任务的线程数量
int O2=1;   //处理线程个数

sem_t ROsem;
pthread_mutex_t mutex1;
TCPServer tcp;
ThreadPool po2;
ThreadPool pw;
bool flagproxy=false;
string proxy;
int destport=80;
map<string,string> cacheforweb;

void close_app(int s) {//CTRL C 调用，终止所有线程并退出
    tcp.closed();
    exit(0);
}

struct Message{
    string type;
    string path;
    string name,id;
    int port=0;
};

void parse(string s,Message &m){//这里的解析是否会成为性能瓶颈？？
    //Message m;
    int beg=s.find_first_of(' ',2);
    int end=s.find(' ',beg+1);
    m.type=s.substr(0,beg);
    //cout << "type--------------\n" << m.type << endl;
    m.path=s.substr(beg+1,end-beg-1);
    if(m.path.find("http://")!=m.path.npos){
        m.path=m.path.substr(m.path.find("http://")+7);//这里默认是http://www.baidu.com/的格式
        if(m.path.find("/")!=m.path.npos){
            m.path=m.path.substr(0,m.path.find("/"));
        }
        if(m.path.find(":")!=m.path.npos){
            m.port=stoi(m.path.substr(m.path.find(":")+1,m.path.length()));
            m.path=m.path.substr(0,m.path.find(":"));
        }
        else{
            m.port=80;
        }
    }
    else if(m.path.find("https://")!=m.path.npos){
        m.path=m.path.substr(m.path.find("https://")+8);//这里默认是http://www.baidu.com/的格式
        if(m.path.find("/")!=m.path.npos){
            m.path=m.path.substr(0,m.path.find("/"));
        }
        if(m.path.find(":")!=m.path.npos){
            m.port=stoi(m.path.substr(m.path.find(":")+1,m.path.length()));
        }
        else{
            m.port=80;
        }
    }
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
        // cout << "f4--------------\n" << f4 << endl;
        if(f1!=-1 && f2!=-1 && f4!=-1){
            m.name=s.substr(f2+5,f3-f2-5);
            m.id=s.substr(f4+3);
            //cout << "name--------------\n" << m.name << endl;
            // cout << "id--------------------------------------------------------------------\n" << m.id << endl;
        }
    }
}

string sendCM(void *arg){
    struct descript_socket *desc =  (struct descript_socket*)(arg);
    Message m;//每次创建消耗时间
    string date;
    // cout << "================================================ \n" <<  desc->message << "\n=============================================\n";
    parse(desc->message,m);
    // cout << "type " << m.type  << " path " << m.path << "--------------\n";
    // if(m.path=="/Post_show") cout << "YES\n";
    // cout << m.name << ' ' << m.id << endl;
    if(m.type=="GET" && m.path=="/index.html"){
        date = "HTTP/1.1 200 OK\r\nServer: ReUp Server\nContent-type: text/html\nContent-length: 107\n\r\n<html><title>Get the ReUp</title><body bgcolor=ffffff>\n GOOD! \n<hr><em>HTTP Web server</em>\n</body></html>\n";
    }else if(m.type=="POST" && !m.name.empty() && !m.id.empty() && m.path=="/Post_show"){//119
        int lenT=m.name.length()+m.id.length();
        string lengthT=to_string(lenT+119);
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
        date += "<p>Dose not implement this method: "+m.type+"\n";
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
        date += "<p>Dose not implement this method: "+m.type+"\n";
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
        date += "<p>Dose not implement this method: "+m.type+"\n";
        date += "<hr><em>HTTP Web server</em>\n";
        date += "</body></html>\n";
    }
    else if(m.type=="GET" && m.path==proxy && destport==m.port){//使用代理
        cerr<<"Using proxy: "<<m.path<<endl;
        TCPClient tcpc;
        if(tcpc.setup(m.path,destport)==false){///////////////////////////////////////////////////////////////////////////////////////////
            cerr<<"ERROR: create tcp to upstream server failed!\n";
            date = "HTTP/1.1 404 Not Found\r\n";

            date += "Server: ReUp Server\n";
            date += "Content-type: text/html\n";
            date += "Content-length: 112\n";

            date += "\r\n";

            date += "<html><title>404 Not Found</title><body bgcolor=ffffff>\n";
            date += " Not Found \n";
            date += "<hr><em>HTTP Web server</em>\n";
            date += "</body></html>\n";
            return date;
        }
        if(cacheforweb.find(m.path)!=cacheforweb.end()){//如果缓存里有，那么发送的包加上if-modified-since
            desc->message=desc->message.insert(desc->message.find_first_of('\n')+1,"If-Modified-Since: "+cacheforweb[m.path].substr(cacheforweb[m.path].find("Last-Modified: ")+15,cacheforweb[m.path].find_first_of('\n',cacheforweb[m.path].find("Last-Modified: "))-cacheforweb[m.path].find_first_of(' ',cacheforweb[m.path].find("Last-Mo"))));
            cout << "========================================================yes cache!\n";
        }
        if(tcpc.Send(desc->message)==false){
            cerr<<"ERROR: send data to upstream server failed!\n";
            date = "HTTP/1.1 404 Not Found\r\n";

            date += "Server: ReUp Server\n";
            date += "Content-type: text/html\n";
            date += "Content-length: 112\n";

            date += "\r\n";

            date += "<html><title>404 Not Found</title><body bgcolor=ffffff>\n";
            date += " Not Found \n";
            date += "<hr><em>HTTP Web server</em>\n";
            date += "</body></html>\n";
            return date;
        }
        date=tcpc.receive();
        // cout << "========================================================NOOOOOOOO!\n";
        //     cout << date << endl;
        //     cout << "========================================================NOOOOOOO!\n";
        if(date.find("Last-Modified: ")!=date.npos&&date.substr(9,3)=="200"){//如果返回的response中有modified信息，将其response存入缓存
            cacheforweb[m.path]=date;
        }
        else if(date.substr(9,3)=="304"){
            // date=cacheforweb[m.path];
            date=cacheforweb[m.path];
            date=date.insert(date.find_first_of('\n')+1,"Using-Cache-From: 127.0.0.1\n");
        }
        tcpc.exit();
        return date;
    }
    else{
        date = "HTTP/1.1 404 Not Found\r\n";

        date += "Server: ReUp Server\n";
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

void* LHZSEND(void *arg){
    struct descript_socket *desc =  (struct descript_socket*)(arg);
    // cout << "ok3==================================\n";
    tcp.Send(desc,desc->message);
    // cout << "ok4==================================\n";
    // }
    tcp.CloseConnection(desc);
    // cout << "ok5==================================\n";
    return 0;
}

void * send_client(void * m) {
    // pthread_detach(pthread_self());
    // cerr << "-------------------begin send_client" << endl;
    struct descript_socket *desc =  (struct descript_socket*)(m);
    // while(1) {
        // if(!tcp.is_online() && tcp.get_last_closed_sockets() == desc->id) {
        //     cerr << "Connessione chiusa: stop send_clients( id:" << desc->id << " ip:" << desc->ip << " )"<< endl;
        //     break;
        // }//这部分是对连接的持久性进行测试，过于高端，实验用不上，咱直接发送，鲁棒性永远滴神
        // string date;
        desc->message=sendCM(m);
        // cout << "ok==================================\n";
        pw.dispatch(LHZSEND,desc);
        // cout << "ok2==================================\n";
            
    //     tcp.Send(desc,date);
    // // }
    // tcp.CloseConnection(desc);
    // cerr << "-------------------send_client" << endl;
    //pthread_exit(NULL);
    // pthread_exit(NULL);
    return 0;
}

void * received(void * m)//之后的优化可以考虑一次分配几个任务!!!
{
    queue<descript_socket*> descT;
    while(1){
        sem_wait(&ROsem);
        pthread_mutex_lock(&mutex1);
        descT = tcp.getMessage();
        pthread_mutex_unlock(&mutex1);
        while(!descT.empty()) {
            descript_socket* desc=descT.front();
            descT.pop();
                if(!desc->enable_message_runtime)
                {
                    desc->enable_message_runtime = true;
                    if(DEBUG)
                    cout << "id:      " << desc->id      << endl
                     << "ip:      " << desc->ip      << endl
                     << "message: " << desc->message << endl
                     << "socket:  " << desc->socket  << endl
                     << "enable:  " << desc->enable_message_runtime << endl;
                    // pthread_t p;
                    // pthread_create(&p, NULL, &send_client, (void *) desc);
                    po2.dispatch(send_client,(void *) desc);
                }
        }
    }
    return 0;
}

void* LHZFUN(void *arg){
    while(1){//accepted()如果没有接受到连接请求，则会自己休眠，所以直接while(1)即可
    //cerr << "-------------------1" << endl;
        tcp.accepted();
    }
    return 0;
}

int main(int argc, char **argv)
{
    int port=-1;

    for(int i=0;i<argc;i++){
        string argv_temp=argv[i];
        if(i==0)continue;
        else if(argv_temp=="--ip"&&argc>=i+2){
            i=i+1;
        }
        else if(argv_temp=="--port"&&argc>=i+2){
            i=i+1;
            port=atoi(argv[i]);
        }
        else if(argv_temp=="--number-thread"&&argc>=i+2){
            i=i+1;
            O2=atoi(argv[i]);
        }
        else if(argv_temp=="--proxy"&&argc>=i+2){
            i=i+1;
            flagproxy=true;
            proxy=argv[i];
            if(proxy.find("http://")!=proxy.npos){
                proxy=proxy.substr(proxy.find("http://")+7);//这里默认是http://www.baidu.com/的格式
                if(proxy.find("/")!=proxy.npos){
                    proxy=proxy.substr(0,proxy.find("/"));
                }
                if(proxy.find(":")!=proxy.npos){
                    destport=stoi(proxy.substr(proxy.find(":")+1,proxy.length()));
                    proxy=proxy.substr(0,proxy.find(":"));
                }
            }
            else if(proxy.find("https://")!=proxy.npos){
                proxy=proxy.substr(proxy.find("https://")+8);//这里默认是http://www.baidu.com/的格式
                if(proxy.find("/")!=proxy.npos){
                    proxy=proxy.substr(0,proxy.find("/"));
                }
                if(proxy.find(":")!=proxy.npos){
                    destport=stoi(proxy.substr(proxy.find(":")+1,proxy.length()));
                    proxy=proxy.substr(0,proxy.find(":"));
                }
            }
        }
        else{
            cerr << "Usage: ./httpserver \n      --ip (ip) \n      --port (port) \n      [--number-thread (thread number)] \n      [--proxy (proxy)]" << endl;
            tcp.closed();
            po2.setNum(1);
            return 0;
        }
    }
    if(port==-1){
        cerr << "Usage: ./httpserver \n      --ip (ip) \n      --port (port) \n      [--number-thread (thread number)] \n      [--proxy (proxy)]" << endl;
        tcp.closed();
        po2.setNum(1);
        return 0;
    }
    else{
        cerr<<"[httpserver: port: "<<port<<" thread number: "<<O2<<" ";
        if(flagproxy==true){
            cout<<"proxy: "<<proxy<<' ';
            cout<<"destport: "<<destport<<' ';
        }
        cerr  <<"]\n";
    }

    if (sem_init(&ROsem,0,0)) {
        printf("Semaphore initialization failed!!\n");
        exit(EXIT_FAILURE);
    }
    
    
    std::signal(SIGINT, close_app);
    pthread_mutex_init(&mutex1,NULL);
    ThreadPool po1(O1);
    ThreadPool pr(R);
    po2.setNum(O2);
    pw.setNum(W);
	pr.run();po1.run();po2.run();pw.run();
    pthread_t msg;
    vector<int> opts = { SO_REUSEPORT, SO_REUSEADDR };
    
    if(flagproxy){
        
    }
    if( tcp.setup(port,opts) == 0) {
        for(int i=0;i<O1;i++){
            po1.dispatch(received,(void *)0);
        }  
        for(int i=0;i<R;i++){
            pr.dispatch(LHZFUN,(void*)0);
        }
        cout << "Running... ...\n" << endl;
        string cmd;
        cin >> cmd;//cin阻塞
        if(cmd=="quit") exit(0);
    }
    else
        cerr << "Errore apertura socket" << endl;
    return 0;
}