#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

const DISTRO_KEY = 0x123;
const SHM_SIZE = 10000;
const MAX_CLIENTS = 10;
const MAX_PROVIDERS = 10;
const MAX_NOTIFICATIONS = 10;



/// IPC msgs

struct sign_msg{
    long mtype; // w zależności od typu dodajemy lub usuwamy subskrypcje kontretnego powiadomienia dla usera dla providera poprostu dodajemy 
    int id;
    int notification_type;
};
struct notification{
    long mtype;
    int key;
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
    int notification_types[MAX_NOTIFICATIONS];
    int queue;
};

struct shm_data {
    struct provider providers[MAX_PROVIDERS];
    struct client clients[MAX_CLIENTS];
};


void handleNotifications();

struct provider *providers[MAX_PROVIDERS];
struct client *clients[MAX_CLIENTS];

bool isTypeFree(type)
{
    for (int i = 0; i < MAX_PROVIDERS; i++)
    {
        if (providers[i].type == type)
        {
            return false;
        }
    }
    return true;
}
int getClientById(id)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        providers[i].id == id;
        return id;
    }
    return -1;
}

int main()
{

    int shm_id = shmget(DISTRO_KEY, SHM_SIZE, 0666 | IPC_CREAT);

    struct shared_data *data_ptr = (struct shared_data *)shmat(shm_id, NULL, 0);

    struct shared_data data = *data_ptr;

    int sid = msgget(DISTRO_KEY, 0666 | IPC_CREAT);

    if(fork() == 0)
    {
        handleNotifications();
    }
    struct sign_msg sign;

    while (1)
    {
        msgrcv(sid,&sign, sizeof(struct sign_msg) - sizeof(long), 0,0);
        switch (sign.mtype)
        {
        case 100:
            for (int i = 0; i < MAX_PROVIDERS; i++)
            {
                if (data.providers[i].id == sign.id)
                {
                    if (data.providers[i].type == sign.notification_type)
                    {
                        continue;
                    }
                    if (isTypeFree(data.provider, sign.notification_type)) // Jeżeli jeden proces jest już zalogowany to nie obsłużymy tylko dostanie 2 procesy ale no
                    {
                        data.providers[i].type = sign.notification_type;
                        continue;
                    }
                    else
                    {
                        printf("Odrzucono zapis ponieważ typ jest już wykorzystywany przez innego providera");
                        continue;
                    }
                    
                }
                if (data.providers[i].id == NULL)
                {
                    data.providers[i].id = sign.id;
                    if (isTypeFree(data.providers,sign.notification_type))
                    {
                        data.providers[i].type = sign.notification_type;
                        continue;
                    }
                    else
                    {
                        printf("Odrzucono zapis ponieważ typ jest już wykorzystywany przez innego providera");
                        continue;
                    }

                }
            }
            
            break;

        case 101:
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (data.clients[i].id == sign.id)
                {
                    continue;
                }
                if (data.clients[i].id == NULL)
                {
                    data.clients[i].id = sign.id;
                    data.clients[i].queue = msgget(sign.id, 0666 | IPC_CREAT);
                }
                
            }
            
            break;

        case 11:
            strcpy(types_msg.types, "");
            for (int i = 0; i < 10; i++)
            {
                if (data.providers[i].type != 0) { // Odczyt z pamięci współdzielonej
                char buffer[10];
                sprintf(buffer, "%d", data.providers[i].type);
                strcat(types_msg.types, buffer);
                strcat(types_msg.types, "\n");
        }
            }
            msgsnd(data.clients[getClientById(sign.id)].queue,&types_msg,sizeof(struct notification_types_msg) - sizeof(long),IPC_NOWAIT);
            break;

        case 12:
            data.clients[getClientById(sign.id)].notification_types[sign.notification_type] = 1;
            break;

        case 13:
            data.clients[getClientById(sign.id)].notification_types[sign.notification_type] = 0;
            break;

        default:
            break;
        }
    }
    


}


void handleNotifications()
{

}