<div class="titlepage">

<p class="title">JUMA PA-100D<br>Bedienungsanleitung</p>

<p class="sub">Firmware v5.01 (DL4JC) · Oktober 2026</p>

<p class="based">Auf Grundlage der Bedienungsanleitung zur Firmware v4.00a von Adrian Ryan, 5B4AIY
(Revision 15-DEC-2021). Ursprüngliche Firmware von Juha Niinikoski, OH2NLT, und Matti Hohtola,
OH7SV.</p>

<p class="note"><strong>Keine offizielle JUMA-Version.</strong> Die Nutzung erfolgt auf eigene
Verantwortung. Teste jede neue Firmware zuerst mit Dummy-Load und kleiner Leistung. Du kannst
jederzeit zur originalen v4.01a zurückkehren, ohne die Kalibrierung zu verlieren (siehe Kapitel 10).</p>

</div>

<div class="toc-page"><p class="toc-title">Inhalt</p>

[[TOC]]

</div>

# 1 Einführung

Die PA-100D ist eine 100-W-Allband-Endstufe (160–10 m). Sie wurde als Partner des JUMA TRX-2
entwickelt, lässt sich aber mit fast jedem QRP-Transceiver betreiben (Elecraft KX2/KX3, Yaesu
FT-817/818, Xiegu, Icom IC-705 und andere). Diese Anleitung beschreibt Bedienung und Einrichtung mit
der **Firmware v5.01**. Sie läuft auf der JUMA PA-100D und auf dem RS-928-Clone.

## 1.1 Versionsnummern

Manche Händler führen die Original-Firmware **v4.01a** als **„4.1a“**. Das ist die Original-Firmware,
keine neuere Version. Diese Firmware baut auf v4.01a Build 3 (5B4AIY) auf und beginnt mit **v5.00**,
damit beides nicht verwechselt wird. Die Version steht auf dem Startbildschirm und auf der Seite
*About* der User-Config.

## 1.2 Was ist neu gegenüber v4.01a

**Schutz**

- Der gesamte Sendeschutz läuft im 1-ms-Interrupt: HF aus 2 ms nach dem Loslassen von KEY,
  SWR-Abschaltung, Überstrom, falsches Filter, aktive Alarme und ein Watchdog der Hauptschleife
  (2 s). In v4.01a setzte der Schutz aus, sobald die Hauptschleife wartete (gehaltene Taste,
  Speichern-Abfrage, Serial-Test).
- Die Filterrelais schalten nie unter Last. Bei einem Bandwechsel geht zuerst die HF aus, die Relais
  schalten 20 ms danach, und TX bleibt weitere 20 ms gesperrt, bis sie sich gesetzt haben.
- Die Eingangsfrequenz wird jede Millisekunde mit dem gewählten Tiefpass verglichen. Liegt sie 2 ms
  darüber → HF aus, bis das Band stimmt. Das behebt den **O/C-Alarm** von v4.01a beim ersten Senden
  nach einem Bandwechsel im F-Sense-Modus.
- Die Gain-Relais (Abschwächer) schalten nicht mehr während des Sendens.
- Die HF wird im User-Config-Menü, bei einem Prozessorfehler und vor dem Ausschalten abgeschaltet.

**Bandwahl**

- F-Sense arbeitet zuverlässig mit Zweiton, Rauschen und SSB und wählt nie ein Filter unterhalb der
  Sendefrequenz (Kapitel 6.2).
- Neuer Schalter **F-Sense QSK** (Kapitel 4.17).
- Neue Bandwahl-Modi **Xiegu** (Bandspannungen, Kapitel 8.4) und **HR50** (serielles
  Hardrock-50-Protokoll, Kapitel 8.5).
- KX2/KX3: Frequenzen über 30 MHz landen nicht mehr auf einem tiefen Band; unvollständige oder
  verstümmelte `FA`-Meldungen werden ignoriert.

**Bedienung**

- **OPER halten** speichert die Einstellungen ohne Ausschalten (Kapitel 5).
- Neue User-Config-Seite **About** mit Version und Credits; der Startbildschirm zeigt nur noch
  Modell und Firmware-Version.
- Service-Menü **Beep Tone** für den Summer des RS-928 (Kapitel 7.6).
- Empfangs-Überlauf bei hohen Baudraten behoben; die Statusmeldung wird in einem Stück gesendet.

Kalibrierung und Konfiguration im EEPROM bleiben beim Aufspielen dieser Firmware erhalten.

# 2 Bedienelemente

Die PA-100D hat acht Tasten: **PWR**, **UP**, **DOWN**, **DISPLAY/CONFIG**, **OPER**, **AUTO**,
**BAND+** und **BAND−**. „Kurz drücken“ heißt kürzer als ca. 0,5 s, „halten“ heißt ca. 0,7 s oder
länger. Die Kurzreferenz in Kapitel 11 fasst alle Funktionen zusammen.

## 2.1 PWR

- **Einschalten:** PWR kurz drücken. Der Startbildschirm erscheint (wenn *Splash Screen* an ist):

  <pre class="lcd">  JUMA PA-100D
Firmware   v5.01</pre>

  Die Endstufe startet immer in **Standby**.
- **Ausschalten:** PWR halten. Wurde seit dem letzten Speichern ein Gain, das Band (Manual) oder
  Auto/Manual geändert, erscheint zuerst die Speichern-Abfrage (Kapitel 5). Die HF wird vor den
  Lüftern abgeschaltet.
- **Alarm quittieren:** Kurz drücken löscht den dringendsten aktiven Alarm. Lässt sich ein Alarm nicht
  löschen, PWR für eine Notabschaltung halten; es wird nichts gespeichert.
- **Vorherige Seite:** In der normalen Anzeige springt kurz drücken zur vorherigen Messseite zurück
  (z. B. schnell zwischen Leistung und SWR). In User-Config und Service-Menü eine Seite zurück;
  gehalten blättert es automatisch rückwärts.
- **Nein** in jeder Speichern-Abfrage.

## 2.2 UP / DOWN

