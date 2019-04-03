#! /usr/bin/bash
ab -n 10000 -c 1000 http://localhost:8888/httpd.log
