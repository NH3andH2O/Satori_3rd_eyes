#include "unity.h"

#include <cstdint>

#include "app_events.hpp"

namespace
{
	void initializeEvents()
	{
		app_events::deinitialize();
		TEST_ASSERT_EQUAL(ESP_OK, app_events::initialize());
		TEST_ASSERT_TRUE(app_events::isInitialized());
	}

	std::uint8_t maskValue(const app_events::WitReaderMask mask) { return static_cast<std::uint8_t>(mask); }
} // namespace

TEST_CASE("app events lifecycle is explicit and reusable", "[app_events]")
{
	app_events::deinitialize();
	TEST_ASSERT_FALSE(app_events::isInitialized());
	TEST_ASSERT_EQUAL(ESP_OK, app_events::initialize());
	TEST_ASSERT_TRUE(app_events::isInitialized());
	TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, app_events::initialize());

	app_events::deinitialize();
	TEST_ASSERT_FALSE(app_events::isInitialized());
	app_events::deinitialize();
	TEST_ASSERT_EQUAL(ESP_OK, app_events::initialize());
	app_events::deinitialize();
}

TEST_CASE("uninitialized app event APIs fail without touching payloads", "[app_events]")
{
	app_events::deinitialize();
	app_types::ImuSample imuSample{};
	app_types::RelativeImuMotion relativeMotion{};
	app_types::EyeMotionCommand eyeMotion{};
	app_types::EyeDisplayCommand eyeDisplay{};
	app_types::NetworkCommand networkCommand = app_types::NetworkCommand::ApplyStationConfig;
	app_types::AdvancedConfig advancedConfig{};
	app_types::AppMode mode = app_types::AppMode::ServoSetup;
	app_types::NetworkControlInput networkInput{};
	app_types::NetworkControlVelocity velocity{};
	app_types::ServoPreviewCommand servoPreview{};

	TEST_ASSERT_FALSE(app_events::sendImuSample(imuSample, 0));
	TEST_ASSERT_FALSE(app_events::receiveImuSample(imuSample, 0));
	TEST_ASSERT_FALSE(app_events::sendRelativeImuMotion(relativeMotion, 0));
	TEST_ASSERT_FALSE(app_events::receiveRelativeImuMotion(relativeMotion, 0));
	TEST_ASSERT_FALSE(app_events::sendEyeMotionCommand(eyeMotion, 0));
	TEST_ASSERT_FALSE(app_events::receiveEyeMotionCommand(eyeMotion, 0));
	TEST_ASSERT_FALSE(app_events::sendEyeDisplayCommand(eyeDisplay, 0));
	TEST_ASSERT_FALSE(app_events::receiveEyeDisplayCommand(eyeDisplay, 0));
	TEST_ASSERT_FALSE(app_events::sendNetworkCommand(networkCommand, 0));
	TEST_ASSERT_FALSE(app_events::receiveNetworkCommand(networkCommand, 0));
	TEST_ASSERT_FALSE(app_events::publishAdvancedConfig(advancedConfig));
	TEST_ASSERT_FALSE(app_events::receiveWitAdvancedConfig(advancedConfig, 0));
	TEST_ASSERT_FALSE(app_events::receiveGyroscopeAdvancedConfig(advancedConfig, 0));
	TEST_ASSERT_FALSE(app_events::publishMode(mode));
	TEST_ASSERT_FALSE(app_events::receiveMode(mode, 0));
	TEST_ASSERT_FALSE(app_events::sendNetworkControlInput(networkInput, 0));
	TEST_ASSERT_FALSE(app_events::receiveNetworkControlInput(networkInput, 0));
	TEST_ASSERT_FALSE(app_events::sendNetworkControlVelocity(velocity, 0));
	TEST_ASSERT_FALSE(app_events::receiveNetworkControlVelocity(velocity, 0));
	TEST_ASSERT_FALSE(app_events::sendServoPreviewCommand(servoPreview, 0));
	TEST_ASSERT_FALSE(app_events::receiveServoPreviewCommand(servoPreview, 0));
	TEST_ASSERT_FALSE(app_events::resetModeChannels());
	TEST_ASSERT_FALSE(app_events::clearWitStopped(app_events::WitReaderMask::All));
	TEST_ASSERT_FALSE(app_events::signalWitStopped(app_types::ImuSensor::Eyes));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::None), app_events::waitForWitStopped(app_events::WitReaderMask::All, 0));
}