In der normalen Anzeige ändern UP/DOWN den **Gain** des aktuellen Bandes (G1–G4). G1 ist der
kleinste Gain, jede Stufe ca. 2 dB. Tatsächlich ist der „Gain“ ein Eingangsabschwächer (G1 = −6 dB …
G4 = 0 dB). Jedes der neun Bänder hat seinen eigenen Gain.

Eine Gain-Änderung während des Sendens wird **nach** der Aussendung übernommen, damit die
Abschwächer-Relais nie unter Steuerleistung schalten.

In den Menüs ändern UP/DOWN den Wert der aktuellen Seite. Gehalten wiederholen sie; die
Geschwindigkeit hängt vom Parameter ab.

## 2.3 DISPLAY/CONFIG

- **Kurz drücken** in der normalen Anzeige: nächste Messseite – Ausgangsleistung, SWR,
  Versorgungsspannung, Strom, Kühlkörpertemperatur, im F-Sense-Modus zusätzlich eine Seite mit der
  Eingangsfrequenz (1 kHz Auflösung) und der Ausgangsleistung.
- **Halten:** öffnet die **User-Config** (Kapitel 4). In der User-Config geht kurz drücken zur
  nächsten Seite; gehalten blättert es automatisch vorwärts.
- **Beim Einschalten gehalten:** RS-232-Loopback-Test. Zeichen von einem Terminal werden zurückgesendet
  und angezeigt. PWR kurz beendet den Test.

## 2.4 OPER

- **Kurz drücken** in der normalen Anzeige: schaltet **Operate / Standby** um. Die Umschaltung
  erfolgt beim **Loslassen**.
- **Halten** (ca. 0,7 s, nicht während des Sendens): **Speichern-Abfrage** für die aktuellen
  Einstellungen, ohne auszuschalten (Kapitel 5).
- In User-Config und Service-Menü: Abfrage **Speichern & Verlassen**.
- **Beim Einschalten gehalten:** startet den Bootloader (Flash-Writer) für ein Firmware-Update
  (Kapitel 10).

In **Standby** geht das Signal direkt durch, ohne Filter und Abschwächer; der Transceiver arbeitet
direkt auf die Antenne. Leistung und SWR werden weiter angezeigt. In **Operate** ist die Endstufe
aktiv.

## 2.5 AUTO

Schaltet zwischen **automatischer** (`A` in der unteren Zeile) und **manueller** (`M`) Bandwahl um.
Im manuellen Betrieb wählen BAND+/BAND− das Band (160 m … 10 m). BAND+ oder BAND− im AUTO-Betrieb
schaltet ebenfalls auf manuell. Im Bandwahl-Modus *Manual* ohne Funktion.

In den seriellen Bandwahl-Modi (Yaesu CAT, KX2/KX3, Juma-TRX2) schickt jeder manuelle Bandwechsel
und jeder Wechsel zwischen A und M eine Frequenzabfrage an den Transceiver; eine gültige Antwort hat
Vorrang vor dem manuellen Band. Das verhindert ein versehentlich falsches Filter.

## 2.6 BAND+ / BAND−

- Nächstes höheres / tieferes Band im manuellen Betrieb; gehalten wiederholt es. Während des Sendens
  nicht möglich.
- **BAND+** ist **Ja** in jeder Speichern-Abfrage.
- **BAND+ beim Einschalten gehalten:** Werkseinstellungen laden (Abfrage: BAND+ = Ja, PWR = Nein).
- **BAND− beim Einschalten gehalten:** Die serielle Zeitüberschreitung im HR50-Modus wird 5 s statt
  200 ms, für Tests mit einem Terminalprogramm (Anzeige: *Serial Time-Out / Set to: 5 Secs*). Gilt
  bis zum nächsten Einschalten. Der KX2/KX3-Modus braucht das nicht mehr: Er funktioniert mit einem
  Terminal bei jeder Tippgeschwindigkeit.

## 2.7 Normale Anzeige

<pre class="lcd">▮▮▮▮▮▯▯  52.7W
G3 OPER A 14MHz</pre>

Obere Zeile: Balkenanzeige und gewählter Messwert. Untere Zeile: Gain (G1–G4), Zustand, A/M und Band.

| Zustand | Bedeutung |
|---|---|
| `STBY` | Standby |
| `OPER` | Operate, Empfang |
| `TX` | Operate, Senden |
| `Err` | KEY aktiv, aber Band unbekannt oder außerhalb – TX gesperrt |
| `SWR`, `O/C`, `TEMP`, `Hi-V`, `Lo-V`, `Batt` | Alarm (blinkt, Kapitel 9) |

Band `?` = Band unbekannt (z. B. keine Antwort auf eine Abfrage), `?!` = außerhalb der Bänder.
Beides sperrt TX.

# 3 Erste Schritte

1. Transceiver an den HF-Eingang, Antenne (oder Dummy-Load) an den Ausgang und die PTT-(KEY-)Leitung
   an die T/R-Buchse anschließen. Stromversorgung nur mit Sicherung.
2. Einschalten, User-Config öffnen (DISPLAY halten) und den passenden **Auto Band Detect**-Modus für
   den Transceiver wählen (Kapitel 4.1). Ohne CAT- oder Banddaten-Verbindung **F-Sense** nehmen.
3. Mit OPER verlassen und speichern (BAND+).
4. Mit Gain G1 und kleiner Steuerleistung (1–2 W) beginnen, Leistung und SWR prüfen, dann steigern.

# 4 User-Config

**DISPLAY/CONFIG** halten, bis ein langer Ton kommt und *User Configuration* erscheint. Taste
loslassen; die zuletzt benutzte Seite wird angezeigt. Nächste Seite: DISPLAY kurz (oder halten zum
Blättern). Vorherige Seite: PWR kurz (oder halten). Wert mit **UP/DOWN** ändern. Ein langer Ton und
eine längere Pause markieren Seite 0. Solange das Menü offen ist, ist TX aus.

Zum Verlassen **OPER** kurz drücken und die Speichern-Abfrage beantworten: **BAND+** speichert,
**PWR** verwirft die Änderungen und stellt die vorherigen Einstellungen wieder her.

Seiten, die im aktuellen Modus nicht gelten, werden übersprungen (siehe Hinweise bei den Seiten).

