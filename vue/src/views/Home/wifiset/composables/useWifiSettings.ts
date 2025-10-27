import { ref, onMounted } from 'vue';
import { wifiApi, handleApiError, handleApiSuccess } from '@/api';
import i18n from '@/i18n';

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
		try {
			const data = await wifiApi.getConfig();
			config.value = {
				...config.value,
				...data,
			};
		} catch (error: any) {
			handleApiError(error, i18n.global.t('setting_transmission_failed'));
		} finally {
			config.value.isLoading = false;
		}
	}

	onMounted(() => {
		fetchWifiConfig();
	});

	/* 更新wifi設定 */
	async function update() {
		config.value.isSaving = true;
		const { is_wifi, ssid, password } = config.value;
		try {
			await wifiApi.setConfig({ is_wifi, ssid, password });
			handleApiSuccess(i18n.global.t('wifi_setting_successfully'));
		} catch (error: any) {
			handleApiError(error, i18n.global.t('setting_transmission_failed'));
		} finally {
			config.value.isSaving = false;
		}
	}

	return {
		config,
		update,
	};
}
