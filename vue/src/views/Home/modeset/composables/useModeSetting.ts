import { ref, onMounted } from 'vue';
import axios from 'axios';
import { ElMessage } from 'element-plus';
import i18n from '@/i18n';
import { storeToRefs } from 'pinia';
import { useModeStore } from '@/stores/mode';

export function useModeSetting() {
	// 局部 UI 状态
	const isLoading = ref(true);
	const isSaving = ref(false);

	// 业务状态来自 Pinia
	const modeStore = useModeStore();
	const { mode } = storeToRefs(modeStore);

	async function fetchModeConfig() {
		isLoading.value = true;
		try {
			const resp = await axios.get('/api/mode_config', { timeout: 5000 });
			const data = resp.data;
			if (typeof data !== 'object' || data == null || !('mode' in data)) {
				throw new Error('Invalid response from server');
			}
			modeStore.patchFromServer({
				mode: Number(data.mode),
			});
		} catch (error: any) {
			console.error('Failed to fetch mode config:', error);
			ElMessage.error(i18n.global.t('setting_transmission_failed') + `: ${error?.message || error}`);
		} finally {
			isLoading.value = false;
		}
	}

	async function update() {
		isSaving.value = true;
		try {
			const payloadBuilders: Record<number, () => any> = {
				1: () => ({ mode: mode.value }),
				2: () => ({ mode: mode.value }),
			};
			const payload = (
				payloadBuilders[mode.value] ??
				(() => {
					throw new Error(`Unsupported mode value: ${mode.value}`);
				})
			)();

			const resp = await axios.post('/api/set_mode_config', payload);
			if (resp?.data?.success) {
				ElMessage.success(i18n.global.t('mode_setting_successfully'));
			} else {
				throw new Error(resp?.data?.message || 'Unknown error');
			}
		} catch (error: any) {
			console.error('Failed to update mode config:', error);
			ElMessage.error(i18n.global.t('mode_setting_failed') + `: ${error?.message || error}`);
		} finally {
			isSaving.value = false;
		}
	}

	onMounted(fetchModeConfig);

	return {
		// 局部 UI 状态
		isLoading,
		isSaving,
		// 来自 Pinia 的业务状态（双向绑定用）
		mode,
		// 操作
		fetchModeConfig,
		update,
	};
}
