#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json, sys
import sqlite3 

print("Content-Type: application/json")
print("")

# Valeurs par defaut
data = {
    "luminosite" : None,
    "bouton": None,
}

DB_FILE = '/tmp/capteurs.db'

try:
    # Connexion à la base de données
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    # 1. Lecture de la luminosité
    cursor.execute("SELECT valeur FROM capteurs WHERE nom='lumiere'")
    row_lumiere = cursor.fetchone() # Récupère la première ligne correspondante
    if row_lumiere:
        data['luminosite'] = row_lumiere[0] # L'index 0 contient la 'valeur'
        sys.stderr.write("Luminosite lue en BDD : %s\n" % data['luminosite'])
    else:
        sys.stderr.write("Aucune donnee de luminosite en BDD.\n")

    # 2. Lecture du bouton
    cursor.execute("SELECT valeur FROM capteurs WHERE nom='bouton'")
    row_bouton = cursor.fetchone()
    if row_bouton:
        data['bouton'] = row_bouton[0]
        sys.stderr.write("Bouton lu en BDD : %s\n" % data['bouton'])
    else:
        sys.stderr.write("Aucune donnee de bouton en BDD.\n")

    conn.close()

except sqlite3.Error as e:
    # Capture les erreurs liées à SQLite (par ex: la base n'est pas encore créée par la gateway)
    sys.stderr.write("Erreur d'accès à la BDD SQLite : %s\n" % e)

# On renvoie le JSON quoi qu'il arrive (avec les valeurs ou None)
print(json.dumps(data))