import { ref, onMounted } from 'vue';
import { storeToRefs } from 'pinia';
import { useModeStore } from '@/stores/mode';
import { modeApi, handleApiError, handleApiSuccess } from '@/api';
import i18n from '@/i18n';

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
			const data = await modeApi.getConfig();
			modeStore.patchFromServer({
				mode: Number(data.mode),
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
			await modeApi.setConfig({ mode: mode.value });
			handleApiSuccess(i18n.global.t('mode_setting_successfully'));
		} catch (error: unknown) {
			handleApiError(error, i18n.global.t('mode_setting_failed'));
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
