
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "socketutil.h"

void startListeningAndPrintMessagesOnNewThread(int fd);
void stringToBinary(char* buffer,char* returnBuffer);
void binaryToString(char* buffer,char* returnBuffer);
void listenAndPrint(int socketFD);
void readConsoleEntriesAndSendToServer(int socketFD);
void removeParity(char*buffer);
void addPariy(char* buffer,char * buffer2);
int checkParity(char * buffer);
char *name = NULL;
int parityControl;
size_t nameSize= 0;


int main() {

    int socketFD = createTCPIpv4Socket();//   SocketFD < 0 ise socket oluşturulamadı !
    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);


    int result = connect(socketFD,address,sizeof (*address));
    if(result == 0) {
        printf("connection was successful\n");

    }


    startListeningAndPrintMessagesOnNewThread(socketFD);
    readConsoleEntriesAndSendToServer(socketFD);


    close(socketFD);

    return 0;
}

void readConsoleEntriesAndSendToServer(int socketFD) {
    char buffer[2000];
    strcpy(buffer,"");
    char buffer2[2000];
    strcpy(buffer2,"");
    char nameBuff[20];
    char nameBuff2[20];
    strcpy(nameBuff2,"");
    strcpy(nameBuff,"");
    char *line = NULL;
    size_t lineSize= 0;


    printf("please enter your name?\n");
    ssize_t  nameCount = getline(&name,&nameSize,stdin);
    name[nameCount-1]=0;
    stringToBinary(name,nameBuff);
    ssize_t sendNameAndAddress =  send(socketFD,
                                       nameBuff,
                                       strlen(nameBuff), 0);

    printf("Type user id and '>' and your message for private chat (type \"exit\")\n");

    while(true)
    {
        strcpy(buffer2,"");
        strcpy(buffer,"");
        ssize_t  charCount = getline(&line,&lineSize,stdin);
        line[charCount-1]=0;

        sprintf(buffer," %s : %s",name,line);
        if(charCount>0)
        {

            if(strcmp(line,"exit")==0) {
                strcpy(buffer2,"");
                sprintf(buffer2,"! %s is disconnected",name);
                strcpy(buffer,"");
                stringToBinary(buffer2,buffer);
                ssize_t amountWasSent =  send(socketFD,
                                              buffer,
                                              strlen(buffer), 0);
                break;
            }
            stringToBinary(buffer,buffer2);
            ssize_t amountWasSent =  send(socketFD,
                                          buffer2,
                                          strlen(buffer2), 0);
        }
    }
}
void removeParity(char*buffer){
    size_t len = strlen(buffer);
    buffer[len-1]='\0';
}
void addPariy(char* buffer,char * buffer2){
    if(buffer==NULL)return;
    size_t len = strlen(buffer);
    int countForOne=0;
    for (int i = 0; i < len; i++){
        if(buffer[i]=='1')countForOne++;
    }
    if(countForOne%2==0){

        strcpy(buffer2,buffer);
        strcat(buffer2,"1");
    } else{
        strcpy(buffer2,buffer);
        strcat(buffer2,"0");
    }
}
int checkParity(char * buffer){
    if(buffer==NULL)return 0;
    char parity;
    size_t len = strlen(buffer);
    int countForOne=0;
    for (int i = 0; i < len-1; i++){
        if(buffer[i]=='1')countForOne++;
    }
    if(countForOne%2==0){
        parity='1';
    } else{
        parity='0';
    }
    if(parity==buffer[len-1]){
        printf("parity True\n");
        return 1;
    } else
        printf("parity False\n");
    return 0;
}
void stringToBinary(char* buffer,char *returnBuffer) {
    char array[2000];
    char array2[2000];
    strcpy(array,"");
    strcpy(array2,"");
    strcpy(returnBuffer,"");
    if(buffer == NULL) return; /* no input string */
    size_t len = strlen(buffer);
    for(size_t i = 0; i < len; ++i) {
        char ch = buffer[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat(returnBuffer,"1");
            } else {
                strcat(returnBuffer,"0");
            }
        }
    }

    strcpy(array,returnBuffer);
    addPariy(array,array2);
    strcpy(returnBuffer,array2);


}
void binaryToSTring(char *buffer,char *returnBuffer){
    removeParity(buffer);
    int size= strlen(buffer);
    if(size==0)return;
    //printf("%d",size);
    char temp[8];
    int counter=0;
    int arraycount=0;
    strcpy(temp,"");

    for (int i = 0; i <size ; i++) {
        temp[counter]=buffer[i];
        counter++;
        if(counter==8){
            char c = strtol(temp,0, 2);
            returnBuffer[arraycount]=c;
            arraycount++;
            counter=0;
        }
    }
    returnBuffer[arraycount]='\0';
}
void startListeningAndPrintMessagesOnNewThread(int socketFD) {
    pthread_t id ;
    pthread_create(&id,NULL,listenAndPrint,socketFD);
}

void listenAndPrint(int socketFD) {
    char buffer[2000];
    char buffer2[2000];
    while (true)
    {
        strcpy(buffer2,"");
        strcpy(buffer,"");
        ssize_t  amountReceived = (recv(socketFD,buffer2,2000,0)-1)/8;
        parityControl= checkParity(buffer2);
        binaryToSTring(buffer2,buffer);
        if(parityControl==0){
            strcpy(buffer,"###ERROR MESSAGE###\n");
            strcpy(buffer2,"");
            stringToBinary(buffer,buffer2);
            send(socketFD,
                 buffer2,
                 strlen(buffer2), 0);
        }

        if(amountReceived>0)
        {
            buffer[amountReceived] = 0;
            printf("->%s\n ",buffer);
        }

        if(amountReceived==0)
            break;
    }

    close(socketFD);
}
