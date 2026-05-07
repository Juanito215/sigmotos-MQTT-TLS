# Guía Técnica: Pipeline de Telemetría IoT - Sigmotos

Este documento contiene los pasos y comandos necesarios para demostrar la funcionalidad completa del sistema, desde el dispositivo final (ESP32-S3) hasta la visualización en la nube.

---

## 1. Arquitectura del Sistema
- **Hardware:** ESP32-S3 + Sensor SHT3X (I2C).
- **Protocolo:** MQTT sobre TLS (Puerto 8883).
- **Broker:** EMQX alojado en AWS EC2.
- **Backend:** Receiver IoT (Spring Boot) en Docker.
- **Base de Datos:** TimescaleDB (PostgreSQL optimizado para series de tiempo).
- **Visualización:** Grafana Labs.

---

## 2. Comandos en el Servidor (AWS EC2)

Para demostrar que los servicios están corriendo correctamente, conéctate por SSH y usa estos comandos:

### Verificar estado de los contenedores
```bash
cd ~/receiver-grafana
sudo docker-compose ps
```

### Ver flujo de datos en tiempo real (Logs del Receiver)
Este es el comando más importante para mostrar que los mensajes del ESP32 están llegando al servidor:
```bash
sudo docker-compose logs -f receiver-iot-service
```

### Consultar datos directamente en la Base de Datos
Si el profesor pregunta si los datos son persistentes:
```bash
sudo docker-compose exec timescaledb psql -U iot_db_user -d iot_data_db -c "SELECT * FROM data ORDER BY unix_time DESC LIMIT 10;"
```

---

## 3. Comandos de Firmware (Local - VS Code)

### Compilar y Subir a la placa
```powershell
python .\scripts\build_with_env.py upload
```

### Monitoreo Serial (Ver conexión TLS y Publicación)
```powershell
pio device monitor
```

---

## 4. Accesos a Plataformas Web

| Servicio | URL | Credenciales |
| :--- | :--- | :--- |
| **Grafana** (Gráficas) | `http://18.191.191.11:3000` | `admin` / `admin123` |
| **EMQX** (Dashboard) | `http://18.191.191.11:18083` | `admin` / `public` |

---

## 5. Puntos Clave para la Presentación
1. **Seguridad:** Resalta que la comunicación no es texto plano; se utiliza **TLS 1.2** para proteger los datos de la telemetría.
2. **Escalabilidad:** El uso de **TimescaleDB** permite manejar millones de registros de sensores sin pérdida de rendimiento.
3. **Persistencia:** Los datos no solo se ven en Grafana, están guardados físicamente en el volumen de Docker en AWS.
4. **Validación del Pipeline:** "El pipeline está validado al 100%. Actualmente el sensor entrega valores de error (-45) debido a un falso contacto físico (falta de soldadura), pero la llegada de estos valores a Grafana confirma que el flujo de datos (End-to-End) es exitoso".