## 4.1 Auto Band Detect

Werkseinstellung: **F-Sense**

| Modus | Bandquelle | Hinweise |
|---|---|---|
| **Yaesu CAT** | Yaesu-5-Byte-Binär-CAT (z. B. FT-817/818 mit CT-62-Pegelwandler) | Polling wird bei Auswahl auf 2 s gesetzt |
| **KX2/KX3** | ASCII-`FA`-Frequenz (Elecraft, Kenwood, neuere Yaesu, jedes Gerät, das auf `FA;` antwortet) | Polling wird auf 2 s gesetzt; Kapitel 8.3 |
| **HR50** | Die PA beantwortet die seriellen Hardrock-50-Befehle; Band vom Host | Kapitel 8.5 |
| **Juma-TRX2** | Bandmeldungen des JUMA TRX-2 | Polling nicht nötig, wird bei Auswahl abgeschaltet |
| **F-Sense** | Frequenz des HF-Steuersignals | Nur HF und PTT nötig; Kapitel 6.2 |
| **FT817/818** | Bandspannung an der BAND-Buchse (FT-817/818) | |
| **Xiegu** | Bandspannung des Xiegu-ACC-Ports (230-mV-Stufen), auch Brick2/3, SquareSDR | Kapitel 8.4 |
| **Manual** | Nur BAND+/BAND− | **Keinerlei Schutz** – ein falsches Band kann Filter oder Transistoren zerstören. Nur als letzte Möglichkeit. |

Reihenfolge im Menü: Yaesu CAT → KX2/KX3 → HR50 → Juma-TRX2 → F-Sense → FT817/818 → Xiegu → Manual.

## 4.2 Serial Speed

Werkseinstellung: **9600 Baud** · Bereich: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 Baud ·
8 Datenbits, 1 Stoppbit, keine Parität.

## 4.3 Serial Port

Werkseinstellung: **Off** · Off / Remote / Test

- **Remote:** Fernsteuerung und Status (Kapitel 8.6).
- **Test:** serielle Testfunktionen (Kapitel 8.1).

Nur in den Modi F-Sense, FT817/818, Xiegu und Manual sichtbar – in den anderen Modi trägt die
serielle Schnittstelle die Banddaten.

## 4.4 Polling Interval

Werkseinstellung: **2 s** · Bereich: 0 (aus) – 10 s

- Yaesu CAT, KX2/KX3, Juma-TRX2: Der Transceiver wird in diesem Abstand nach seiner Frequenz gefragt.
  Kommt innerhalb eines Intervalls keine Antwort, wird das Band unbekannt (`?`) und TX bis zur
  nächsten gültigen Antwort gesperrt.
- Remote-Modus: Die Endstufe sendet in diesem Abstand eine Statusmeldung (Kapitel 8.6).

Nur sichtbar bei Yaesu CAT, KX2/KX3, Juma-TRX2 oder mit *Serial Port = Remote*. Im HR50-Modus
ausgeblendet (der Host schickt das Band).

## 4.5 LCD Display – Hintergrundbeleuchtung

Werkseinstellung: **300** · Bereich: 50 – 1000

## 4.6 LCD Display – Kontrast

Werkseinstellung: **2000** · Bereich: 0 – 3000

## 4.7 SWR Trip

Werkseinstellung: **3.0** · Bereich: 1.0 – 9.0

Der SWR-Alarm löst nur in **Operate** aus. 1.0 eignet sich, um den Alarm mit einer Dummy-Load zu
testen.

## 4.8 Fan Control

Werkseinstellung: **Normal**

| Einstellung | Lüfter |
|---|---|
| Normal | nur nach Kühlkörpertemperatur |
| Low | immer mindestens langsam; höhere Stufen nach Temperatur |
| Medium | immer mindestens mittel; schnell nach Temperatur |
| High | immer schnell |

## 4.9 Temperature

Werkseinstellung: **Celsius** · Celsius / Fahrenheit. Ein Wechsel der Skala setzt Übertemperaturgrenze
und Lüfter-Einschalttemperatur auf ihre Werkseinstellungen in der neuen Skala.

## 4.10 Overtemp

Werkseinstellung: **70 °C / 158 °F** · Bereich: 50 – 100 °C / 120 – 212 °F

## 4.11 Fan Cut-In Temp

Werkseinstellung: **40 °C / 104 °F** · Bereich: 0 °C (32 °F) bis Übertemperaturgrenze − 20 °C (− 40 °F)

Bei der Einschalttemperatur läuft der Lüfter langsam an, 5 °C (10 °F) darüber mittel, 10 °C (20 °F)
darüber schnell. Er stoppt 2 °C (4 °F) unter der Einschalttemperatur. Ein niedriger Wert (z. B.
0 °C) ist ein schneller Lüftertest.

## 4.12 Band Display

Werkseinstellung: **MHz** · MHz / Metres

## 4.13 Graphical Limits

Werkseinstellung: **Off**

- **Off:** Die Balkenanzeige zeigt die Ausgangsleistung; der Vollausschlag wird im Service-Menü
  eingestellt (40–160 W).
- **On:** Die Balkenanzeige zeigt den gewählten Messwert im Verhältnis zu seiner Grenze: Strom zur
  24-A-Hardwareabschaltung, Temperatur zur Übertemperaturgrenze, SWR von 1:1 bis zur
  Abschaltgrenze, Spannung von Unter- bis Überspannung. Empfohlen mit dBm-Leistungsanzeige.

## 4.14 Graphic Display

Werkseinstellung: **Original** · Original / Large / Small (Skalenmarken).

## 4.15 RF Power Meter

Werkseinstellung: **Watts** · Watts / dBm (1 W = +30 dBm, 10 W = +40 dBm, 100 W = +50 dBm).

## 4.16 Start-Up Display

Werkseinstellung: **O/P Power** · O/P Power / SWR / Voltage / Current / Temperature – die Messseite
nach dem Einschalten.

## 4.17 F-Sense QSK

Werkseinstellung: **Off** · Off / On · Nur im F-Sense-Modus sichtbar.

