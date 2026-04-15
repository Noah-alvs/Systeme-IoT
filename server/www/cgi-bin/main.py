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
<h1>Controle du ESP32</h1>

<hr>
<h2>LED</h2>
<form method="POST" action="cmd.py" onsubmit="injectBlink()">
  <input type="radio" id="on" name="val" value="led_on">
  <label for="on">Allumer</label><br><br>

  <input type="radio" id="off" name="val" value="led_off">
  <label for="off">Eteindre</label><br><br>

  <input type="radio" id="led_auto" name="val" value="led_auto">
  <label for="led_auto">Auto (luminosite)</label><br><br>

  <input type="radio" id="blink" name="val" value="">
  <label for="blink">Clignoter toutes les</label>
  <input type="number" id="ms" value="500" min="100" max="9999" style="width:60px"
         onfocus="document.getElementById('blink').checked=true">
  <label>ms</label><br><br>

  <input type="submit" value="Envoyer">
</form>

<hr>
<h2>Buzzer</h2>
<form method="POST" action="cmd.py" onsubmit="injectBeep()">
  <input type="radio" id="buz_on" name="val" value="buzzer_on">
  <label for="buz_on">Activer</label><br><br>

  <input type="radio" id="buz_off" name="val" value="buzzer_off">
  <label for="buz_off">Desactiver</label><br><br>

  <input type="radio" id="buz_beep" name="val" value="">
  <label for="buz_beep">Biper</label>
  <input type="number" id="beep_n" value="3" min="1" max="20" style="width:40px"
         onfocus="document.getElementById('buz_beep').checked=true">
  <label>fois</label><br><br>

  <input type="submit" value="Envoyer">
</form>

<hr>
<h2>Ecran OLED</h2>
<form method="POST" action="cmd.py" onsubmit="return injectOled()">
  <input type="hidden" name="val" id="val_oled">

  <input type="radio" id="oled_txt" name="val_radio">
  <label for="oled_txt">Afficher :</label>
  <input type="text" id="oled_msg" maxlength="32" style="width:200px" placeholder="texte a afficher"
         onfocus="document.getElementById('oled_txt').checked=true"><br><br>

  <input type="radio" id="oled_clear" name="val_radio">
  <label for="oled_clear">Effacer l'ecran</label><br><br>

  <input type="submit" value="Envoyer">
</form>

<hr>
<h2>Capteurs ESP32</h2>

<p><strong>Luminosite :</strong> <span id="luminosite">--</span></p>

<p><strong>Bouton :</strong> <span id="bouton">--</span></p>

<script>
  // Injecte led_blink:XXX avant envoi
  function injectBlink() {
    var blink = document.getElementById('blink');
    if (blink.checked) {
      blink.value = 'led_blink:' + (document.getElementById('ms').value || 500);
    }
  }

  // Injecte buzzer_beep:N quand on clique sur la radio beep
  function injectBeep() {
    var beep = document.getElementById('buz_beep');
    if (beep.checked) {
      beep.value = 'buzzer_beep:' + (document.getElementById('beep_n').value || 3);
    }
  }

  // Injecte oled:texte avant envoi
  function injectOled() {
    var hidden = document.getElementById('val_oled');
    if (document.getElementById('oled_txt').checked) {
      var texte = document.getElementById('oled_msg').value;
      if (texte === '') { alert('Entrez un texte.'); return false; }
      hidden.value = 'oled:' + texte;       // écrit dans le hidden
    } else if (document.getElementById('oled_clear').checked) {
      hidden.value = 'oled_clear';           // écrit dans le hidden
    }
    return true;
  }

  function refresh() {
    fetch('/cgi-bin/get_capteurs.py')
      .then(function(r) { return r.json(); })
      .then(function(data) {
        document.getElementById('luminosite').textContent = (data.luminosite !== null ? data.luminosite + '%' : '--');
        document.getElementById('bouton').textContent = data.bouton || '--';
      })
      .catch(function() {});
  }

  refresh();
  setInterval(refresh, 2000);
</script>
</body>
"""

print (html)