# Windows_Sensor_Sim

## System Summary

This is an audio simulation module which sends is designed to send data to a [processing server](https://github.com/Sense-Scape/Windows_Proc_Serv).
This module is configured uising the ```Config.json``` file.. The file itself contains descriptions of configuration parameters. 

## Getting Started

- Clone this repo using the ```git clone``` command
- Run ```git submodule update --remote --recursive --init``` to pull in the submodule code
- Open the solution file in visual studio and build/run it

## Block Diagram

``` mermaid
  graph TD;
  SimulatorModule-->ChunkToBytesModule
  ChunkToBytesModule-->WinTCPTxModule
```
