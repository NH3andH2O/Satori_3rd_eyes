#pragma once

#include <cstdint>

#include "app_types.hpp"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

namespace app_events
{
	enum class WitReaderMask : std::uint8_t
	{
		None = 0,
		Eyes = 1 << 0,
		Head = 1 << 1,
		All = Eyes | Head,
	};

	[[nodiscard]] constexpr WitReaderMask operator|(const WitReaderMask left, const WitReaderMask right) noexcept
	{
		return static_cast<WitReaderMask>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
	}

	[[nodiscard]] constexpr WitReaderMask operator&(const WitReaderMask left, const WitReaderMask right) noexcept
	{
		return static_cast<WitReaderMask>(static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
	}

	constexpr WitReaderMask &operator|=(WitReaderMask &left, const WitReaderMask right) noexcept
	{
		left = left | right;
		return left;
	}

	// Initialize all channels before starting any producer or consumer tasks.
	[[nodiscard]] esp_err_t initialize() noexcept;
	// Stop all channel users before deinitializing runtime resources.
	void deinitialize() noexcept;
	[[nodiscard]] bool isInitialized() noexcept;

	[[nodiscard]] bool sendImuSample(const app_types::ImuSample &sample, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveImuSample(app_types::ImuSample &sample, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool sendRelativeImuMotion(const app_types::RelativeImuMotion &motion, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveRelativeImuMotion(app_types::RelativeImuMotion &motion, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool sendEyeMotionCommand(const app_types::EyeMotionCommand &command, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveEyeMotionCommand(app_types::EyeMotionCommand &command, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool sendEyeDisplayCommand(const app_types::EyeDisplayCommand &command, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveEyeDisplayCommand(app_types::EyeDisplayCommand &command, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool sendNetworkCommand(app_types::NetworkCommand command, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveNetworkCommand(app_types::NetworkCommand &command, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool publishAdvancedConfig(const app_types::AdvancedConfig &config) noexcept;
	[[nodiscard]] bool receiveWitAdvancedConfig(app_types::AdvancedConfig &config, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveGyroscopeAdvancedConfig(app_types::AdvancedConfig &config, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool publishMode(app_types::AppMode mode) noexcept;
	[[nodiscard]] bool receiveMode(app_types::AppMode &mode, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool sendNetworkControlInput(const app_types::NetworkControlInput &input, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveNetworkControlInput(app_types::NetworkControlInput &input, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool sendNetworkControlVelocity(const app_types::NetworkControlVelocity &velocity, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveNetworkControlVelocity(app_types::NetworkControlVelocity &velocity, TickType_t timeoutTicks) noexcept;

	[[nodiscard]] bool sendServoPreviewCommand(const app_types::ServoPreviewCommand &command, TickType_t timeoutTicks) noexcept;
	[[nodiscard]] bool receiveServoPreviewCommand(app_types::ServoPreviewCommand &command, TickType_t timeoutTicks) noexcept;

	// Stop producers for mode-owned channels before clearing queued data.
	[[nodiscard]] bool resetModeChannels() noexcept;

	[[nodiscard]] bool clearWitStopped(WitReaderMask readers) noexcept;
	[[nodiscard]] bool signalWitStopped(app_types::ImuSensor sensor) noexcept;
	[[nodiscard]] WitReaderMask waitForWitStopped(WitReaderMask expectedReaders, TickType_t timeoutTicks) noexcept;
} // namespace app_events