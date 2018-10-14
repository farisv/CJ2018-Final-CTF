/*
 * CJ Firmware
 * Cyber Jawara 2018 Final - Attack & Defense CTF
 * Based on https://gist.github.com/laobubu/d6d0e9beb934b60b2e552c2d03e1409e
 *
 * $ make
 * $ ./server
*/
#include "helper.h"
#include "httpd.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int main(int c, char** v)
{
    serve_forever("8000");
    return 0;
}

void display_not_found(const char *file)
{
    printf("HTTP/1.1 404 Not Found\r\n\r\n");
    printf("Not Found: %s\n", file);
}

void display(const char* file)
{
    FILE *fp;
    char* content;
    size_t sz;

    if (is_directory(file))
    {
        display_not_found(file);
        return;
    }

    fp = fopen(file, "r");

    if (fp == NULL)
    {
        display_not_found(file);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    rewind(fp);

    if (sz == 0)
    {
        display_not_found(file);
        return;
    }

    content = malloc(sz);
    fread(content, sz, 1, fp);

    printf("HTTP/1.1 200 OK\r\n\r\n");
    fflush(stdout);
    write(1, content, sz);

    free(content);
}

void route()
{
    ROUTE_START()

    ROUTE_GET("/")
    {
        display("index.html");
    }

    ROUTE_GET_P("/static/")
    {
        char fpath[1024];
        strcpy(fpath, "static/");
        strcat(fpath, path);
        display(fpath);
    }

    ROUTE_GET("/login")
    {
        char* username;
        char* password;
        char* system_username;
        char* system_password;
        size_t username_len;
        size_t password_len;
        size_t i = 0, j = 0, k = 9;

        // Username must be 'admin_CJ' (you are not allowed to change this)
        // Password must be in password.conf (you are allowed to change the content of password.conf)
        system_username = get_system_username();
        system_password = get_system_password();

        if (system_username == NULL || system_password == NULL)
        {
            printf("HTTP/1.1 500 Internal Server Error\r\n\r\n");
            fflush(stdout);
            printf("Internal Server Error");
            return;
        }

        username_len = strlen(system_username);
        password_len = strlen(system_password);

        while (i < strlen(qs))
        {
            if (strncmp(qs + i, "username=", 9) == 0)
            {
                j = k = i + 9;
                while (k < strlen(qs) && qs[k] != '&')
                {
                    k++;
                }
                if (k - j > 0)
                {
                    username = malloc(k - j + 1);
                    strncpy(username, qs + j, k - j);
                    username_len = k - j;
                }
                i = k + 1;
                continue;
            }
            if (strncmp(qs + i, "password=", 9) == 0)
            {
                j = k = i + 9;
                while (k < strlen(qs) && qs[k] != '&')
                {
                    k++;
                }
                if (k - j > 0)
                {
                    password = malloc(k - j + 1);
                    strncpy(password, qs + j, k - j);
                    password_len = k - j;
                }
                i = k + 1;
                continue;
            }
            i++;
        }

       if (username != NULL && password != NULL)
       {
           if (strncmp(username, system_username, username_len) == 0
                  && strncmp(password, system_password, password_len) == 0)
           {
                printf("HTTP/1.1 200 OK\r\n\r\n");
                fflush(stdout);
                printf("<h1>Welcome!</h1><br>");
                print_flag();
           }
           else
           {
               printf("HTTP/1.1 200 OK\r\n\r\n");
               fflush(stdout);
               printf("<script>alert('Wrong!'); window.location.href = '/';</script>");
           }
       }
       else
       {
           printf("HTTP/1.1 200 OK\r\n\r\n");
           fflush(stdout);
           printf("<script>window.location.href = '/';</script>");   
       }
    }
  
    ROUTE_END()
}
