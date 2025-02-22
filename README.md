# 🚗 Vehículo Robótico Autónomo con Redes Neuronales y MQTT  

![Robot Autónomo](https://i.ibb.co/1GpFbxvm/Whats-App-Image-2025-02-13-at-6-58-26-PM-1.jpg)

## 📌 Descripción  
Este proyecto consiste en un **vehículo autónomo** capaz de detectar obstáculos y tomar decisiones en tiempo real utilizando una **red neuronal entrenada en MATLAB**.   
Se integra con **Arduino Mega, ESP8266 y sensores ultrasónicos**, además de una aplicación en **App Inventor** para monitoreo remoto mediante **MQTT**.  

---

## 🎯 Objetivo  
✔️ Identificar obstáculos con sensores ultrasónicos.  
✔️ Interpretar la información mediante una red neuronal entrenada.  
✔️ Emitir comandos a los motores para moverse sin intervención humana.  
✔️ Control y supervisión a través de una aplicación móvil.  

---

## 🔧 Tecnologías Utilizadas  

| Tecnología | Uso |
|------------|--------------------------------|
| Arduino Mega | Control del hardware |
| ESP8266 | Comunicación remota |
| MATLAB | Entrenamiento de red neuronal |
| MQTT | Comunicación en la nube |
| App Inventor | Creación de la interfaz móvil |
| Motores DC y Servos | Movimiento del vehículo |
| Sensores ultrasónicos | Detección de obstáculos |

---

## ⚙️ Principales Funcionalidades  

✅ **Detección de distancias** con sensores ultrasónicos  
✅ **Red neuronal** que decide las acciones del vehículo  
✅ **Control de motores y servo**  
✅ **Comunicación vía MQTT** para monitoreo remoto  
✅ **Interfaz de usuario en App Inventor**  

---

## 🧠 Arquitectura de la Red Neuronal  
```python
nn = NeuralNetwork([2, 3, 4], 'tanh')
# Capa de Entrada: 2 neuronas (distancia y ángulo)
# Capa Oculta: 3 neuronas
# Capa de Salida: 4 neuronas (control de motores)

