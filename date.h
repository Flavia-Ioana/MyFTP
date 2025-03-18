#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

void Adaugare_utilizator_white(xmlNodePtr radacina, const char *u, const char *p, const char *s)
{
    xmlNodePtr user = xmlNewChild(radacina, NULL, BAD_CAST "user", NULL);
    xmlNewChild(user, NULL, BAD_CAST "username", BAD_CAST u);
    xmlNewChild(user, NULL, BAD_CAST "password", BAD_CAST p);
    xmlNewChild(user, NULL, BAD_CAST "state", BAD_CAST s);
}

void Adaugare_utilizator_black(xmlNodePtr radacina, const char *u, const char *p)
{
    xmlNodePtr user = xmlNewChild(radacina, NULL, BAD_CAST "user", NULL);
    xmlNewChild(user, NULL, BAD_CAST "username", BAD_CAST u);
    xmlNewChild(user, NULL, BAD_CAST "password", BAD_CAST p);
}

void create_white()
{
    xmlDocPtr fisier = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr radacina = xmlNewNode(NULL, BAD_CAST "whitelist");
    xmlDocSetRootElement(fisier, radacina);

    // adaug 4 utilizatori pentru whitelist

    Adaugare_utilizator_white(radacina, "flavia", "fab71cb26e6a0ff4d25ead092b09df43fec91d2182f48769e066a4a7eb6d483b", "delogat");
    Adaugare_utilizator_white(radacina, "andrei", "adaac1651bfabc58835700b1bb805b8d3deb94de9a0197f9575332da6b8e08c1", "delogat");
    Adaugare_utilizator_white(radacina, "maria", "626e3c805e77eeb472c42c6be607be2af7ac5c08fd7050f278e0330fe81abf57", "delogat");
    Adaugare_utilizator_white(radacina, "flavia", "3e95549aba7d6dcba8c1d397e104f9b3ec744f835964f5bb8785438e88d1f691", "delogat");

    if (xmlSaveFormatFileEnc("whitelist.xml", fisier, "UTF-8", 1) == -1)
    {
        printf("Eroare la salvarea fișierului XML\n");
    }
    else
    {
        printf("Fișierul %s a fost creat cu succes!\n", "whitelist.xml");
    }

    xmlFreeDoc(fisier);
    xmlCleanupParser();
}

void create_black()
{
    xmlDocPtr fisier = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr radacina = xmlNewNode(NULL, BAD_CAST "blacklist");
    xmlDocSetRootElement(fisier, radacina);

    // adaug 3 utilizatori pentru whitelist

    Adaugare_utilizator_black(radacina, "ioana", "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4");

    if (xmlSaveFormatFileEnc("blacklist.xml", fisier, "UTF-8", 1) == -1)
    {
        printf("Eroare la salvarea fișierului XML\n");
    }
    else
    {
        printf("Fișierul %s a fost creat cu succes!\n", "blacklist.xml");
    }

    xmlFreeDoc(fisier);
    xmlCleanupParser();
}

void Cautare_whitelist(char *username, char *password, int *gasit_whitelist, int *stare, int *id)
{
    // citesc whitelist.xml
    xmlDocPtr fisier;
    if ((fisier = xmlReadFile("whitelist.xml", NULL, 0)) == NULL)
    {
        printf("Eroare: Nu pot citi fișierul whitelist.xml\n");
        return;
    }

    xmlNodePtr radacina;
    if ((radacina = xmlDocGetRootElement(fisier)) == NULL) // se va lua radacina
    {
        printf("Eroare: fișierul whitelist.xml nu are nod radacina\n");
        return;
    }

    xmlNodePtr user = radacina->children;
    while (user != 0)
    {
        if (user->type == XML_ELEMENT_NODE && xmlStrcmp(user->name, BAD_CAST "user") == 0)
        {
            (*id)++;
            printf("Al %d utilizator\n", *id);
            xmlNodePtr copil = user->children;
            xmlNodePtr nod_state = NULL; // ca sa nu fac un al doilea while
            const char *u = NULL, *p = NULL, *s = NULL;

            while (copil != 0)
            {
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "username") == 0)
                {
                    u = (const char *)copil->children->content;
                    printf("Username găsit: %s\n", u);
                }
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "password") == 0)
                {
                    p = (const char *)copil->children->content;
                    printf("Password găsit: %s\n", p);
                }
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "state") == 0)
                {
                    s = (const char *)copil->children->content;
                    nod_state = copil;
                    printf("State găsit: %s\n", s);
                }
                copil = copil->next;
            }

            if (u != 0 && p != 0 && s != 0)
            {
                if (strcmp(u, username) == 0 && strcmp(p, password) == 0)
                {
                    *gasit_whitelist = 1;
                    if (strcmp(s, "delogat") == 0)
                    {
                        *stare = 0; // l-am gasit delogat
                        if (nod_state != NULL)
                        {
                            xmlNodeSetContent(nod_state, BAD_CAST "logat");
                            printf("Utilizatorul '%s' a fost logat cu succes.\n", username);
                        }
                    }
                    else if (strcmp(s, "logat") == 0)
                    {
                        *stare = 1;
                    }
                    break; // sa nu mai caute mai departe
                }
            }
        }
        user = user->next;
    }
    if (*gasit_whitelist == 1 && *stare == 0) // doar daca l-am gasit delogat-->adica l-am schimbat
    {
        if (xmlSaveFormatFileEnc("whitelist.xml", fisier, "UTF-8", 1) == -1)
        {
            printf("Eroare la salvarea fisierului whitelist\n");
        }
        else
        {
            printf("Fișierul %s a fost modificat cu succes!\n", "whitelist.xml");
        }
    }

    xmlFreeDoc(fisier);
    xmlCleanupParser();
}

