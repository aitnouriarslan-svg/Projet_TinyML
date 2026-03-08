# Documentation Technique

## 1. Câblage de la caméra OV7670 vers l'Arduino Nano 33 BLE Sense

| Broche OV7670 | Broche Arduino Nano 33 BLE |
|---|---|
| 3.3V   | 3.3V |
| GND    | GND  |
| SIOC   | A5   |
| SIOD   | A4   |
| VSYNC  | 8    |
| HREF   | A1   |
| PCLK   | A0   |
| XCLK   | 9    |
| D7     | 4    |
| D6     | 6    |
| D5     | 5    |
| D4     | 3    |
| D3     | 2    |
| D2     | 0    |
| D1     | 1    |
| D0     | 10   |
| RESET  | 3.3V |
| PWDN   | GND  |

> Attention : la caméra OV7670 fonctionne en 3.3V. Ne pas connecter à 5V.

---

## 2. Installation des librairies

### Librairie Arduino_OV767X (pilote caméra)
1. Ouvrir Arduino IDE
2. Menu **Tools** → **Manage Libraries**
3. Chercher `Arduino_OV767X`
4. Cliquer **Install**

### Librairie Edge Impulse (modèle d'inférence)
1. Aller sur le projet Edge Impulse
2. Menu gauche → **Deployment** → **Arduino library**
3. Cliquer **Build** puis télécharger le fichier `.zip`
4. Dans Arduino IDE : **Sketch** → **Include Library** → **Add .ZIP Library**
5. Sélectionner le fichier `.zip` téléchargé

---

## 3. Configuration Node-RED

### Installation
```bash
npm install -g node-red
```

### Installer les nodes nécessaires
1. Lancer Node-RED : `node-red`
2. Ouvrir `http://localhost:1880`
3. Menu hamburger → **Gérer la palette** → **Installer**
4. Installer `node-red-dashboard`
5. Installer `node-red-node-serialport`

### Importer le flow
1. Menu hamburger → **Importer**
2. Sélectionner le fichier `flows.json`
3. Cliquer **Importer** puis **Déployer**

### Configurer le port Serial
1. Double-cliquer sur le node **Arduino Serial**
2. Modifier le port selon votre système :
   - Windows : `COM3`, `COM4`...
   - Linux/Mac : `/dev/ttyUSB0`, `/dev/ttyACM0`...
3. Baud rate : `115200`
4. Cliquer **Done** puis **Déployer**

### Lancer le dashboard
Ouvrir dans le navigateur :
```
http://localhost:1880/ui
