#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "socketutil.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};
void removeParity(char*buffer);
int randint(int n);
void clientDisconnect(int socketFD,char* buffer);
void stringToBinary(char* buffer,char* returnBuffer);
void binaryToString(char* buffer,char* returnBuffer);
void sendUserList();
void addToLogs(char *buffer,int socketFD);
void addUsertoList(int socketFD,char *buffer);
int userCheck(int socketFD);
int checkParity(char * buffer);
void brokeBinary(char*buffer);
void addPariy(char* buffer,char * buffer2);
struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD);
void acceptNewConnectionAndReceiveAndPrintItsData(int serverSocketFD);
void receiveAndPrintIncomingData(int socketFD);
void startAcceptingIncomingConnections(int serverSocketFD);
void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket);
void sendReceivedMessageToTheOtherClients(char *buffer,int socketFD);
void sendReceivedMessageToTheSpecialClients(char *buffer,int socketFD);
int checkPrivateChat(char *buffer,int socketFD);

struct AcceptedSocket acceptedSockets[10] ;
int acceptedSocketsCount = 0;
int clientID[10];
char userList[10][30];
int clientNumber=0;
char *filename = "logs.txt";
FILE *fp;
char backupBuffer[2000];
char backup[2000];
int isBroken=0;
int parityControl;
void brokeBinary(char*buffer){
    size_t len = strlen(buffer)-2;
    int binaryNum= randint(len);
    if(buffer[binaryNum]=='0'){
        buffer[binaryNum]='1';
    } else{
        buffer[binaryNum]='0';
    }
    isBroken=1;
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
int randint(int n) {
    if ((n - 1) == RAND_MAX) {
        return rand();
    } else {
        // Supporting larger values for n would requires an even more
        // elaborate implementation that combines multiple calls to rand()
        assert (n <= RAND_MAX);

        // Chop off all of the values that would cause skew...
        int end = RAND_MAX / n; // truncate skew
        assert (end > 0);
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end);

        return r % n;
    }
}
int userCheck(int socketFD) {
    int count = 0;
    do {
        if (clientID[count] == socketFD)
            return count+1;
        count++;
    } while (count <= clientNumber);

    return 0;
}
void sendUserList(){
    char buffer[512];
    char buffer2[512];
    strcpy(buffer,"");
    strcpy(buffer2,"");

    if (clientNumber==0) {
        // do nothing
        printf("user list emty");
        return;
    } else {
        strcpy(buffer,"User List :\n");
        for(int i = 0 ; i<acceptedSocketsCount ; i++) {
            strcat(buffer, userList[i]);
            strcat(buffer, "\n");

        }
        stringToBinary(buffer,buffer2);
        for (int j = 0; j < acceptedSocketsCount; j++) {
            send(acceptedSockets[j].acceptedSocketFD, buffer2, strlen(buffer2), 0);
        }
    }

}
void startAcceptingIncomingConnections(int serverSocketFD) {

    while(acceptedSocketsCount!=10)
    {
        struct AcceptedSocket* clientSocket  = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketsCount++] = *clientSocket;
        printf("New user ID = %d\n",clientSocket->acceptedSocketFD);
        receiveAndPrintIncomingDataOnSeparateThread(clientSocket);
    }
}
void addUsertoList(int socketFD, char *buffer){
    clientID[clientNumber]=socketFD;
    char userName[20];
    strcpy(userName,buffer);
    char addedUser[30];
    sprintf(addedUser,"ID:%d -> ",socketFD);
    strcat(addedUser,userName);
    strcpy(userList[clientNumber],addedUser);
    sprintf(buffer,"%s added and user number :%d",userList[clientNumber],clientNumber+1);
    printf("%s\n",buffer);
    clientNumber++;
}


void removeParity(char*buffer){
    size_t len = strlen(buffer);
    buffer[len-1]='\0';
}
void receiveAndPrintIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket) {

    pthread_t id;
    pthread_create(&id,NULL,receiveAndPrintIncomingData,pSocket->acceptedSocketFD);
}

