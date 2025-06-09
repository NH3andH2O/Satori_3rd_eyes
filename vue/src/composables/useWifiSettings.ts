// src/composables/useWifiSettings.ts
import { ref, onMounted, onUnmounted } from 'vue'
import { useWebSocket } from './useWebSocket'

interface WifiConfig {
	iswifi: boolean
	wifi_ssid: string
	wifi_password: string
}

export function useWifiSettings() {
	const { on, off, send } = useWebSocket()
	const config = ref<WifiConfig>({
		iswifi: false,
		wifi_ssid: '',
		wifi_password: '',
	})

	// 处理服务器下发的当前 Wi-Fi 设置
	function handleUpdate(payload: WifiConfig) {
		config.value = payload
	}

	onMounted(() => {
		on('wifi_config', handleUpdate)
	})

	onUnmounted(() => {
		off('wifi_config', handleUpdate)
	})

	// 用户更改 Wi-Fi
	function update(newConfig: Partial<WifiConfig>) {
		send('set_wifi_config', newConfig)
	}

	return {
		config,
		update,
	}
}
