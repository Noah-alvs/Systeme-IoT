#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cgi, os, time,sys



s2fName = '/tmp/s2f_fw'
f2sName = '/tmp/f2s_fw'
# s2f = open(s2fName,'w+')
# f2s = open(f2sName,'r',0)



# s2f.write("w %s\n" % val)
# s2f.flush()
# res = f2s.readline()
# f2s.close()
# s2f.close()


#Récupération des données reçues du formulaire. 
form = cgi.FieldStorage()
val = form.getvalue('val')


#Ecriture des données dans la FIFO
s2f = open(s2fName,'w')
s2f.write("%s\n" % val)
sys.stderr.write("La donnee val=%s mise dans la FIFO\n" % val)
s2f.flush()
s2f.close()



print("Content-Type: text/html; charset=utf-8")
print ("") 


# Générer le code HTML pour la réponse
html = """
<head> 
  <meta charset="utf-8">
  <title>Peri Web Server - LED Control</title>
  <META HTTP-EQUIV="Refresh" CONTENT="1; URL=/cgi-bin/main.py">
</head>
<body>
<h1>Contrôle des LEDs</h1>
<form method="POST" action="led.py">
  <input type="radio" id="on" name="val" value="ON" {0}>
  <label for="on">Allumer</label><br><br>

  <input type="radio" id="off" name="val" value="OFF" {1}>
  <label for="off">Éteindre</label><br><br>

  <input type="submit" value="Envoyer">
</form>
<p>Valeur envoyée : {2}</p>
</body>
""".format(
    'checked' if val == 'ON' else '',
    'checked' if val == 'OFF' else '',
    val if val else 'Aucune valeur envoyée'
)

# Envoyer l'entête HTTP et le code HTML
print("Content-Type: text/html\n")
print(html)
