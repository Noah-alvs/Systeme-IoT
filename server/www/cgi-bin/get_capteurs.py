#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json, sys

print("Content-Type: application/json")
print("")

# Valeurs par defaut
data = {
    "luminosite" : None,
}

# Lire le fichier ecrit par la gateway
try:
    with open('/tmp/capteur_lumiere.txt', 'r') as f:
        data['luminosite'] = f.read().strip()
    sys.stderr.write("Luminosite lue : %s\n" % data['luminosite'])
except OSError:
    # Fichier absent : ESP32 pas encore connecte ou pas encore publie
    sys.stderr.write("Fichier capteur_lumiere.txt absent\n")
 
print(json.dumps(data))