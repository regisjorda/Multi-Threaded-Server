/* 
 * File:   main.cpp
 * Author: r_jorda12
 *
 * Created on September 10, 2012, 7:07 PM
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctime>
#include <pthread.h>
#include <sstream>
#include "globsem.h"    //adds in globsem header files used for semaphores (made some changes to file)

using namespace std;

/*
 * 
 */
void *doServerRun(void *ptr);
void GET(string documentRoot, string fname, int newsocketfd, int n);
void HEAD(string documentRoot, string fname, int newsocketfd, int n);
void POST(string documentRoot, string fname, int newsocketfd, int n, char buffer[256], string check);

    string contentType;
    string documentRoot;
    int socketfd;

    int sem;
    
int main(int argc, char** argv) {
    
    string s_pool, s_queue;
    int pool, queue;
    
    ifstream myfile;
    
    myfile.open("myhttpd.conf");
    
    if (myfile.is_open()){
        
        myfile >> contentType;  //gets the contentType from the .conf file
        myfile >> documentRoot; // gets the documentRoot from the .conf file
        myfile >> s_pool >> s_pool >> s_pool >> s_pool >> s_pool; //gets the pool number
        myfile >> s_queue >> s_queue;    //gets the queue number
        myfile.close();
    }
    else { 
        cout << "unable to open file";
    }
    
    pool = atoi(s_pool.c_str());
    queue = atoi(s_queue.c_str());
    
    int portNum = atoi(argv[1]); //gets port number from command line
    
    struct serv_addr;
    
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socketfd < 0) {
        
        cout << "socket failed";
        exit(1);
    }
    
    struct sockaddr_in serv_addr;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNum);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //use my IP Address
    
    
    if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << "error on binding";
    }
    
    if (listen(socketfd, queue) < 0) {
        cout << "error listening";
    }
    
    struct sockaddr_in client;          //create a client structure       
    int clientlen = sizeof(client);     //get length of client 
    
    
    
    //newsocketfd = accept(socketfd, (struct sockaddr *) &client, (socklen_t *) &clientlen);
        
    //thread goes here (put accept as part of the loop)
    int newsocketfd;
  /*  if (newsocketfd < 0) {
        cout << "error on accept";
    }
   */
    
    pthread_t threadID[pool];
     
    int i=0;
    while ((newsocketfd = accept(socketfd, (struct sockaddr *) &client, (socklen_t *) &clientlen))>0){
        sem = createSem();
        down(sem);
        if (pthread_create(&(threadID[i++]),NULL,doServerRun, (void *) newsocketfd) != 0){
               perror("pthread_create() error");
                exit(1);
        }
        up(sem);
        
    }
          close (socketfd);
        close (newsocketfd);
     
    return 0;
}