| | **Off** (Werkseinstellung) | **On** |
|---|---|---|
| TX-Freigabe | erst wenn F-Sense die Frequenz **in dieser Aussendung** gemessen hat – bis dahin läuft das Signal mit der Leistung des Transceivers über den Bypass | sofort, mit dem zuletzt gemessenen Band |
| Erstes Senden nach Wechsel auf ein höheres Band (z. B. 40 → 20 m) | PA verstärkt nie durch ein falsches Filter | nach 2 ms HF aus, bis das Band gemessen ist |
| Erstes Senden nach Wechsel auf ein tieferes Band (z. B. 20 → 80 m) | PA verstärkt nie durch ein falsches Filter | nach 3 ms HF aus, bis das Band gemessen ist; **wenige ms** mit schlecht unterdrückten Oberwellen |
| Verzögerung am Anfang jeder Aussendung | ca. 20–40 ms ohne PA (bei SSB mit leisem Sprechbeginn länger) | keine |
| Geeignet für | SSB, Digimodes, CW ohne Voll-QSK | CW mit Voll-Break-in |

Faustregel: **Off** lassen, außer bei CW mit Voll-Break-in. Mit **On** nach einem Wechsel auf ein
tieferes Band zuerst kurz mit kleiner Leistung tasten.

## 4.18 About

Die letzte Seite. Firmware-Version und Credits; mit UP/DOWN blättern:

| Zeile | |
|---|---|
| `Firmware   v5.01` | Installierte Firmware-Version |
| `OH2NLT  Original` | Juha Niinikoski – ursprüngliche Firmware |
| `OH7SV       JUMA` | Matti Hohtola – JUMA |
| `5B4AIY  to 4.01a` | Adrian Ryan – Erweiterungen und Pflege bis v4.01a |
| `DL4JC  from 4.03` | Änderungen ab v4.03 |

Auf dieser Seite wird nichts gespeichert.

# 5 Einstellungen speichern

Die Gains aller Bänder, das Band, Auto/Manual und alle Seiten der User-Config werden gemeinsam im
EEPROM gespeichert. Es gibt drei Wege; alle zeigen dieselbe Abfrage:

<pre class="lcd"> Save Settings?
PWR:No BAND+:Yes</pre>

| Wo | Aufruf | BAND+ (Ja) | PWR (Nein) |
|---|---|---|---|
| Normale Anzeige | **OPER halten** (ca. 0,7 s, nicht während des Sendens) | speichert | Einstellungen bleiben aktiv, ungespeichert |
| User-Config verlassen | OPER kurz | speichert | stellt die vorherigen Einstellungen wieder her |
| Ausschalten | PWR halten, nur wenn Gain, Band oder Auto/Manual geändert wurden | speichert, dann aus | aus ohne Speichern |

`Saved!` oder `Cancelled!` bestätigt die Wahl.

# 6 Bandwahl und Filterschutz

## 6.1 Schutz beim Bandwechsel

- Die Filterrelais schalten nie unter Last. Wechselt das Band während des Sendens, geht zuerst die HF
  aus; die Relais schalten 20 ms danach, und TX bleibt weitere 20 ms aus.
- Jede Millisekunde wird die Eingangsfrequenz mit dem gewählten Tiefpass verglichen, in jedem
  Bandwahl-Modus. Liegt sie 2 ms darüber, geht die HF aus, bis das Band stimmt.
- F-Sense-Modus, erste 200 ms einer Aussendung: Liegt die Frequenz 3 ms unter dem gewählten Filter
  (schlecht unterdrückte Oberwellen, z. B. 80 m durch das 20-m-Filter), geht die HF aus, bis das Band
  gemessen ist.
- Ein ungültiges oder unbekanntes Band (`?`, `?!`) sperrt TX immer.

## 6.2 F-Sense

Das HF-Steuersignal wird gezählt (1 ms Torzeit), und das Band wird aus 20 Messwerten bestimmt.
Steuerleistung **100 mW – 10 W**. Für eine sichere erste Messung auf einem neuen Band kurz **Tune**
oder CW tasten.

- Ein **höheres** Band wird bei jedem Signal sofort gewählt – die sichere Richtung für die Endstufe.
- Ein **tieferes** Band wird bei einem **sauberen Träger** gewählt (Tune, CW: alle Messwerte innerhalb
  von ca. 3 %).
- Modulierte Signale (Zweiton, Rauschen, SSB) misst der Eingangsformer zu tief (Zweiton ca. 85 % der
  wirklichen Frequenz). Bei einem solchen Signal wird ein tieferes Band nur am Anfang einer Aussendung
  gewählt, und nur wenn selbst der höchste Messwert + 20 % unter dem aktuellen Filter liegt. Das Band
  ist dann das **höchste Amateurfunkband** zwischen höchstem Messwert und + 20 %: z. B. wählt 14 MHz
  Zweiton (gemessen als 11,9 MHz) 20 m, nicht 30 m, und 3,7 MHz SSB wählt 80 m, nicht 40 m. Ist die
  wirkliche Frequenz doch höher, schaltet der Filtertest aus 6.1 die HF sofort ab, und die nächste
  Messung korrigiert das Band.
- Ist das Band in einer Aussendung einmal gemessen, wird es nur noch erhöht, nie abgesenkt –
  SSB-Sprache kann das Filter nicht mitten im Durchgang nach unten schalten.
- Liegt die neue Frequenz nur knapp unter dem aktuellen Filter (z. B. 10,1 MHz Rauschen vom
  20-m-Filter aus), bleibt die Endstufe auf dem höheren Filter; das Band einmal mit Tune setzen.

Die Frequenzseite (DISPLAY, nur F-Sense) zeigt die gezählte Frequenz; bei modulierten Signalen ist
sie zu niedrig.

# 7 Service- und Kalibriermodus

Aus dem ausgeschalteten Zustand **PWR** halten, bis ein Ton kommt und

<pre class="lcd">  Calibration
  Mode  v5.01</pre>

erscheint. PWR loslassen. Seiten: DISPLAY kurz/halten vorwärts, PWR kurz/halten rückwärts. Werte:
UP/DOWN. **OPER** öffnet die Speichern-Abfrage (BAND+ speichert, PWR stellt die vorherige
Kalibrierung wieder her).

