# Car-Park-Management-System-and-Simulator
- This repository contains the code for the assignment 2 for CAB403 Systems Programming. This project was done in C. It is compilable and runnable on the MX Linux virtual machine.
---
## Installation Instructions
- To run this project, open terminal and navigate to the location of the pulled code. From here, you can type *make* which will make all 3 files required for this project to run. These files are Manager.c Simulator.c and FireAlarm.c. When ready to run, open 3 terminals and run *./Simulator* then *./FireAlarm* then *./Manager*.
---
### Fire Simulation Initiation:
Within the Simulator.c, there is a public variable *FIRE* which can be set in the following ways for 
simulation of temperatures that initiate the required scenarios:
- FIRE = 0: Provides normal temperature readings to let the car park function as intended.
- FIRE = 1: Provides high temperatures readings to activate the fire alarm due to "Fixed temperature fire detection".
- FIRE = 2: Provides rising temperatures readings to activate the fire alarm due to "Rate-of-rise fire detection"
---
### Car Initiation:
- Within the Simulator.c, there is a public variable *CAR_LIMIT* which can be set to allow a certain number of cars into the carpark. The final define is *RANDOM_CHANCE* and will establish the probablity of the plate appearing from the list of allowed plates. 
---
### Carpark Initiation:
- Within the SharedMemory.c, there are definitions for *ENTRANCES*, *EXITS* and *LEVELS* to allow a flexible carpark design. Additionally, in *CarQueue.c* there is a definition for the capacity of each of these levels
---
## Notes about the Project
- This project is open source and will allow anyone to be able to open and contribute to it.
---


https://user-images.githubusercontent.com/71302996/151725194-ab76fc4e-b8e5-43ba-9ce0-a23a33391edb.mp4