void *doServerRun(void *ptr){
    char buffer[256];
    int n;
    string check;
    bzero(buffer,265);       //initialize buffer to empty
    int newsocketfd;
    newsocketfd = (intptr_t)ptr;
    
    while(1) {    
        //Get HTTP Method From Socket
        string method;
        for (;;){
                n=read(newsocketfd,buffer, 1);
                check = buffer;
                if (check==" "){
                        break;
                }
                method = method + buffer;
        }
    
        //Get Request File from Socket
        string fname;
        for (;;){
        n=read(newsocketfd,buffer, 1);
                check = buffer;
                if (check==" "){
                        if (fname=="/"){
                                fname="/index.html";
                        }
                break;
                }
        fname = fname + buffer;
        }
    
        //Get Content Type
        string ctype;
        for (;;){
                n=read(newsocketfd,buffer, 1);
                check = buffer;
                if (check=="\n"){
                        break;  
                }
                ctype = ctype + buffer;
        }

   /*
    //Get Headers from Request
     string headerBody;
     for (;;){
        n=read(newsocketfd,buffer, 1);
        check = buffer;
                headerBody = headerBody + buffer;
                if(check=="\r" || check=="\n"){
                        //n=read(newsocketfd,buffer, 1); //to get "\r" (Because when pressing enter, it gives \n\r)
                        //n=read(newsocketfd,buffer, 1); //to get the next character
                        check = buffer;
                        if (check=="\n" || check=="/r"){
                            break;
                        }
                        else {
                            headerBody = headerBody + buffer;
                        }
                    //break;
                    }   
                    
        }
    */
    
        //initialize time
        time_t now = time(0);
        char *dt = ctime(&now);
        
        if (fname[0]!='/'){ 
                n=write(newsocketfd,"400 (bad requests such as no leading / on path, HTTP/2.0+, no Content-Length for POST)\n",strlen("400 (bad requests such as no leading / on path, HTTP/2.0+, no Content-Length for POST)\n")); 
                n=write(newsocketfd,dt,strlen(dt)); 

        }
   
        else if (method=="GET"){
                GET(documentRoot, fname, newsocketfd, n);
        }
   
        else if (method=="HEAD"){
                HEAD(documentRoot, fname, newsocketfd, n);
        }
   
        else if (method=="POST"){
                POST(documentRoot, fname, newsocketfd, n, buffer, check);
        }
        
       //temporary exit-------------------------------------------
       else if (method=="EXIT"){
        char * ret;
        strcpy(ret, "EXIT");
        pthread_exit(ret);
           break;
        }
        //temporary ----------------------------------------------------------
        
        else if (method!="GET" || method !="HEAD" || method !="POST"){
                n=write(newsocketfd,"HTTP/1.0 501 (not implemented - ie not GET, HEAD, or POST methods that implemented in your program)\n",strlen("HTTP/1.0 501 (not implemented - ie not GET, HEAD, or POST methods that implemented in your program)\n")); 
                n=write(newsocketfd,dt,strlen(dt)); 
        }
        
        //sleep(4);
        
        //char * ret;
        //strcpy(ret, "EXIT");
        //pthread_exit(ret);
        
        }
    
}

void GET(string documentRoot, string fname, int newsocketfd, int n){
       //Implement GET Method

           string filestring = documentRoot + fname;     //concatenates document root with file requested
           string sline_of_file;        //line of strings
           ifstream requestFile(filestring.c_str());  //open requested file
            
           //Get Byte size of file
           int size;
           string stringSize;
           requestFile.seekg(0, std::ios::end);
           size = requestFile.tellg();
           requestFile.seekg(0);
           ostringstream convert;       //stream used for conversion
           convert << size;             //insert the textual representation of 'size' in the characters in the stream
           stringSize = "Content-Length: " + convert.str() + "\r\n"; //set stringSize to the contents of the stream          
           
           if(access(filestring.c_str(),R_OK)==-1){
               n=write(newsocketfd,"HTTP/1.0 403 (No Read Permissions)\n",strlen("HTTP/1.0 403 (No Read Permissions)\n"));
           }
           else if (requestFile.is_open()){
               
               n=write(newsocketfd,"HTTP/1.0 200 OK\r\n",strlen("HTTP/1.0 200 OK\r\n"));
               n=write(newsocketfd,"Accept-Ranges: bytes\r\n",strlen("Accept-Ranges: bytes\r\n"));
               n=write(newsocketfd,"Connection: Keep-Alive\r\n",strlen("Connection: Keep-Alive\r\n"));
               n=write(newsocketfd,"Content-Language: en\r\n",strlen("Content-Language: en\r\n"));
               n=write(newsocketfd,stringSize.c_str(),strlen(stringSize.c_str()));
               n=write(newsocketfd,"Keep-Alive: timeout=15, max=100\r\n",strlen("Keep-Alive: timeout=15, max=100\r\n"));
               //n=write(newsocketfd,"Content-Location: index.html\r\n",strlen("Content-Location: index.html\r\n"));
               n=write(newsocketfd,"Content-Type: text/html\r\n",strlen("Content-Type: text/html\r\n"));
               n=write(newsocketfd,"\r\n",strlen("\r\n"));
               
               while (requestFile.good()){
                   getline(requestFile,sline_of_file);  //get line by line of file and store in the string variable
                    n=write(newsocketfd,sline_of_file.c_str(),strlen(sline_of_file.c_str()));      //write to socket
                   n=write(newsocketfd,"\n",strlen("\n"));
               }
               
               requestFile.close();
           }
           else {
               n=write(newsocketfd,"HTTP/1.0 404 (file does not exist)\n",strlen("HTTP/1.0 404 (file does not exist)\n")); 
           }
           n=write(newsocketfd,"\r\n",strlen("\r\n"));      
           
}

