# GPS-Tracker
Project files for an autonomous, portable GPS tracker

### Project Summary
A battery powered self sustaining tracking device designed for implementation in vehicles. Capable of measuring location and driving route history and network streaming. Also capable of smart power management.

### Objectives
-
-

### Current Status
Layer 1 Testing. GPS Module single test done

### Repository Structure
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

