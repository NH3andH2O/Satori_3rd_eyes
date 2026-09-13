#include "unity.h"

#include <cstdint>
#include <type_traits>

#include "app_types.hpp"

TEST_CASE("app enum values preserve external contracts", "[app_types]")
{
	TEST_ASSERT_EQUAL_UINT8(1, static_cast<std::uint8_t>(app_types::NetworkCommand::ApplyStationConfig));
	TEST_ASSERT_EQUAL_UINT8(2, static_cast<std::uint8_t>(app_types::NetworkCommand::ApplySoftApConfig));
	TEST_ASSERT_EQUAL_UINT8(3, static_cast<std::uint8_t>(app_types::NetworkCommand::ReconnectStation));
	TEST_ASSERT_EQUAL_UINT8(0, static_cast<std::uint8_t>(app_types::AppMode::ServoSetup));
	TEST_ASSERT_EQUAL_UINT8(1, static_cast<std::uint8_t>(app_types::AppMode::GyroscopeTracking));
	TEST_ASSERT_EQUAL_UINT8(2, static_cast<std::uint8_t>(app_types::AppMode::NetworkControl));
	TEST_ASSERT_EQUAL_UINT8(0, static_cast<std::uint8_t>(app_types::NetworkControlKind::EyePosition));
	TEST_ASSERT_EQUAL_UINT8(1, static_cast<std::uint8_t>(app_types::NetworkControlKind::EyelidMotion));
	TEST_ASSERT_EQUAL_UINT8(1, static_cast<std::uint8_t>(app_types::ImuSensor::Eyes));
	TEST_ASSERT_EQUAL_UINT8(2, static_cast<std::uint8_t>(app_types::ImuSensor::Head));
	TEST_ASSERT_EQUAL_UINT8(0, static_cast<std::uint8_t>(app_types::ImuSampleStatus::Ok));
	TEST_ASSERT_EQUAL_UINT8(1, static_cast<std::uint8_t>(app_types::ImuSampleStatus::NoData));
	TEST_ASSERT_EQUAL_UINT8(2, static_cast<std::uint8_t>(app_types::ImuSampleStatus::Timeout));
	TEST_ASSERT_EQUAL_UINT8(3, static_cast<std::uint8_t>(app_types::ImuSampleStatus::InvalidData));
	TEST_ASSERT_EQUAL_UINT8(4, static_cast<std::uint8_t>(app_types::ImuSampleStatus::UnknownError));
	TEST_ASSERT_EQUAL_UINT8(5, static_cast<std::uint8_t>(app_types::ImuSampleStatus::Cancelled));
}

TEST_CASE("IMU aggregate uses canonical math types", "[app_types]")
{
	static_assert(std::is_same_v<decltype(app_types::ImuSample{}.eulerDegrees), imu_angle::EulerAngles>);
	static_assert(std::is_same_v<decltype(app_types::ImuSample{}.angularVelocity), imu_angle::AngularVelocityDps>);
	static_assert(std::is_same_v<decltype(app_types::ImuSample{}.linearAcceleration), imu_angle::LinearAccelerationMps2>);
	static_assert(std::is_same_v<decltype(app_types::ImuSample{}.orientation), imu_angle::Quaternion>);

	app_types::ImuSample sample{};
	sample.sensor = app_types::ImuSensor::Head;
	sample.status = app_types::ImuSampleStatus::Ok;
	sample.eulerDegrees = {10.0, -20.0, 30.0};
	sample.orientation = {1.0, 0.0, 0.0, 0.0};

	TEST_ASSERT_EQUAL_UINT8(2, static_cast<std::uint8_t>(sample.sensor));
	TEST_ASSERT_EQUAL_UINT8(0, static_cast<std::uint8_t>(sample.status));
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, -20.0, sample.eulerDegrees.y);
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 1.0, sample.orientation.w);
}

TEST_CASE("signed command fields preserve negative ranges", "[app_types]")
{
	const app_types::NetworkControlInput networkInput{
		.kind = app_types::NetworkControlKind::EyePosition,
		.xPercent = -100,
		.yPercent = 100,
	};
	const app_types::EyeMotionCommand eyeMotion{
		.eyelidOpenAngleDegrees = 40,
		.xAngleDegrees = -55,
		.yAngleDegrees = 35,
	};

	TEST_ASSERT_EQUAL_INT8(-100, networkInput.xPercent);
	TEST_ASSERT_EQUAL_INT8(100, networkInput.yPercent);
	TEST_ASSERT_EQUAL_INT8(-55, eyeMotion.xAngleDegrees);
	TEST_ASSERT_EQUAL_INT8(35, eyeMotion.yAngleDegrees);
}

TEST_CASE("pure configuration DTOs retain expected ranges", "[app_types]")
{
	const app_types::ModeConfig modeConfig{.mode = app_types::AppMode::NetworkControl};
	const app_types::AdvancedConfig advancedConfig{
		.correctionIntervalMs = UINT16_MAX,
		.gyroscopeEyelidOpenAngleDegrees = 80,
	};
	const app_types::ServoConfig servoConfig{
		.isSetup = true,
		.maximumUpperEyelidAngleDegrees = 180,
	};

	TEST_ASSERT_EQUAL_UINT8(2, static_cast<std::uint8_t>(modeConfig.mode));
	TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, advancedConfig.correctionIntervalMs);
	TEST_ASSERT_EQUAL_UINT8(80, advancedConfig.gyroscopeEyelidOpenAngleDegrees);
	TEST_ASSERT_TRUE(servoConfig.isSetup);
	TEST_ASSERT_EQUAL_UINT8(180, servoConfig.maximumUpperEyelidAngleDegrees);
}