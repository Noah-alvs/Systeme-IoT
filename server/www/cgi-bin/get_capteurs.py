#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json, sys

print("Content-Type: application/json")
print("")

# Valeurs par defaut
data = {
    "luminosite" : None,
    "bouton": None,
}

# Lire le fichier ecrit par la gateway
try:
    with open('/tmp/capteur_lumiere.txt', 'r') as f:
        data['luminosite'] = f.read().strip()
    sys.stderr.write("Luminosite lue : %s\n" % data['luminosite'])
except OSError:
    # Fichier absent : ESP32 pas encore connecte ou pas encore publie
    sys.stderr.write("Fichier capteur_lumiere.txt absent\n")
try:
    with open('/tmp/capteur_bouton.txt', 'r') as f:
        data['bouton'] = f.read().strip()
    sys.stderr.write("Bouton lue : %s\n" % data['bouton'])
except OSError:
    sys.stderr.write("Fichier capteur_bouton.txt absent\n")
print(json.dumps(data))