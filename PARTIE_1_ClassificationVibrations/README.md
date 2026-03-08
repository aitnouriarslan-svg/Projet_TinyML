# Classification de Vibrations par TinyML (Arduino Nano 33 BLE Sense)

Ce projet implémente un système de maintenance prédictive intelligent. Il permet à un microcontrôleur Arduino de classifier en temps réel les vibrations d'une machine pour déterminer son état (Repos, Faible/Normal, Forte/Anormale).

## Description du Projet

Le système utilise un accéléromètre pour capter les mouvements et un réseau de neurones embarqué pour l'analyse. Le processus est divisé en trois étapes principales : la génération de données, l'entraînement du modèle et l'inférence (détection) en temps réel.

## Structure du Répertoire
```
PARTIE_1_ClassificationVibrations/
├── 1-DataGeneration_Arduino/
│   └── data_generation/
│       └── data_generation.ino      # Code pour capturer les données brutes de l'accéléromètre
├── 2-Training/
│   ├── dataset/                     # Données CSV utilisées pour l'entraînement
│   │   ├── faible.csv               # Vibrations normales
│   │   ├── forte.csv                # Vibrations intenses (Heavy)
│   │   └── repos.csv                # État inactif (Off)
│   ├── models/
│   │   └── model.h                  # Le modèle entraîné exporté (fichier C++)
│   └── notebooks/
│       └── training.ipynb           # Notebook d'entraînement
├── 3-Inference_Arduino/
│   └── inference_vibrations/
│       ├── inference_vibrations.ino # Programme principal de détection
│       └── model.h                  # Copie du modèle utilisé par le programme
└── doc/
    └── documentation.md             # Documentation technique détaillée
```

## Guide d'Utilisation

### 1. Génération de Données
Le dossier `1-DataGeneration_Arduino` contient le code permettant de lire les valeurs brutes de l'accéléromètre.
- Ouvrir `data_generation.ino` dans Arduino IDE
- Téléverser sur la carte et ouvrir le moniteur série à **9600 baud**
- Taper `S` pour démarrer une capture de 120 secondes
- Répéter pour chaque classe : `repos`, `faible`, `forte`

### 2. Entraînement (Dossier 2-Training)
Ce dossier contient les archives du Machine Learning.
- Le sous-dossier `dataset` contient les fichiers CSV bruts pour les trois classes :
  - `repos.csv` correspond à la classe OFF
  - `faible.csv` correspond à la classe NORMAL
  - `forte.csv` correspond à la classe HEAVY
- Le modèle résultant est stocké dans `models/model.h`

### 3. Inférence et Détection (Dossier 3-Inference_Arduino)
C'est le code final à utiliser sur la machine.
- Ouvrir `inference_vibrations/inference_vibrations.ino` dans Arduino IDE
- S'assurer que `model.h` est bien présent dans le même dossier
- Téléverser le code sur l'Arduino Nano 33 BLE Sense
- Ouvrir le moniteur série à **115200 baud** pour voir la classification en direct

## Matériel Requis

| Composant | Rôle |
|---|---|
| Arduino Nano 33 BLE Sense | Microcontrôleur + IMU intégré |
| Câble Micro-USB | Communication Serial avec le PC |

## Détails Techniques

| Paramètre | Valeur |
|---|---|
| Fenêtre d'analyse | 2000 ms |
| Fréquence d'échantillonnage | 50 Hz |
| Classes | Repos, Faible, Forte |
| Méthode | Analyse Spectrale (FFT) + Réseau de Neurones Dense |

## Performances du Modèle

| Classe | Précision |
|---|---|
| Repos (OFF) | 100% |
| Faible (NORMAL) | 96.7% |
| Forte (HEAVY) | 100% |
| **Global** | **~99%** |

## Documentation complète

Voir `doc/Rapport.pdf` pour la méthodologie complète, les résultats et les captures d'écran.