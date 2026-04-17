#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json, sqlite3, sys

print("Content-Type: application/json\n")

DB_FILE = '/tmp/capteurs.db'
data = {
    "historique_lumiere": [],
    "labels_temps": [],
    "dernier_bouton": "--"
}

try:
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    # 1. Récupérer l'historique de la luminosité
    cursor.execute("SELECT valeur, strftime('%H:%M:%S', date) FROM historique WHERE nom='lumiere' ORDER BY id DESC LIMIT 20")
    rows = cursor.fetchall()
    
    for row in reversed(rows):
        data["historique_lumiere"].append(float(row[0]))
        data["labels_temps"].append(row[1])

    # 1. Récupérer l'historique de la luminosité (40 points = 20 secondes)
    cursor.execute("SELECT valeur, date FROM historique WHERE nom='lumiere' ORDER BY id DESC LIMIT 40")
    row_b = cursor.fetchone()
    if row_b: 
        data["dernier_bouton"] = row_b[0]

    conn.close()
except Exception as e:
    sys.stderr.write("Erreur BDD: %s\n" % e)

print(json.dumps(data))