TEST_CASE("all typed app channels preserve payload values", "[app_events]")
{
	initializeEvents();

	app_types::ImuSample imuSent{};
	imuSent.sensor = app_types::ImuSensor::Head;
	imuSent.status = app_types::ImuSampleStatus::Ok;
	imuSent.eulerDegrees = {1.0, 2.0, 3.0};
	imuSent.temperatureCelsius = 24.5;
	app_types::ImuSample imuReceived{};
	TEST_ASSERT_TRUE(app_events::sendImuSample(imuSent, 0));
	TEST_ASSERT_TRUE(app_events::receiveImuSample(imuReceived, 0));
	TEST_ASSERT_EQUAL_UINT8(static_cast<std::uint8_t>(app_types::ImuSensor::Head), static_cast<std::uint8_t>(imuReceived.sensor));
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 3.0, imuReceived.eulerDegrees.z);
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 24.5, imuReceived.temperatureCelsius);

	app_types::RelativeImuMotion relativeSent{};
	relativeSent.relativeEulerDegrees = {-10.0, 20.0, -30.0};
	app_types::RelativeImuMotion relativeReceived{};
	TEST_ASSERT_TRUE(app_events::sendRelativeImuMotion(relativeSent, 0));
	TEST_ASSERT_TRUE(app_events::receiveRelativeImuMotion(relativeReceived, 0));
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, -30.0, relativeReceived.relativeEulerDegrees.z);

	const app_types::EyeMotionCommand eyeMotionSent{40, -55, 35};
	app_types::EyeMotionCommand eyeMotionReceived{};
	TEST_ASSERT_TRUE(app_events::sendEyeMotionCommand(eyeMotionSent, 0));
	TEST_ASSERT_TRUE(app_events::receiveEyeMotionCommand(eyeMotionReceived, 0));
	TEST_ASSERT_EQUAL_INT8(-55, eyeMotionReceived.xAngleDegrees);

	const app_types::EyeDisplayCommand eyeDisplaySent{65, 0.9, 20.0, 250, 0.8, 15.0};
	app_types::EyeDisplayCommand eyeDisplayReceived{};
	TEST_ASSERT_TRUE(app_events::sendEyeDisplayCommand(eyeDisplaySent, 0));
	TEST_ASSERT_TRUE(app_events::receiveEyeDisplayCommand(eyeDisplayReceived, 0));
	TEST_ASSERT_EQUAL_UINT8(65, eyeDisplayReceived.pupilRadiusPixels);
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 15.0, eyeDisplayReceived.glowNaturalFrequency);

	app_types::NetworkCommand networkCommand = app_types::NetworkCommand::ReconnectStation;
	TEST_ASSERT_TRUE(app_events::sendNetworkCommand(networkCommand, 0));
	networkCommand = app_types::NetworkCommand::ApplyStationConfig;
	TEST_ASSERT_TRUE(app_events::receiveNetworkCommand(networkCommand, 0));
	TEST_ASSERT_EQUAL_UINT8(static_cast<std::uint8_t>(app_types::NetworkCommand::ReconnectStation), static_cast<std::uint8_t>(networkCommand));

	const app_types::AdvancedConfig advancedSent{1234, 60};
	app_types::AdvancedConfig witAdvancedReceived{};
	app_types::AdvancedConfig gyroscopeAdvancedReceived{};
	TEST_ASSERT_TRUE(app_events::publishAdvancedConfig(advancedSent));
	TEST_ASSERT_TRUE(app_events::receiveWitAdvancedConfig(witAdvancedReceived, 0));
	TEST_ASSERT_TRUE(app_events::receiveGyroscopeAdvancedConfig(gyroscopeAdvancedReceived, 0));
	TEST_ASSERT_EQUAL_UINT16(1234, witAdvancedReceived.correctionIntervalMs);
	TEST_ASSERT_EQUAL_UINT8(60, gyroscopeAdvancedReceived.gyroscopeEyelidOpenAngleDegrees);

	TEST_ASSERT_TRUE(app_events::publishMode(app_types::AppMode::NetworkControl));
	app_types::AppMode mode = app_types::AppMode::ServoSetup;
	TEST_ASSERT_TRUE(app_events::receiveMode(mode, 0));
	TEST_ASSERT_EQUAL_UINT8(static_cast<std::uint8_t>(app_types::AppMode::NetworkControl), static_cast<std::uint8_t>(mode));

	const app_types::NetworkControlInput networkInputSent{app_types::NetworkControlKind::EyelidMotion, -10, 80};
	app_types::NetworkControlInput networkInputReceived{};
	TEST_ASSERT_TRUE(app_events::sendNetworkControlInput(networkInputSent, 0));
	TEST_ASSERT_TRUE(app_events::receiveNetworkControlInput(networkInputReceived, 0));
	TEST_ASSERT_EQUAL_INT8(80, networkInputReceived.yPercent);

	const app_types::NetworkControlVelocity velocitySent{42.25};
	app_types::NetworkControlVelocity velocityReceived{};
	TEST_ASSERT_TRUE(app_events::sendNetworkControlVelocity(velocitySent, 0));
	TEST_ASSERT_TRUE(app_events::receiveNetworkControlVelocity(velocityReceived, 0));
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 42.25, velocityReceived.magnitude);

	const app_types::ServoPreviewCommand servoSent{120, 70, 85};
	app_types::ServoPreviewCommand servoReceived{};
	TEST_ASSERT_TRUE(app_events::sendServoPreviewCommand(servoSent, 0));
	TEST_ASSERT_TRUE(app_events::receiveServoPreviewCommand(servoReceived, 0));
	TEST_ASSERT_EQUAL_UINT8(85, servoReceived.eyeballAngleDegrees);

	app_events::deinitialize();
}

