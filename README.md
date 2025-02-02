# MediCare - Système de gestion de médicaments 💊

MediCare est un système intelligent de gestion de la prise de médicaments conçu pour aider les patients, les personnes âgées et ceux qui ont tendance à oublier leurs médicaments. Il permet :

- De rappeler aux patients de prendre leurs médicaments à temps
- De suivre l'état de prise des médicaments grâce à un pilulier intelligent
- De fournir une interface aux responsables (famille, soignants) pour consulter les états de prise de médicaments des patients
- Le système repose sur une architecture IoT et cloud, intégrant un ESP8266 avec des capteurs, un backend Spring Boot & MongoDB, et un frontend Angular & Bootstrap. La communication entre les composants se fait via RabbitMQ.

 # ⚡  ESP8266 & RabbitMQ (C++ & Docker Compose)
Description :
Ce repository contient le code embarqué pour l'ESP8266 ainsi que la configuration Docker Compose pour RabbitMQ. L'ESP8266 est connecté à trois capteurs de distance, un buzzer et des LED. Il détecte si les médicaments ont été pris et envoie les données via RabbitMQ au backend.

Fonctionnalités principales :

- Mesure de distance avec des capteurs ultrasoniques pour détecter la prise de médicaments
- Envoi de messages JSON à RabbitMQ si un médicament est manqué
- Activation d'un buzzer et d'une LED en cas d'oubli de prise de médicament
- Configuration Docker Compose pour RabbitMQ
  
Technologies utilisées :

- C++ (Arduino IDE)
- RabbitMQ
- Docker Compose
- ESP8266
- Capteurs Ultrason
- Buzzer
- LEDs
