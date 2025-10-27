import { ref, onMounted } from 'vue';
import { softAPApi, handleApiError, handleApiSuccess } from '@/api';
import i18n from '@/i18n';

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
		try {
			const data = await softAPApi.getConfig();
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
		fetchSoftAPConfig();
	});

	async function update() {
		config.value.isSaving = true;
		const { ssid, is_change_password: isChangePassword, password, password_confirm } = config.value;

		const newConfig = {
			ssid,
			is_change_password: isChangePassword,
			password: password || undefined,
			password_confirm: password_confirm || undefined,
		};

		try {
			await softAPApi.setConfig(newConfig);
			handleApiSuccess(i18n.global.t('softAP_setting_successfully'));
		} catch (error: any) {
			handleApiError(error, i18n.global.t('softAP_setting_failed'));
		} finally {
			config.value.isSaving = false;
		}
	}

	return {
		config,
		update,
	};
}
