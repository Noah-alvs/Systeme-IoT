#!/usr/bin/env python3
# -*- coding: utf-8 -*-

print ("Content-Type: text/html")

print ("")

html = """
<head>
  <meta charset="utf-8">
  <title>Peri Web Server</title>
</head>
<body>
<h1>Contrôle de la LED du ESP32</h1>
<form method="POST" action="led.py" onsubmit="injectBlink()">
  
  <input type="radio" id="on" name="val" value="led_on">
  <label for="on">Allumer</label><br><br>

  <input type="radio" id="off" name="val" value="led_off">
  <label for="off">Éteindre</label><br><br>

  <input type="radio" id="blink" name="val" value="">
  <label for="blink">Clignoter toutes les</label>
  <input type="number" id="ms" value="500" min="100" max="9999" style="width:60px">
  <label>ms</label><br><br>

  <input type="submit" value="Envoyer">
</form>

<script>
  // Injecte la valeur led_blink:XXX dans le radio blink juste avant l'envoi du formulaire
  function injectBlink() {
    var blink = document.getElementById('blink');
    if (blink.checked) {
      var ms = document.getElementById('ms').value || 500;
      blink.value = 'led_blink:' + ms;
    }
  }
</script>
</body>
"""

print (html)