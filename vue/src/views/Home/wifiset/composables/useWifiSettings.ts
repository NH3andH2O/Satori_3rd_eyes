import { ref, onMounted, onUnmounted } from 'vue';
import i18n from '@/i18n';
import axios from 'axios';
import { ElMessage } from 'element-plus';
import 'element-plus/es/components/message/style/css';

interface WifiConfig {
	is_wifi: boolean;
	isLoading: boolean;
	isSaving: boolean;
	ssid: string;
	password: string;
}

export function useWifiSettings() {
	const config = ref<WifiConfig>({
		is_wifi: false,
		ssid: '',
		password: '',
		isSaving: false,
		isLoading: true,
	});

	/* 獲取wifi設定 */
	async function fetchWifiConfig() {
		config.value.isLoading = true;
		axios
			.get('/api/wifi_config', {
				timeout: 5000,
			})
			.then((response) => {
				if (typeof response.data !== 'object' || !('is_wifi' in response.data)) {
					throw new Error('Invalid response from server');
				}
				config.value = {
					...config.value,
					...response.data,
				};
			})
			.catch((error) => {
				console.error('Failed to fetch WiFi config:', error);
				ElMessage.error(i18n.global.t('setting_transmission_failed') + `: ${error.message || error}`);
			})
			.finally(() => {
				config.value.isLoading = false;
			});
	}

	onMounted(() => {
		fetchWifiConfig();
	});

	onUnmounted(() => {});

	/* 更新wifi設定 */
	async function update() {
		config.value.isSaving = true;
		const { is_wifi, ssid, password } = config.value;
		const newConfig: Partial<WifiConfig> = {
			is_wifi: is_wifi,
			ssid: ssid,
			password: password,
		};
		axios
			.post('/api/set_wifi_config', newConfig)
			.then((response) => {
				if (response.data.success) {
					console.log('WiFi config updated successfully:', response.data);
					ElMessage.success(i18n.global.t('wifi_setting_successfully'));
				} else {
					throw new Error(response.data.message || 'Unknown error');
				}
			})
			.catch((error) => {
				console.error('Failed to update WiFi config:', error);
				ElMessage.error(i18n.global.t('setting_transmission_failed') + `: ${error.message || error}`);
			})
			.finally(() => {
				config.value.isSaving = false;
			});
	}

	return {
		config,
		update,
	};
}
