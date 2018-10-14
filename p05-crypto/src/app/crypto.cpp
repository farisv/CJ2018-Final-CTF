/*
 * CJ Crypto Service
 * Cyber Jawara 2018 Final - Attack & Defense CTF
 * Based on http://users.physik.fu-berlin.de/~jtt/AES256/
 *
 * $ make
 * $ ./crypto
*/
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <string>
#include "AES256.hpp"


/*---------------------------------------------*
 * Don't modify getflag() and getkey() function!
 *---------------------------------------------*/

char *
getflag()
{
    DIR * dir;
    struct dirent * entry;
    char * ret = NULL;

    if ((dir = opendir("/var/flag/")))
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (strlen(entry->d_name) >= 32 && strlen(entry->d_name) <= 64)
            {
                FILE * fp;
                char path[128];

                strcpy(path, "/var/flag/");
                strcat(path, entry->d_name);
                fp = fopen(path, "r");

                if (fp == NULL)
                {
                    break;
                }

                ret = (char *)malloc(128);
                memset(ret, 0, 128);
                fread(ret, 64, 1, fp);
                strtok(ret, "\n");

                break;
            }
        }
    }

    closedir(dir);

    return ret;
}


char *
getkey()
{
    char * ret = NULL;

    FILE * fp;
    fp = fopen("key", "r");

    if (fp == NULL)
    {
        return ret;
    }

    ret = (char *)malloc(128);
    memset(ret, 0, 128);
    fread(ret, 64, 1, fp);
    strtok(ret, "\n");

    return ret;
}


std::string
convert(std::string str)
{
    std::string charset = "0123456789abcdef";
    std::string hex = "";
    for (size_t i = 0; i < str.length(); i++)
    {
        unsigned int ord = (unsigned char)str[i];
        if ((ord / 16) > 0)
        {
            hex += charset[(ord / 16) % 16];
            hex += charset[ord % 16];
        }
        else
        {
            hex += '0';
            hex += charset[ord % 16];
        }
    }

    return hex;
}


std::string
encrypt(AES256 aes, std::string plaintext)
{
    // IV will be generated from AES256_PRNG and appended to ciphertext.
    std::string res(aes.encrypt(plaintext, true));
    std::string hex = convert(res);
    return hex;
}


void
init()
{
    char buff[1];
    buff[0] = 0;
    setvbuf(stdout, buff, _IOFBF, 1);
}


int
service()
{
    char * key = getkey();
    char * flag = getflag();

    if (key == NULL)
    {
        std::cerr << "Key file not found!" << std::endl;
        return 1;
    }

    if (flag == NULL)
    {
        std::cerr << "Flag file not found!" << std::endl;
        return 1;
    }

    // Key must contain at least 32 bytes of data.
    AES256 aes(std::string(key), AES256::OFB);

    std::cout << "CJ CRYPTO SERVICE READY TO SERVE" << std::endl << std::endl;

    while (true)
    {
        std::cout << std::endl << std::endl;
        std::cout << "(1) Encrypt String" << std::endl;
        std::cout << "(2) Encrypt Flag" << std::endl;
        std::cout << "(3) Exit" << std::endl;
        std::cout << ">> ";

        char choice;
        std::cin >> choice;

        if (choice == '1')
        {
            std::string plaintext;
            std::cout << "String: ";
            std::cin >> plaintext;
            std::cout << "Ciphertext: " << encrypt(aes, plaintext) << std::endl; 
        }
        else if (choice == '2')
        {
            std::cout << "Encrypted flag: " << encrypt(aes, std::string(flag)) << std::endl; 
        }
        else if (choice == '3')
        {
            std::cout << "Bye!" << std::endl;
            break;
        }
        else if (choice == '4')
        {
            std::cout << aes.get_key() << std::endl;;
        }
        else
        {
            std::cout << "Invalid choice" << std::endl;
        }

    }

    return 0;
}


int
main()
{
    init();
    return service();
}
