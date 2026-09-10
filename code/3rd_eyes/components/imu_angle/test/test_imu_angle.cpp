#include "unity.h"
#include "imu_angle.hpp"

#include <cmath>

namespace
{
	constexpr double kTolerance = 1e-9;

	void assertQuaternionWithin(const imu_angle::Quaternion &expected, const imu_angle::Quaternion &actual)
	{
		TEST_ASSERT_DOUBLE_WITHIN(kTolerance, expected.w, actual.w);
		TEST_ASSERT_DOUBLE_WITHIN(kTolerance, expected.x, actual.x);
		TEST_ASSERT_DOUBLE_WITHIN(kTolerance, expected.y, actual.y);
		TEST_ASSERT_DOUBLE_WITHIN(kTolerance, expected.z, actual.z);
	}
} // namespace

TEST_CASE("quaternion conjugate preserves scalar and negates vector", "[imu_angle]")
{
	const imu_angle::Quaternion quaternion{1.5, -2.0, 3.25, -4.5};
	const imu_angle::Quaternion expected{1.5, 2.0, -3.25, 4.5};

	assertQuaternionWithin(expected, imu_angle::ImuAngle::conjugate(quaternion));
}

TEST_CASE("Hamilton product preserves operand order", "[imu_angle]")
{
	const imu_angle::Quaternion left{1.0, 2.0, 3.0, 4.0};
	const imu_angle::Quaternion right{5.0, 6.0, 7.0, 8.0};

	assertQuaternionWithin({-60.0, 12.0, 30.0, 24.0}, imu_angle::ImuAngle::multiply(left, right));
	assertQuaternionWithin({-60.0, 20.0, 14.0, 32.0}, imu_angle::ImuAngle::multiply(right, left));
}

TEST_CASE("multiplying by conjugate returns squared magnitude", "[imu_angle]")
{
	const imu_angle::Quaternion quaternion{0.5, -1.0, 2.0, -3.0};

	assertQuaternionWithin({14.25, 0.0, 0.0, 0.0}, imu_angle::ImuAngle::multiply(quaternion, imu_angle::ImuAngle::conjugate(quaternion)));
}

TEST_CASE("identity quaternion converts to zero Euler angles", "[imu_angle]")
{
	const imu_angle::EulerAngles angles = imu_angle::ImuAngle::toEulerDegrees({1.0, 0.0, 0.0, 0.0});

	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, angles.x);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, angles.y);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, angles.z);
}

TEST_CASE("single-axis quaternions preserve Euler axes signs and degrees", "[imu_angle]")
{
	const imu_angle::EulerAngles x_angles = imu_angle::ImuAngle::toEulerDegrees({0.7071067811865476, 0.7071067811865476, 0.0, 0.0});
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 90.0, x_angles.x);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, x_angles.y);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, x_angles.z);

	const imu_angle::EulerAngles y_angles = imu_angle::ImuAngle::toEulerDegrees({0.9238795325112867, 0.0, -0.3826834323650898, 0.0});
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, y_angles.x);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, -45.0, y_angles.y);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, y_angles.z);

	const imu_angle::EulerAngles z_angles = imu_angle::ImuAngle::toEulerDegrees({0.5, 0.0, 0.0, 0.8660254037844386});
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, z_angles.x);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 0.0, z_angles.y);
	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 120.0, z_angles.z);
}

TEST_CASE("Euler conversion does not normalize input", "[imu_angle]")
{
	const imu_angle::EulerAngles angles = imu_angle::ImuAngle::toEulerDegrees({2.0, 0.0, 0.125, 0.0});

	TEST_ASSERT_DOUBLE_WITHIN(kTolerance, 30.0, angles.y);
}

TEST_CASE("Euler conversion preserves out-of-domain NaN behavior", "[imu_angle]")
{
	const imu_angle::EulerAngles angles = imu_angle::ImuAngle::toEulerDegrees({1.0, 0.0, 1.0, 0.0});

	TEST_ASSERT_TRUE(std::isnan(angles.y));
}
