# Smart Street Lighting Monitoring and Fault Reporting System

This repository contains the source code, simulation configurations, and the final report for the **CNG 476 System Simulation** final project.

## Team 13
* Zanyar Erkozan (2584985)
* Berke Filiz (2585008)

## Project Overview
This project simulates a LoRaWAN-based smart street lighting system using OMNeT++, INET, and FLoRa. The main focus of our simulation is to evaluate a priority queueing mechanism at the network server that processes critical fault alarms before routine telemetry data to reduce End-to-End Latency for emergency situations.

## How to Run
1. Install OMNeT++ (v6.0 or compatible) and the INET framework.
2. Install the FLoRa framework for LoRaWAN capabilities.
3. Import this project folder into your OMNeT++ IDE.
4. Open the `omnetpp.ini` file.
5. Select either the `NodeSweep` or `HighLoad` configuration and click Run.

## Repository Structure
* **`Gateway.cc / .ned`**: Implements the LoRa gateway and packet forwarding logic.
* **`SmartNetworkServerApp.cc / .ned`**: Implements the server-side priority queueing and delay calculations.
* **`StreetLightApp.cc / .ned`**: Handles periodic telemetry and stochastic fault packet generation.
* **`omnetpp.ini`**: Contains the simulation settings, traffic configurations, and Monte Carlo repetition seeds.
* **`Team13_FinalReport.pdf`**: The complete final project report formatted in IEEE style.
