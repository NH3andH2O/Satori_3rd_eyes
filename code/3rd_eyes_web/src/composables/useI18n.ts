import { useI18n as vueUseI18n } from 'vue-i18n';
import { updateHtmlLang } from '@/i18n';

/**
 * 擴展的 i18n composable
 * 提供額外的功能，如自動更新 HTML lang 屬性
 */
export function useI18n() {
	const i18n = vueUseI18n();

	/**
	 * 切換語言並更新 HTML lang 屬性
	 */
	const setLocale = (locale: string) => {
		i18n.locale.value = locale;
		updateHtmlLang(locale);
	};

	return {
		...i18n,
		setLocale,
	};
}