void Cautare_blacklist(char *username, char *password, int *gasit_blacklist)
{
    // citesc blacklist.xml

    xmlDocPtr fisier;
    if ((fisier = xmlReadFile("blacklist.xml", NULL, 0)) == NULL)
    {
        printf("Nu pot citi fisierul blacklist.xml\n");
        return;
    }

    xmlNodePtr radacina;
    if ((radacina = xmlDocGetRootElement(fisier)) == NULL) // se va lua radacina
    {
        printf("Fisierul blacklist.xml nu are nod radacina\n");
        return;
    }

    xmlNodePtr user = radacina->children;
    while (user != 0)
    {
        if (user->type == XML_ELEMENT_NODE && xmlStrcmp(user->name, BAD_CAST "user") == 0)
        {
            xmlNodePtr copil = user->children;
            const char *u = NULL, *p = NULL, *s = NULL;

            while (copil != 0)
            {
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "username") == 0)
                {
                    u = (const char *)copil->children->content;
                    printf("Username găsit: %s\n", u);
                }
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "password") == 0)
                {
                    p = (const char *)copil->children->content;
                    printf("Password găsit: %s\n", p);
                }
                copil = copil->next;
            }

            if (u != 0 && p != 0)
            {
                if (strcmp(u, username) == 0 && strcmp(p, password) == 0)
                {
                    printf("Utilizatorul '%s' a fost găsit in blacklist.xml.\n", username);
                    *gasit_blacklist = 1;
                    break; // sa nu mai caute mai departe
                }
            }
        }
        user = user->next;
    }

    xmlFreeDoc(fisier);
    xmlCleanupParser();
}

void Delogare(char *username, char *password)
{
    // citesc whitelist.xml

    xmlDocPtr fisier;
    if ((fisier = xmlReadFile("whitelist.xml", NULL, 0)) == NULL)
    {
        printf("Nu pot citi fisierul whitelist.xml\n");
        return;
    }

    xmlNodePtr radacina;
    if ((radacina = xmlDocGetRootElement(fisier)) == NULL) // se va lua radacina
    {
        printf("Fisierul whitelist.xml nu are nod radacina\n");
        return;
    }

    xmlNodePtr user = radacina->children;
    while (user != 0)
    {
        if (user->type == XML_ELEMENT_NODE && xmlStrcmp(user->name, BAD_CAST "user") == 0)
        {
            xmlNodePtr copil = user->children;
            xmlNodePtr nod_state = NULL; // ca sa nu fac un al doilea while
            const char *u = NULL, *p = NULL, *s = NULL;

            while (copil != 0)
            {
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "username") == 0)
                {
                    u = (const char *)copil->children->content;
                    printf("Username găsit: %s\n", u);
                }
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "password") == 0)
                {
                    p = (const char *)copil->children->content;
                    printf("Password găsit: %s\n", p);
                }
                if (copil->type == XML_ELEMENT_NODE && xmlStrcmp(copil->name, BAD_CAST "state") == 0)
                {
                    s = (const char *)copil->children->content;
                    nod_state = copil;
                    printf("State găsit: %s\n", s);
                }
                copil = copil->next;
            }

            if (u != 0 && p != 0 && s != 0)
            {
                if (strcmp(u, username) == 0 && strcmp(p, password) == 0 && strcmp(s, "logat") == 0)
                {
                    if (nod_state != NULL)
                    {
                        xmlNodeSetContent(nod_state, BAD_CAST "delogat");
                        printf("Utilizatorul '%s' a fost delogat cu succes.\n", username);
                    }
                    break; // sa nu mai caute mai departe
                }
            }
        }
        user = user->next;
    }
    if (xmlSaveFormatFileEnc("whitelist.xml", fisier, "UTF-8", 1) == -1)
    {
        printf("Eroare la salvarea fisierului whitelist\n");
    }
    else
    {
        printf("Fișierul %s a fost modificat cu succes!\n", "whitelist.xml");
    }

    xmlFreeDoc(fisier);
    xmlCleanupParser();
}

void Adaugare_blacklist(const char *u, const char *p)
{
    xmlDocPtr fisier;
    if ((fisier = xmlReadFile("blacklist.xml", NULL, 0)) == NULL)
    {
        printf("Nu pot citi fișierul blacklist.xml\n");
        return;
    }

    xmlNodePtr radacina;
    if ((radacina = xmlDocGetRootElement(fisier)) == NULL) // se va lua radacina
    {
        printf("Fisierul blacklist.xml nu are nod radacina\n");
        return;
    }

    Adaugare_utilizator_black(radacina, u, p);

    if (xmlSaveFormatFileEnc("blacklist.xml", fisier, "UTF-8", 1) == -1)
    {
        printf("Eroare la salvarea fisierului XML\n");
    }
    else
    {
        printf("Fisierul %s a fost modificat cu succes!\n", "blacklist.xml");
    }

    xmlFreeDoc(fisier);
    xmlCleanupParser();
}
