<script setup lang="ts">
import { useI18n } from 'vue-i18n';
import { watchEffect } from 'vue';
import wifiset from '@/components/wifiset.vue';
import modeset from '@/components/modeset.vue';
import advancedset from '@/components/advancedset.vue';

const { t } = useI18n();

watchEffect(() => {
	document.title = t('pageTitle');
});
</script>

<template>
	<div class="title">
		<h1>{{ $t('title') }}</h1>
	</div>
	<el-row :gutter="24">
		<el-col :sx="24" :md="8">
			<div class="grid-content semi-transparent">
				<modeset />
			</div>
		</el-col>
		<el-col :sx="24" :md="8">
			<div class="grid-content semi-transparent">
				<advancedset />
			</div>
		</el-col>
		<el-col :sx="24" :md="8">
			<div class="grid-content semi-transparent">
				<wifiset />
			</div>
		</el-col>
	</el-row>
</template>

<style src="@/styles/page.css" scoped></style>
