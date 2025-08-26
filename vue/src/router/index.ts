import { createRouter, createWebHistory } from 'vue-router';
import type { RouteRecordRaw } from 'vue-router';
import Home from '@/views/Home.vue';

const routes: Array<RouteRecordRaw> = [{ path: '/', name: 'Home', component: Home }];

const router = createRouter({
	history: createWebHistory(),
	routes,
});

export default router;
