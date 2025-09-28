import { ref, onMounted } from 'vue';
import i18n from '@/i18n';
import axios from 'axios';
import { ElMessage } from 'element-plus';
import { storeToRefs } from 'pinia';
import { useModeStore } from '@/stores/mode';
import { useCorrectionStore } from '@/stores/correction';

export function useAdvancedSetting() {
	/* 局部 UI 状态 */
	const isLoading = ref(true);
	const isSaving = ref(false);

	/* Pinia */
	const modeStore = useModeStore();
	const correctionStore = useCorrectionStore();
	const { mode } = storeToRefs(modeStore);
	const { correction_timer } = storeToRefs(correctionStore);

	async function fetchAdvancedConfig() {
		isLoading.value = true;
		try {
			const resp = await axios.get('/api/advanced_config', { timeout: 5000 });
			const data = resp.data;
			if (typeof data !== 'object' || data == null) {
				throw new Error('Invalid response from server');
			}
			correctionStore.patchFromServer({
				correction_timer: typeof data.correction_timer === 'number' ? data.correction_timer : undefined,
			});
		} catch (error: any) {
			console.error('Failed to fetch advanced config:', error);
			ElMessage.error(i18n.global.t('setting_transmission_failed') + `: ${error?.message || error}`);
		} finally {
			isLoading.value = false;
		}
	}

	async function update() {
		isSaving.value = true;
		try {
			const payloadBuilders: Record<number, () => any> = {
				1: () => ({ correction_timer: correction_timer.value }),
			};
			const payload = (
				payloadBuilders[mode.value] ??
				(() => {
					throw new Error(`Unsupported mode value: ${mode.value}`);
				})
			)();

			const resp = await axios.post('/api/set_advanced_config', payload);
			if (resp?.data?.success) {
				ElMessage.success(i18n.global.t('advanced_setting_successfully'));
			} else {
				throw new Error(resp?.data?.message || 'Unknown error');
			}
		} catch (error: any) {
			console.error('Failed to update advanced config:', error);
			ElMessage.error(i18n.global.t('advanced_setting_failed') + `: ${error?.message || error}`);
		} finally {
			isSaving.value = false;
		}
	}

	onMounted(fetchAdvancedConfig);

	return {
		// 局部 UI 状态
		isLoading,
		isSaving,
		// 来自 Pinia 的业务状态（双向绑定用）
		correction_timer,
		// 操作
		fetchAdvancedConfig,
		update,
	};
}