TEST_CASE("buffered app channels retain depth ten", "[app_events]")
{
	initializeEvents();
	const app_types::ImuSample imuSample{};
	const app_types::RelativeImuMotion relativeMotion{};
	const app_types::EyeMotionCommand eyeMotion{};
	const app_types::EyeDisplayCommand eyeDisplay{};
	const app_types::NetworkControlInput networkInput{};
	const app_types::NetworkControlVelocity velocity{};
	const app_types::ServoPreviewCommand servoPreview{};

	for (int index = 0; index < 10; ++index)
	{
		TEST_ASSERT_TRUE(app_events::sendImuSample(imuSample, 0));
		TEST_ASSERT_TRUE(app_events::sendRelativeImuMotion(relativeMotion, 0));
		TEST_ASSERT_TRUE(app_events::sendEyeMotionCommand(eyeMotion, 0));
		TEST_ASSERT_TRUE(app_events::sendEyeDisplayCommand(eyeDisplay, 0));
		TEST_ASSERT_TRUE(app_events::sendNetworkCommand(app_types::NetworkCommand::ApplyStationConfig, 0));
		TEST_ASSERT_TRUE(app_events::sendNetworkControlInput(networkInput, 0));
		TEST_ASSERT_TRUE(app_events::sendNetworkControlVelocity(velocity, 0));
		TEST_ASSERT_TRUE(app_events::sendServoPreviewCommand(servoPreview, 0));
	}

	TEST_ASSERT_FALSE(app_events::sendImuSample(imuSample, 0));
	TEST_ASSERT_FALSE(app_events::sendRelativeImuMotion(relativeMotion, 0));
	TEST_ASSERT_FALSE(app_events::sendEyeMotionCommand(eyeMotion, 0));
	TEST_ASSERT_FALSE(app_events::sendEyeDisplayCommand(eyeDisplay, 0));
	TEST_ASSERT_FALSE(app_events::sendNetworkCommand(app_types::NetworkCommand::ApplyStationConfig, 0));
	TEST_ASSERT_FALSE(app_events::sendNetworkControlInput(networkInput, 0));
	TEST_ASSERT_FALSE(app_events::sendNetworkControlVelocity(velocity, 0));
	TEST_ASSERT_FALSE(app_events::sendServoPreviewCommand(servoPreview, 0));

	app_events::deinitialize();
}

