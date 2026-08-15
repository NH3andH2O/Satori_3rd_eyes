<script setup lang="ts">
import { useI18n } from 'vue-i18n';
import { watchEffect, ref, provide, computed } from 'vue';
import { useRouter } from 'vue-router';
import wifiset from '@/views/Home/wifiset/wifiset.vue';
import modeset from '@/views/Home/modeset/modeset.vue';
import advancedset from '@/views/Home/advancedset/advancedset.vue';
import { servoApi, handleApiError } from '@/api';
import { MODES } from '@/config';

const { t } = useI18n();
const router = useRouter();
const setupCheckPerformed = ref(false);

watchEffect(() => {
	document.title = t('homepage.page_title');
});

// 跟踪每个卡片的加载状态
const loadingStates = ref({
	modeset: true,
	advancedset: true,
	wifiset: true,
});

// 计算所有卡片是否都加载完成
const isAllLoaded = computed(() => {
	return !Object.values(loadingStates.value).some((state) => state === true);
});

// 提供更新加载状态的方法给子组件
const updateLoadingState = (component: keyof typeof loadingStates.value, isLoading: boolean) => {
	loadingStates.value[component] = isLoading;
};

provide('updateLoadingState', updateLoadingState);

async function handleModeLoaded(mode: number) {
	if (setupCheckPerformed.value || mode !== MODES.SERVO_SETUP) return;

	setupCheckPerformed.value = true;
	try {
		const servoConfig = await servoApi.getConfig();
		if (!servoConfig.is_setup) {
			await router.replace({ name: 'Setup' });
		}
	} catch (error: unknown) {
		handleApiError(error, t('setup.state.get_failed'));
	}
}
</script>

<template>
	<div class="title">
		<h1>{{ $t('homepage.title') }}</h1>
	</div>
	<el-row :gutter="24">
		<el-col :sx="24" :md="8">
			<div v-loading="!isAllLoaded" element-loading-background="rgba(0, 0, 0, 0.3)" class="grid-content semi-transparent card-wrapper">
				<modeset @loaded="handleModeLoaded" />
			</div>
		</el-col>
		<el-col :sx="24" :md="8">
			<div v-loading="!isAllLoaded" element-loading-background="rgba(0, 0, 0, 0.3)" class="grid-content semi-transparent card-wrapper">
				<advancedset />
			</div>
		</el-col>
		<el-col :sx="24" :md="8">
			<div v-loading="!isAllLoaded" element-loading-background="rgba(0, 0, 0, 0.3)" class="grid-content semi-transparent card-wrapper">
				<wifiset />
			</div>
		</el-col>
	</el-row>
</template>

<style src="@/styles/page.css" scoped></style>
<style scoped>
.card-wrapper {
	position: relative;
}
</style>
