### IMU (Accelerometer) Single Test
This was done on Aug 15  

I was able to write a script to initialize and boot up the IMU. Calibrate the necessary registers and printed necessary sensor data:
- Acceleration in all three axes
- Angular velocity in all three axes
- Temperature (from the onboard sensor)
- Tilt angles (roll and pitch)
- Derived linear velocity in three axes
  
Results were sucessful. Screenshot attached in parent directory.  

Worth noting that I'm yet to apply calibration offsets and correct for drift.  

But I have verified the unit is functional.