void HEAD(string documentRoot, string fname, int newsocketfd, int n){
       
           string filestring = documentRoot + fname;     //concatenates document root with file requested
           string sline_of_file;        //line of strings
           ifstream requestFile(filestring.c_str());  //open requested file
            
           //Get Byte size of file
           int size;
           string stringSize;
           requestFile.seekg(0, std::ios::end);
           size = requestFile.tellg();
           requestFile.seekg(0);
           ostringstream convert;       //stream used for conversion
           convert << size;             //insert the textual representation of 'size' in the characters in the stream
           stringSize = "Content-Length: " + convert.str() + "\r\n"; //set stringSize to the contents of the stream          
           
           if(access(filestring.c_str(),R_OK)==-1){
               n=write(newsocketfd,"HTTP/1.0 403 (No Read Permissions)\n",strlen("HTTP/1.0 403 (No Read Permissions)\n"));
           }
           else if (requestFile.is_open()){
               
               n=write(newsocketfd,"HTTP/1.0 200 OK\r\n",strlen("HTTP/1.0 200 OK\r\n"));
               n=write(newsocketfd,"Accept-Ranges: bytes\r\n",strlen("Accept-Ranges: bytes\r\n"));
               n=write(newsocketfd,"Connection: Keep-Alive\r\n",strlen("Connection: Keep-Alive\r\n"));
               n=write(newsocketfd,"Content-Language: en\r\n",strlen("Content-Language: en\r\n"));
               n=write(newsocketfd,stringSize.c_str(),strlen(stringSize.c_str()));
               n=write(newsocketfd,"Keep-Alive: timeout=15, max=100\r\n",strlen("Keep-Alive: timeout=15, max=100\r\n"));
               //n=write(newsocketfd,"Content-Location: index.html\r\n",strlen("Content-Location: index.html\r\n"));
               n=write(newsocketfd,"Content-Type: text/html\r\n",strlen("Content-Type: text/html\r\n"));
               n=write(newsocketfd,"\r\n",strlen("\r\n"));
               
               requestFile.close();
           }
           else {
               n=write(newsocketfd,"HTTP/1.0 404 (file does not exist)\n",strlen("HTTP/1.0 404 (file does not exist)\n")); 
           }
           n=write(newsocketfd,"\r\n",strlen("\r\n"));
           
}

void POST(string documentRoot, string fname, int newsocketfd, int n, char buffer[256], string check){
          //Get Content Length
        string contentLength;
        n=write(newsocketfd,"Content-Length: ",strlen("Content-Length: "));
        for (;;){
                n=read(newsocketfd,buffer, 1);
                check = buffer;
                contentLength = contentLength + buffer;
                if(check=="\n"){
                     break;
                    }             
         }
         //get Content body
        string contentBody;
         for (;;){
                n=read(newsocketfd,buffer, 1);
                check = buffer;
                contentBody = contentBody + buffer;
                if(check=="\n"){
                        n=read(newsocketfd,buffer, 1); //to get "\r" (Because when pressing enter, it gives \n\r)
                        n=read(newsocketfd,buffer, 1); //to get the next character
                        check = buffer;
                        if (check=="\n"){
                            break;
                        }
                        else {
                            contentBody = contentBody + buffer;
                        }
                    //break;
                    }   
                    
        }
        //Create File
        string filestring;
        filestring = documentRoot + fname;
        ofstream outputfile;
        outputfile.open(filestring.c_str());
        if (outputfile.is_open()){
                outputfile << contentBody;
                outputfile.close();
               n=write(newsocketfd,"201 (File created successfully - POST)\n",strlen("201 (File created successfully - POST)\n")); 
        }
         else {
               n=write(newsocketfd,"HTTP/1.0 404 (Cannot Open file)\n",strlen("HTTP/1.0 404 (Cannot Open file)\n"));  
           }
      
}
