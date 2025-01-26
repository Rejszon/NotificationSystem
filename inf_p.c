#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct msgLogin {
    long mtype;
    int prov_id;
    int notification_type;
};

struct msgNotification {
    long mtype;
    int key;
    char content[200];
};

const DISTRO_KEY = 0x123;
const NOTIFICATION_KEY= 0x420;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Podaj swój identyfikator oraz typ powiadomienia\n");
        return 1;
    }
    int provider_key = atoi(argv[1]);
    int notification_type = atoi(argv[2]);

    struct msgLogin sign;
    sign.mtype = 100; // Logowanie ma typ 100
    sign.prov_id = provider_key;
    sign.notification_type = notification_type;

    int id = msgget(DISTRO_KEY, 0666 | IPC_CREAT);
    
    //login
    msgsend(id,&sign,sizeof(struct msgLogin) - sizeof(long),0);

    struct msgNotification msg;
    msg.mtype = notification_type;
    msg.key = provider_key;
    int nid = msgget(DISTRO_KEY, 0666 | IPC_CREAT);
    while (1)
    {
        printf("Podaj treść powiadomienia\n");
        if (fgets(msg.content, sizeof(msg.content), stdin) == NULL) {
            printf("Blad odczytu. Sprobuj ponownie.\n");
            continue;
        }
        msgsnd(nid,&msg, sizeof(struct msgNotification) - sizeof(long),0); // TODO obsługa błędów
        printf("Powiadomienie wysłane\n"); 
    }
    return 0;
}