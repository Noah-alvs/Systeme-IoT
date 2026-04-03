#!/bin/bash

# Cleanup : tuer les anciens processus en une seule commande
echo "Nettoyage des anciens processus..."
pkill -f "gateway.py|server.py"

# Creer la FIFO si elle n'existe pas encore
if [ ! -p /tmp/s2f_fw ]; then
    mkfifo /tmp/s2f_fw
    echo "FIFO /tmp/s2f_fw creee"
fi

echo "Lancement de la Gateway et du Serveur HTTP..."

# Lancer la gateway dans un nouveau terminal
gnome-terminal --title="Gateway MQTT" -- bash -c "python3 $(pwd)/gateway.py; exec bash"

sleep 1

# Lancer le serveur HTTP dans un nouveau terminal
gnome-terminal --title="Serveur HTTP" -- bash -c "cd $(pwd)/www && python3 ../server.py; exec bash"

echo ""
echo "============================================"
echo "  Systeme en cours d'execution."
echo "  Pour tout stopper :"
echo ""
echo "  pkill -f \"gateway.py|../server.py\""
echo "============================================"