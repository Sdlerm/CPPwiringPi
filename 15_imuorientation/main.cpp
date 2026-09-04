#include <RTIMULib.h>
#include <thread>
#include <chrono>
#include <numbers>
#include <iostream>
using namespace std;

RTIMUSettings settings("RTIMULib");
RTIMU* imu = RTIMU::createIMU(&settings);

int main()
{
	imu->IMUInit();
	int interval = imu->IMUGetPollInterval();

	while (true)
	{
		if (imu->IMURead())
		{
			RTIMU_DATA d = imu->getIMUData();
			double roll = d.fusionPose.x() * 180.0 / numbers::pi;
			double pitch = d.fusionPose.y() * 180.0 / numbers::pi;
			double yaw = d.fusionPose.z() * 180.0 / numbers::pi;

			//Print roll/pitch/yaw...
			cout << "Roll: " << roll << " degrees\n";
			cout << "Pitch " << pitch << " degrees\n";
			cout << "Yaw " << yaw << " degrees\n";
		}
		this_thread::sleep_for(
			chrono::milliseconds(imu->IMUGetPollInterval())
		);
	}

}