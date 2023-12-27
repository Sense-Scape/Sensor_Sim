# Windows Sensor Simulator

## Summary

This is an audio simulation module which sends is designed to send data to a [processing server](https://github.com/Sense-Scape/Windows_Proc_Serv).
This module is configured uising the ```Config.json``` file.. The file itself contains descriptions of configuration parameters. 

## Getting Started

### The Easy Way

- Download a release from this repository
- Configure the `Config.json` file
- Run the simulator

### The Not As Easy Way

- Clone this repo using the ```git clone``` command
- Run ```git submodule update --remote --recursive --init``` to pull in the submodule code
- Open the solution file in visual studio and build
- Configure the `Config.json` file
- Run the simulator

## System Overview

``` mermaid
  graph TD;
  SimulatorModule-->ChunkToBytesModule
  ChunkToBytesModule-->WinTCPTxModule
```
