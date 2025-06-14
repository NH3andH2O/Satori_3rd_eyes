import { ref, onMounted, onUnmounted } from 'vue'
import i18n from '@/i18n'
import axios from 'axios'
import { ElMessage } from 'element-plus'
import 'element-plus/es/components/message/style/css'

interface WifiConfig {
	iswifi: boolean
	isLoading: boolean
	isSaving: boolean
	wifi_ssid: string
	wifi_password: string
}

export function useWifiSettings() {
	const config = ref<WifiConfig>({
		iswifi: false,
		wifi_ssid: '',
		wifi_password: '',
		isSaving: false,
		isLoading: true,
	})

	/* 獲取wifi設定 */
	async function fetchWifiConfig() {
		try {
			const response = await axios.get('/api/wifi_config', {
				timeout: 5000, // 5秒超時
			})

			if (typeof response.data !== 'object' || !('iswifi' in response.data)) {
				throw new Error('Invalid response from server')
			}

			config.value = response.data
		}
		catch (error: any) {
			console.error('Failed to fetch Wi-Fi config:', error)
			ElMessage.error(i18n.global.t('wifi_setting_transmission_failed') + `: ${error.message || error}`)
		}
		finally {
			config.value.isLoading = false
		}
	}

	onMounted(() => {
		fetchWifiConfig()
	})

	onUnmounted(() => {
	})

	/* 更新wifi設定 */
	async function update(newConfig: Partial<WifiConfig>) {
		config.value.isSaving = true
		try {
			const response = await axios.post('/api/set_wifi_config', newConfig)
			if (response.data.success) {
				console.log('setting updated:', response.data.data)
				ElMessage.success(i18n.global.t('wifi_setting_successfully'))
			} else {
				console.error('setting failed', response.data.message)
				ElMessage.error(`${i18n.global.t('wifi_setting_failed')}: ${response.data.message}`)
			}
		}
		catch (err) {
			console.error('transmission failed:', err)
			const message = err instanceof Error ? err.message : String(err)
			ElMessage.error(`${i18n.global.t('wifi_setting_transmission_failed')}: ${message}`)
		}
		finally {
			config.value.isSaving = false
		}
	}

	return {
		config,
		update,
	}
}
