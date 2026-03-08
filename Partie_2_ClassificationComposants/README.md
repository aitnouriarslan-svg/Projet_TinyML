# Partie 2 : Classification de Composants Électroniques

## Description

Ce projet implémente un système de reconnaissance et de comptage de composants électroniques en temps réel. Une caméra OV7670 connectée à un Arduino Nano 33 BLE Sense capture les images des composants, un modèle Edge Impulse effectue la classification, et les résultats sont transmis via USB (Serial) à Node-RED pour le comptage et l'affichage sur un dashboard.

---

## Structure du dossier

```
PARTIE_2_ClassificationComposants/
│
├── 1-EdgeImpulse/
│   ├── link_to_edge_impulse.md   # URL publique du projet Edge Impulse
│   └── instructions.md           # Comment reproduire le pipeline d'entraînement
│
├── 2-ArduinoCamera/
│   └── arduino_camera_classification/
│       └── arduino_camera_classification.ino  # Sketch principal Arduino
│
├── 3-NodeRED/
│   ├── flows.json                # Export du flow Node-RED
│   └── README.md                 # Instructions Node-RED
│
└── doc/
    └── Rapport  
    └── documentation       # Documentation complète (librairies... etc.)
```

---

## Matériel requis

| Composant | Rôle |
|---|---|
| Arduino Nano 33 BLE Sense | Microcontrôleur + inférence |
| Caméra OV7670 | Capture des images |
| Câble USB Micro-B | Communication Serial avec le PC |
| PC avec Node-RED | Dashboard de comptage |

---

## Classes détectées

| Classe | Description |
|---|---|
| `capacitor` | Condensateurs électrolytiques et céramiques |
| `diode` | Diodes de redressement |
| `resistor` | Résistances à couche de carbone |
| `transistor` | Transistors BJT |
| `unknown` | Fond neutre / aucun composant reconnu |

---

## Lien Edge Impulse

 https://studio.edgeimpulse.com/public/920188/live

---

## Démarrage rapide

### 1. Arduino
- Installer la librairie Edge Impulse (`.zip`) via Arduino IDE
- Téléverser `arduino_camera_classification.ino`
- Ouvrir le Serial Monitor à **115200 baud**

### 2. Node-RED
```bash
node-red
```
- Ouvrir http://localhost:1880
- Importer `3-NodeRED/flows.json`
- Configurer le bon port COM
- Ouvrir le dashboard : http://localhost:1880/ui

---

## Performances du modèle

| Classe | Précision |
|---|---|
| capacitor | 82.4% |
| diode | 76.5% |
| resistor | 83.3% |
| transistor | 55.0% |
| unknown | 100% |
| **Global** | **76.9%** |

> Note : La faible précision du transistor (55%) s'explique par sa ressemblance visuelle avec les autres composants à boîtier similaire.

---

## Documentation complète

Voir `doc/documentation.md` pour :
- L'installation des librairies
- La configuration Node-RED
- Les limitations connues
