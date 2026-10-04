# JUMA PA-100D Firmware – v4.02a (DL4JC)

Deutsch | [English](README.md)

Überarbeitete Firmware für die KW-Endstufe **JUMA PA-100D** (dsPIC30F6014A, läuft auch auf dem
[RS-928-Clone](#rs-928-clone)). Grundlage ist die
**v4.01a Build 3** von Adrian Ryan, 5B4AIY. Die ursprüngliche Firmware stammt von Juha Niinikoski,
OH2NLT, und Matti Hohtola, OH7SV.

Schwerpunkt dieser Version ist der **Schutz beim Senden** (Endstufe und Tiefpassfilter). Dazu kommen
mehrere Fehlerbehebungen, die auch v4.01a betreffen, ein **Xiegu**-Bandspannungsmodus, ein
**Hardrock-50**-kompatibler serieller Modus und die Möglichkeit, mit dem aktuellen Microchip-Compiler
**XC16** zu bauen.

> **Keine offizielle JUMA-Version.** Die Nutzung erfolgt auf eigene Verantwortung. Das Aufspielen
> einer Firmware, die nicht von JUMA stammt, kann Garantie und Support des Herstellers erlöschen
> lassen. Teste jeden neuen Build zuerst mit Dummy-Load und kleiner Leistung. Du kannst jederzeit zur originalen v4.01a
> zurückkehren, siehe [Zurück zur Original-Firmware](#zurück-zur-original-firmware).

---

## Inhalt

- [Neuerungen](#neuerungen)
- [Bedienung – neue Einstellungen](#bedienung--neue-einstellungen)
- [EEPROM und Kompatibilität](#eeprom-und-kompatibilität)
- [Flashen mit dem Ingenia-Bootloader](#flashen-mit-dem-ingenia-bootloader)
- [RS-928-Clone](#rs-928-clone)
- [Firmware bauen](#firmware-bauen)
- [Aufbau des Repos](#aufbau-des-repos)
- [Bekannte Einschränkungen](#bekannte-einschränkungen)
- [Urheberrecht und Danksagung](#urheberrecht-und-danksagung)

---

## Neuerungen

### Schutz beim Senden

| Änderung | Wirkung |
|---|---|
| **Schutz läuft im 1-ms-Interrupt** (`tx_guard()` in `timers_pwm.c`) | In v4.01a lag der gesamte Schutz in der Hauptschleife und stand still, sobald diese wartete, z. B. bei gedrückter Taste, bei der Speicherabfrage oder bei Serial-Test-Befehlen. TX blieb dabei eingeschaltet. |
| **KEY losgelassen** | HF aus 2 ms nachdem KEY inaktiv wird, egal wo die Hauptschleife gerade steht. |
| **SWR im Interrupt** | Die ADC-Messungen laufen jetzt im 1-ms-Interrupt. Das SWR wird über *Power Averaging* × 4 ms gemittelt (mindestens 8 ms) und 20 ms nach TX-Beginn geprüft. Das schaltet schnell und sicher ab, ohne Fehlauslösungen. |
| **Watchdog der Hauptschleife** | HF aus, wenn die Hauptschleife 2 s lang nicht gelaufen ist. |
| **User-Config-Menü** | TX_ON wird zwangsweise abgeschaltet. In v4.01a blieb es in dem Zustand, den es beim Betreten des Menüs hatte. |
| **Trap-Handler** | Bei einem Prozessorfehler (Address-, Stack- oder Math-Trap) wird zuerst die HF abgeschaltet und der Lüfter eingeschaltet. |
| **Service-Mode** | Ein Alarm beendet den Service-Mode jetzt wirklich. In v4.01a hing das Gerät in der Schleife. |

### Schutz der Tiefpassfilter beim Bandwechsel

| Änderung | Wirkung |
|---|---|
| **Kein Relais-Umschalten unter Last** | Bei einem Bandwechsel wird zuerst die HF abgeschaltet. Die Filterrelais schalten, wenn die PA-Relais abgefallen sind, und TX bleibt 20 ms gesperrt, bis die Relais eingeschwungen sind. |
| **Frequenz oberhalb des Filters** | Liegt die mit F-Sense gemessene Eingangsfrequenz 2 ms lang über dem gewählten Filter, geht die HF aus, bis das Band korrigiert ist. Das behebt den **O/C-Alarm** aus v4.01a beim ersten Senden nach einem Bandwechsel im F-Sense-Modus, z. B. 40 m → 20 m. Er entstand, weil 20 m durch das 40-m-Filter verstärkt wurde. Wirkt in allen Bandwahl-Modi. |
| **Frequenz unterhalb des Filters** (F-Sense) | In den ersten 200 ms einer Aussendung: 3 ms unter dem gewählten Filter bedeutet HF aus, bis das Band gemessen ist (z. B. 80 m durch das 20-m-Filter, schlechte Oberwellenunterdrückung). |
| **Schalter F-Sense QSK** | Neue Menüseite, siehe [unten](#f-sense-qsk). |

### Bandwahl

| Änderung | Wirkung |
|---|---|
| **Xiegu-Modus** (neu) | Bandspannungen vom Xiegu-ACC-Anschluss (230-mV-Schritte, inklusive 60 m). Siehe [Tabelle](#xiegu-bandspannungen). |
| **HR50-Modus** (neu) | Die PA beantwortet die seriellen Befehle der HobbyPCB Hardrock-50: Band, Oper/Standby und Status von Programmen und Transceivern mit HR50-Unterstützung. Siehe [Hardrock-50-Modus](#hardrock-50-modus). |
| **KX2/KX3 (ASCII)** | Frequenzen über 30 MHz werden vor der 16-Bit-Umrechnung begrenzt. In v4.01a wurden z. B. aus 144 MHz rechnerisch 12,9 MHz, und das **20-m-Filter wurde mit TX-Freigabe** gewählt. |
| **Juma TRX-2** | Ein ungültiges Band vom TRX-2 ergibt jetzt „unbekannt“ (TX gesperrt) statt 10 m. |
| **Menüreihenfolge** | Yaesu CAT → KX2/KX3 → **HR50** → Juma-TRX2 → F-Sense → FT817/818 → **Xiegu** → Manual |

### Serielle Schnittstelle

| Änderung | Wirkung |
|---|---|
| **Empfangs-Überlauf** | Die Empfangsroutine liest jetzt den ganzen UART-Puffer aus, und ein Überlauf wird automatisch zurückgesetzt. In v4.01a konnte der Empfang bei hoher Baudrate (z. B. 115200) nach einem kurzen Stoß dauerhaft ausfallen, während das Senden weiterlief. |
| **Statuszeile am Stück** | Die Statusantwort (`=R` und Polling) wird komplett formatiert und dann in einem Zug gesendet. Vorher konnten Lücken zwischen den Feldern die Zeile zerteilen, und JUMA_CTRL zeigte dann `???`. Das Format ist unverändert. |

### Sonstiges

- Baut mit **MPLAB XC16 v2.10** ohne Compiler-Warnungen. Die Konfigurationsbits sind identisch mit dem Original-HEX. Die C30-Projektdateien sind weiterhin enthalten.
- Linker-Skript: Der Programmspeicher endet **unterhalb des Bootloaders** (0x17D00). Das Build-Skript erzeugt keine HEX-Datei, die Daten im Bootloader-Bereich enthält.
- **EEPROM-Erweiterungsblock** für die neuen Einstellungen, siehe [EEPROM](#eeprom-und-kompatibilität).
- Startbildschirm: `JUMA PA100v4.02a` / `OH2NLT/7SV DL4JC`.

Die vollständige technische Änderungshistorie steht im Kommentarkopf von `juma-pa100.c` (Abschnitt *DL4JC Modifications*).

---

## Bedienung – neue Einstellungen

### F-Sense QSK

User-Config, letzte Seite **„F-Sense QSK“**. Die Seite erscheint nur bei *Auto Band Detect = F-Sense*.

Vergleich mit dem F-Sense-Modus der originalen v4.01a:

| | v4.01a (Original) | F-Sense QSK **Off** (Werkseinstellung) | F-Sense QSK **On** |
|---|---|---|---|
| **TX-Freigabe** | sofort, mit dem zuletzt gemessenen Band | erst wenn F-Sense die Frequenz **in dieser Aussendung** gemessen hat; bis dahin läuft das Signal mit der Leistung des Transceivers über den Bypass | sofort, mit dem zuletzt gemessenen Band |
| **Erstes Senden nach Wechsel auf höheres Band** (z. B. 40 → 20 m) | PA verstärkt durch das 40-m-Filter, bis gemessen ist → **O/C-Alarm** | PA verstärkt nie durch ein falsches Filter | nach 2 ms HF aus, bis das Band gemessen ist |
| **Erstes Senden nach Wechsel auf tieferes Band** (z. B. 20 → 80 m) | PA verstärkt durch das 20-m-Filter, bis gemessen ist → schlecht unterdrückte Oberwellen | PA verstärkt nie durch ein falsches Filter | nach 3 ms HF aus, bis das Band gemessen ist; **wenige ms** mit schlecht unterdrückten Oberwellen |
| **Filterrelais beim Bandwechsel** | schalten unter voller Leistung | schalten ohne HF, danach 20 ms TX-Sperre | schalten ohne HF, danach 20 ms TX-Sperre |
| **Verzögerung am Anfang jeder Aussendung** | keine | ca. 20–40 ms ohne PA (bei SSB mit leisem Sprechbeginn auch länger) | keine |
| **Geeignet für** | – | SSB, Digimodes, CW ohne Voll-QSK | CW mit Voll-QSK |

Faustregel: **Off** lassen, außer bei CW mit Voll-Break-in. Mit **On** nach einem Wechsel auf ein tieferes Band zuerst kurz mit kleiner Leistung tasten.

### Xiegu-Bandspannungen

Eingang wie bei der FT-817-Bandspannung. Schaltschwellen mittig zwischen den Stufen, Toleranz ±115 mV:

| Band | 160 m | 80 m | 60 m* | 40 m | 30 m | 20 m | 17 m | 15 m | 12 m | 10 m |
|---|---|---|---|---|---|---|---|---|---|---|
| Spannung | 0,23 V | 0,46 V | 0,69 V | 0,92 V | 1,15 V | 1,38 V | 1,61 V | 1,84 V | 2,07 V | 2,30 V |

\* 60 m nutzt das 40-m-Filter. Unter 115 mV gilt „Out of Band“, über 2,415 V „unbekannt“. Beides sperrt TX.

Geräte mit Yaesu-Bandspannungen (z. B. Brick2/3) nutzen wie bisher den Modus **FT817/818**.

### Hardrock-50-Modus

*Auto Band Detect = HR50*: Die PA verhält sich am seriellen Port wie eine HobbyPCB Hardrock-50.
Programme und Transceiver mit HR50-Unterstützung können das Band wählen, zwischen Oper und Standby
umschalten und den Status lesen. Der Port spricht dann nur HR50: JUMA-Fernsteuerung, Serial-Test und
Polling sind aus, die Seiten *Serial Port* und *Polling Interval* werden ausgeblendet. *Serial Speed*
auf die Geschwindigkeit des Hosts stellen (der USB-Port der HR50 steht ab Werk auf 19200).

| Befehl | Funktion |
|---|---|
| `FAxxxxxxxxxxx;` | Frequenz in Hz, wählt das Band. Kenwood-`IF…;`-Daten werden genauso ausgewertet. |
| `HRBNn;` / `HRBN;` | Band setzen / lesen: 0 = 6 m, 1 = 10 m, 2 = 12 m, 3 = 15 m, 4 = 17 m, 5 = 20 m, 6 = 30 m, 7 = 40 m, 8 = 60 m, 9 = 80 m, 10 = 160 m, 99 = unbekannt |
| `HRMDn;` / `HRMD;` | 1 = PTT (Oper), 0 = OFF (Standby). 2 (COR) und 3 (QRP) schalten auf Standby. |
| `HRRX;` | Status, z. B. `RX,PTT,20M,27C,13.8V;` |
| `HRTP;` / `HRVT;` | Temperatur `HRTP27C;` / Versorgungsspannung `HRVT13.8V;` |
| `HRAT;` / `HRKX;` / `HRBR;` | `HRAT0;` (kein ATU) / `HRKX0;` / serielle Geschwindigkeit 0–3 (4800–38400) |
| `HRTM…;` | ATU-Durchreichung, Antwort `HRTM;` wie bei einer HR50 ohne ATU |

- Befehle enden mit `;`, Groß- oder Kleinschreibung. Antworten enden mit `;\r\n`. Wie bei der HR50
  werden SET-Befehle nicht beantwortet.
- Das Band bleibt erhalten, bis der Host ein neues schickt; die PA fragt nie nach. 6 m und
  „unbekannt“ sperren TX, 60 m nutzt das 40-m-Filter. Der Filterschutz arbeitet wie in den anderen
  Modi.
- `HRBR`, `HRTP` und `HRKX` lassen sich nur lesen. Geschwindigkeit und Temperaturskala werden im Menü
  eingestellt (die Temperaturskala setzt auch die Alarm- und Lüftergrenzen).

### Fernsteuerung

Unverändert (`=A`, `=Bn`, `=C`, `=Gn`, `=O`, `=Pn`, `=R`, `=S`; Status `O:M:R:C: 5:4:0.0:13.81: 0.0:  0.0: 24:0: 0`).
Zur bekannten Einschränkung mit Polling siehe [Bekannte Einschränkungen](#bekannte-einschränkungen).

---

## EEPROM und Kompatibilität

- Die originalen **Konfigurations- und Kalibrierblöcke sind unverändert**. Beim Laden dieser
  Firmware gibt es **keinen Checksummenfehler**; Kalibrierung und Einstellungen bleiben erhalten.
- Die neuen Einstellungen liegen in einem **Erweiterungsblock an EEPROM-Adresse 0xF100** mit eigener
  Kennung, Versionsnummer und CRC. Fehlt er (erster Start) oder ist er ungültig, bekommen nur die
  neuen Einstellungen ihre Standardwerte.
- Der **Xiegu-Modus** steht im Original-Block als *F-Sense* und nur im Erweiterungsblock als *Xiegu*.
  Der **HR50-Modus** steht im Original-Block als *KX2/KX3*.

### Zurück zur Original-Firmware

Jederzeit möglich, ohne Vorbereitung und ohne Verlust der Kalibrierung:

| Einstellung in dieser Firmware | Was die originale v4.01a daraus macht |
|---|---|
| Kalibrierung, alle Original-Einstellungen | unverändert übernommen |
| Xiegu | F-Sense (Bandwahl über Frequenzmessung, funktioniert mit jedem Transceiver) |
| HR50 | KX2/KX3 (Band aus den `FA`-Frequenzdaten, Polling aus); die `HR…`-Befehle werden ignoriert |
| F-Sense QSK | ignoriert |

Das Original-HEX liegt im Repo: `Juma PA-100D.hex` (v4.01a Build 3).
Hinweis: Der originale F-Sense-Modus hat weiterhin das oben beschriebene O/C-Problem.

---

## Flashen mit dem Ingenia-Bootloader

Die PA-100D hat den **Ingenia-dsPIC-Bootloader** im oberen Flash (0x17D00–0x17FFE). Die Firmware
wird über die serielle Schnittstelle von einem Windows-PC geladen. Grundlage ist das JUMA-Dokument
*„Firmware Updating for the JUMA TRX2 & PA100D“* (5B4AIY).

### 1. Serielles Kabel

| Signal (PC) | DB-9-Pin | 3,5-mm-Stereostecker (PA-100D) |
|---|---|---|
| RX-Daten | 2 | Ring (TX-Daten der PA) |
| TX-Daten | 3 | Spitze (RX-Daten der PA) |
| Masse | 5 | Schaft |

Das TRX-2-Kabel (Spitze ↔ Pin 2, Ring ↔ Pin 3) funktioniert, wenn die internen Jumper der PA auf
*PROGRAM* stehen, oder mit einem Nullmodem-Adapter. USB-Seriell-Adapter mit **FTDI**-Chipsatz
arbeiten zuverlässig; manche andere Adapter schaffen 115200 Baud nicht.

### 2. Zuerst die Schnittstelle prüfen – nicht überspringen

Ist die Verbindung nicht zuverlässig, kann der Bootloader beschädigt werden. Dann hilft nur noch
ein Programmer (siehe [Wiederherstellung](#wiederherstellung-mit-programmer)).

1. An der PA: User-Config → *Serial Speed* **115200**, *Serial Port* **Test**, speichern.
   (Die Seite Serial Port erscheint nur bei F-Sense, FT817, Xiegu und Manual.)
2. Terminalprogramm am PC: 115200 Baud, 8N1.
3. `H` eingeben → Hilfetext, `F` eingeben → EEPROM-Dump. Mehrmals wiederholen; die Ausgaben müssen
   identisch und fehlerfrei sein.
4. **Einstellungen sichern:** `E` eingeben und die Ausgabe aufbewahren (Kalibrierung und
   Konfiguration).

### 3. Ingenia-Loader installieren

1. `ingeniadsPICbootloader.exe` installieren (Ingenia dsPIC bootloader 1.1, liegt den
   JUMA-Firmware-Paketen bei).
2. **Gerätedatei ersetzen:** [`tools/ingenia/ibl_dspiclist.xml`](tools/ingenia/ibl_dspiclist.xml) in
   den Installationsordner kopieren und die vorhandene Datei überschreiben, typischerweise
   `C:\Program Files\Ingenia\ingeniadsPICbootloader\` (64-Bit-Windows: `C:\Program Files (x86)\...`).
   Sie enthält den dsPIC30F6014A mit dem Bootloader-Bereich 0x17D00–0x17FFE. Ohne sie wird das Gerät
   nicht erkannt, oder das Tool kennt den geschützten Bereich nicht.
3. Ab Windows Vista: Symbol → Eigenschaften → Kompatibilität → *Windows XP (Service Pack 3)* und
   *Programm als Administrator ausführen*.

### 4. Firmware flashen

1. PA anschließen und **ausschalten**.
2. Ingenia starten → *OK, my platform is shut down*.
3. COM-Port und Baudrate wählen (115200, bei Problemen niedriger) → *configuration done*.
4. **OPER gedrückt halten, dann PWR drücken.** Der Bootloader prüft OPER direkt beim Einschalten,
   schaltet die Selbsthaltung der Versorgung selbst ein und zeigt an, dass der Flash-Writer läuft.
   Danach beide Tasten loslassen. (Der Hinweis „PWR gedrückt halten“ in der alten TRX-2-Anleitung
   stammt aus der Zeit vor der Selbsthaltung, die der Bootloader seit 23.01.2007 hat, siehe `iBL.s` /
   `mini_lcd-trx2.c`.)
5. Warten, bis *dsPIC6014A detected, firmware version 1.1* erscheint → OK.
6. *open HEX file* → `Juma PA-100D v4.02a Build 4-DL4JC.hex` wählen.
7. Nur **„program flash“** darf angehakt sein. **„write data EEPROM“ und „configure registers“ dürfen
   nicht angehakt sein.** Es darf keine Fehlermeldung erscheinen (siehe unten).
8. *start write* → dauert bei 115200 Baud ca. 10–15 s → *write completed*.
9. Ingenia schließen, **Netzteil abschalten** (die PWR-Taste funktioniert im Bootloader nicht), dann
   normal einschalten.

**„Your hex file contains data in bootloader addresses“**: Diese Datei nicht flashen. Die
Release-Builds dieses Repos prüft das Build-Skript darauf.

**Störungen:** Beim Flashen alle nicht benötigten Programme schließen (Virenscanner,
Netzwerk-Tools). Fehler beim Schreiben kommen fast immer von der seriellen Verbindung: Kabel prüfen,
niedrigere Baudrate versuchen.

### Wiederherstellung mit Programmer

Ist der Bootloader beschädigt, muss `bootloader/PA100_boot_loader.hex` mit einem Programmer
(ICD/PICkit) über den ICD-Anschluss geladen werden. Danach lässt sich die Firmware wieder mit Ingenia
laden. Der Quellcode des Bootloaders (`iBL.s`, von OH2NLT für JUMA angepasst) liegt in
[`bootloader/`](bootloader/).

---

## RS-928-Clone

Diese Firmware läuft auch auf dem **RS-928**, einem Nachbau der PA-100D, dessen Hardware praktisch
dem Original entspricht. Zwei Dinge sind anders:

### Erstinstallation: PICkit über den Header J19

Der RS-928 wird mit eigener Firmware (v1.05q) ausgeliefert, **ohne den Ingenia-Bootloader** und ohne
nutzbare Fernsteuerung. Das serielle Update funktioniert deshalb erst nach einem einmaligen
Programmierschritt:

1. Mit einem **PICkit** (oder einem anderen dsPIC-Programmer) am Header **J19**
   [`bootloader/Bootldr_Juma-PA100_v104.hex`](bootloader/Bootldr_Juma-PA100_v104.hex) in den
   dsPIC30F6014A schreiben. Die Datei ist ein komplettes Chip-Abbild: JUMA-Firmware v1.04,
   Ingenia-Bootloader und Konfigurationsbits. J19 ist die 6-polige Stiftleiste neben dem dsPIC (IC9)
   auf der Rückseite der Frontplatine, siehe das Foto *rs928ampfrontpanel.jpg* im
   [OARC-Wiki](https://wiki.oarc.uk/rs928ampreview). Der PICkit wird direkt auf J19 gesteckt: **der
   Pfeil auf dem PICkit (Pin 1) kommt auf den umrahmten Pin von J19**. Anleitung und Dateien:
   [hermes-lite-Thread](https://groups.google.com/g/hermes-lite/c/breb9kSmeYc/m/xKfDIW6zEQAJ) von
   KD2NFC.

   **Versorgung beim Programmieren – der knifflige Teil:**
   - Die PA über ihr **eigenes Netzteil** versorgen, nicht über den PICkit. In der PICkit-Software das
     Ziel auf **self-powered** stellen (der PICkit darf VDD nicht liefern).
   - **PWR während des gesamten Programmiervorgangs gedrückt halten.** Der Programmer hält den
     Prozessor im Reset, deshalb kann nichts die Selbsthaltung einschalten; Loslassen schaltet die PA
     aus und bricht das Programmieren ab. (Anders als später beim Update mit Ingenia, siehe unten.)
2. Prüfen, ob die PA mit der JUMA-Firmware startet.
3. Danach diese Firmware über die serielle Schnittstelle mit Ingenia laden, genau wie unter
   [Flashen mit dem Ingenia-Bootloader](#flashen-mit-dem-ingenia-bootloader) beschrieben (OPER + PWR).

Die Original-Firmware des Clones wird in Schritt 1 überschrieben. Wer sie eventuell zurückhaben
möchte, liest den Chip vorher mit dem Programmer aus und hebt die Datei auf.

### Buchsen andersherum belegt

Beim RS-928 sind **Spitze und Ring gegenüber der PA-100D vertauscht** – an der **seriellen Buchse und
an der PTT-Buchse**. Der Schaft ist bei beiden Masse.

| Sub-D-Pin (PC) | Signal | PA-100D | RS-928 |
|---|---|---|---|
| 3 | TxD, in die PA | Spitze | **Ring** |
| 2 | RxD, aus der PA | Ring | **Spitze** |
| 5 | Masse | Schaft | Schaft |

An der **PTT-Buchse** liegt beim RS-928 ebenfalls auf dem Ring, was die PA-100D auf der Spitze hat,
und umgekehrt. Das vor dem ersten Senden prüfen, sonst tastet die Endstufe nicht oder dauerhaft.
Ein vertauschtes serielles Kabel führt nur zu Stille (keine Daten, keine Erkennung in Ingenia).

Hintergrund: [RS-928-Test im OARC-Wiki](https://wiki.oarc.uk/rs928ampreview).

---

## Firmware bauen

### Mit MPLAB XC16 (macOS, Linux, Windows)

Voraussetzung: [MPLAB XC16](https://www.microchip.com/xc16) v2.10 (die kostenlose Version reicht;
das Projekt baut wie bisher ohne Optimierung, `-O0`).

```sh
./build-xc16.sh
```

Ergebnis: `Juma PA-100D <VERSION> Build <BUILD>.hex` im Projektordner. Version und Build-Nummer
kommen aus `juma-pa100.h` (`VERSION`, `BUILD_NUMBER`). Das Skript

- kompiliert alle Module mit `-mcpu=30F6014A -Wall`,
- linkt mit `juma-trx2.gld` (bootloader-spezifisch: Code ab 0x100, Programmspeicher unterhalb 0x17D00),
- erzeugt das HEX und **bricht ab, wenn etwas im Bootloader-Bereich landet**.

Anderer XC16-Pfad: `XC16=/pfad/zu/xc16/v2.10 ./build-xc16.sh`.

**macOS mit Apple Silicon:** XC16 ist ein Intel-Programm und läuft unter Rosetta (das Skript ruft es
mit `arch -x86_64` auf). Das Installationsprogramm bricht auf Apple Silicon kommentarlos ab; dann das
innere Installationsprogramm direkt starten:
`arch -x86_64 ".../xc16-v2.10-osx-installer.app/Contents/MacOS/osx-x86_64"` (braucht Admin-Rechte).

### Mit MPLAB C30 (Original-Toolchain)

`Juma PA-100D.mcp` ist das originale MPLAB-8/C30-Projekt. Der Quellcode enthält `#ifdef __XC16__`
nur für die Konfigurationsbits; alles andere ist gemeinsamer Code. C30-Builds sind für diese Version
nicht getestet.

---

## Aufbau des Repos

| Pfad | Inhalt |
|---|---|
| `juma-pa100.c` | Hauptprogramm, Menüs, Bandwahl, Fernsteuerung, Änderungshistorie |
| `timers_pwm.c` | 1-ms-Interrupt: Frequenzzähler, Tasten, `tx_guard()` (TX-, SWR- und Filterschutz) |
| `adc12.c` | ADC-Messungen im Interrupt |
| `uart.c`, `serial_pa100.c`, `serial_test.c` | Serielle Schnittstelle, TRX-2-Protokoll, Testsuite |
| `service.c` | Kalibrier- und Service-Mode |
| `lcd-trx2.c`, `traps.c`, `tmr5delay.c`, `spi1.c`, `DataEEPROM.s` | LCD, Trap-Handler, Wartezeiten, SPI, EEPROM-Zugriff |
| `juma-pa100.h`, `pa100_eeprom.h` | Hardware-Definitionen, EEPROM-Strukturen |
| `juma-trx2.gld` | Linker-Skript für den Ingenia-Bootloader |
| `build-xc16.sh` | Build-Skript für XC16 |
| `Juma PA-100D v4.02a Build *-DL4JC.hex` | Aktueller Build |
| `Juma PA-100D.hex` | Original v4.01a Build 3 (zum Zurückgehen) |
| `tools/ingenia/ibl_dspiclist.xml` | Gerätedatei für den Ingenia-Loader |
| `bootloader/` | Bootloader-Quellcode und HEX; `Bootldr_Juma-PA100_v104.hex` = komplettes Abbild für die Erstinstallation per Programmer (RS-928) |
| `Juma PA-100D.mcp/.mcw/.mcs` | Originales MPLAB-8-Projekt |

---

## Bekannte Einschränkungen

- **F-Sense QSK = On:** Nach einem Wechsel auf ein tieferes Band bleibt ein Fenster von wenigen
  Millisekunden mit schlecht unterdrückten Oberwellen (die Frequenz lässt sich erst messen, wenn HF
  anliegt). Wem das wichtig ist: Einstellung auf **Off** lassen oder nach einem Bandwechsel zuerst
  kurz mit kleiner Leistung tasten.
- **Fernsteuerung mit Polling:** Wie in v4.01a läuft der Remote-Timeout bei eingeschaltetem Polling
  nie ab, weil jede automatische Statusmeldung ihn neu startet. Ein ausgefallener Remote-Host wird
  deshalb nicht erkannt. Das ist bewusst so belassen, damit bestehende Fernsteuer-Programme weiter
  funktionieren.
- **USB-Seriell-Adapter** zerteilen Zeilen gemäß ihrem Latency-Timer (FTDI: 16 ms).
  Fernsteuer-Programme sollten Zeilen bis `\n\r` zusammensetzen. Unter Windows lässt sich der
  *Latency Timer* auf 1 ms stellen (Gerätemanager → COM-Port → Erweitert).

---

## Urheberrecht und Danksagung

- Ursprüngliche Firmware: **Juha Niinikoski, OH2NLT**, und **Matti Hohtola, OH7SV** (JUMA)
- Erweiterungen und Pflege bis v4.01a: **Adrian Ryan, 5B4AIY**
- Änderungen v4.02a: **DL4JC**
- `DataEEPROM.s`, `DataEEPROM.h`: Microchip Technology Inc. (Microchip-Lizenz, siehe Dateikopf)
- Ingenia-dsPIC-Bootloader: Ingenia-CAT S.L., angepasst von OH2NLT

Die Original-Quellen werden von JUMA (jumaradio.com) ohne ausdrückliche Lizenz frei verteilt. Das
Urheberrecht der ursprünglichen Autoren gilt weiter; dieses Repo vergibt keine eigene Lizenz. Die
Änderungen von DL4JC dürfen unter denselben Bedingungen genutzt werden wie die Original-Firmware.
