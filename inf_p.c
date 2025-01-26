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
    char key[6];
    int notification_types[12];
};

int DISTRO_KEY = 0x123;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Podaj klucz dla dystrybutora\n");
        return 1;
    }

    struct msgLogin sign;
    printf("Enter up to 4-character key:\n");
    if (fgets(sign.key, sizeof(sign.key), stdin)) {
        size_t len = strlen(sign.key);
        if (len > 0 && sign.key[len - 1] == '\n') {
            sign.key[len - 1] = '\0';
        }
    }

    printf("Enter up to 10-character notification types:\n");
    if (fgets(sign.notification_types, sizeof(sign.notification_types), stdin)) {
        size_t len = strlen(sign.notification_types);
        if (len > 0 && sign.notification_types[len - 1] == '\n') {
            sign.notification_types[len - 1] = '\0';
        }
    }
    int id = msgget(DISTRO_KEY, 0666 | IPC_CREAT);
    
    //login
    msgsend(id,&sign,sizeof(sign) - sizeof(long),IPC_NOWAIT);



}