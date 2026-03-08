# Inférence — Classification en Temps Réel

## Description

Ce dossier contient le sketch Arduino pour classifier les vibrations
en temps réel grâce au modèle entraîné.

## Contenu

| Fichier | Description |
|---|---|
| `inference_vibrations.ino` | Sketch principal d'inférence |
| `model.h` | Modèle TFLite embarqué (copie du fichier de 2-Training) |

## Utilisation

1. S'assurer que `model.h` est dans le même dossier que `inference_vibrations.ino`
2. Ouvrir `inference_vibrations.ino` dans Arduino IDE
3. Sélectionner la carte : **Arduino Nano 33 BLE Sense**
4. Téléverser le sketch
5. Ouvrir le moniteur série à **115200 baud**
6. Observer la classification en temps réel

## Format de sortie
```
Repos:0.62 Faible:0.36 Forte:0.00    >>> REPOS
Repos:0.26 Faible:0.33 Forte:0.41    >>> ALERTE FORTE !
```

## Librairies requises

- `Arduino_LSM9DS1` : pilote de l'IMU
- `TensorFlowLite` : moteur d'inférence embarqué