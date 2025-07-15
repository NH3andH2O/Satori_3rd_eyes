import { ref, onMounted, onUnmounted } from 'vue';
import i18n from '@/i18n';
import axios from 'axios';
import { ElMessage } from 'element-plus';
import 'element-plus/es/components/message/style/css';

interface ModeConfig {
	isLoading: boolean;
	isSaving: boolean;
	mode: number;
	correction_timer: number;
}

export function useModeSetting() {
	const config = ref<ModeConfig>({
		isLoading: true,
		isSaving: false,
		mode: 0,
		correction_timer: 0,
	});

	async function fetchModeConfig() {
		config.value.isLoading = true;
		axios
			.get('/api/mode_config', {
				timeout: 5000,
			})
			.then((response) => {
				if (typeof response.data !== 'object' || !('mode' in response.data)) {
					throw new Error('Invalid response from server');
				}
				config.value = {
					...config.value,
					...response.data,
				};
			})
			.catch((error) => {
				console.error('Failed to fetch mode config:', error);
				ElMessage.error(i18n.global.t('setting_transmission_failed') + `: ${error.message || error}`);
			})
			.finally(() => {
				config.value.isLoading = false;
			});
	}

	onMounted(() => {
		fetchModeConfig();
	});

	onUnmounted(() => {});

	async function update() {
		config.value.isSaving = true;
		const { mode, correction_timer } = config.value;
		let newConfig: Partial<ModeConfig>;
		if (mode === 1) {
			newConfig = {
				mode: mode,
				correction_timer: correction_timer,
			};
		} else {
			newConfig = {
				mode: mode,
			};
		}
		axios
			.post('/api/set_mode_config', newConfig)
			.then((response) => {
				if (response.data.success) {
					ElMessage.success(i18n.global.t('mode_setting_successfully'));
				} else {
					throw new Error(response.data.message || 'Unknown error');
				}
			})
			.catch((error) => {
				console.error('Failed to update mode config:', error);
				ElMessage.error(i18n.global.t('mode_setting_failed') + `: ${error.message || error}`);
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