void receiveAndPrintIncomingData(int socketFD) {
    char buffer[2000];
    char buffer2[2000];
    int userCheckResult=1;
    int privateChatSocketFD=0;
    while (true)
    {   int createError= randint(100);
        strcpy(buffer2,"");
        strcpy(buffer,"");
        ssize_t  amountReceived = (recv(socketFD,buffer2,2000,0)-1)/8;
        binaryToSTring(buffer2,buffer);
        printf("----%c\n",buffer[0]);
        if(amountReceived>0)
        {
            buffer[amountReceived] = 0;
            int userCheckResult=userCheck(socketFD);// kullanıcı yeni ise tüm üyelere kullanıcı listesinin güncel halini gönderir
            if(userCheckResult==0){
                addUsertoList(socketFD,buffer);
                sendUserList();
                addToLogs(buffer,socketFD);
                userCheckResult=1;
                continue;
            }
            if(buffer[0]=='#'){
                printf("Hata yakalandı\n  :: %s :::\n",buffer);
                stringToBinary(buffer2,buffer);
                send(acceptedSockets[userCheck(socketFD)-1].acceptedSocketFD,buffer, strlen(buffer),0);
                continue;
            } else{
                strcpy(backupBuffer,buffer);
            }
            if(buffer[0]=='!'){ //client disconnect. refresh user list ,connections and send to other user
                int userNo=userCheck(socketFD)-1;
                printf("%s left the server\n",userList[userNo]);
                clientDisconnect(socketFD,buffer);
                sendUserList();
                break;
            }
            privateChatSocketFD=checkPrivateChat(buffer,socketFD);
            printf(" %d -> %s \n",socketFD,buffer);
            addToLogs(&buffer,socketFD);

            if(privateChatSocketFD&&isBroken==0){
              if(createError%7==0){
                  strcpy(backup,buffer2);
                  brokeBinary(backup);
                  sendReceivedMessageToTheOtherClients(backup,socketFD);
                  createError=1;
                  isBroken=1;
              }else{
                sendReceivedMessageToTheSpecialClients(buffer2,privateChatSocketFD);}
            }else{
                if(createError%7==0&&isBroken==0){
                    strcpy(backup,buffer2);
                    brokeBinary(backup);
                    sendReceivedMessageToTheOtherClients(backup,socketFD);
                    createError=1;
                    isBroken=1;
                }else{
                sendReceivedMessageToTheOtherClients(buffer2,socketFD);}
            }

        }

        if(amountReceived==0)
            break;
    }

    close(socketFD);
}

int checkPrivateChat(char *buffer,int socketFD) {
    int testSocketFD;
    for (int i = 0; i <50; i++) {
        if(buffer[i]=='>'){
            char str[2] = {buffer[i-1], '\0'};
            testSocketFD= atoi(str);    //find specific character and convert to int
            if(userCheck(testSocketFD)){       // 2 haneli ID numaraları için çalışır durumda değil
                printf("%d private chat with %d\n",socketFD,testSocketFD);
                return testSocketFD;
            }
        }
    }

    return 0;
}


void sendReceivedMessageToTheOtherClients(char *buffer,int socketFD) {

    for(int i = 0 ; i<acceptedSocketsCount ; i++)
        if(acceptedSockets[i].acceptedSocketFD !=socketFD)
        {
            send(acceptedSockets[i].acceptedSocketFD,buffer, strlen(buffer),0);
        }

}
void sendReceivedMessageToTheSpecialClients(char *buffer,int socketFD){
    for(int i = 0 ; i<acceptedSocketsCount ; i++)
        if(acceptedSockets[i].acceptedSocketFD ==socketFD)
        {
            send(acceptedSockets[i].acceptedSocketFD,buffer, strlen(buffer),0);
        }
}

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in  clientAddress ;
    int clientAddressSize = sizeof (struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD,&clientAddress,&clientAddressSize);

    struct AcceptedSocket* acceptedSocket = malloc(sizeof (struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD>0;

    if(!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = clientSocketFD;

    return acceptedSocket;
}
void addToLogs(char *buffer,int socketFD) {
    char tempBuffer[1100];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[64];
    size_t ret = strftime(date, sizeof(date), "%c", tm);
    assert(ret);
    strcpy(tempBuffer,"\tDate:");
    strcat(tempBuffer,date);
    strcat(tempBuffer,"\t");
    strcat(tempBuffer,buffer);


    tempBuffer[strcspn(tempBuffer,"\n")]=0;
// Open the file for appending
    fp = fopen("logs.txt", "a+");
    if (fp == NULL) {
        printf("Error opening file!\n");
    }else {
        // Write the first string to the file
        fprintf(fp, "%d -> %s\n",socketFD, tempBuffer);
    }
    fclose(fp);

}
void clientDisconnect(int socketFD,char * buffer) {
    char buffer2[50];
    int memNo= userCheck(socketFD)-1;
    sprintf(buffer2,"%d %s",clientID[memNo],buffer);
    strcpy(buffer,"");
    addToLogs(buffer2,socketFD);
    stringToBinary(buffer2,buffer);
    for (int i = 0; i < acceptedSocketsCount ; i++) {
        if (acceptedSockets[i].acceptedSocketFD != socketFD) {
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);
        }
    }
    for (int i = memNo; i < clientNumber; i++) {
        if(memNo==0)
            break;
        strcpy(userList[i],userList[i+1]);
        clientID[i]=clientID[i+1];
        acceptedSockets[i].acceptedSocketFD=acceptedSockets[i+1].acceptedSocketFD;
        acceptedSockets[i].acceptedSuccessfully=acceptedSockets[i+1].acceptedSuccessfully;
        acceptedSockets[i].address=acceptedSockets[i+1].address;
        acceptedSockets[i].error=acceptedSockets[i+1].error;
    }
    acceptedSocketsCount--;
    clientNumber--;
}

int main() {


    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("",2000);

    int result = bind(serverSocketFD,serverAddress, sizeof(*serverAddress));
    if(result == 0)
        printf("socket was bound successfully\n");
    else
        printf("socket create error");
    int listenResult = listen(serverSocketFD,10);

    startAcceptingIncomingConnections(serverSocketFD);
    fclose(fp);
    shutdown(serverSocketFD,SHUT_RDWR);

    return 0;
}