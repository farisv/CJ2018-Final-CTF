#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int is_directory(const char* file)
{
    FILE *fp;
    fp = fopen(file, "r+");

    if (fp == NULL)
    {
        if (errno == EISDIR) 
        {
            return 1;
        }
    }

    return 0;
}

void print_flag()
{
    DIR *dir;
    struct dirent *entry;
    char* ret = NULL;

    if (!(dir = opendir("/var/flag/")))
    {
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strlen(entry->d_name) >= 32 && strlen(entry->d_name) <= 64)
        {
            FILE* fp;
            char path[128];

            strcpy(path, "/var/flag/");
            strcat(path, entry->d_name);
            fp = fopen(path, "r");
            if (fp == NULL)
            {
                return;
            }

            ret = malloc(128);
            memset(ret, 0, 128);
            fread(ret, 64, 1, fp);

            printf("Flag: %s\n", ret);

            closedir(dir);
            return;
        }
    }

    closedir(dir);

    puts("Flag not found. Please contact administrator.");

    return;
}

char* get_system_password()
{
    FILE* fp;
    char* ret;

    fp = fopen("password.conf", "r");
    if (fp == NULL)
    {
        return NULL;
    }

    ret = malloc(65);
    memset(ret, 0, 65);
    fread(ret, 64, 1, fp);

    return ret;
}

char* get_system_username()
{
    return "admin_CJ";
}
