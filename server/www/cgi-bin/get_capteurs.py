#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Version PC
# import json, sqlite3, sys

# Version Raspberry Pi
import sys
sys.path.insert(0,'/usr/local/lib/python3.13/dist-packages')
import json, sqlite3

# Séparation claire de l'en-tête HTTP pour éviter les erreurs du serveur
print("Content-Type: application/json")
print("")

DB_FILE = '/tmp/capteurs.db'
data = {
    "historique_lumiere": [],
    "labels_temps": [],
    "dernier_bouton": "--"
}

try:
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    # 1. Récupérer l'historique de la luminosité (40 points = 20 secondes)
    cursor.execute("SELECT valeur, date FROM historique WHERE nom='lumiere' ORDER BY id DESC LIMIT 40")
    rows = cursor.fetchall()
    
    for row in reversed(rows):
        try:
            # Sécurité : on s'assure que la valeur est bien un nombre
            valeur_float = float(row[0])
            data["historique_lumiere"].append(valeur_float)
            data["labels_temps"].append(row[1])
        except ValueError:
            pass # Si ce n'est pas un chiffre, on ignore ce point

    # 2. Récupérer le dernier état du bouton dans la table 'etat_actuel'
    cursor.execute("SELECT valeur FROM etat_actuel WHERE nom='bouton'")
    row_b = cursor.fetchone()
    if row_b: 
        data["dernier_bouton"] = row_b[0]

    conn.close()
except Exception as e:
    sys.stderr.write("Erreur BDD: %s\n" % e)

print(json.dumps(data))