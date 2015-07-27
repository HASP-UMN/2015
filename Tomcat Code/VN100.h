// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// VN100.h
// This file facilitates interaction between the Tomcat
// and the VN100 IMU.
// Edited By: Luke Granlund
// Edited On: 18 June 2015, 16:00

#ifndef VN100_H_
#define VN100_H_
//extern const char* IMU_PORT; // "/dev/ttyUSB0"                // TTY PORT TO VN100 (SET UP BY ftdi_sio KERNEL DRIVER)
//extern const char* IMU_DATAFILE; // = "IMU_VN100.txt";        // Path to file where VN100 IMU date will be recorded

// Functions
void init_vn100(struct imu* imuData_ptr);
void read_vn100(struct imu* imuData_ptr);


#endif /* VN100_H_ */