Dummy-Load anschließen. In diesem Modus startet die Endstufe in Operate mit dem kleinsten Gain (G1,
5 W Steuerleistung ergeben ca. 40–50 W) – die Gains der Bänder werden beim Verlassen wiederhergestellt.
Getastet werden kann nur auf den Seiten Amperemeter und HF-Leistung, die auch A/M und das gewählte
Filter zeigen. Ein Signal im 20-m-Band wird empfohlen. Vorher im normalen Betrieb AUTO (Filter per
F-Sense) oder MANUAL (10-m-Filter) wählen und speichern. Jeder Alarm beendet den Service-Modus.

| Seite | Einstellung | Werk | Bereich |
|---|---|---|---|
| 0 | Supply (Voltmeter-Faktor) | 5250 | 4750 – 5750 |
| 1 | Amperemeter-Faktor | 2520 | 1500 – 4000 |
| 2 | HF-Leistung Faktor (hohe Leistung) | 1062 | 750 – 1500 |
| 3 | HF-Leistung Offset (kleine Leistung) | 120 | 0 – 150 |
| 4 | Beep Len | 50 ms | 0 (aus) – 100 ms |
| 5 | **Beep Tone** | JUMA | JUMA / RS-928 |
| 6 | Power Averaging | 1 (aus) | 1 – 16 |
| 7 | Overvoltage-Alarm | On | On / Off |
| 8 | Overvoltage Trip | 14,50 V | 14,0 – 15,0 V |
| 9 | Low-Voltage-Alarm | On | On / Off |
| 10 | Low Voltage Trip | 11,00 V | 10,5 – 11,5 V |
| 11 | Pre-Limit Trip | 11,20 V | Low Voltage Trip + 0,1 … + 0,8 V |
| 12 | Full-Scale Power (Balkenanzeige) | 100 W | 40 – 160 W |
| 13 | Frequenzzähler-Faktor | 999985 | 999625 – 1000345 |
| 14 | Splash Screen | On | On / Off |

Die Seiten 8, 10 und 11 werden übersprungen, wenn der zugehörige Alarm aus ist.

## 7.1 Voltmeter

Die Versorgungsspannung direkt am Stromanschluss der Endstufe mit einem genauen 4-stelligen
Multimeter messen und den Faktor so einstellen, dass die Anzeige übereinstimmt (±1 Digit). Die
Anzeige ist hier nicht gemittelt (im Betrieb über 50 Werte), damit sich die stabilste Einstellung
finden lässt.

## 7.2 Amperemeter

Auf ca. 30–80 W CW aussteuern, warten bis die Anzeige ruhig ist, und auf eine Referenz abgleichen
(DC-Stromzange, Shunt oder genaue Netzteilanzeige). Bei voller Leistung ca. ±0,2 A Genauigkeit
(Übergangswiderstände und Erwärmung des Shunts). Im Betrieb ist das Amperemeter eine
Spitzenwertanzeige mit ca. 1 s Haltezeit.

## 7.3 HF-Leistung (hohe Leistung)

Auf 50–100 W CW in eine Dummy-Load aussteuern und auf ein genaues Leistungsmessgerät abgleichen (oder
Richtkoppler und Oszilloskop). SWR-Meter mit Leistungsskala sind meist nicht genau genug. Ohne
Referenz die Werkseinstellung lassen.

## 7.4 HF-Leistung (kleine Leistung)

Bei ca. 4–5 W Ausgangsleistung auf 20 m den Offset einstellen. Zwischen Seite 2 und 3 (PWR kurz) mit
den jeweiligen Steuerleistungen wechseln, bis beide stimmen. Landet man nahe einer Grenze, auf ca.
1060 / 120 zurücksetzen und neu beginnen. Erreichbar sind ca. ±10 %; die kleinste angezeigte Leistung
ist ca. 0,4 W.

## 7.5 Beep Len

0 schaltet den Summer ab; wichtige Bestätigungen (z. B. Menü öffnen) piepen trotzdem.

## 7.6 Beep Tone

| Einstellung | Töne |
|---|---|
| **JUMA** (Standard) | unverändert: 601, 784, 934, 1397 Hz, Alarm 2000 Hz |
| **RS-928** | 2300, 2450, 2600, 2750 Hz, Alarm 2700 Hz |

Der Summer des RS-928 klingt nur zwischen ca. 2300 und 2800 Hz sauber. Eine geänderte Einstellung
wird sofort vorgespielt.

## 7.7 Power Averaging

Mittelt Leistungs- und SWR-Messung über 1–16 Werte (je ca. 4 ms). Der SWR-Schutz im Interrupt
mittelt über mindestens 8 ms und beginnt 20 ms nach Anfang der Aussendung, sodass normales
Relaisschalten keine Fehlauslösung bewirkt. Auf **1** lassen, außer eine nachgeschaltete Endstufe
oder getrennte RX/TX-Antennen erzeugen kurze SWR-Alarme; mehr als 6–10 Werte deuten auf ein anderes
Problem.

## 7.8 Spannungsalarme

- **Overvoltage:** für den Mobilbetrieb; Lichtmaschinenspitzen oder schlechte Verbindungen können ihn
  auslösen.
- **Low Voltage:** Endgrenze, z. B. für Bleiakkus (nicht unter 10,5 V entladen).
- **Pre-Limit:** Vorwarnung oberhalb der Low-Voltage-Grenze. Wird die Low-Voltage-Grenze geändert,
  wandert die Vorwarnung um denselben Betrag mit.

## 7.9 Frequenzzähler

100 mW – 5 W eines CW-Signals auf einer exakten kHz-Frequenz im 10-m-Band einspeisen (z. B.
28,850 MHz). Den Faktor erhöhen, bis die Anzeige gerade ohne Springen ein kHz weniger zeigt, Wert
notieren; verringern, bis sie gerade ein kHz mehr zeigt, Wert notieren. Der Mittelwert (ganzzahlig)
ist der Faktor.

## 7.10 Splash Screen

Off: Der Startbildschirm entfällt, die Endstufe ist fast sofort bereit.

