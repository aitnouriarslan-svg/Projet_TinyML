# Génération de Données — Accéléromètre IMU

## Description

Ce dossier contient le code Arduino pour collecter les données brutes
de l'accéléromètre et constituer le dataset d'entraînement.

## Fonctionnement

1. Téléverser `data_generation.ino` sur l'Arduino Nano 33 BLE Sense
2. Ouvrir le moniteur série à **9600 baud**
3. Taper `S` pour démarrer une capture de 120 secondes
4. Copier les données CSV affichées et les sauvegarder dans un fichier `.csv`
5. Répéter pour chaque classe : `repos`, `faible`, `forte`

## Format des données

Les données sont envoyées au format CSV :
```
aX,aY,aZ
0.12,0.03,0.98
0.11,0.02,0.97
...
```

## Librairie requise

- `Arduino_LSM9DS1` : Arduino IDE → Tools → Manage Libraries → chercher `Arduino_LSM9DS1`