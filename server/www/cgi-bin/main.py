#!/usr/bin/env python
# -*- coding: utf-8 -*-

print "Content-Type: text/html"

print ""

html = """
<head>
  <meta charset="utf-8">
  <title>Peri Web Server</title>
</head>
<body>
<h1>Contrôle de la LED du ESP32</h1>
<form method="POST" action="led.py">
  <input type="radio" id="on" name="val" value="led_on">
  <label for="on">Allumer</label><br><br>

  <input type="radio" id="off" name="val" value="led_off">
  <label for="off">Éteindre</label><br><br>

  <input type="submit" value="Envoyer">
</form>
</body>
"""

print html
