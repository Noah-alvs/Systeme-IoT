#!/bin/bash

# Cleanup : tuer les anciens processus en une seule commande (sécurité au lancement)
echo "Nettoyage des anciens processus..."
pkill -f "gateway.py|server.py"

# Verifier que Mosquitto est lance, le demarrer sinon
echo "Verification du broker Mosquitto..."
if systemctl is-active --quiet mosquitto; then
    echo "  Mosquitto deja actif."
else
    echo "  Mosquitto non actif, demarrage..."
    sudo systemctl start mosquitto
    if systemctl is-active --quiet mosquitto; then
        echo "  Mosquitto demarre avec succes."
    else
        echo "  ERREUR : impossible de demarrer Mosquitto. Abandon."
        exit 1
    fi
fi

# Creer la FIFO si elle n'existe pas encore
if [ ! -p /tmp/s2f_fw ]; then
    mkfifo /tmp/s2f_fw
    echo "FIFO /tmp/s2f_fw creee"
fi

echo "Lancement de la Gateway et du Serveur HTTP..."

# Suppression de l'ancienne BDD sans sudo (le -f force sans erreur si le fichier n'existe pas)
rm -f /tmp/ioc.db

# Lancer la gateway dans un nouveau terminal lxterminal (terminal par defaut Raspberry Pi OS)
lxterminal --title="Gateway MQTT" -e "bash -c 'python3 $(pwd)/gateway.py; exec bash'" &

sleep 1

# Lancer le serveur HTTP dans un nouveau terminal
lxterminal --title="Serveur HTTP" -e "bash -c 'cd $(pwd)/www && python3 ../server.py; exec bash'" &

echo ""
echo "============================================"
echo "  Mosquitto : actif"
echo "  Gateway   : lancee"
echo "  Serveur   : lance"
echo ""
echo "  IP de la Raspberry Pi :"
echo "  $(hostname -I | awk '{print $1}')"
echo ""
echo "  Acces site web :"
echo "  http://$(hostname -I | awk '{print $1}'):8000"
echo ""
echo "  Pour tout stopper proprement :"
echo "  ./stop.sh"
echo "============================================"