#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Gateway : lit les commandes depuis la FIFO s2f et les publie vers le broker MQTT.
Subscribe aux topics capteurs et stocke les valeurs dans des fichiers /tmp/.
"""

import paho.mqtt.client as mqtt
import sys, os, time

FIFO_S2F  = '/tmp/s2f_fw'
BROKER_IP  = 'localhost'   # adresse du broker Mosquitto.
BROKER_PORT = 1883

# topic MQTT que l'ESP32 va recevoir.
TOPIC_LED      = 'esp32/led'  
TOPIC_BUZZER = 'esp32/buzzer'
TOPIC_OLED   = 'esp32/oled'

#topic MQTT que l'ESP32 va envoyer
TOPIC_LUMIERE = 'esp32/lumiere'
TOPIC_BOUTON = 'esp32/bouton'

# Fichiers de stockage, il sera lu par get_capteurs.py
FICHIER_LUMIERE = '/tmp/capteur_lumiere.txt'
FICHIER_BOUTON = '/tmp/capteur_bouton.txt'


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1) # Création d'une instance client.


fd_s2f   = os.open(FIFO_S2F, os.O_RDONLY | os.O_NONBLOCK)
fifo_s2f = os.fdopen(fd_s2f, 'r')


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        sys.stderr.write("Gateway connectee au broker MQTT\n")
        client.subscribe(TOPIC_LUMIERE)
        client.subscribe(TOPIC_BOUTON)
        sys.stderr.write("Subscribe sur : %s et %s\n" % (TOPIC_LUMIERE, TOPIC_BOUTON))
    else:
        sys.stderr.write("Erreur connexion broker : rc=%d\n" % rc)

def on_message(client, userdata, msg):
    valeur = msg.payload.decode()
    sys.stderr.write("Capteur recu [%s] : %s\n" % (msg.topic, valeur))
 
    # Ecrire dans le fichier selon le topic
    if msg.topic == TOPIC_LUMIERE:
        with open(FICHIER_LUMIERE, 'w') as f:
            f.write(valeur)
        sys.stderr.write("Ecrit dans %s : %s\n" % (FICHIER_LUMIERE, valeur))
    if msg.topic == TOPIC_BOUTON:
        with open(FICHIER_BOUTON, 'w') as f:
            f.write(msg.payload.decode())
        sys.stderr.write("Ecrit dans %s : %s\n" % (FICHIER_BOUTON, valeur))

client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER_IP, BROKER_PORT, 60)
client.loop_start()  # thread interne non bloquant

sys.stderr.write("Gateway en attente sur %s...\n" % FIFO_S2F)


while True:
    try:
        ligne = fifo_s2f.readline() #Récupération des données.
    except OSError:
        # FIFO vide temporairement
        time.sleep(0.1)
        continue

    commande = ligne.strip() #Copie de la chaîne, dans laquelle les caractèes d'espacement sont retirés en début et fin de chaîne.
    if not commande:
        # EOF : cmd.py a ferme la FIFO, on reouvre
        fifo_s2f.close()
        fd_s2f   = os.open(FIFO_S2F, os.O_RDONLY | os.O_NONBLOCK)
        fifo_s2f = os.fdopen(fd_s2f, 'r')
        time.sleep(0.1)
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