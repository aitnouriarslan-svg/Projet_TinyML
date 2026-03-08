# Arduino Camera — Classification avec OV7670

## Description

Ce dossier contient le sketch Arduino principal pour la capture d'images
via la caméra OV7670 et l'inférence avec le modèle Edge Impulse.

## Contenu

| Fichier | Description |
|---|---|
| `arduino_camera_classification/arduino_camera_classification.ino` | Sketch principal |

## Fonctionnement

1. La caméra OV7670 capture une image toutes les 2 secondes
2. L'image est redimensionnée en 96x96 pixels
3. Le modèle Edge Impulse effectue la classification
4. Le résultat est envoyé via Serial au format `classe,confiance`
5. Les détections avec une confiance inférieure à 60% sont ignorées
6. La classe `unknown` est ignorée

## Librairies requises

- `Arduino_OV767X` — pilote de la caméra
- `Classification_Composants_Electroniques_inferencing` — modèle Edge Impulse (fichier .zip)

## Installation des librairies

1. `Arduino_OV767X` : Arduino IDE → Tools → Manage Libraries → chercher `Arduino_OV767X`
2. Librairie Edge Impulse : Sketch → Include Library → Add .ZIP Library → sélectionner le `.zip`

## Téléversement

1. Ouvrir `arduino_camera_classification.ino` dans Arduino IDE
2. Sélectionner la carte : **Arduino Nano 33 BLE**
3. Sélectionner le port : **COM3** (ou selon votre système)
4. Cliquer **Upload**

> Note : le téléversement peut prendre 10 à 15 minutes en raison de la taille du modèle embarqué.
```

