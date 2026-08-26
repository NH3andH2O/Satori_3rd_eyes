import { createRouter, createWebHistory } from 'vue-router';
import type { RouteRecordRaw } from 'vue-router';
import { ElMessage } from 'element-plus';
import { modeApi } from '@/api';
import { MODES } from '@/config';
import i18n from '@/i18n';

const routes: Array<RouteRecordRaw> = [
	{ path: '/', name: 'Home', component: () => import('@/views/Home/Home.vue') },
	{ path: '/control', name: 'Control', component: () => import('@/views/Control/Control.vue') },
	{ path: '/setup', name: 'Setup', component: () => import('@/views/Setup/Setup.vue') },
];

const router = createRouter({
	history: createWebHistory(),
	routes,
});

router.beforeEach(async (to) => {
	if (to.name !== 'Setup') return true;

	try {
		const config = await modeApi.getConfig();
		if (Number(config.mode) === MODES.SERVO_SETUP) return true;

		ElMessage.error(i18n.global.t('setup.mode_error'));
	} catch {
		ElMessage.error(i18n.global.t('setup.state.mode_failed'));
	}

	return { name: 'Home', replace: true };
});

export default router;
