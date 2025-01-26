#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int DISTRO_KEY = 0x123;

struct prov_msg{ //TODO zapytać jaka ma być funkcjonalność providerów
    long mtype;
    char key[6];
    int notification_types[12];
};

struct usr_msg{
    long mtype; // w zależności od typu dodajemy lub usuwamy subskrypcje kontretnego powiadomienia
    int usr_id;
    int notification_type;
};

struct provider{
    int id;
    int types[4];
};

struct clients{
    int id;
    int notification_types[10];
};

struct notification_type{
    int id;
    char title[20];
};

struct notification{
    long mtype;
    int type;
    char content[100];
};



int main()
{
    int pid = msgget(DISTRO_KEY, 0666 | IPC_CREAT);


}