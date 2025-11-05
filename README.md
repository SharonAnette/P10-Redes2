# Práctica 10 — Protocolo TFTP

Este repositorio contiene el desarrollo de la **Práctica 10** de la asignatura **Aplicaciones para Comunicaciones en Red (6CV2)**, impartida en la **Escuela Superior de Cómputo (ESCOM)** del **Instituto Politécnico Nacional (IPN)**.

---

## Objetivo
Demostrar el funcionamiento del **protocolo TFTP (Trivial File Transfer Protocol)** mediante la implementación de un **cliente y servidor** capaces de transferir archivos, utilizando comunicación basada en **UDP** dentro de una distribución de Linux:contentReference[oaicite:0]{index=0}.

---

## Funcionamiento
El protocolo **TFTP** permite la **transferencia de archivos sencilla** entre un cliente y un servidor sin autenticación ni cifrado.  
Opera sobre **UDP** y realiza el intercambio de datos en bloques de 512 bytes:

1. El **cliente** envía una solicitud de lectura (RRQ) o escritura (WRQ) al servidor.  
2. El **servidor** responde enviando o recibiendo bloques de datos.  
3. Cada bloque requiere un **ACK (Acknowledgment)** por parte del cliente para continuar la transferencia.  
4. El proceso termina cuando se envía un bloque más pequeño de lo habitual, indicando el final del archivo.
