# TinyML Project — Arduino Nano 33 BLE Sense


Ce projet implémente deux systèmes d'intelligence artificielle embarquée (TinyML) sur Arduino Nano 33 BLE Sense :
- Classification de vibrations : détection en temps réel de l'état d'une machine (repos, normal, intense) via l'IMU intégré et un réseau de neurones TensorFlow Lite.
- Classification de composants électroniques : reconnaissance de composants (condensateur, résistance, diode, transistor) via une caméra OV7670 et un modèle Edge Impulse, avec comptage en temps réel sur Node-RED.


This project implements two embedded AI (TinyML) systems on Arduino Nano 33 BLE Sense:
- Vibration classification**: real-time detection of machine state (idle, normal, intense) using the built-in IMU and a TensorFlow Lite neural network.
- Electronic component classification**: recognition of components (capacitor, resistor, diode, transistor) using an OV7670 camera and an Edge Impulse model, with real-time counting on Node-RED.

---

## Structure
```
TinyML_project/
├── PARTIE_1_ClassificationVibrations/
└── PARTIE_2_ClassificationComposants/
```