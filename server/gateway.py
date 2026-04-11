#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Ce fichier lit les commandes depuis la FIFO /tmp/s2f_fw et les publie vers le broker MQTT local. 
"""

import paho.mqtt.client as mqtt
import sys, os

FIFO_PATH  = '/tmp/s2f_fw'
BROKER_IP  = 'localhost'   # adresse du broker Mosquitto.
BROKER_PORT = 1883

# topic MQTT que l'ESP32 va recevoir.
TOPIC_LED      = 'esp32/led'  
TOPIC_BUZZER = 'esp32/buzzer'
TOPIC_OLED   = 'esp32/oled'


client = mqtt.Client() # Création d'une instance client.

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        sys.stderr.write("Gateway connectee au broker MQTT\n")
    else:
        sys.stderr.write("Erreur connexion broker : rc=%d\n" % rc)

client.on_connect = on_connect
client.connect(BROKER_IP, BROKER_PORT, 60)
client.loop_start()  # thread interne non bloquant

sys.stderr.write("Gateway en attente sur %s...\n" % FIFO_PATH)

fifo_s2f = open(FIFO_PATH,'r') #Ouverture de la fifo s2f en lecture, afin de récupérer les infos écritent pas le serveur HTTP.

while True:
    ligne = fifo_s2f.readline() #Récupération des données.

    commande = ligne.strip() #Copie de la chaîne, dans laquelle les caractèes d'espacement sont retirés en début et fin de chaîne.
    if not commande:
        # EOF : cmd.py a ferme la FIFO, on reouvre
        fifo_s2f.close()
        fifo_s2f = open(FIFO_PATH, 'r')
        continue

    # Choisir le topic selon le prefixe de la commande
    if commande.startswith("led"):
        topic = TOPIC_LED
    elif commande.startswith("buzzer"):
        topic = TOPIC_BUZZER
    elif commande.startswith("oled"):
        topic = TOPIC_OLED
    else:
        sys.stderr.write("Commande inconnue : %s\n" % commande)
        continue
 
    client.publish(topic, commande)
    sys.stderr.write("Publie sur %s : %s\n" % (topic, commande))