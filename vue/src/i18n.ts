import { createI18n } from 'vue-i18n'
import zh_tw from './locales/zh-TW'
import zh_cn from './locales/zh-CN'
import en from './locales/en'

function getLocaleFromBrowser(): string {
	const lang = navigator.language.toLowerCase()
	const map: Record<string, string> = {
		'zh-tw': 'zh-TW',
		'zh-cn': 'zh-CN',
		'zh': 'zh-CN',
		'en': 'en',
		'en-us': 'en',
		'en-gb': 'en',
	}

	for (const key in map) {
		if (lang.includes(key)) return map[key]
	}

	return 'en' // fallback
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
})

export default i18n
