#include "app_events.hpp"

#include "freertos/event_groups.h"
#include "freertos/queue.h"

namespace app_events
{
	namespace
	{
		constexpr UBaseType_t kBufferedChannelDepth = 10;
		constexpr UBaseType_t kLatestValueChannelDepth = 1;
		constexpr EventBits_t kWitEyesStoppedBit = BIT0;
		constexpr EventBits_t kWitHeadStoppedBit = BIT1;
		constexpr EventBits_t kWitStoppedBits = kWitEyesStoppedBit | kWitHeadStoppedBit;

		struct Resources
		{
			QueueHandle_t imuSamples{};
			QueueHandle_t relativeImuMotion{};
			QueueHandle_t eyeMotionCommands{};
			QueueHandle_t eyeDisplayCommands{};
			QueueHandle_t networkCommands{};
			QueueHandle_t witAdvancedConfig{};
			QueueHandle_t gyroscopeAdvancedConfig{};
			QueueHandle_t modes{};
			QueueHandle_t networkControlInputs{};
			QueueHandle_t networkControlVelocities{};
			QueueHandle_t servoPreviewCommands{};
			EventGroupHandle_t witStopped{};
			bool initialized{};
		};

		Resources resources;

		template <typename Payload> [[nodiscard]] bool createQueue(QueueHandle_t &queue, const UBaseType_t depth) noexcept
		{
			queue = xQueueCreate(depth, sizeof(Payload));
			return queue != nullptr;
		}

		void deleteQueue(QueueHandle_t &queue) noexcept
		{
			if (queue != nullptr)
			{
				vQueueDelete(queue);
				queue = nullptr;
			}
		}

		void destroyResources() noexcept
		{
			resources.initialized = false;
			deleteQueue(resources.imuSamples);
			deleteQueue(resources.relativeImuMotion);
			deleteQueue(resources.eyeMotionCommands);
			deleteQueue(resources.eyeDisplayCommands);
			deleteQueue(resources.networkCommands);
			deleteQueue(resources.witAdvancedConfig);
			deleteQueue(resources.gyroscopeAdvancedConfig);
			deleteQueue(resources.modes);
			deleteQueue(resources.networkControlInputs);
			deleteQueue(resources.networkControlVelocities);
			deleteQueue(resources.servoPreviewCommands);

			if (resources.witStopped != nullptr)
			{
				vEventGroupDelete(resources.witStopped);
				resources.witStopped = nullptr;
			}
		}

		template <typename Payload> [[nodiscard]] bool send(const QueueHandle_t queue, const Payload &payload, const TickType_t timeoutTicks) noexcept
		{
			return resources.initialized && queue != nullptr && xQueueSend(queue, &payload, timeoutTicks) == pdPASS;
		}

		template <typename Payload> [[nodiscard]] bool receive(const QueueHandle_t queue, Payload &payload, const TickType_t timeoutTicks) noexcept
		{
			return resources.initialized && queue != nullptr && xQueueReceive(queue, &payload, timeoutTicks) == pdPASS;
		}

		template <typename Payload> [[nodiscard]] bool overwrite(const QueueHandle_t queue, const Payload &payload) noexcept
		{
			return resources.initialized && queue != nullptr && xQueueOverwrite(queue, &payload) == pdPASS;
		}

		[[nodiscard]] EventBits_t toEventBits(const WitReaderMask readers) noexcept
		{
			return static_cast<EventBits_t>(static_cast<std::uint8_t>(readers)) & kWitStoppedBits;
		}

		[[nodiscard]] WitReaderMask toReaderMask(const EventBits_t bits) noexcept
		{
			return static_cast<WitReaderMask>(static_cast<std::uint8_t>(bits & kWitStoppedBits));
		}
	} // namespace

	esp_err_t initialize() noexcept
	{
		if (resources.initialized)
		{
			return ESP_ERR_INVALID_STATE;
		}

		if (!createQueue<app_types::ImuSample>(resources.imuSamples, kBufferedChannelDepth) ||
			!createQueue<app_types::RelativeImuMotion>(resources.relativeImuMotion, kBufferedChannelDepth) ||
			!createQueue<app_types::EyeMotionCommand>(resources.eyeMotionCommands, kBufferedChannelDepth) ||
			!createQueue<app_types::EyeDisplayCommand>(resources.eyeDisplayCommands, kBufferedChannelDepth) ||
			!createQueue<app_types::NetworkCommand>(resources.networkCommands, kBufferedChannelDepth) ||
			!createQueue<app_types::AdvancedConfig>(resources.witAdvancedConfig, kLatestValueChannelDepth) ||
			!createQueue<app_types::AdvancedConfig>(resources.gyroscopeAdvancedConfig, kLatestValueChannelDepth) ||
			!createQueue<app_types::AppMode>(resources.modes, kLatestValueChannelDepth) ||
			!createQueue<app_types::NetworkControlInput>(resources.networkControlInputs, kBufferedChannelDepth) ||
			!createQueue<app_types::NetworkControlVelocity>(resources.networkControlVelocities, kBufferedChannelDepth) ||
			!createQueue<app_types::ServoPreviewCommand>(resources.servoPreviewCommands, kBufferedChannelDepth))
		{
			destroyResources();
			return ESP_ERR_NO_MEM;
		}

		resources.witStopped = xEventGroupCreate();
		if (resources.witStopped == nullptr)
		{
			destroyResources();
			return ESP_ERR_NO_MEM;
		}

		resources.initialized = true;
		return ESP_OK;
	}

	void deinitialize() noexcept { destroyResources(); }

