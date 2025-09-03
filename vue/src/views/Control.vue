<script setup lang="ts">
import { useI18n } from 'vue-i18n';
import { watchEffect, onMounted, onBeforeUnmount } from 'vue';
import { useWebSocket } from '@/composables/useWebSocket';
import '@/components/BackToPrev.vue';

const { t } = useI18n();
const { connect, disconnect } = useWebSocket();

watchEffect(() => {
	document.title = t('controlTitle');
});

onMounted(() => {
	connect();
});

onBeforeUnmount(() => {
	disconnect(1000, 'leave /control');
});
</script>

<template>
	<div class="title">
		<h1>{{ $t('controlTitle') }}</h1>
	</div>
	<el-row :gutter="24">
		<el-col :sx="0" :md="6" />
		<el-col :sx="24" :md="12">
			<div class="grid-content semi-transparent">
				<p>text</p>
			</div>
		</el-col>
		<el-col :sx="0" :md="6" />
	</el-row>
	<BackToPrev :visibility-height="0" return-to="/" />
</template>

<style src="@/styles/page.css" scoped></style>