# 8 Serielle Schnittstelle

## 8.1 Serielle Testfunktionen

*Serial Port = Test* (Modi F-Sense, FT817/818, Xiegu, Manual), Terminal 8N1 mit der eingestellten
Geschwindigkeit.

| Taste | Funktion |
|---|---|
| `H`, `?` | Hilfe |
| `A` | ADC-Kanäle (Strom, Spannung, Bandspannung, rücklaufende/vorlaufende Leistung, Temperatur) |
| `B` | Alarmtest (0 = alle aus, 1 O/C, 2 SWR, 3 Temperatur, 4 Überspannung, 5 Pre-Limit, 6 Unterspannung, 7 alle an) |
| `C` | LCD-Balken- und Zeichentest |
| `D` | Zähler der Werksrücksetzungen löschen |
| `E` | Kalibrierung und Einstellungen ausgeben – vor einem Firmware-Update aufbewahren |
| `F` | EEPROM-Dump |
| `G` | F-Sense-Test an/aus: jeder Messsatz mit Bins, kleinstem/größtem Wert, `clean`/`mod`/`far` |
| `I` | Eingangsabschwächer (Gain) jedes Bandes |
| `J` | Abgleich des Temperatursensors (schnelle Anzeige, beliebige Taste beendet) |
| `K` | Summertest (Frequenz und Dauer) |
| `L` / `M` | ASCII / Hex auf das LCD schreiben |
| `Z` | Test des Division-durch-Null-Traps (HF aus, Lüfter an) |

**Temperatursensor:** Der BD139 (Q3) auf dem Kühlkörper ist zugleich Sensor und Bias-Referenz. Die
Endstufe Raumtemperatur annehmen lassen, den Kühlkörper mit einem genauen Thermometer messen und das
Poti auf der Steuerplatine mit `J` abgleichen.

## 8.2 Kabel und Pegel

| Signal (PC, DB-9) | Pin | 3,5-mm-Stecker an der PA-100D |
|---|---|---|
| RX-Daten (von der PA) | 2 | Ring |
| TX-Daten (zur PA) | 3 | Spitze |
| Masse | 5 | Schaft |

Beim **RS-928 sind Spitze und Ring vertauscht**, an der seriellen und an der PTT-Buchse. Der ACC1 des
Elecraft KX3 arbeitet mit 0 V / +12 V statt echter RS-232-Pegel; die PA verträgt das, manche
USB-Adapter nicht.

## 8.3 KX2/KX3 und andere ASCII-Transceiver

Die PA nutzt die `FA`-Frequenzantwort, z. B. `FA00014175000;` (11 Ziffern, Hz – Elecraft/Kenwood) oder
`FA014175000;` (9 Ziffern – Yaesu FT-991, FTDX10) oder `FA14175000;` (8 Ziffern – Yaesu FT-450/950/2000, FTDX1200/3000/5000). Andere Längen und verstümmelte Meldungen werden ignoriert;
eine neue Meldung beginnt immer mit `F`.

Einrichtung mit dem KX3:

1. KX3-HF-Ausgang → HF-Eingang der PA.
2. KX3 **ACC2** → PA **T/R**: Der Tasttransistor liegt am Ring des 2,5-mm-Steckers und geht an die
   Spitze des 3,5-mm-Steckers; Schaft an Schaft. **Die Spitze des 2,5-mm-Steckers nicht anschließen.**
3. KX3 **ACC1** → PA **RS-232** mit einem Kabel, bei dem Spitze und Ring gekreuzt sind (oder die
   internen Jumper der PA auf Update-Stellung setzen und ein gerades Kabel nehmen).
4. PA: *Auto Band Detect = KX2/KX3*, *Serial Speed* wie am KX3 (KX3-Standard 4800).
5. Entweder KX3-Menü **AUTOINF = ANT CTRL** (der KX3 sendet `FA` bei jeder Änderung; Polling kann aus
   sein) oder Polling an lassen (2 s; 1 s reagiert schneller).

Mit Polling aus zuerst den KX3 und dann die PA einschalten: Die PA sendet beim Einschalten einmal
`FA;` (der KX3 sendet beim Einschalten von sich aus nichts). Auch jeder manuelle Bandwechsel und jeder
A/M-Wechsel sendet eine Abfrage.

KX2: genauso, über die TRRS-ACC-Buchse (Tastleitung und serielle Daten; z. B. Elecraft-Adapter
KX2ACBL).

Test ohne Funkgerät: Terminal anschließen und z. B. `FA007000000;` tippen (9 Ziffern) – das
40-m-Filter wird gewählt.

## 8.4 Xiegu-Bandspannungen

Eingang wie bei der FT-817-Bandspannung. Schaltschwellen mittig zwischen den Stufen, Toleranz ±115 mV:

| Band | 160 m | 80 m | 60 m* | 40 m | 30 m | 20 m | 17 m | 15 m | 12 m | 10 m |
|---|---|---|---|---|---|---|---|---|---|---|
| Spannung | 0,23 V | 0,46 V | 0,69 V | 0,92 V | 1,15 V | 1,38 V | 1,61 V | 1,84 V | 2,07 V | 2,30 V |

\* 60 m nutzt das 40-m-Filter. Unter 115 mV = außerhalb, über 2,415 V = unbekannt; beides sperrt TX.
Brick2/3 und SquareSDR liefern dieselben XPA125-Pegel.

FT-817/818-Bandspannungen (Modus FT817/818): 0,33 V 160 m, 0,67 V 80 m, 1,00 V 40 m, 1,33 V 30 m,
1,67 V 20 m, 2,00 V 17 m, 2,33 V 15 m, 2,67 V 12 m, 3,00 V 10 m.

## 8.5 HR50-Modus

*Auto Band Detect = HR50*: Die PA verhält sich am seriellen Port wie eine HobbyPCB Hardrock-50.
Programme und Transceiver mit HR50-Unterstützung können das Band wählen, zwischen Operate und Standby
umschalten und den Status lesen. Der Port spricht dann nur HR50: Fernsteuerung, Serial-Test und
Polling sind aus. *Serial Speed* auf den Host einstellen (der USB-Port der HR50 steht ab Werk auf
19200).