TEST_CASE("typed app channels remain isolated", "[app_events]")
{
	initializeEvents();
	const app_types::EyeMotionCommand eyeMotionSent{40, -20, 10};
	const app_types::NetworkControlInput networkInputSent{app_types::NetworkControlKind::EyePosition, -50, 25};
	TEST_ASSERT_TRUE(app_events::sendEyeMotionCommand(eyeMotionSent, 0));
	TEST_ASSERT_TRUE(app_events::sendNetworkControlInput(networkInputSent, 0));
	TEST_ASSERT_TRUE(app_events::publishMode(app_types::AppMode::GyroscopeTracking));

	app_types::EyeDisplayCommand eyeDisplay{};
	app_types::ServoPreviewCommand servoPreview{};
	app_types::NetworkControlVelocity velocity{};
	app_types::NetworkCommand networkCommand = app_types::NetworkCommand::ApplyStationConfig;
	TEST_ASSERT_FALSE(app_events::receiveEyeDisplayCommand(eyeDisplay, 0));
	TEST_ASSERT_FALSE(app_events::receiveServoPreviewCommand(servoPreview, 0));
	TEST_ASSERT_FALSE(app_events::receiveNetworkControlVelocity(velocity, 0));
	TEST_ASSERT_FALSE(app_events::receiveNetworkCommand(networkCommand, 0));

	app_types::EyeMotionCommand eyeMotionReceived{};
	app_types::NetworkControlInput networkInputReceived{};
	app_types::AppMode mode = app_types::AppMode::ServoSetup;
	TEST_ASSERT_TRUE(app_events::receiveEyeMotionCommand(eyeMotionReceived, 0));
	TEST_ASSERT_TRUE(app_events::receiveNetworkControlInput(networkInputReceived, 0));
	TEST_ASSERT_TRUE(app_events::receiveMode(mode, 0));
	TEST_ASSERT_EQUAL_INT8(-20, eyeMotionReceived.xAngleDegrees);
	TEST_ASSERT_EQUAL_INT8(-50, networkInputReceived.xPercent);
	TEST_ASSERT_EQUAL_UINT8(static_cast<std::uint8_t>(app_types::AppMode::GyroscopeTracking), static_cast<std::uint8_t>(mode));

	app_events::deinitialize();
}

TEST_CASE("latest-value channels overwrite and advanced config fans out", "[app_events]")
{
	initializeEvents();
	TEST_ASSERT_TRUE(app_events::publishAdvancedConfig({100, 20}));
	TEST_ASSERT_TRUE(app_events::publishAdvancedConfig({200, 40}));
	TEST_ASSERT_TRUE(app_events::publishMode(app_types::AppMode::GyroscopeTracking));
	TEST_ASSERT_TRUE(app_events::publishMode(app_types::AppMode::NetworkControl));

	app_types::AdvancedConfig witConfig{};
	app_types::AdvancedConfig gyroscopeConfig{};
	app_types::AppMode mode = app_types::AppMode::ServoSetup;
	TEST_ASSERT_TRUE(app_events::receiveWitAdvancedConfig(witConfig, 0));
	TEST_ASSERT_TRUE(app_events::receiveGyroscopeAdvancedConfig(gyroscopeConfig, 0));
	TEST_ASSERT_TRUE(app_events::receiveMode(mode, 0));
	TEST_ASSERT_EQUAL_UINT16(200, witConfig.correctionIntervalMs);
	TEST_ASSERT_EQUAL_UINT16(200, gyroscopeConfig.correctionIntervalMs);
	TEST_ASSERT_EQUAL_UINT8(40, witConfig.gyroscopeEyelidOpenAngleDegrees);
	TEST_ASSERT_EQUAL_UINT8(static_cast<std::uint8_t>(app_types::AppMode::NetworkControl), static_cast<std::uint8_t>(mode));
	TEST_ASSERT_FALSE(app_events::receiveWitAdvancedConfig(witConfig, 0));
	TEST_ASSERT_FALSE(app_events::receiveGyroscopeAdvancedConfig(gyroscopeConfig, 0));
	TEST_ASSERT_FALSE(app_events::receiveMode(mode, 0));

	app_events::deinitialize();
}

