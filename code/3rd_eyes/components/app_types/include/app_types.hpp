#pragma once

#include <cstdint>
#include <type_traits>

#include "imu_angle.hpp"

namespace app_types
{
	enum class NetworkCommand : std::uint8_t
	{
		ApplyStationConfig = 1,
		ApplySoftApConfig = 2,
		ReconnectStation = 3,
	};

	enum class AppMode : std::uint8_t
	{
		ServoSetup = 0,
		GyroscopeTracking = 1,
		NetworkControl = 2,
	};

	enum class NetworkControlKind : std::uint8_t
	{
		EyePosition = 0,
		EyelidMotion = 1,
	};

	enum class ImuSensor : std::uint8_t
	{
		Eyes = 1,
		Head = 2,
	};

	enum class ImuSampleStatus : std::uint8_t
	{
		Ok = 0,
		NoData = 1,
		Timeout = 2,
		InvalidData = 3,
		UnknownError = 4,
		Cancelled = 5,
	};

	struct ImuSample
	{
		ImuSensor sensor{ImuSensor::Eyes};
		ImuSampleStatus status{ImuSampleStatus::NoData};
		imu_angle::EulerAngles eulerDegrees{};
		imu_angle::AngularVelocityDps angularVelocity{};
		imu_angle::LinearAccelerationMps2 linearAcceleration{};
		imu_angle::Quaternion orientation{};
		double temperatureCelsius{};
	};

	struct RelativeImuMotion
	{
		imu_angle::EulerAngles relativeEulerDegrees{};
		imu_angle::AngularVelocityDps relativeAngularVelocity{};
		imu_angle::LinearAccelerationMps2 eyesAccelerationWorld{};
		imu_angle::LinearAccelerationMps2 headAccelerationWorld{};
	};

	struct EyeMotionCommand
	{
		std::uint8_t eyelidOpenAngleDegrees{};
		std::int8_t xAngleDegrees{};
		std::int8_t yAngleDegrees{};
	};

	struct EyeDisplayCommand
	{
		std::uint8_t pupilRadiusPixels{};
		double pupilRadiusDampingRatio{};
		double pupilRadiusNaturalFrequency{};
		std::uint8_t glowMaximum{};
		double glowDampingRatio{};
		double glowNaturalFrequency{};
	};

	struct ServoPreviewCommand
	{
		std::uint8_t upperEyelidAngleDegrees{};
		std::uint8_t lowerEyelidAngleDegrees{};
		std::uint8_t eyeballAngleDegrees{};
	};

	struct NetworkControlInput
	{
		NetworkControlKind kind{NetworkControlKind::EyePosition};
		std::int8_t xPercent{};
		std::int8_t yPercent{};
	};

	struct NetworkControlVelocity
	{
		double magnitude{};
	};

	struct ModeConfig
	{
		AppMode mode{AppMode::ServoSetup};
	};

	struct ServoConfig
	{
		bool isSetup{};
		std::uint8_t maximumUpperEyelidAngleDegrees{};
		std::uint8_t middleUpperEyelidAngleDegrees{};
		std::uint8_t minimumUpperEyelidAngleDegrees{};
		std::uint8_t maximumLowerEyelidAngleDegrees{};
		std::uint8_t middleLowerEyelidAngleDegrees{};
		std::uint8_t minimumLowerEyelidAngleDegrees{};
		std::uint8_t maximumEyeballAngleDegrees{};
		std::uint8_t middleEyeballAngleDegrees{};
		std::uint8_t minimumEyeballAngleDegrees{};
	};

	struct AdvancedConfig
	{
		std::uint16_t correctionIntervalMs{};
		std::uint8_t gyroscopeEyelidOpenAngleDegrees{};
	};

	template <typename Type> inline constexpr bool isQueuePayload = std::is_trivially_copyable_v<Type> && std::is_standard_layout_v<Type>;

	static_assert(isQueuePayload<ImuSample>);
	static_assert(isQueuePayload<RelativeImuMotion>);
	static_assert(isQueuePayload<EyeMotionCommand>);
	static_assert(isQueuePayload<EyeDisplayCommand>);
	static_assert(isQueuePayload<ServoPreviewCommand>);
	static_assert(isQueuePayload<NetworkControlInput>);
	static_assert(isQueuePayload<NetworkControlVelocity>);
	static_assert(isQueuePayload<NetworkCommand>);
	static_assert(isQueuePayload<AppMode>);
	static_assert(isQueuePayload<ModeConfig>);
	static_assert(isQueuePayload<ServoConfig>);
	static_assert(isQueuePayload<AdvancedConfig>);
} // namespace app_types