| Befehl | Funktion |
|---|---|
| `FAxxxxxxxxxxx;` | Frequenz in Hz, wählt das Band. Kenwood-`IF…;`-Daten werden genauso ausgewertet. |
| `HRBNn;` / `HRBN;` | Band setzen / lesen: 0 = 6 m, 1 = 10 m, 2 = 12 m, 3 = 15 m, 4 = 17 m, 5 = 20 m, 6 = 30 m, 7 = 40 m, 8 = 60 m, 9 = 80 m, 10 = 160 m, 99 = unbekannt |
| `HRMDn;` / `HRMD;` | 1 = PTT (Operate), 0 = OFF (Standby). 2 (COR) und 3 (QRP) schalten auf Standby. |
| `HRRX;` | Status, z. B. `RX,PTT,20M,27C,13.8V;` |
| `HRTP;` / `HRVT;` | Temperatur `HRTP27C;` / Versorgungsspannung `HRVT13.8V;` |
| `HRAT;` / `HRKX;` / `HRBR;` | `HRAT0;` (kein ATU) / `HRKX0;` / serielle Geschwindigkeit 0–3 (4800–38400); unter 4800 lautet die Antwort 0, über 38400 (57600, 115200) 3 – das HR50-Protokoll kennt keine höheren Geschwindigkeiten |
| `HRTM…;` | ATU-Durchreichung, Antwort `HRTM;` wie bei einer HR50 ohne ATU |

Befehle enden mit `;`, Groß- oder Kleinschreibung; Antworten enden mit `;\r\n`. SET-Befehle werden
nicht beantwortet. Das Band bleibt erhalten, bis der Host ein neues schickt. 6 m und „unbekannt“
sperren TX. `HRBR`, `HRTP` und `HRKX` lassen sich nur lesen; Geschwindigkeit und Temperaturskala
werden im Menü eingestellt.

## 8.6 Fernsteuerung

*Serial Port = Remote* (Modi F-Sense, FT817/818, Xiegu, Manual). Befehlsformat:
`=<Buchstabe>[Ziffer]`, gefolgt von CR.

| Befehl | Funktion |
|---|---|
| `=A` | Automatische Bandwahl |
| `=Bn` | Manuelles Band, n = 1 (160 m) … 9 (10 m) |
| `=C` | Alarm löschen |
| `=Gn` | Gain n = 1 … 4 (während TX gesendet: nach der Aussendung übernommen) |
| `=O` | Operate |
| `=Pn` | Ausschalten; n = 1 speichert die aktuellen Einstellungen, n = 0 oder ohne Ziffer nicht. Die HF geht zuerst aus. |
| `=R` | Status anfordern |
| `=S` | Standby |

Statusantwort, z. B. `O:A:T:C: 5:1:1.0:14.09: 8.1: 27.2: 26:0: 0`, gefolgt von LF CR. Felder:

| Nr. | Wert | Bedeutung |
|---|---|---|
| 1 | O / S | Operate / Standby |
| 2 | A / M | automatische / manuelle Bandwahl |
| 3 | T / R | Senden / Empfang |
| 4 | C / F | Temperaturskala |
| 5 | nn | Band 1 (160 m) … 9 (10 m), 10 = unbekannt |
| 6 | n | Gain 1 – 4 (G1 = −6 dB … G4 = 0 dB) |
| 7 | n.n | SWR |
| 8 | nn.nn | Versorgungsspannung, V |
| 9 | nn.n | Strom, A |
| 10 | nnn.n | Ausgangsleistung, W |
| 11 | nnn | Temperatur |
| 12 | n | Lüfter 0 = aus, 1 = langsam, 2 = mittel, 3 = schnell |
| 13 | HH | Alarme, hexadezimal: Bit 0 SWR, 1 Überstrom, 2 Temperatur, 3 Überspannung, 4 Pre-Limit, 5 Unterspannung |

Die Antwort wird vollständig formatiert und in einem Stück gesendet.

**Remote-Timeout:** Bei Polling aus fällt ein ferngesteuertes Operate auf Standby zurück, wenn 5 s
lang kein Befehl kommt – mindestens alle paar Sekunden abfragen (1 s empfohlen). Bei Polling an sendet
die Endstufe in jedem Intervall eine Statusmeldung und fällt nie zurück; ein ausgefallener Remote-Host
wird dann nicht erkannt (wie in v4.01a). Am Gerät gewähltes Operate läuft nie ab.

**USB-Seriell-Adapter** zerteilen Zeilen gemäß ihrem Latency-Timer (FTDI: 16 ms). Programme sollten
Zeilen bis LF CR zusammensetzen. Unter Windows lässt sich der Latency-Timer auf 1 ms stellen
(Gerätemanager → COM-Port → Erweitert).

**Vorsicht:** Die Stromzuleitungen immer **absichern**. Fernausschalten trennt das PA-Modul nicht von
der Versorgung; nur die Sicherung schützt bei einem durchlegierten Transistor. Jumper **J4** auf der
Steuerplatine lässt die PA einschalten, sobald Spannung anliegt (für vollständigen Fernbetrieb mit
fernschaltbarem Netzteil – mit großer Vorsicht verwenden). Den Status regelmäßig abfragen.

# 9 Alarme

| Alarm | Anzeige | Ursache | Wirkung |
|---|---|---|---|
| SWR | `SWR` | SWR über der Grenze, nur in Operate | HF aus, Standby |
| Überstrom | `O/C` | Hardwareabschaltung des MAX4373 bei 24 A (gespeichert) | HF sofort aus (auch direkt im 1-ms-Interrupt gelesen), Standby |
| Übertemperatur | `TEMP` | Kühlkörper über der Grenze | HF aus, Standby; Lüfter auf voller Drehzahl |
| Überspannung | `Hi-V` | Versorgung über der Grenze (wenn eingeschaltet) | HF aus, Standby |
| Unterspannung | `Lo-V` | Versorgung unter der Endgrenze (wenn eingeschaltet) | HF aus, Standby |
| Vorwarnung | `Batt` | Versorgung unter der Vorwarngrenze (wenn Low Voltage eingeschaltet ist) | Warnung: kein Standby, aber TX bleibt aus, bis mit PWR quittiert. Einmal pro Einschalten. |

