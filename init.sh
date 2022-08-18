#!/bin/bash
cd sqlserver
docker build -t sql2017centos:1.0 .
cd ..
docker run -e 'ACCEPT_EULA=Y' -e 'SA_PASSWORD=7u8i9o0P' -p 1415:1433 --name sql-linuxcon15 -d -h linuxsql15 sql2017centos:1.0
docker-compose up -d --build
