# Cyber Jawara 2018 Final - Attack & Defense CTF Services Environments

Cyber Jawara is an annual national cyber security competition in Indonesia. The 2018 final stage was held in Bali as part of [Code Bali International Cyber Security Conference and Exhibitions](https://codebali.net/).

The 2018 final contest is driven by Attack & Defense [CTF](https://en.wikipedia.org/wiki/Capture_the_flag#Computer_security) rule with custom scoring and mechanisms. Each team is given a set of vulnerable services. CTF Administrator generate and distribute the flags to each service for each tick (1 tick = ~10 minutes). Score is calculated for each round (1 round = 3 ticks) based on successful attacks (number of captured flags) and successful defense (number of defended flags). The goal of each team is to find vulnerabilities, patch their services, and exploit other team's services in order to get the flags.

This repository contains Cyber Jawara 2018 Final services docker environments. No pre-existing knowledge of docker is required, just execute some simple commands and you can test the services. No Attack & Defense Platform & Docker Manager included (only the services).

## Installation

Install the docker/docker-compose on Ubuntu:

```bash
# Install pip
curl -s https://bootstrap.pypa.io/get-pip.py | python

# Install the latest version docker
curl -s https://get.docker.com/ | sh

# Run docker service
service docker start

# Install docker compose
pip install docker-compose
```

The installation steps of docker and docker-compose for others operating system might be slightly different, please refer to the [docker documentation](https://docs.docker.com/) for details.

## Usage

```bash
# Download repository
wget https://github.com/farisv/CJ2018-Final-CTF/archive/master.zip -O CJ2018-Final-CTF.zip
unzip CJ2018-Final-CTF.zip
cd CJ2018-Final-CTF-master

# Enter the directory of service. Example for p00-warmup:
cd p00-warmup

# Compile environment
docker-compose build

# Run environment
docker-compose up -d
```

Each service run with different port.

You may want to delete the environment after the test.

```
docker-compose down -v
```

## List of Services

|ID                     |Description    |How to Connect in Local     |
|-----------------------|---------------|----------------------------|
| p00-warmup            | Welcome to Cyber Jawara 2018 and welcome to Bali! Please familarize yourself with this Attack & Defense CTF. You can try to patch this web service by using this Patcher Platform and attack the others with the Flagger Platform. Score for this service is 0. | http://127.0.0.1:50000 |
| p01-firmware          | This web service is running with a standalone binary executable and simulating a firmware (not a real firmware). You need to find the ways to get into administrator area. | http://127.0.0.1:50100 |
| p02-sandbox           | In this web service, you can run a sandboxed x64 Linux Shellcode by pasting hex string that encodes x64 instructions. Sandboxing is done with a ptrace-based syscall filter to block execve and execveat sysall. You need to find the ways to get the flag. For patching, you are only allowed to patch sandbox.c. Functionality tests to write a string to stdout or file and read a file will be performed. | http://127.0.0.1:50200 |
| p03-imagemanipulator  | An image manipulator with PHP for resize, crop, rotate, flip, mirror, grayscale, and sepia effect. | http://127.0.0.1:50300 |
| p04-coderunner        | Evaluating user inputted variable for code execution is dangerous. For protection, dangerous PHP functions are disabled. For additional layer of protection, Web Application Firewall (WAF) with ModSecurity is also used. You can configure the WAF by modifying waf.conf to protect this service. | http://127.0.0.1:50400 |
| p05-cyrpto            | This is a crypto service using AES/Rijndael 256. This service will encrypt the flag file and user inputted string. You need to break the encryption and try to recover the flag. For patching, you are only allowed to patch crypto.cpp and key. Functionality test will try to encrypt and decrypt with your key. Please don't change the block cipher mode of operation. | `nc 127.0.0.1 50500` |
| p06-hades             | This binary can compute the longest increasing subsequence of 64 bit numbers with O(n^2) time complexity. Can you pwn it like a pro? For patching, you are only allowed to patch 64 bytes. For this service, all attack points are multiplied by 1.5x. | `nc 127.0.0.1 50600` |
| p07-exifreader        | EXIF or Exchangable Image File is a format that is a standard for storing interchange information in digital photography image files using JPEG compression. This web based app is useful for forensic analysis to quickly read the EXIF data inside an image. | http://127.0.0.1:50700 |
| p08-hackme            | Can you hack this simple and obviously vulnerable website? It seems easy, but the website is protected by Web Application Firewall (WAF) with ModSecurity. You can configure the WAF by modifying waf.conf for more protection. | http://127.0.0.1:50800 |
| p09-poseidon          | Hackers also need to understand data structure and algorithm. Do you know if ext3 filesystem tracks directory entries in a balanced binary search tree using red-black tree? Virtual memory areas (VMAs) in Linux are also tracked with red-black trees, as are epoll file descriptors, cryptographic keys, and network packets in the "hierarchical token bucket" scheduler. Now, please try to pwn this binary search tree service. For patching, you are only allowed to patch 16 bytes. For this service, all attack points are multiplied by 1.5x. | `nc 127.0.0.1 50900` |
| p10-assignment        | Laravel is popular and loved by many. This web app is a simple assignment submission apps made with Laravel where you can upload an archive containing all of your exploit code (.c/.cpp/.py/.sh) and compute the SHA1 checksum of each file. For this service, all attack points are multiplied by 1.5x. | http://127.0.0.1:51000 |
| p11-zeus              | In-memory key-value database server like Redis and Memcache is widely used for caching in a production server. Can you pwn this key-value database service? You need to reverse engineered it first to know what data structure is used for storing the data. This service is also run with sandbox. For patching, you are only allowed to patch 32 bytes in 'zeus' binary and only 2 bytes in 'zeus_sandbox'. For this service, all attack points are multiplied by 2x. | `nc 127.0.0.1 51100` |

## Notes About Original Contest Environments

In original Cyber Jawara 2018 final contest, docker environments were built on October 9th, 2018. Results of `apt update` and `apt install` might be different when you build the services. All binary problems (p01-firmware, p02-sandbox, p05-crypto, p06-hades, p09-poseidon, and p11-zeus) are created to be solved in Ubuntu 18.10 with glibc 2.28 but it might be possible to solve the problems in other Ubuntu versions, other Linux distros, or other glibc versions. If you want to contribute so the installed dependency versions are fixed, you are very welcomed.

We know we use docker in "wrong way" when preparing the contest environments but at least they works like expected during competition.

In original contest, flags are generated and distributed for each tick and placed in `/var/flag/` directory with random name. In this test environments, a placeholder flag file is placed in `/var/flag/00000000000000000000000000000000`.

## Write Up

You can contribute by adding the link to your write up here. A service can has more than one vulnerability so various solutions are welcomed.

- p03-imagemanipulator
	- [https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P03-Image-Manipulator](https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P03-Image-Manipulator) 
- p04-coderunner
	- [https://www.rhama.my.id/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P04-Code-Runner](https://www.rhama.my.id/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P04-Code-Runner)
- p07-exifreader
	- [https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P07-Exif-Reader](https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P07-Exif-Reader)
- p08-hackme
	- [https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P08-Hackme](https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P08-Hackme)
- p10-assignment
	- [https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P10-Assignment-Solved-After-Competition](https://rhamaa.github.io/2018/10/17/Cyber-Jawara-2018-Final-Web-Exploitation/#P10-Assignment-Solved-After-Competition) 
- p11-zeus
	- [https://zeroload.github.io/2018/10/14/Cyber-Jawara-2018-Final-zeus/](https://zeroload.github.io/2018/10/14/Cyber-Jawara-2018-Final-zeus/)
