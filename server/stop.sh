#!/bin/bash

echo "Arrêt des processus (Gateway et Serveur)..."
# Le pkill va maintenant déclencher ta fonction de nettoyage dans le Python !
pkill -f "gateway.py|server.py"

# On laisse 1 seconde au script Python pour exécuter sa fonction de nettoyage
sleep 1 

echo "Nettoyage des fichiers résiduels..."
rm -f /tmp/s2f_fw
rm -f /tmp/capteurs.db

echo "Système arrêté et nettoyé avec succès."