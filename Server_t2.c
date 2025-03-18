/* servTcpConc.c - Exemplu de server TCP concurent
   Asteapta un nume de la clienti; intoarce clientului sirul
   "Hello nume".
   */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bits/waitflags.h>
#include <dirent.h>
#include <sys/stat.h>
#include "date.h"

#define PORT 2024
extern int errno;

void handle_error(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void Eroare(char *message)
{
    perror(message);
}

void Mesaj(char msgrasp[1000], char *message)
{

    bzero(msgrasp, 1000);
    strcat(msgrasp, message);
    printf("[server]Trimitem mesajul inapoi...%s\n", msgrasp);
}

int Parsare(char *input, char username[100], char password[100])
{
    char *p;
    p = strtok(input, " ");
    p = strtok(0, " ");
    strcpy(username, p);
    p = strtok(0, " ");
    strcpy(password, p);

    p = strtok(0, " ");
    if (p != 0)
        return 0;
    return 1;
}

int Accesare_director(char folder_user[1000], char username[100], int id)
{
    char id_char[100];
    sprintf(id_char, "%d", id);
    strcpy(folder_user, username);
    strcat(folder_user, id_char);

    DIR *dir = opendir(folder_user);

    if (dir != 0)
    {
        printf("[debug]Directorul exista\n");
    }
    else // directorul nu exista
    {
        if (mkdir(folder_user, 0700) == 0)
        {
            printf("[debug]S-a creat directorul %s\n", folder_user);
        }
        else
        {
            printf("[eroare]Eroare la crearea directorului\n");
            return -1;
        }
    }
    if (chdir(folder_user) != 0)
    {
        printf("[eroare]Eroare la schimbarea directorului\n");
        return -1;
    }
    else
    {
        printf("[debug]S-a intrat in director %s\n", folder_user);
    }
    return 0;
}

int exista(char *fisier)
{
    struct stat info;

    if (stat(fisier, &info) == 0)
    {
        if (S_ISDIR(info.st_mode) != 0)
        {
            return 1; // este director
        }
        else if (S_ISREG(info.st_mode) != 0)
        {
            return 2; // este fisier
        }
        return 3; // in caz ca e altceva
    }
    return 0; // nu exista
}

void Create_file(char fisier[100], char msgrasp[1000])
{
    // verific sa nu am alt fisier cu acelasi nume si director
    char msg[500];
    if (exista(fisier) == 0)
    {
        FILE *fd;
        if ((fd = fopen(fisier, "w")) == NULL)
        {
            // transmit si la client ca nu s-a putut crea fisierul
            sprintf(msg, "Eroare la crearea fisierului %s", fisier);
        }
        else
        {
            sprintf(msg, "S-a creat fisierul %s cu succes!", fisier);
            fclose(fd);
        }
        Mesaj(msgrasp, msg);
    }
    else
    {
        if (exista(fisier) == 1)
        {
            sprintf(msg, "Exista deja un director cu numele %s", fisier);
        }
        else
        {
            sprintf(msg, "Exista deja un alt fisier cu numele %s", fisier);
        }
        Mesaj(msgrasp, msg);
    }
}

void Create_dir(char folder[100], char msgrasp[1000])
{
    // verific sa nu am deja un folder cu numele respectiv
    // verific sa nu am alt fisier cu acelasi nume si director
    char msg[500];
    if (exista(folder) == 0)
    {
        if (mkdir(folder, 0700) == 0)
        {
            sprintf(msg, "S-a creat folderul %s cu succes!", folder);
        }
        else
        {
            // transmit si la client ca nu s-a putut crea folderul
            sprintf(msg, "Eroare la crearea folderului %s", folder);
        }
        Mesaj(msgrasp, msg);
    }
    else
    {
        if (exista(folder) == 1)
        {
            sprintf(msg, "Exista deja un director cu numele %s", folder);
        }
        else
        {
            sprintf(msg, "Exista deja un alt fisier cu numele %s", folder);
        }
        Mesaj(msgrasp, msg);
    }
}

void Remove(char nume[100], char msgrasp[1000])
{
    char msg[500];
    if (strchr(nume, '/') != 0)
    {
        sprintf(msg, "Sa nu ai / in numele %s", nume);
    }
    else
    {
        if (exista(nume) == 0) // nu exista nimic cu numele acesta
        {
            sprintf(msg, "Nu se poate sterge pentru ca nu exista %s", nume);
        }
        else if (exista(nume) == 1) // exista un director cu numele acesta
        {
            if (remove(nume) == 0) // in caz in care este gol
            {
                sprintf(msg, "S-a sters cu succes directorul %s!", nume);
            }
            else
            {
                sprintf(msg, "Eroare la stergere directorului %s. Nu este gol!.", nume);
            }
        }
        else if (exista(nume) == 2) // exista un fisier cu numele acesta
        {
            if (remove(nume) == 0)
            {
                sprintf(msg, "S-a sters cu succes fisierul %s!", nume);
            }
            else
            {
                sprintf(msg, "Eroare la stergere fisierului %s", nume);
            }
        }
    }
    Mesaj(msgrasp, msg);
}

int Comenzi_to(char msg[1000], char *a1, char *a2)
{
    char *p = strtok(msg, " ");
    int ct = 0; // cate argumente avem (trebuie sa avem 2 neaparat la comenzile de tip ... to ...)
    while (p != 0)
    {
        if (ct == 1)
        {
            strcpy(a1, p);
        }
        else if (ct == 3)
        {
            strcpy(a2, p);
        }
        p = strtok(0, " ");
        ct++;
    }
    ct--;
    if (ct == 3)
        return 3;
    if (ct == 2)
        return 2;
    if (ct == 1)
        return 1;
    return 0;
}

int Comenzi_simple(char msg[1000], char *a)
{
    char *p = strtok(msg, " ");
    int ct = 0;
    while (p != 0)
    {
        if (ct == 1)
        {
            strcpy(a, p);
        }
        p = strtok(0, " ");
        ct++;
    }
    ct--;
    if (ct != 1)
        return 0;
    return 1;
}

void Rename(char old[100], char new[100], char msgrasp[1000])
{
    char msg[500];
    // verificam sa folosim doar nume simple fara / in old s new ca sa asigur ca lucrez doar cu cele din directorul curent de lucru
    if (strchr(old, '/') != 0)
    {
        sprintf(msg, "Nu poti sa ai / in numele %s", old);
    }
    else if (strchr(new, '/') != 0)
    {
        sprintf(msg, "Nu poti sa ai / in numele %s", new);
    }
    else
    {
        if (exista(old) == 0) // nu exista nimic cu numele acesta
        {
            sprintf(msg, "Nu se poate redenumi pentru ca nu exista %s", old);
        }
        else // exista un fisier/director cu numele acesta
        {
            // trebuie sa vedem daca exista ceva cu numele new deja
            if (exista(new) == 0)
            {
                if (rename(old, new) == 0)
                {
                    sprintf(msg, "S-a redenumit cu succes din %s in %s!", old, new);
                }
                else
                {
                    sprintf(msg, "Eroare la redenumirea din %s in %s", old, new);
                }
            }
            else if (exista(new) == 1)
            {
                sprintf(msg, "Exista deja un director cu numele %s!", new);
            }
            else
            {
                sprintf(msg, "Exista deja un fisier cu numele %s!", new);
            }
        }
    }
    Mesaj(msgrasp, msg);
}

void Change_dir(char dir[100], char msgrasp[1000])
{
    char msg[500];
    if (strchr(dir, '/') != 0)
    {
        sprintf(msg, "Te poti muta doar un pas spre adancimea directorului curent. Numele %s are /", dir);
    }
    else
    {
        if (exista(dir) == 1) // daca exista directorul unde vreau sa merg
        {
            if (chdir(dir) == 0)
            {
                char cale[1000];
                if (getcwd(cale, sizeof(cale)) != 0)
                {
                    printf("%s\n", cale);
                }
                sprintf(msg, "S-a ajuns in directorul %s cu sucess", dir);
            }
            else
            {
                sprintf(msg, "Nu s-a putut trece la directorul %s", dir);
            }
        }
        else if (exista(dir) == 2)
        {
            sprintf(msg, "Nu poti sa mergi in %s pentru ca este un fisier", dir);
        }
        else if (exista(dir) == 0)
        {
            sprintf(msg, "Nu exista directorul %s", dir);
        }
    }
    Mesaj(msgrasp, msg);
}

int Cautare(char parinte[1000], char folder_user[1000])
{
    char *p = strtok(parinte, "/");
    while (p != 0)
    {
        if (strcmp(p, folder_user) == 0)
            return 1;
        p = strtok(0, "/");
    }
    return 0;
}

void Change_p_dir(char folder_user[1000], char msgrasp[1000])
{
    printf("Folderul userului este %s\n", folder_user);

    char msg[500];
    char parinte[1000];
    char copie_parinte[1000];
    if (chdir("..") == 0)
    {
        if (getcwd(parinte, sizeof(parinte)) != 0)
        {
            strcpy(copie_parinte, parinte);
            // trebuie sa verific daca a depasit de folder_user
            if (Cautare(copie_parinte, folder_user) == 1) // inca nu am depasit limita
            {
                strcpy(msg, "S-a ajuns in directorul parinte cu sucess");
                printf("%s\n", parinte);
            }
            else // am depasit ->raman in folder_user
            {
                if (chdir(folder_user) == 0)
                {
                    strcpy(msg, "Nu poti sa mergi in directorul parinte asa ca ramai in cel corespunzator userului");
                }
                else
                {
                    strcpy(msg, "Nu se poate trece pentru a ramane in directorul corespunzator userului");
                }
            }
        }
        else
        {
            strcpy(msg, "Eroare la obtinerea directorului curent");
        }
    }
    else
    {
        strcpy(msg, "Nu s-a putut trece la directorul parinte");
    }
    Mesaj(msgrasp, msg);
}

int Printf(char msgrasp[1000])
{
    char msg[1000];

    // deschidem directorul curent de lucru
    char dir_curr[1000];
    DIR *dir_d;
    struct dirent *entry;
    if (getcwd(dir_curr, sizeof(dir_curr)) != 0)
    {
        printf("[debug]Directorul curent unde se face cautarea %s\n", dir_curr);
        if ((dir_d = opendir(dir_curr)) == 0)
        {
            strcpy(msg, "Eroare la deschiderea directorului");
        }
        else
        {
            strcpy(msg, "\n");
            char temp[300];
            while ((entry = readdir(dir_d)) != 0)
                if (exista(entry->d_name) == 1)
                {
                    sprintf(temp, "FOLDER: %s\n", entry->d_name);
                    strcat(msg, temp);
                }
                else if (exista(entry->d_name) == 2)
                {
                    sprintf(temp, "FISIER: %s\n", entry->d_name);
                    strcat(msg, temp);
                }
                else if (exista(entry->d_name) == 3)
                {
                    sprintf(temp, "ALTCEVA: %s\n", entry->d_name);
                    strcat(msg, temp);
                }
            closedir(dir_d);
        }
    }
    else
    {
        strcpy(msg, "Eroare la getcwd");
    }
    Mesaj(msgrasp, msg);
}

void Copiere_continut(char *nume1, char *nume2, char msg[1000])
{
    ssize_t fd_r, fd_w;
    char buffer[1000];

    FILE *fd1;
    if ((fd1 = fopen(nume1, "r")) == 0)
    {
        strcpy(msg, "Eroare la deschiderea fisierului ptr download");
        return;
    }

    FILE *fd2;
    if ((fd2 = fopen(nume2, "w")) == 0)
    {
        strcpy(msg, "Eroare la crearea fisierului ptr download");
        fclose(fd1);
        return;
    }
    while ((fd_r = fread(buffer, 1, sizeof(buffer), fd1)) > 0) // citirea a cate un octet
    {
        if ((fd_w = fwrite(buffer, 1, fd_r, fd2)) < 0)
        {
            strcpy(msg, "Eroare la scrierea in fd2 la descarcare");
            fclose(fd1);
            fclose(fd2);
            return;
        }
        fwrite(buffer, 1, fd_r, stdout); // afisam pe ecran sa vedem ce bagam in fisier
    }
    if (fd_r == -1)
    {
        strcpy(msg, "Eroare la citirea din fd1 la descarcare");
    }
    else // daca totul a mers ok
    {
        sprintf(msg, "S-a descarcat cu succes fisierul in directorul curent cu numele %s", nume2);
    }

    fclose(fd1);
    fclose(fd2);
}

void Download(char fisier[1000], char nume_new[100], char msgrasp[1000])
{
    char msg[1000];
    if (exista(fisier) == 2) // daca este fisier
    {
        if (strcmp(nume_new, "") == 0)
        {
            // trebuie sa aflu numele fisierului din calea respectiva
            char *p = strchr(fisier, '/');
            char *ultimul = 0;
            char nume_fisier[100];
            while (p != 0)
            {
                ultimul = p;
                p = strchr(p + 1, '/');
            }
            if (ultimul == 0)
            {
                strcpy(nume_fisier, fisier); // daca nu am calea absoluta
            }
            else
            {
                strcpy(nume_fisier, ultimul + 1);
            }

            // verific daca exista deja cu numele nume_fisier in directorul curent
            if (exista(nume_fisier) == 1)
            {
                sprintf(msg, "Exista deja un folder cu numele %s.Nu se poate face descarcarea", nume_fisier);
            }
            else if (exista(nume_fisier) == 2)
            {
                sprintf(msg, "Exista deja un fisier cu numele %s.Nu se poate face descarcarea", nume_fisier);
            }
            else
            {
                // deschidem fisier si punem in directorul curent cu acelasi nume
                Copiere_continut(fisier, nume_fisier, msg);
            }
        }
        else
        {
            // trebuie sa verific daca mai exista ceva cu numele nume_new
            if (strchr(nume_new, '/') != 0)
            {
                sprintf(msg, "Numele nou  %s al fisierului descarcat nu are voie sa aiba /. Descarcare doar in directorul de lucru", nume_new);
            }
            else
            {
                if (exista(nume_new) == 1)
                {
                    sprintf(msg, "Exista deja un folder cu numele %s.Nu se poate face descarcarea", nume_new);
                }
                else if (exista(nume_new) == 2)
                {
                    sprintf(msg, "Exista deja un fisier cu numele %s.Nu se poate face descarcarea", nume_new);
                }
                else
                {
                    // deschidem fisier si punem in directorul curent cu acelasi nume
                    Copiere_continut(fisier, nume_new, msg);
                }
            }
        }
    }
    else
    {
        strcpy(msg, "Nu exista acest fisier pentru a face descarcarea");
    }
    Mesaj(msgrasp, msg);
}

void Copy(char fisier[1000], char dir[1000], char msgrasp[1000])
{
    char msg[1000];
    char calea[2000];
    if (strchr(fisier, '/') != 0)
    {
        strcpy(msg, "Ai un / in nume");
    }
    else
    {
        if (exista(fisier) == 2) // daca este fisier
        {
            // trebuie sa creeam fisierul cu calea finala in directorul dir si cu continutul din fisier
            sprintf(calea, "%s/%s", dir, fisier);

            if (exista(dir) == 1) // daca e folder
            {
                // verific sa nu am alt fisier nu numele calea
                if (exista(calea) == 1)
                {
                    strcpy(msg, "Exista deja un directorul numele in care vrei sa faci copierea");
                }
                else if (exista(calea) == 2)
                {
                    strcpy(msg, "Exista deja un fisier numele in care vrei sa faci copierea");
                }
                else
                {
                    // copierea daca nu exista acest cu numele respectiv nimic altceva
                    Copiere_continut(fisier, calea, msg);
                }
            }
            else if (exista(dir) == 2) // daca e fisier
            {
                strcpy(msg, "Al doilea argument nu este un director");
            }
            else
            {
                strcpy(msg, "Nu exista aceasta cale catre care vrei sa copiezi");
            }
        }
        else
        {
            strcpy(msg, "Nu exita acest fisier pentru a face copie ala el");
        }
    }
    Mesaj(msgrasp, msg);
}

void Move(char fisier[1000], char dir[1000], char msgrasp[1000])
{
    char msg[1000];
    char calea[2000];
    if (strchr(fisier, '/') != 0)
    {
        strcpy(msg, "Nu poti sa ai un / in nume");
    }
    else
    {
        if (exista(fisier) == 2) // daca este fisier
        {
            // trebuie sa creeam fisierul cu calea finala in directorul dir si cu continutul din fisier
            sprintf(calea, "%s/%s", dir, fisier);

            if (exista(dir) == 1) // daca e folder
            {
                // verific sa nu am alt fisier cu numele calea
                if (exista(calea) == 1)
                {
                    strcpy(msg, "Exista deja un directorul numele in care vrei sa faci mutarea");
                }
                else if (exista(calea) == 2)
                {
                    strcpy(msg, "Exista deja un fisier cu numele pe care vrei sa il muti din directorul curent");
                }
                else
                {
                    // ii dam direct rename
                    if (rename(fisier, calea) == 0)
                    {
                        strcpy(msg, "S-a mutat cu succes fisierul");
                    }
                    else
                    {
                        strcpy(msg, "Eroare la mutarea fisierlui");
                    }
                }
            }
            else if (exista(dir) == 2) // daca e fisier
            {
                strcpy(msg, "Al doilea argument nu este un director");
            }
            else
            {
                strcpy(msg, "Nu exista aceasta cale catre care vrei sa muti");
            }
        }
        else
        {
            strcpy(msg, "Nu exista acest fisier pentru a face mutare la el");
        }
    }
    Mesaj(msgrasp, msg);
}

void Intoarcere(char initial_dir[1000], char msgrasp[1000], char *msg)
{
    if (chdir(initial_dir) != 0)
    {
        Mesaj(msgrasp, "[eroare]Eroare la revenirea in directorul initial");
    }
    else
    {
        printf("[debug]S-a revenit la directorul initial\n");
        char cale[1000];
        if (getcwd(cale, sizeof(cale)) != 0)
        {
            printf("%s\n", cale);
        }
        Mesaj(msgrasp, msg);
    }
}

int main()
{
    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    char msg[1000];           // mesajul de la client
    char msgrasp[1000] = " "; // mesaj pentru client
    int sd;
    int optval = 1;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        handle_error("Eroare la creare socket");

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        handle_error("[eroare]Eroare la atasarea socketului");

    if (listen(sd, 1) == -1)
        handle_error("[eroare]Eroare la ascultarea clientilor");

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    // preluam directorul curent
    char initial_dir[1000];
    if (getcwd(initial_dir, sizeof(initial_dir)) == 0)
    {
        handle_error("[eroare]Nu s-a putut obtine directorul curent");
    }
    printf("%s\n", initial_dir);
    // creez xml daca nu exista sau il deschid
    if (access("whitelist.xml", F_OK) == -1)
    {
        create_white();
    }
    if (access("blacklist.xml", F_OK) == -1)
    {
        create_black();
    }

    /* servim in mod concurent clientii... */
    while (1)
    {
        int client;
        int length = sizeof(from);
        int pid;

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0) // acceptare a clientilor
        {
            perror("[eroare]Eroare la acceptare clienti");
            continue;
        }

        printf("[server]Avem alt client\n");
        fflush(stdout);

        if ((pid = fork()) == -1) // cream un proces pentru fiecare client
        {
            close(client);
            continue;
        }
        else if (pid != 0) // parinte
        {
            close(client); // se inchide conexiunea cu clientul
            while (waitpid(-1, NULL, WNOHANG))
                ;
            continue; // se intoarce la while(1) ca sa accepte si alti clienti
        }
        else if (pid == 0) // copil
        {
            close(sd); // se inchide socketul care comunica cu server
            // nu trebuie sa ascultam alte conexiuni

            char username[100];
            char password[100];
            int logat = 0;            /// 0 daca nu este logat si 1 daca este logat
            int incercari_logare = 0; // ai 2 incercari ca a treia oara te baga in blacklist
            char folder_user[1000];

            while (1)
            {
                /* s-a realizat conexiunea, se astepta mesajul */
                bzero(msg, 1000); // toti octetii setati la 0
                printf("\n[server]Asteptam mesajul...\n");
                fflush(stdout);

                /* citirea mesajului */
                if (read(client, msg, 1000) <= 0)
                {
                    perror("[server]Eroare la read() de la client.\n");
                    close(client); /* inchidem conexiunea cu clientul */
                    exit(0);
                    continue; /* continuam sa ascultam */
                }
                printf("[server]Mesajul a fost receptionat...%s\n", msg);
                if (logat == 0) // daca nu este logat pot sa fac login sau quit sau comanda invalida
                {
                    if (strstr(msg, "login") != 0)
                    {
                        int rez;
                        rez = Parsare(msg, username, password); // voi lua doar perechea si o voi parsa
                        if (rez == 0)                           // in cazul in care am prea multe spatii si in username si in parola
                        {
                            Mesaj(msgrasp, "Prea multe argumente. Fara spatii la parola si username");
                        }
                        else
                        {
                            int gasit_whitelist = 0;
                            int gasit_blacklist = 0;
                            int stare = 0; // este 0 sau 1 daca e delogat si respectiv logat
                            int id = 0;    // al catalea este din baza de date
                            // partea de cautare
                            Cautare_whitelist(username, password, &gasit_whitelist, &stare, &id);
                            printf("%d\n", id);
                            if (gasit_whitelist == 0)
                            {
                                Cautare_blacklist(username, password, &gasit_blacklist);
                            }

                            if (gasit_whitelist == 1)
                            {
                                if (stare == 0)
                                {
                                    if (Accesare_director(folder_user, username, id) == 0)
                                    {
                                        Mesaj(msgrasp, "Gasit in whitelist. Conectare cu succes!");
                                        logat = 1;
                                        incercari_logare = 0;
                                    }
                                    else
                                    {
                                        Mesaj(msgrasp, "Nu s-a putut crea sau intra in directorul cu userul");
                                    }
                                }
                                else
                                {
                                    Mesaj(msgrasp, "Deja esti logat de pe alt dispozitiv.");
                                }
                            }
                            else if (gasit_blacklist == 1)
                            {
                                Mesaj(msgrasp, "Gasit in blacklist. Nu te poti conecta!");
                                if (write(client, msgrasp, 1000) <= 0)
                                {
                                    perror("[server]Eroare la write() catre client.\n");
                                    continue; /* continuam sa ascultam */
                                }
                                else
                                {
                                    printf("[server]Mesajul a fost trasmis cu succes.\n");
                                }
                                break;
                            }
                            else
                            {
                                if (incercari_logare != 2)
                                {
                                    Mesaj(msgrasp, "Nu te-am putut gasi! Introdu din nou datele");
                                    incercari_logare++;
                                }
                                else
                                {
                                    Mesaj(msgrasp, "Ai incercat de 3 ori. Esti adaugat in blacklist.");
                                    Adaugare_blacklist(username, password);
                                    if (write(client, msgrasp, 1000) <= 0)
                                    {
                                        perror("[server]Eroare la write() catre client.\n");
                                        continue; /* continuam sa ascultam */
                                    }
                                    else
                                    {
                                        printf("[server]Mesajul a fost trasmis cu succes.\n");
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    else if (strstr(msg, "quit") != 0) // aici nu este logat(nu am de ce sa ii dau delogare)
                    {
                        logat = 0;
                        Mesaj(msgrasp, "Iesire din server a clientului! La revedere!");
                        if (write(client, msgrasp, 1000) <= 0)
                        {
                            perror("[server]Eroare la write() catre client.\n");
                            continue; /* continuam sa ascultam */
                        }
                        else
                        {
                            printf("[server]Mesajul a fost trasmis cu succes.\n");
                        }
                        break;
                    }
                    else
                    {
                        Mesaj(msgrasp, "Inca nu esti logat! Incearca logarea!");
                    }
                }
                else if (logat == 1) // daca este logat
                {
                    if (strstr(msg, "login") != 0)
                    {
                        Mesaj(msgrasp, "Deja esti conectat!");
                    }
                    else if (strstr(msg, "create_file") != 0)
                    {
                        char fisier[100];
                        int ok = Comenzi_simple(msg, fisier);
                        if (ok == 0)
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incearca create_file <name>!");
                        }
                        else
                        {
                            Create_file(fisier, msgrasp);
                        }
                    }
                    else if (strstr(msg, "create_dir") != 0)
                    {
                        char folder[100];
                        int ok = Comenzi_simple(msg, folder);
                        if (ok == 0)
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incearca create_dir <name>!");
                        }
                        else
                        {
                            Create_dir(folder, msgrasp);
                        }
                    }
                    else if (strstr(msg, "exists") != 0)
                    {
                        char fisier[100];
                        char m[500];
                        int ok = Comenzi_simple(msg, fisier);
                        if (ok == 0)
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incearca exists <file/dir>!");
                        }
                        else
                        {
                            printf("[debug] %s\n", fisier);
                            if (exista(fisier) == 0)
                            {
                                sprintf(m, "Nu exista numele %s", fisier);
                            }
                            else if (exista(fisier) == 1)
                            {
                                sprintf(m, "Exista un director cu numele %s", fisier);
                            }
                            else
                            {
                                sprintf(m, "Exista un fisier cu numele %s", fisier);
                            }
                            Mesaj(msgrasp, m);
                        }
                    }
                    else if (strstr(msg, "remove") != 0)
                    {
                        char fisier[100];
                        int ok = Comenzi_simple(msg, fisier);
                        printf("[debug]%s\n", fisier);
                        if (ok == 0)
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incercati remove <file/dir_empty>!");
                        }
                        else
                        {
                            Remove(fisier, msgrasp);
                        }
                    }
                    else if (strstr(msg, "rename") != 0)
                    {
                        char name_old[100], name_new[100];
                        int ok = Comenzi_to(msg, name_old, name_new);
                        printf("[debug]%s %s\n", name_old, name_new);
                        if (ok != 3) // trebuie sa am 4 argumente neaparat de la 0 la 3
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incercati rename <name_old> to <name_new>!");
                        }
                        else
                        {
                            Rename(name_old, name_new, msgrasp);
                        }
                    }
                    else if (strstr(msg, "change_dir") != 0)
                    { // pot sa merg inainte in folderele din directorul curent sau in altele daca am toata calea
                        char dir[100];
                        int ok = Comenzi_simple(msg, dir);
                        printf("[debug]%s\n", dir);
                        if (ok == 0)
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incercati change_dir <dir>!");
                        }
                        else
                        {
                            Change_dir(dir, msgrasp);
                        }
                    }
                    else if (strstr(msg, "change_p_dir") != 0)
                    { // merg inapoi in directorul cu serverul si clientul
                        Change_p_dir(folder_user, msgrasp);
                    }
                    else if (strstr(msg, "printf_dir") != 0)
                    {
                        // trebuie afisat directorul curent
                        Printf(msgrasp);
                    }
                    else if (strstr(msg, "download") != 0) // aici ori am doar 2 argumente ori 4
                    {
                        char fisier[1000], name_new[100] = ""; // poate sa existe sau nu
                        int ok = Comenzi_to(msg, fisier, name_new);
                        printf("[debug]%s %s\n", fisier, name_new);
                        if (ok != 3 && ok != 1) // daca nu are nici 2 si nici 4 argumente
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incercati download <file> (with <new_nume>)!");
                        }
                        else
                        {
                            Download(fisier, name_new, msgrasp);
                        }
                    }
                    else if (strstr(msg, "copy") != 0)
                    {
                        char fisier[1000], dir[1000];
                        int ok = Comenzi_to(msg, fisier, dir);
                        printf("[debug]%s %s\n", fisier, dir);
                        if (ok != 3) // trebuie sa am 4 argumente
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incercati copy <file> to <dir>!");
                        }
                        else
                        {
                            Copy(fisier, dir, msgrasp);
                        }
                    }
                    else if (strstr(msg, "move") != 0)
                    {
                        char fisier[1000], dir[1000];
                        int ok = Comenzi_to(msg, fisier, dir);
                        printf("[debug]%s %s\n", fisier, dir);
                        if (ok != 3) // trebuie sa am 4 argumente
                        {
                            Mesaj(msgrasp, "Comanda introdusa gresit. Incercati move <file> to <dir>!");
                        }
                        else
                        {
                            Move(fisier, dir, msgrasp);
                        }
                    }
                    else if (strstr(msg, "logout") != 0)
                    {
                        logat = 0;
                        Intoarcere(initial_dir, msgrasp, "Deconectare utilizator");
                        Delogare(username, password);
                    }
                    else if (strstr(msg, "quit") != 0)
                    {
                        logat = 0;
                        Intoarcere(initial_dir, msgrasp, "Iesire din server a clientului! La revedere!");
                        Delogare(username, password);

                        if (write(client, msgrasp, 1000) <= 0)
                        {
                            perror("[server]Eroare la write() catre client.\n");
                            continue; /* continuam sa ascultam */
                        }
                        else
                        {
                            printf("[server]Mesajul a fost trasmis cu succes.\n");
                        }
                        break;
                    }
                    else
                    {
                        Mesaj(msgrasp, "Comanda invalida");
                    }
                }

                if (write(client, msgrasp, 1000) <= 0)
                {
                    perror("[server]Eroare la write() catre client.\n");
                    continue; /* continuam sa ascultam */
                }
                else
                {
                    printf("[server]Mesajul a fost trasmis cu succes.\n");
                }
            }
            close(client);
            exit(0);
        }
    }
}