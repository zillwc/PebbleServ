#PebbleServ

PebbleServ Monitor is a watchapp for the Pebble smartwatch that enables realtime monitoring of a host machine's resources.

## Project Setup

_Split into two components_ 

1. watchapp - Application for pebble watch, which includes backend code utilising Pebblekit JS for portability
2. webserver_content - Code to be run on host machine to be monitored [strictly read-only]

## Deployment

### _Requirements for usage_

- Pebble smartwatch with 2.0+ firmware
- Android [4.0+] or iOS device running Pebble App 2.0-BETA4+
- Host machine with Python 2.7.9+ installed

- _OPTIONAL - For compiling code, please install PebbleSDK, pebble ARM toolchain and other dependancies according to the procedure outlined at https://developer.getpebble.com_

### _How to deploy_

- Build the watch app and copy servermon.pbw to your phone and open file, select pebble app to install
- Copy host script to the machine[s] you want to monitor

MIT License