	bool isInitialized() noexcept { return resources.initialized; }

	bool sendImuSample(const app_types::ImuSample &sample, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.imuSamples, sample, timeoutTicks);
	}

	bool receiveImuSample(app_types::ImuSample &sample, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.imuSamples, sample, timeoutTicks);
	}

	bool sendRelativeImuMotion(const app_types::RelativeImuMotion &motion, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.relativeImuMotion, motion, timeoutTicks);
	}

	bool receiveRelativeImuMotion(app_types::RelativeImuMotion &motion, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.relativeImuMotion, motion, timeoutTicks);
	}

	bool sendEyeMotionCommand(const app_types::EyeMotionCommand &command, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.eyeMotionCommands, command, timeoutTicks);
	}

	bool receiveEyeMotionCommand(app_types::EyeMotionCommand &command, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.eyeMotionCommands, command, timeoutTicks);
	}

	bool sendEyeDisplayCommand(const app_types::EyeDisplayCommand &command, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.eyeDisplayCommands, command, timeoutTicks);
	}

	bool receiveEyeDisplayCommand(app_types::EyeDisplayCommand &command, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.eyeDisplayCommands, command, timeoutTicks);
	}

	bool sendNetworkCommand(const app_types::NetworkCommand command, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.networkCommands, command, timeoutTicks);
	}

	bool receiveNetworkCommand(app_types::NetworkCommand &command, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.networkCommands, command, timeoutTicks);
	}

	bool publishAdvancedConfig(const app_types::AdvancedConfig &config) noexcept
	{
		const bool witPublished = overwrite(resources.witAdvancedConfig, config);
		const bool gyroscopePublished = overwrite(resources.gyroscopeAdvancedConfig, config);
		return witPublished && gyroscopePublished;
	}

	bool receiveWitAdvancedConfig(app_types::AdvancedConfig &config, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.witAdvancedConfig, config, timeoutTicks);
	}

	bool receiveGyroscopeAdvancedConfig(app_types::AdvancedConfig &config, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.gyroscopeAdvancedConfig, config, timeoutTicks);
	}

	bool publishMode(const app_types::AppMode mode) noexcept { return overwrite(resources.modes, mode); }

	bool receiveMode(app_types::AppMode &mode, const TickType_t timeoutTicks) noexcept { return receive(resources.modes, mode, timeoutTicks); }

	bool sendNetworkControlInput(const app_types::NetworkControlInput &input, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.networkControlInputs, input, timeoutTicks);
	}

	bool receiveNetworkControlInput(app_types::NetworkControlInput &input, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.networkControlInputs, input, timeoutTicks);
	}

	bool sendNetworkControlVelocity(const app_types::NetworkControlVelocity &velocity, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.networkControlVelocities, velocity, timeoutTicks);
	}

	bool receiveNetworkControlVelocity(app_types::NetworkControlVelocity &velocity, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.networkControlVelocities, velocity, timeoutTicks);
	}

	bool sendServoPreviewCommand(const app_types::ServoPreviewCommand &command, const TickType_t timeoutTicks) noexcept
	{
		return send(resources.servoPreviewCommands, command, timeoutTicks);
	}

	bool receiveServoPreviewCommand(app_types::ServoPreviewCommand &command, const TickType_t timeoutTicks) noexcept
	{
		return receive(resources.servoPreviewCommands, command, timeoutTicks);
	}

	bool resetModeChannels() noexcept
	{
		if (!resources.initialized)
		{
			return false;
		}

		const QueueHandle_t modeChannels[] = {
			resources.imuSamples,			resources.relativeImuMotion,		resources.witAdvancedConfig,	resources.gyroscopeAdvancedConfig,
			resources.networkControlInputs, resources.networkControlVelocities, resources.servoPreviewCommands, resources.eyeMotionCommands,
			resources.eyeDisplayCommands,
		};

		bool resetSucceeded = true;
		for (const QueueHandle_t queue : modeChannels)
		{
			resetSucceeded = queue != nullptr && xQueueReset(queue) == pdPASS && resetSucceeded;
		}
		return resetSucceeded;
	}

	bool clearWitStopped(const WitReaderMask readers) noexcept
	{
		if (!resources.initialized || resources.witStopped == nullptr)
		{
			return false;
		}

		const EventBits_t bits = toEventBits(readers);
		if (bits != 0)
		{
			xEventGroupClearBits(resources.witStopped, bits);
		}
		return true;
	}

	bool signalWitStopped(const app_types::ImuSensor sensor) noexcept
	{
		if (!resources.initialized || resources.witStopped == nullptr)
		{
			return false;
		}

		EventBits_t bit = 0;
		switch (sensor)
		{
			case app_types::ImuSensor::Eyes:
				bit = kWitEyesStoppedBit;
				break;
			case app_types::ImuSensor::Head:
				bit = kWitHeadStoppedBit;
				break;
			default:
				return false;
		}

		xEventGroupSetBits(resources.witStopped, bit);
		return true;
	}

	WitReaderMask waitForWitStopped(const WitReaderMask expectedReaders, const TickType_t timeoutTicks) noexcept
	{
		if (!resources.initialized || resources.witStopped == nullptr)
		{
			return WitReaderMask::None;
		}

		const EventBits_t expectedBits = toEventBits(expectedReaders);
		if (expectedBits == 0)
		{
			return WitReaderMask::None;
		}

		const EventBits_t observedBits = xEventGroupWaitBits(resources.witStopped, expectedBits, pdTRUE, pdTRUE, timeoutTicks);
		return toReaderMask(observedBits);
	}
} // namespace app_events