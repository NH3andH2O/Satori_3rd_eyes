#pragma once

namespace imu_angle
{
	struct Quaternion
	{
		double w;
		double x;
		double y;
		double z;
	};

	struct EulerAngles
	{
		double x;
		double y;
		double z;
	};

	struct AngularVelocityDps
	{
		double xDegreesPerSecond{};
		double yDegreesPerSecond{};
		double zDegreesPerSecond{};
	};

	struct LinearAccelerationMps2
	{
		double xMetersPerSecondSquared{};
		double yMetersPerSecondSquared{};
		double zMetersPerSecondSquared{};
	};

	class ImuAngle
	{
	  public:
		[[nodiscard]] static Quaternion conjugate(const Quaternion &quaternion);
		[[nodiscard]] static Quaternion multiply(const Quaternion &left, const Quaternion &right);
		[[nodiscard]] static EulerAngles toEulerDegrees(const Quaternion &quaternion);
	};
} // namespace imu_angle
