#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Gateway : lit les commandes depuis la FIFO s2f et les publie vers le broker MQTT.
Subscribe aux topics capteurs et stocke les valeurs dans des fichiers /tmp/.
"""

import paho.mqtt.client as mqtt
import sys, os, time, sqlite3

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

# Chemin de la base de données
DB_FILE = '/tmp/capteurs.db'

# Fonction pour initialiser la base de données
def init_db():
    # Se connecte à la base (la crée si elle n'existe pas)
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    # Création d'une table avec le nom du capteur comme clé primaire (pour écraser la valeur précédente)
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS capteurs (
            nom TEXT PRIMARY KEY,
            valeur TEXT
        )
    ''')
    conn.commit()
    conn.close()
    sys.stderr.write("Base de donnees SQLite initialisee.\n")

init_db() # Appel de l'initialisation au démarrage

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
    
    # Ouverture de la connexion à la BDD à chaque message reçu
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    # Ecrire dans la bdd
    if msg.topic == TOPIC_LUMIERE:
        cursor.execute("INSERT OR REPLACE INTO capteurs (nom, valeur) VALUES (?, ?)", ('lumiere', valeur))
        sys.stderr.write("BDD : lumiere mise a jour a %s\n" % valeur)
    elif msg.topic == TOPIC_BOUTON:
        cursor.execute("INSERT OR REPLACE INTO capteurs (nom, valeur) VALUES (?, ?)", ('bouton', valeur))
        sys.stderr.write("BDD : bouton mis a jour a %s\n" % valeur)
    conn.commit()
    conn.close()

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