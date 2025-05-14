#!/bin/bash

#initalise client
sudo systemctl restart mosquitto.service

result=0
var=1

mosquitto_pub -h 104.196.229.154 -t inTopic -m "3"

while true; do
    echo "Play X!"
	mosquitto_pub -h 104.196.229.154 -t inTopic -m "4"
	sleep 1s

    echo "Play O!"
	mosquitto_pub -h 104.196.229.154 -t inTopic -m "5"
    sleep 1s
done

