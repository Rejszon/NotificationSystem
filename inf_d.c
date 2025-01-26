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
    int notification_types[10];
    int queue;
};


void handleNotifications();

struct provider providers[10];
struct client clients[10];

bool isTypeFree(type)
{
    for (int i = 0; i < 10; i++)
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
    for (int i = 0; i < 10; i++)
    {
        providers[i].id == id;
        return id;
    }
    return -1;
}

int main()
{
    int sid = msgget(DISTRO_KEY, 0666 | IPC_CREAT);
    if(fork() == 0)
    {
        handleNotifications();
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
            for (int i = 0; i < 10; i++)
            {
                if (providers[i].id == sign.id)
                {
                    if (providers[i].type == sign.notification_type)
                    {
                        continue;
                    }
                    if (isTypeFree(sign.notification_type)) // Jeżeli jeden proces jest już zalogowany to nie obsłużymy tylko dostanie 2 procesy ale no
                    {
                        providers[i].type = sign.notification_type;
                        continue;
                    }
                    else
                    {
                        printf("Odrzucono zapis ponieważ typ jest już wykorzystywany przez innego providera");
                        continue;
                    }
                    
                }
                if (providers[i].id == NULL)
                {
                    providers[i].id = sign.id;
                    if (isTypeFree(sign.notification_type))
                    {
                        providers[i].type = sign.notification_type;
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
            for (int i = 0; i < 10; i++)
            {
                if (clients[i].id == sign.id)
                {
                    continue;
                }
                if (clients[i].id == NULL)
                {
                    clients[i].id = sign.id;
                    clients[i].queue = msgget(sign.id, 0666 | IPC_CREAT);
                }
                
            }
            
            break;

        case 11:
            strcpy(types_msg.types, "");
            for (int i = 0; i < 10; i++)
            {
                strcat(types_msg.types,providers[i].type);
                strcat(types_msg.types,"\n");
            }
            msgsnd(clients[getClientById(sign.id)].queue,&types_msg,sizeof(struct notification_types_msg) - sizeof(long),IPC_NOWAIT);
            break;

        case 12:
            clients[getClientById(sign.id)].notification_types[sign.notification_type] = 1;
            break;

        case 13:
            clients[getClientById(sign.id)].notification_types[sign.notification_type] = 0;
            break;

        default:
            break;
        }
    }
    


}


void handleNotifications()
{

}