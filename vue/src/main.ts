import { createApp } from 'vue';
import { createPinia } from 'pinia';
import * as ElementPlusIconsVue from '@element-plus/icons-vue';
import './style.css';
import App from './App.vue';
import i18n from './i18n';
import router from './router';

// 導入 Element Plus 樣式
import 'element-plus/theme-chalk/el-message.css';
import 'element-plus/theme-chalk/el-message-box.css';

const app = createApp(App);

// 全局錯誤處理器
app.config.errorHandler = (err, instance, info) => {
	console.error('全局錯誤捕獲:', err);
	console.error('錯誤組件:', instance);
	console.error('錯誤信息:', info);

	// 可選：使用 Element Plus 顯示錯誤通知
	// ElNotification.error({
	//   title: '應用錯誤',
	//   message: err instanceof Error ? err.message : '未知錯誤',
	// });
};

// 全局警告處理器（開發環境）
if (import.meta.env.DEV) {
	app.config.warnHandler = (msg, instance, trace) => {
		console.warn('Vue 警告:', msg);
		console.warn('警告組件:', instance);
		console.warn('警告追蹤:', trace);
	};
}

// 全局未捕獲的 Promise 錯誤
window.addEventListener('unhandledrejection', (event) => {
	console.error('未處理的 Promise 錯誤:', event.reason);
	event.preventDefault(); // 阻止默認的控制台錯誤輸出
});

// 全局腳本錯誤
window.addEventListener('error', (event) => {
	console.error('全局腳本錯誤:', event.error || event.message);
});

for (const [key, component] of Object.entries(ElementPlusIconsVue)) {
	app.component(key, component);
}
app.use(i18n);
app.use(createPinia());
app.use(router);
app.mount('#app');
