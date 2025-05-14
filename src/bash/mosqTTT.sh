#!/bin/bash

#initalise client
sudo systemctl restart mosquitto.service

result=0
var=1
turn=0

mosquitto_pub -h 104.196.229.154 -t inTopic -m "3"

echo "CPP - CS 2600 - Tic Tac Toe! MQTT STYLE! Version 0.0.0"
read -p "What mode do you which to play? (1: One player, 2: Two player, 3: For 100): " mode

while true; do
    if [[ "$mode" = "1" ]]; then
        if [[ "$turn" -eq 0 ]]; then
            read -p "Where do you want to place your X?: " pos
            mosquitto_pub -h 104.196.229.154 -t inTopic -m "7${pos}"
        fi
    elif [[ "$mode" = "2"]]; then
        if [[ "$turn" -eq 0 ]]; then
            read -p "Where do you want to place your X?: " pos
            mosquitto_pub -h 104.196.229.154 -t inTopic -m "7${pos}"
            turn=1
        else
            read -p "Where do you want to place your O?: " pos
            mosquitto_pub -h 104.196.229.154 -t inTopic -m "8${pos}"
        fi
    else
        if [[ "$turn" -eq 0 ]]; then
           echo "Where do you want to place your X?: "
            mosquitto_pub -h 104.196.229.154 -t inTopic -m "4"
            turn=1
        else
            echo "Where do you want to place your O?: "
            mosquitto_pub -h 104.196.229.154 -t inTopic -m "5"
        fi
    fi
done

