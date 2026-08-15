import { ref, onMounted, inject } from 'vue';
import { storeToRefs } from 'pinia';
import { useModeStore } from '@/stores/mode';
import { modeApi, handleApiError, handleApiSuccess } from '@/api';
import i18n from '@/i18n';

export function useModeSetting(onModeLoaded?: (mode: number) => void) {
	// 局部 UI 状态
	const isLoading = ref(true);
	const isSaving = ref(false);

	// 业务状态来自 Pinia
	const modeStore = useModeStore();
	const { mode } = storeToRefs(modeStore);

	// 注入父组件提供的更新方法
	const updateLoadingState = inject<((component: string, isLoading: boolean) => void) | undefined>('updateLoadingState');

	async function fetchModeConfig(): Promise<number | null> {
		isLoading.value = true;
		updateLoadingState?.('modeset', true);
		try {
			const data = await modeApi.getConfig();
			const currentMode = Number(data.mode);
			modeStore.patchFromServer({
				mode: currentMode,
			});
			onModeLoaded?.(currentMode);
			return currentMode;
		} catch (error: unknown) {
			handleApiError(error, i18n.global.t('modeset.state.get_failed'));
			return null;
		} finally {
			isLoading.value = false;
			updateLoadingState?.('modeset', false);
		}
	}

	async function update() {
		isSaving.value = true;
		try {
			await modeApi.setConfig({ mode: mode.value });
			handleApiSuccess(i18n.global.t('modeset.state.set_successfully'));
		} catch (error: unknown) {
			handleApiError(error, i18n.global.t('modeset.state.set_failed'));
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
