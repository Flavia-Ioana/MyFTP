/* cliTcpConc.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".

   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <openssl/sha.h>
#include <arpa/inet.h>

extern int errno;
int port; // portul de conectare la server

void handle_error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void hash(const char *password, char *hashed_password)
{
    unsigned char hash[32];
    SHA256((unsigned char *)password, strlen(password), hash);

    for (int i = 0; i < 32; i++)
    {
        sprintf(hashed_password + (i * 2), "%02x", hash[i]); // convertire in hexa
    }
    hashed_password[64] = '\0';
}

void Comenzi()
{
    printf("%s\n", "Introduceti comanda dorita:");
    printf("%s\n", "pentru connectare a unui utilizator -> login");
    printf("%s\n", "pentru cateva operatii de baza pe fisiere si directoare");
    printf("%s\n", "create_file <name>");
    printf("%s\n", "create_dir <name>");
    printf("%s\n", "exists <file/dir>"); // oare sa adaug si daca exista si alte tipuri de fisiere? symlink,socket
    printf("%s\n", "remove <dir/file> (pentru fisiere si directoare goale)");
    printf("%s\n", "rename <name_old> to <name_new>");
    printf("%s\n", "copy <file> to <dir>");              // copierea unui fisier din directorul curent in alt director
    printf("%s\n", "move <file> to <dir>");              // mutarea unui fisier din directorul curent in alt director
    printf("%s\n", "download <file> (with <new_nume>)"); // in directorul curent
    printf("%s\n", "change_dir <dir>");
    printf("%s\n", "change_p_dir <dir>"); // sa pun o limita ca sa nu treca de folderul cu numele lui?
    printf("%s\n", "printf_dir <dir/file>");
    printf("%s\n", "pentru deconectare -> logout");
    printf("%s\n\n\n", "pentru iesire din program -> quit");
}

int main(int argc, char *argv[])
{
    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare
    char msg[1000];

    if (argc != 3)
        handle_error("Ai uitat: <adresa_server> <port>\n");

    port = atoi(argv[2]); // portul
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        handle_error("Eroare la creare socket");

    server.sin_family = AF_INET;                 // familia socket-ului
    server.sin_addr.s_addr = inet_addr(argv[1]); // adresa IP a serverului
    server.sin_port = htons(port);               // portul de conectare

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        handle_error("Eroare la conectarea clientului la server");

    Comenzi();

    char username[100];
    char password[100];

    int logat = 0;
    while (1)
    {
        int iesire = 0;
        bzero(msg, 1000);
        printf("[client]Introduceti o comanda: ");
        fflush(stdout);

        read(0, msg, 1000); // citire de la stdin
        strtok(msg, "\n");

        if (strstr(msg, "quit") != 0)
        {
            logat = 0;
            iesire = 1;
        }

        if (strstr(msg, "login") != 0 && logat == 0)
        {
            printf("Introduceti username: ");
            if (fgets(username, sizeof(username), stdin) != 0)
            {
                username[strlen(username) - 1] = 0; // Elimină '\n' dacă există
            }

            printf("Introduceti parola: ");
            if (fgets(password, sizeof(password), stdin) != 0)
            {
                password[strlen(password) - 1] = 0; // Elimină '\n' dacă există
            }

            char hashed_password[65]; // 32*2 +1 (1 care este null din final)
            hash(password, hashed_password);

            strcat(msg, " ");
            strcat(msg, username);
            strcat(msg, " ");
            strcat(msg, hashed_password);
            // acum voi trimite perechea username password la server
        }
        if (strstr(msg, "logout") != 0)
        {
            logat = 0;
        }
        if (write(sd, msg, 1000) <= 0)
            handle_error("Eroare la scriere catre server");

        if (read(sd, msg, 1000) < 0) // citim mesajul de la server
            handle_error("Eroare la citire de la server");

        if (strstr(msg, "Gasit in whitelist. Conectare cu succes!") != 0)
        {
            logat = 1;
        }
        if (strcmp(msg, "Gasit in blacklist. Nu te poti conecta!") == 0 || strcmp(msg, "Ai incercat de 3 ori. Esti adaugat in blacklist.") == 0)
        {
            iesire = 1;
        }
        printf("[client]Mesajul primit de la server este: %s\n", msg); // afisam mesajul de la server

        if (iesire == 1)
            break;
    }
    close(sd); // se inchide conexiunea clientului
    return 0;
}