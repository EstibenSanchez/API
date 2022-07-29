#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define PORT 2222
#define MAXLINE 4096
#define TRUE 1
/*creacion socket*/
int crearsocket(int *port, int type){
    int sockfd;
    struct sockaddr_in adr;
    int longitud;
    
    if((sockfd=socket(PF_INET,type,0))==-1){
        perror("Error: Imposible crear socket");
        exit(2);
    }
    
    bzero((char*)&adr,sizeof(adr));
    adr.sin_port=htons(*port);
    adr.sin_addr.s_addr=htonl(INADDR_ANY);
    adr.sin_family=PF_INET;
    
    if(bind(sockfd,(struct sockaddr *) &adr, sizeof(adr))==-1){
        perror("Error:bind");
        exit(3);
    }
    
    longitud = sizeof(adr);
    
    if(getsockname(sockfd,&adr,&longitud)){
        perror("Error:Obtencion del nombre del sock");
        exit(4);
    }
    *port=ntohs(adr.sin_port);
    return(sockfd);
}

void sigchld(){
    pid_t pid;
    int stat;

    pid=wait(&stat);
    fprintf(stderr, "proceso hijo: %d terminado\n",pid);
    return;
}

int main(int argc,char *argv[]){
    int sock_escucha, sock_servicio;/*descriptores de los sockets*/
    struct sockaddr_in adr; /*direccion*/
    int lgadr=sizeof(adr); /*longitud de la direccion*/
    int port=PORT; /*puerta del servicio*/
    
    if(argc!=2){
        fprintf(stderr, "Uso: %s[port]\n",argv[0]);
        exit(1);
    }
    
    port=atoi(argv[1]);
    
    /* Creacion del socket de escucha*/
    if((sock_escucha=crearsocket(&port,SOCK_STREAM))==-1){
        fprintf(stderr,"Error:no se pudo crear/conectar socket\n");
        exit(2);
    }
    signal(SIGCHLD,sigchld);
     /*Creacion de la cola de conexiones pendiente*/
     listen(sock_escucha,1024);
     
     fprintf(stdout, "Inicio servidor en el puerto %d\n",port);
     
     while(TRUE){
            lgadr = sizeof(adr);
            sock_servicio=accept(sock_escucha,&adr,&lgadr);
            fprintf(stdout,"Servicio aceptado\n");
            
            if(fork()==0){
                /*EL proceso de servicio no utiliza el socket de escucha*/
                close(sock_escucha);
                
                /*Llamada a la funcion de servicio*/
                servicio(sock_servicio);
                
                exit(0);
            }
            /*El proceso padre no utiliza el socket de servicio */
            close(sock_servicio);
    }
        
}


servicio(int sock){
    ssize_t n;
    char line[MAXLINE];
    int i = 0;
    
    for(;;){
        if((n = read(sock,line,MAXLINE))<=0)
            return;
        for(i = 0;i<strlen(line)/2;i++)
        {
            char time=line[i];
            line[i]=line[strlen(line)-i-1];
            line[strlen(line)-i-1]=time;
        }
        write(sock,&line,n);
    }
}

