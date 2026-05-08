# Badge3000 Firmware

Firmware ESP32-C6 pour badge DVID (THcon 2026)


- WiFi AP local: `Badge3000`
- Mot de passe WiFi: `badgecontrol` (Modifiable dans board_config.h)
- App web locale: `http://192.168.4.1`

## Structure

```text
firmware-badge-control/
  main.py              # helper build/flash/monitor
  platformio.ini       # config PlatformIO/pioarduino
  build/               # firmwares publies et versionnes
  include/             # headers et board_config.h
  src/                 # firmware Arduino
```


Configuration par défaut dans `include/board_config.h`:

```cpp
static constexpr const char* AP_SSID = "Badge3000";
static constexpr const char* AP_PASSWORD = "badgecontrol";
static constexpr int PIN_I2C_SDA = 6;
static constexpr int PIN_I2C_SCL = 7;
static constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;
static constexpr LedHardware LED_HARDWARE = LedHardware::BuiltinRgb;
static constexpr int PIN_LED_RGB = 11;
```

## Prérequis

Outils nécessaires:

- Python 3.10+
- `python3-venv` si vous utilisez l'installation locale automatique de PlatformIO
- un câble USB data vers le badge

Option recommandée: utiliser le helper Python `main.py`, qui standardise build, flash, monitor et publication des binaires.

Le helper cherche PlatformIO dans cet ordre:

- `--pio /chemin/vers/pio`
- variable d'environnement `PIO`
- executable `pio` dans le `PATH`
- `.tools/pio-venv/bin/pio` dans ce projet
- `python -m platformio`

Si PlatformIO n'est pas disponible, le helper peut l'installer localement dans `.tools/pio-venv` avec `--install-platformio`.

## Configuration initiale

Depuis le dossier `firmware-badge-control/`:

```bash
python3 main.py build --install-platformio
```

Cette commande:

- crée `.tools/pio-venv/` si PlatformIO n'est pas disponible
- installe ou met à jour PlatformIO dans cet environnement local
- compile le firmware
- publie des binaires versionnés dans `build/`

Pour les builds suivants:

```bash
python3 main.py build
```

Pour changer la version fonctionnelle, éditer `VERSION`:

```text
0.1.0
```

`BUILD_NUMBER` est géré automatiquement par `main.py` et ne doit pas être édité manuellement sauf remise à zéro volontaire.

## Commandes rapides

Depuis ce dossier:

```bash
python3 main.py build
python3 main.py flash
python3 main.py monitor
python3 main.py version
```

Si PlatformIO n'est pas installé:

```bash
python3 main.py build --install-platformio
```

Si le port série n'est pas détecté automatiquement:

```bash
python3 main.py flash --port /dev/ttyUSB0
python3 main.py monitor --port /dev/ttyUSB0
```

Lister les ports:

```bash
python3 main.py list
```

Nettoyer le build:

```bash
python3 main.py clean
```

Chaque `build` ou `flash` réussi publie aussi une copie versionnée dans `build/`.

## Commandes PlatformIO directes

Le projet utilise `pioarduino/platform-espressif32`

```bash
pio run
pio run -t upload --upload-port /dev/ttyUSB0
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

## Utilisation

1. Flasher le badge avec `python3 main.py flash`.
2. Se connecter au WiFi `Badge3000`.
3. Mot de passe: `badgecontrol`.
4. Ouvrir `http://192.168.4.1`.
5. Utiliser l'onglet `Couleur` pour la LED RGB.
6. Utiliser l'onglet `Texte` pour l'OLED.

## App web

Onglet `Couleur`:

- roue chromatique native mobile
- intensité LED
- préréglages couleur RGB
- effet fixe
- clignotement
- pulse
- rainbow
- vitesse effet
- off
- test LED

Onglet `Texte`:

- champ texte OLED jusqu'à 180 caractères
- taille 1..8
- hauteur verticale
- alignement gauche/centre/droite
- gras
- inversion OLED
- défilement horizontal
- vitesse de défilement
- effacement
