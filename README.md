# 8-Bit Retro Snake in C

Ein minimalistisches, hochgradig speicheroptimiertes **Snake-Spiel** für das Terminal, geschrieben in reinem C. Das Projekt nutzt fortschrittliche Bit-Manipulationen (Bit-Packing) und Bitfelder (`bit-fields`), um den gesamten Spielzustand und die Schlange auf extrem minimalem Raum im Speicher abzubilden.

---

## 🚀 Features & Speicheroptimierung

Das Spiel wurde mit dem Fokus auf extreme Ressourceneffizienz und hardwarenahe Programmierung entwickelt:

* **Bit-Packed Snake Body (`DATA` Union):** Die Bewegungsrichtung jedes Schlangensegments wird mit nur **2 Bits** codiert (00, 01, 10, 11 für die Richtungen). Ein 64-Byte-Array reicht aus, um eine maximale Schlangenlänge auf einem $16 \times 16$ Gitter vollständig zu verwalten.
* **Bitfelder für Spieler-Status (`PLAYER` Union):** Die X- und Y-Koordinaten des Kopfes, des aktuellen Segments und des Apfels belegen jeweils nur **4 Bits** (perfekt für Werte von 0 bis 15 auf dem $16 \times 16$ Spielfeld).
* **Raw-Mode Terminal:** Das Spiel liest Tastatureingaben direkt und ohne Verzögerung (ohne Enter drücken zu müssen) über die `termios`-Konfiguration ein.
* **Flimmerfreie Darstellung:** Durch die Verwendung von ANSI-Escape-Codes (`\033[H`) wird das Spielfeld flimmerfrei im Terminal aktualisiert.

---

## 🕹️ Steuerung

Die Steuerung erfolgt klassisch über die Pfeiltasten deines Keyboards:

* **🖲️ Pfeiltaste Hoch / Runter / Links / Rechts:** Schlange steuern
* **❌ Taste `q`:** Spiel beenden

*Hinweis: Ein direktes Umdrehen in die entgegengesetzte Richtung (z. B. nach Links drücken, während man nach Rechts läuft) wird durch die Logik blockiert.*

---

## ⚙️ Technische Details & Speicherlayout

### Richtungs-Codierung
* `0`: Links
* `1`: Rechts
* `2`: Oben
* `3`: Unten

### Bitfeld-Struktur der Spieler-Union:
```c
struct {
    uint32_t posx_kopf : 4;  // 4 Bits für X-Koordinate Kopf (0-15)
    uint32_t posy_kopf : 4;  // 4 Bits für Y-Koordinate Kopf (0-15)
    uint32_t posx_now  : 4;  // 4 Bits für X-Koordinate aktuelles Segment
    uint32_t posy_now  : 4;  // 4 Bits für Y-Koordinate aktuelles Segment
    uint32_t posx_apple: 4;  // 4 Bits für X-Koordinate Apfel
    uint32_t posy_apple: 4;  // 4 Bits für Y-Koordinate Apfel
    uint32_t player_lenght : 8; // 8 Bits für die Länge
};
