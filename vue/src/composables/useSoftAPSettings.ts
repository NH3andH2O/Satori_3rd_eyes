import { ref, onMounted, onUnmounted } from 'vue';
import i18n from '@/i18n';
import axios from 'axios';
import { ElMessage } from 'element-plus';
import 'element-plus/es/components/message/style/css';

interface SoftAPConfig {
	is_change_password: boolean;
	isLoading: boolean;
	isSaving: boolean;
	ssid: string;
	password: string;
	password_confirm: string;
}

export function useSoftAPSettings() {
	const config = ref<SoftAPConfig>({
		is_change_password: false,
		isLoading: true,
		isSaving: false,
		ssid: '',
		password: '',
		password_confirm: '',
	});

	/* 獲取熱點設定 */
	async function fetchSoftAPConfig() {
		config.value.isLoading = true;
		axios
			.get('/api/softap_config', {
				timeout: 5000, // 5秒超時
			})
			.then((response) => {
				if (typeof response.data !== 'object' || !('ssid' in response.data)) {
					throw new Error('Invalid response from server');
				}
				config.value = {
					...config.value,
					...response.data,
				};
			})
			.catch((error) => {
				console.error('Failed to fetch SoftAP config:', error);
				ElMessage.error(i18n.global.t('setting_transmission_failed') + `: ${error.message || error}`);
			})
			.finally(() => {
				config.value.isLoading = false;
			});
	}

	onMounted(() => {
		fetchSoftAPConfig();
	});

	onUnmounted(() => {});

	async function update() {
		config.value.isSaving = true;
		const { ssid, is_change_password: isChangePassword, password, password_confirm } = config.value;

		const newConfig: Partial<SoftAPConfig> = {
			ssid,
			is_change_password: isChangePassword,
			password: password || undefined,
			password_confirm: password_confirm || undefined,
		};

		axios
			.post('/api/set_softap_config', newConfig)
			.then((response) => {
				if (response.data.success) {
					console.log('SoftAP setting updated:', response.data);
					ElMessage.success(i18n.global.t('softAP_setting_successfully'));
				} else {
					throw new Error(response.data.message || 'Unknown error');
				}
			})
			.catch((error) => {
				console.error('Failed to update SoftAP config:', error);
				ElMessage.error(i18n.global.t('softAP_setting_failed') + `: ${error.message || error}`);
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
