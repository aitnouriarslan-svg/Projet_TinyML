# Node-RED — Comptage de Composants Électroniques

## Description

Ce flow Node-RED reçoit les détections de composants électroniques envoyées par l'Arduino via le port Serial, et affiche les compteurs en temps réel sur un dashboard.

## Prérequis

- Node.js v18 ou supérieur
- Node-RED v3 ou supérieur
- Node : `node-red-dashboard`
- Node : `node-red-node-serialport`

## Installation

### 1. Installer Node-RED
```bash
npm install -g node-red
```

### 2. Installer les nodes nécessaires
Dans Node-RED → Menu ☰ → Gérer la palette → Installer :
- `node-red-dashboard`
- `node-red-node-serialport`

### 3. Importer le flow
1. Ouvrir Node-RED : `http://localhost:1880`
2. Menu ☰ → Importer
3. Sélectionner le fichier `flows.json`
4. Cliquer **Importer** puis **Déployer**

## Configuration du port Serial

1. Double-cliquer sur le node **"Arduino Serial"**
2. Modifier le port selon votre système :
   - Windows : `COM3`, `COM4`...
   - Linux/Mac : `/dev/ttyUSB0`, `/dev/ttyACM0`...
3. Baud rate : **115200**
4. Cliquer Done → Déployer

## Accès au dashboard

Ouvrir dans le navigateur :
```
http://localhost:1880/ui
```

## Format des données reçues

Le flow attend des données Serial au format :
```
classe,confiance
```
Exemple : `capacitor,0.87`

Les classes reconnues sont : `capacitor`, `diode`, `resistor`, `transistor`.
La classe `unknown` est automatiquement ignorée.

## Fonctionnalités du dashboard

- 4 jauges : une par classe, de 0 à 50 (valeur modifiable dans le flow)
- Bouton Reset : remet tous les compteurs à zéro