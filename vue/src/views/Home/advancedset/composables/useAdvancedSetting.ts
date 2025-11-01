import { ref, onMounted } from 'vue';
import { storeToRefs } from 'pinia';
import { useModeStore } from '@/stores/mode';
import { useCorrectionStore } from '@/stores/correction';
import { advancedApi, handleApiError, handleApiSuccess } from '@/api';
import { AppError } from '@/utils/AppError';
import { getErrorDescription } from '@/utils/errorCodeMapper';
import i18n from '@/i18n';

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
			const data = await advancedApi.getConfig();
			correctionStore.patchFromServer({
				correction_timer: typeof data.correction_timer === 'number' ? data.correction_timer : undefined,
			});
		} catch (error: unknown) {
			handleApiError(error, i18n.global.t('setting_transmission_failed'));
		} finally {
			isLoading.value = false;
		}
	}

	async function update() {
		isSaving.value = true;
		try {
			const payloadBuilders: Record<number, () => Partial<{ correction_timer: number }>> = {
				1: () => ({ correction_timer: correction_timer.value }),
			};

			const payloadBuilder = payloadBuilders[mode.value];
			if (!payloadBuilder) {
				const errorCode = -10301;
				const errorMessage = getErrorDescription(errorCode);
				throw new AppError(errorCode, errorMessage);
			}

			const payload = payloadBuilder();

			await advancedApi.setConfig(payload);
			handleApiSuccess(i18n.global.t('advanced_setting_successfully'));
		} catch (error: unknown) {
			handleApiError(error, i18n.global.t('advanced_setting_failed'));
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
