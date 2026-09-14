# GPS-Tracker
Project files for an autonomous, portable, internet-enabled GPS vehicleracker

### Project Summary
A battery powered self sustaining tracking device designed for implementation in vehicles. Capable of measuring location and driving route history and network streaming. Also capable of smart power management.

### Current Status
Layer 1 Testing. GPS Module lock and location print verified  
Layer 1 Testing. IMU operation verified  
Layer 1 Testing. Issues encountered in SD Module implementation  
Layer 1 Testing. Issues encountered on the LDO while soldering the SD module

### Repo Structure
.<br>
```
gps-tracker/
│
├── README.md
|
├── docs/
│   ├── architecture.md
│   ├── system-overview.md
│   ├── communication.md
│   ├── power-budget.md
│   ├── decisions/
│   ├── meeting-notes/
│   ├── architecture.md
│   ├── lab-notes
|
├── firmware/
│   ├── src/
|   ├── include/
|   ├── drivers/
|       ├── gps/
|       ├── sim800L/
|       ├── sd/
|       ├── imu/
|
├── hardware/
│   ├── bom/
│   ├── datasheets/
|
├── backend/
│   ├── .
|
├── tests/
│   ├── gps/
│   ├── sd/
|
├── tools/
│   ├── .
```

