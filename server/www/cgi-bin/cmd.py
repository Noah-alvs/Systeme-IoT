#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cgi, os, time,sys



s2fName = '/tmp/s2f_fw'


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
print("""
<head>
  <meta charset="utf-8">
  <META HTTP-EQUIV="Refresh" CONTENT="1; URL=/cgi-bin/main.py">
</head>
<body>
<p>Commande envoyee : %s</p>
</body>
""" % val)
