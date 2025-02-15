#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


const int DISTRO_KEY = 0x12345678;

struct usr_msg
{
    long mtype; // w zależności od typu dodajemy lub usuwamy subskrypcje kontretnego powiadomienia
    int usr_id;
    int notification_type;
    char types[100];
};

struct notification_msg // Powiadomienie
{
    long mtype;
    int provider_key;
    char content[200];
};

void requestNotifications(int pid,int mid, int user_id)
{
    struct usr_msg req;
    req.mtype = 11;
    req.usr_id = user_id;
    msgsnd(pid, &req, sizeof(struct usr_msg) - sizeof(long), 0); //request typów
    struct usr_msg res;
    msgrcv(mid,&res,sizeof(struct usr_msg) - sizeof(long),11,0);
    printf("Typy : \n%s",res.types);
    return;
}
int subscribed [11];
void displaySubscribedNotifications()
{
   for (int i = 1; i <= 10; i++)
   {
        if (subscribed[i] == 1)
        {
            printf("%d\n",i);
        }
   }
   return;
}

int main(int argc, char *argv[])
{
    int pid = msgget(DISTRO_KEY, 0666 | IPC_CREAT);
    int user_id = atoi(argv[1]);
    struct usr_msg sign;
    sign.mtype = 101;
    sign.usr_id = user_id;
    msgsnd(pid, &sign, sizeof(struct usr_msg) - sizeof(long), 0); // zalogowanie się
    int mid = msgget(user_id, 0666 | IPC_CREAT); // stworzenie kolejki na komunikaty

    char input[5];
    struct notification_msg msg;
    struct usr_msg post;
    while(1)
    {
        printf("Wybierz opcje:\n");
        printf("1 - dodaj nowy typ komunikatu\n");
        printf("2 - odbierz komunikat\n");
        printf("3 - usuń typ komunikatu\n");

        // Read input from the user
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Blad odczytu. Sprobuj ponownie.\n");
            continue;
        }

        int choice = atoi(input);
        switch (choice) {
            case 1:
                printf("Dodawanie nowego typu komunikatu...\n");
                requestNotifications(pid, mid, user_id);

                printf("Podaj numer komunikatu który chcesz dodać\n");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                printf("Blad odczytu. Sprobuj ponownie.\n");
                continue;
                }
                post.mtype = 12; //dodanie typu
                post.usr_id = user_id;
                int notification_type = atoi(input);
                post.notification_type = notification_type;
                msgsnd(pid, &post, sizeof(struct usr_msg) - sizeof(long), 0); // dodanie typu powiadomienia
                subscribed[notification_type] = 1;
                break;

            case 2:
                printf("Odbieranie komunikatu...\n");

                if (msgrcv(mid, &msg, sizeof(struct notification_msg) - sizeof(long), 0, IPC_NOWAIT) == -1) // -9 bo chcemy wszystkie powiadomienia od 1-9 to są nasze wiadomości a filtracja typów jest po stornie dystrybutora
                {
                    printf("Brak nowych komunikatow.\n");
                    break;
                } 
                printf("Odebrano komunikat typu: %ld\n", msg.mtype);
                printf("Treść: %s\n",msg.content);
                break;
            case 3:
                printf("Usuwanie typu komunikatu...\n");
                displaySubscribedNotifications();

                printf("Podaj numer komunikatu który chcesz usunąć\n");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                printf("Blad odczytu. Sprobuj ponownie.\n");
                continue;
                }
                post.mtype = 13; //usunięcie typu
                notification_type = atoi(input);
                post.notification_type = notification_type;
                msgsnd(pid, &post, sizeof(struct usr_msg) - sizeof(long), 0); // usunięcie typu powiadomienia
                subscribed[notification_type] = 0;
                break;
            default:
                printf("Nieznana opcja. Sprobuj ponownie.\n");
                break;
        }
    }

}
