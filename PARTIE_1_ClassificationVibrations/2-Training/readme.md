# Entraînement du Modèle — Classification de Vibrations

## Description

Ce dossier contient le dataset, le notebook d'entraînement et le modèle
exporté pour la classification de vibrations.

## Contenu

| Fichier/Dossier | Description |
|---|---|
| `dataset/faible.csv` | Données de vibrations normales |
| `dataset/forte.csv` | Données de vibrations intenses |
| `dataset/repos.csv` | Données en état de repos |
| `notebooks/training.ipynb` | Notebook Jupyter d'entraînement |
| `models/model.h` | Modèle entraîné exporté en C++ |

## Classes

| Classe | Description |
|---|---|
| `repos` | Machine à l'arrêt — bruit de fond du capteur |
| `faible` | Fonctionnement normal — vibrations régulières |
| `forte` | Régime intense — vibrations fortes et chaotiques |

## Résultats du modèle

| Classe | Précision |
|---|---|
| Repos (OFF) | 100% |
| Faible (NORMAL) | 96.7% |
| Forte (HEAVY) | 100% |
| **Global** | **~99%** |

## Lancer le notebook
```bash
jupyter notebook notebooks/training.ipynb
```