TEST_CASE("mode reset clears only mode-owned channels", "[app_events]")
{
	initializeEvents();
	TEST_ASSERT_TRUE(app_events::sendImuSample({}, 0));
	TEST_ASSERT_TRUE(app_events::sendRelativeImuMotion({}, 0));
	TEST_ASSERT_TRUE(app_events::sendEyeMotionCommand({}, 0));
	TEST_ASSERT_TRUE(app_events::sendEyeDisplayCommand({}, 0));
	TEST_ASSERT_TRUE(app_events::publishAdvancedConfig({500, 50}));
	TEST_ASSERT_TRUE(app_events::sendNetworkControlInput({}, 0));
	TEST_ASSERT_TRUE(app_events::sendNetworkControlVelocity({}, 0));
	TEST_ASSERT_TRUE(app_events::sendServoPreviewCommand({}, 0));
	TEST_ASSERT_TRUE(app_events::sendNetworkCommand(app_types::NetworkCommand::ReconnectStation, 0));
	TEST_ASSERT_TRUE(app_events::publishMode(app_types::AppMode::NetworkControl));
	TEST_ASSERT_TRUE(app_events::resetModeChannels());

	app_types::ImuSample imuSample{};
	app_types::RelativeImuMotion relativeMotion{};
	app_types::EyeMotionCommand eyeMotion{};
	app_types::EyeDisplayCommand eyeDisplay{};
	app_types::AdvancedConfig advancedConfig{};
	app_types::NetworkControlInput networkInput{};
	app_types::NetworkControlVelocity velocity{};
	app_types::ServoPreviewCommand servoPreview{};
	TEST_ASSERT_FALSE(app_events::receiveImuSample(imuSample, 0));
	TEST_ASSERT_FALSE(app_events::receiveRelativeImuMotion(relativeMotion, 0));
	TEST_ASSERT_FALSE(app_events::receiveEyeMotionCommand(eyeMotion, 0));
	TEST_ASSERT_FALSE(app_events::receiveEyeDisplayCommand(eyeDisplay, 0));
	TEST_ASSERT_FALSE(app_events::receiveWitAdvancedConfig(advancedConfig, 0));
	TEST_ASSERT_FALSE(app_events::receiveGyroscopeAdvancedConfig(advancedConfig, 0));
	TEST_ASSERT_FALSE(app_events::receiveNetworkControlInput(networkInput, 0));
	TEST_ASSERT_FALSE(app_events::receiveNetworkControlVelocity(velocity, 0));
	TEST_ASSERT_FALSE(app_events::receiveServoPreviewCommand(servoPreview, 0));

	app_types::NetworkCommand networkCommand = app_types::NetworkCommand::ApplyStationConfig;
	app_types::AppMode mode = app_types::AppMode::ServoSetup;
	TEST_ASSERT_TRUE(app_events::receiveNetworkCommand(networkCommand, 0));
	TEST_ASSERT_TRUE(app_events::receiveMode(mode, 0));
	TEST_ASSERT_EQUAL_UINT8(static_cast<std::uint8_t>(app_types::NetworkCommand::ReconnectStation), static_cast<std::uint8_t>(networkCommand));
	TEST_ASSERT_EQUAL_UINT8(static_cast<std::uint8_t>(app_types::AppMode::NetworkControl), static_cast<std::uint8_t>(mode));

	app_events::deinitialize();
}

TEST_CASE("WIT stop synchronization preserves reader identity and wait-all behavior", "[app_events]")
{
	initializeEvents();
	TEST_ASSERT_TRUE(app_events::clearWitStopped(app_events::WitReaderMask::All));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::None), app_events::waitForWitStopped(app_events::WitReaderMask::All, 0));

	TEST_ASSERT_TRUE(app_events::signalWitStopped(app_types::ImuSensor::Eyes));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::Eyes), app_events::waitForWitStopped(app_events::WitReaderMask::Eyes, 0));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::None), app_events::waitForWitStopped(app_events::WitReaderMask::Eyes, 0));

	TEST_ASSERT_TRUE(app_events::signalWitStopped(app_types::ImuSensor::Head));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::Head), app_events::waitForWitStopped(app_events::WitReaderMask::All, 0));
	TEST_ASSERT_TRUE(app_events::signalWitStopped(app_types::ImuSensor::Eyes));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::All), app_events::waitForWitStopped(app_events::WitReaderMask::All, 0));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::None), app_events::waitForWitStopped(app_events::WitReaderMask::All, 0));

	TEST_ASSERT_TRUE(app_events::signalWitStopped(app_types::ImuSensor::Eyes));
	TEST_ASSERT_TRUE(app_events::signalWitStopped(app_types::ImuSensor::Head));
	TEST_ASSERT_TRUE(app_events::clearWitStopped(app_events::WitReaderMask::Eyes));
	TEST_ASSERT_EQUAL_UINT8(maskValue(app_events::WitReaderMask::Head), app_events::waitForWitStopped(app_events::WitReaderMask::All, 0));
	TEST_ASSERT_TRUE(app_events::clearWitStopped(app_events::WitReaderMask::Head));

	app_events::deinitialize();
}