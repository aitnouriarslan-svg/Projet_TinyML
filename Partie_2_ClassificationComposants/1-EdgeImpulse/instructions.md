# Instructions — Projet Edge Impulse

## Reproduction du pipeline d'entraînement

Ce document explique comment le projet Edge Impulse a été configuré, étape par étape.

---

## 1. Création du projet

1. Aller sur [edgeimpulse.com](https://edgeimpulse.com) → créer un compte
2. Cliquer **"Create new project"**
3. Nom : `Classification_Composants_Electroniques`

---

## 2. Constitution du dataset

### Classes utilisées

| Classe | Description | Nombre d'images |
|---|---|---|
| `capacitor` | Condensateurs électrolytiques et céramiques | ~100 |
| `diode` | Diodes de redressement et Zener | ~100 |
| `resistor` | Résistances à couche de carbone | ~100 |
| `transistor` | Transistors BJT (NPN/PNP) | ~100 |
| `unknown` | Fond neutre, table vide, objets non reconnus | ~80 |

### Sources des images
- Dataset Kaggle : electronic components images
- Images renommées au format `classe.N.jpg` pour permettre l'inférence automatique du label

### Upload des images
1. **Data acquisition** → **Upload data**
2. **Upload mode** : Select a folder
3. **Upload into category** : Automatically split between training and testing (80/20)
4. **Label** : Enter label → nom de la classe
5. Répéter pour chaque classe

>  La classe `unknown` est essentielle : elle permet au modèle de répondre "je ne sais pas" quand aucun composant connu n'est présent devant la caméra.

---

## 3. Création de l'Impulse

**Create Impulse** → configurer les 3 blocs :

### Bloc 1 — Input
| Paramètre | Valeur |
|---|---|
| Image width | 96 px |
| Image height | 96 px |
| Resize mode | Squash |

### Bloc 2 — Processing block
| Paramètre | Valeur |
|---|---|
| Type | Image |
| Color depth | Grayscale |

### Bloc 3 — Learning block
| Paramètre | Valeur |
|---|---|
| Type | Transfer Learning (Images) |
| Modèle | MobileNetV1 96×96 0.1 |

→ Cliquer **Save Impulse**

---

## 4. Génération des features

1. Menu gauche → **Image**
2. Cliquer **Save parameters**
3. Cliquer **Generate features**
4. Vérifier le **Feature Explorer** : les clusters doivent être relativement séparés

---

## 5. Entraînement

**Transfer Learning** → paramètres utilisés :

| Paramètre | Valeur |
|---|---|
| Training cycles | 50 |
| Learning rate | 0.0001 |
| Data augmentation | Activé |

→ Cliquer **Start training**

### Résultats obtenus

| Classe | Précision |
|---|---|
| capacitor | 82.4% |
| diode | 76.5% |
| resistor | 83.3% |
| transistor | 55.0% |
| unknown | 100% |
| **Global** | **76.9%** |

> **Note** : La classe `transistor` obtient une précision plus faible (55%) en raison de la ressemblance visuelle avec les condensateurs et les diodes. Une amélioration possible serait d'utiliser des images en couleur (RGB) ou d'augmenter le dataset pour cette classe.

---

## 6. Déploiement

1. Menu gauche → **Deployment**
2. Chercher **Arduino library**
3. Cliquer **Build**
4. Télécharger le fichier `.zip`
5. Dans Arduino IDE : **Sketch** → **Include Library** → **Add .ZIP Library**