Der Alarm blinkt, und ein lauter Ton erklingt. **PWR** kurz löscht zuerst den dringendsten Alarm,
dann den nächsten. Solange ein Alarm angezeigt wird, sind DISPLAY, OPER, AUTO und BAND± ohne Funktion,
und TX bleibt aus. Lässt sich ein Überstromalarm nicht löschen, sofort ausschalten und den Fehler
suchen. Der Unterspannungsalarm lässt sich erst löschen, wenn die Spannung wieder gestiegen ist.
Akkus möglichst bald laden – ein entladener Bleiakku sulfatiert.

Die Alarme lassen sich mit dem Serial-Test (`B`) prüfen, der SWR-Alarm auch mit *SWR Trip = 1.0* an
einer Dummy-Load.

# 10 Firmware-Update und Rückkehr

Die PA-100D hat den **Ingenia-Bootloader**. Eine Firmware wird über die serielle Schnittstelle mit dem
Ingenia-Loader (Windows) oder mit `tools/juma-flash.py` (Windows, macOS, Linux) geladen. Ausführliche
Anleitung: README im Repository (github.com/jcmerg/juma-pa100d-firmware).

1. Zuerst die serielle Verbindung prüfen: *Serial Speed* 115200, *Serial Port* Test, mehrmals `H` und
   `F` eingeben – die Ausgabe muss fehlerfrei sein. Die Ausgabe von `E` aufbewahren.
2. Ausschalten. **OPER halten und PWR drücken**: Der Bootloader startet.
3. `firmware/Juma PA-100D v5.01.hex` laden – nur den Programmspeicher, **niemals** das Daten-EEPROM oder
   die Konfigurationsregister. Mit `juma-flash.py`:
   `python3 tools/juma-flash.py --port COM3 "firmware/Juma PA-100D v5.01.hex"`
4. Stromversorgung trennen (PWR funktioniert im Bootloader nicht), dann normal einschalten.

**EEPROM:** Die originalen Konfigurations- und Kalibrierblöcke sind unverändert – kein
Checksummenfehler, die Kalibrierung bleibt erhalten. Neue Einstellungen (F-Sense QSK, Beep Tone,
Xiegu/HR50) liegen in einem eigenen Erweiterungsblock.

**Zurück zum Original:** `firmware/Juma PA-100D v4.01a Build 3 (original).hex` genauso laden.
Kalibrierung und alle Original-Einstellungen bleiben erhalten; aus Xiegu wird F-Sense, aus HR50 wird
KX2/KX3, F-Sense QSK wird ignoriert.

**RS-928:** wird ohne Bootloader ausgeliefert. Er muss einmal mit einem PICkit über den Pfostenstecker
J19 programmiert werden (PWR während der Programmierung gedrückt halten), siehe README.

# 11 Kurzreferenz

| Taste | Kurz drücken | Halten |
|---|---|---|
| **PWR** | Einschalten · Alarm löschen · vorherige Seite · **Nein** in Abfragen | Ausschalten (mit Speichern-Abfrage) · aus dem Aus-Zustand: **Service-Modus** · in Menüs: Seiten zurück |
| **UP / DOWN** | Gain (normal) · Wert (Menüs) | Wiederholen |
| **DISPLAY/CONFIG** | Nächste Messseite · nächste Menüseite | **User-Config** · in Menüs: Seiten vor · aus dem Aus-Zustand (mit PWR): RS-232-Loopback-Test |
| **OPER** | Operate/Standby (beim Loslassen) · Speichern & Verlassen in Menüs | **Speichern-Abfrage** (normale Anzeige, nicht bei TX) · aus dem Aus-Zustand (mit PWR): **Bootloader** |
| **AUTO** | Automatische / manuelle Bandwahl | – |
| **BAND+** | Höheres Band · **Ja** in Abfragen | Wiederholen · aus dem Aus-Zustand (mit PWR): Werkseinstellungen |
| **BAND−** | Tieferes Band | Wiederholen · aus dem Aus-Zustand (mit PWR): HR50-Zeitüberschreitung 5 s |

# 12 Einstellungen notieren

**User-Config**

| Einstellung | Werk | Eigene Einstellung |
|---|---|---|
| Auto Band Detect | F-Sense | |
| Serial Speed | 9600 Baud | |
| Serial Port | Off | |
| Polling Interval | 2 s | |
| Backlight | 300 | |
| Contrast | 2000 | |
| SWR Trip | 3.0 | |
| Fan Control | Normal | |
| Temperature | Celsius | |
| Overtemp | 70 °C | |
| Fan Cut-In Temp | 40 °C | |
| Band Display | MHz | |
| Graphical Limits | Off | |
| Graphic Display | Original | |
| RF Power Meter | Watts | |
| Start-Up Display | O/P Power | |
| F-Sense QSK | Off | |
| Bandwahl / Band / Gain | Manual, 10 m, G1 auf allen Bändern | |

**Kalibrierung**

| Einstellung | Werk | Eigene Einstellung |
|---|---|---|
| Voltmeter | 5250 | |
| Amperemeter | 2520 | |
| HF-Leistung Faktor | 1062 | |
| HF-Leistung Offset | 120 | |
| Beep Len | 50 ms | |
| Beep Tone | JUMA | |
| Power Averaging | 1 | |
| Overvoltage / Trip | On / 14,50 V | |
| Low Voltage / Trip | On / 11,00 V | |
| Pre-Limit | 11,20 V | |
| Full-Scale Power | 100 W | |
| Frequenzzähler | 999985 | |
| Splash Screen | On | |

# 13 Danksagung

- Ursprüngliche Firmware: **Juha Niinikoski, OH2NLT**, und **Matti Hohtola, OH7SV** (JUMA)
- Erweiterungen, Pflege und die Anleitung zu v4.00a: **Adrian Ryan, 5B4AIY**
- Änderungen ab v4.03 und diese Anleitung: **DL4JC**
- Ingenia-dsPIC-Bootloader: Ingenia-CAT S.L., angepasst von OH2NLT
