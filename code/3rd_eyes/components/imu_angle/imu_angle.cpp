#include "imu_angle.hpp"

#include <cmath>

namespace imu_angle
{
	namespace
	{
		constexpr double kPi = 3.14159265358979323846;
	}

	Quaternion ImuAngle::conjugate(const Quaternion &quaternion)
	{
		return Quaternion{
			quaternion.w,
			-quaternion.x,
			-quaternion.y,
			-quaternion.z,
		};
	}

	Quaternion ImuAngle::multiply(const Quaternion &left, const Quaternion &right)
	{
		return Quaternion{
			left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z,
			left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
			left.w * right.y - left.x * right.z + left.y * right.w + left.z * right.x,
			left.w * right.z + left.x * right.y - left.y * right.x + left.z * right.w,
		};
	}

	EulerAngles ImuAngle::toEulerDegrees(const Quaternion &quaternion)
	{
		return EulerAngles{
			std::atan2(2.0 * (quaternion.w * quaternion.x + quaternion.y * quaternion.z),
					   1.0 - 2.0 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y)) *
				180.0 / kPi,
			std::asin(2.0 * (quaternion.w * quaternion.y - quaternion.z * quaternion.x)) * 180.0 / kPi,
			std::atan2(2.0 * (quaternion.w * quaternion.z + quaternion.x * quaternion.y),
					   1.0 - 2.0 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z)) *
				180.0 / kPi,
		};
	}
} // namespace imu_angle
