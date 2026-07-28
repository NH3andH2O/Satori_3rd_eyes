import { createI18n } from 'vue-i18n';
import zh_tw from './locales/zh-TW';
import zh_cn from './locales/zh-CN';
import en from './locales/en';

function getLocaleFromBrowser(): string {
	const lang = navigator.language.toLowerCase();
	const map: Record<string, string> = {
		'zh-tw': 'zh-TW',
		'zh-cn': 'zh-CN',
		zh: 'zh-CN',
		en: 'en',
		'en-us': 'en',
		'en-gb': 'en',
	};

	for (const key in map) {
		if (lang.includes(key)) return map[key] || 'en';
	}

	return 'en'; // fallback
}

/**
 * 將 i18n locale 轉換為 HTML lang 屬性格式
 */
function localeToHtmlLang(locale: string): string {
	const map: Record<string, string> = {
		'zh-TW': 'zh-TW',
		'zh-CN': 'zh-CN',
		en: 'en',
	};
	return map[locale] || 'en';
}

/**
 * 更新 HTML lang 屬性
 */
export function updateHtmlLang(locale: string): void {
	const htmlLang = localeToHtmlLang(locale);
	document.documentElement.setAttribute('lang', htmlLang);
}

const i18n = createI18n({
	legacy: false,
	globalInjection: true,
	locale: getLocaleFromBrowser(), // 根據瀏覽器語系設定
	fallbackLocale: 'en',
	messages: {
		'zh-TW': zh_tw,
		'zh-CN': zh_cn,
		en,
	},
});

// 初始化時設置 HTML lang 屬性
updateHtmlLang(i18n.global.locale.value);

export default i18n;
