#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


const int DISTRO_KEY = 0x123;
const int SHM_SIZE = 10000;
const int MAX_CLIENTS = 10;
const int MAX_PROVIDERS = 10;
const int MAX_NOTIFICATIONS = 10;
const int NOTIFICATION_KEY= 0x420;



/// IPC msgs

struct sign_msg{
    long mtype; // w zależności od typu dodajemy lub usuwamy subskrypcje kontretnego powiadomienia dla usera dla providera poprostu dodajemy 
    int id;
    int notification_type;
};
struct notification{
    long mtype;
    int provider_key;
    char content[200];
};

struct notification_types_msg // Typy do wyboru
{
    long mtype;
    char types[100];
};

struct provider{
    int id;
    int type;
};

struct client{
    int id;
    int notification_types[10];
    int queue;
};

struct shared_data {
    struct provider providers[10];
    struct client clients[10];
};

bool isTypeFree(int type, struct shared_data *data)
{
    for (int i = 0; i < MAX_PROVIDERS; i++) {
        if (data->providers[i].type == type) {
            return false;
        }
    }
    return true;
}

int getClientById(int id, struct shared_data *data) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (data->clients[i].id == id) {
            return i; 
        }
    }
    return -1;
}

int getProviderByKey(int id, struct shared_data *data) {
    for (int i = 0; i < MAX_PROVIDERS; i++) {
        if (data->providers[i].id == id) {
            return i; // Return index or pointer
        }
    }
    return -1;
}

int main()
{

    int shm_id = shmget(DISTRO_KEY, SHM_SIZE, 0666 | IPC_CREAT);

    struct shared_data *data = (struct shared_data *)shmat(shm_id, NULL, 0);
    memset(data, -1, sizeof(struct shared_data));

    int sid = msgget(DISTRO_KEY, 0666 | IPC_CREAT);

    if(fork() == 0)
    {
        int nid = msgget(NOTIFICATION_KEY, 0666 | IPC_CREAT);
        struct notification notification;
        while (1)
        {
            msgrcv(nid,&notification,sizeof(struct notification) - sizeof(long),100,0);
            if (data->providers[getProviderByKey(notification.provider_key, data)].type == notification.mtype)
            {
                for (int i = 0; i < 10; i++)
                {
                    if (data->clients[i].notification_types[notification.mtype] == 1)
                    {
                        msgsnd(data->clients[i].queue,&notification,sizeof(notification) - sizeof(long),0);
                    }
                }
            }
        }
        exit(0);
    }
    struct sign_msg sign;

    struct notification_types_msg types_msg;
    types_msg.mtype = 11;
    while (1)
    {
        msgrcv(sid,&sign, sizeof(struct sign_msg) - sizeof(long), 0,0);
        switch (sign.mtype)
        {
        case 100:
            for (int i = 0; i < MAX_PROVIDERS; i++)
            {
                if (data->providers[i].id == sign.id)
                {
                    if (data->providers[i].type == sign.notification_type)
                    {
                        break;
                    }
                    if (isTypeFree(sign.notification_type,data)) // Jeżeli jeden proces jest już zalogowany to nie obsłużymy tylko dostanie 2 procesy ale no
                    {
                        data->providers[i].type = sign.notification_type;
                        break;
                    }
                    else
                    {
                        printf("Odrzucono zapis ponieważ typ jest już wykorzystywany przez innego providera");
                        break;
                    }
                    
                }
                if (data->providers[i].id == -1)
                {
                    if (isTypeFree(sign.notification_type,data))
                    {
                        data->providers[i].id = sign.id;
                        data->providers[i].type = sign.notification_type;
                        break;
                    }
                    else
                    {
                        printf("Odrzucono zapis ponieważ typ jest już wykorzystywany przez innego providera");
                        break;
                    }

                }
            }
            
            break;

        case 101:
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (data->clients[i].id == sign.id)
                {
                    break;
                }
                if (data->clients[i].id == -1)
                {
                    data->clients[i].id = sign.id;
                    data->clients[i].queue = msgget(sign.id, 0666 | IPC_CREAT);
                    break;
                }
                
            }
            
            break;

        case 11:
            strcpy(types_msg.types, "");
            for (int i = 0; i < 10; i++)
            {
                if (data->providers[i].type != 0) // Odczyt z pamięci współdzielonej
                { 
                    strcat(types_msg.types,data->providers[i].type);
                    strcat(types_msg.types,"\n");
                    break;
                }
            }
            strcat(types_msg.types,"koniec");
            msgsnd(data->clients[getClientById(sign.id, data)].queue,&types_msg,sizeof(struct notification_types_msg) - sizeof(long),0);
            break;

        case 12:
            data->clients[getClientById(sign.id,data)].notification_types[sign.notification_type] = 1;
            break;

        case 13:
            data->clients[getClientById(sign.id,data)].notification_types[sign.notification_type] = 0;
            break;

        default:
            break;
        }
    }
    


}
