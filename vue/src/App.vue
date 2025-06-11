<script setup lang="ts">
import { useI18n } from 'vue-i18n'
import { watchEffect } from 'vue'
import wifiset from './components/wifiset.vue'
import modeset from './components/modeset.vue'
import advancedset from './components/advancedset.vue'

const { t } = useI18n()

watchEffect(() => {
	document.title = t('pageTitle')
})
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

<style scoped>
.title {
	color: rgb(255, 233, 36);
}

.el-row {
	margin-bottom: 20px;
}

.el-row:last-child {
	margin-bottom: 4px;
}

.el-col {
	border-radius: 4px;
}

.grid-content {
	border-radius: 4px;
	min-height: 36px;
	margin-bottom: 20px;
}

.semi-transparent {
	background-color: rgba(0, 0, 0, 0.164);
	backdrop-filter: blur(10px);
	/* 毛玻璃效果 */
	-webkit-backdrop-filter: blur(12px);
	padding: 1px 10px 10px 10px;
	border-radius: 12px;
	display: block;
}

@media (prefers-color-scheme: dark) {
	.title {
		color: rgb(184, 165, 0);
	}

	.semi-transparent {
		background-color: rgba(255, 255, 255, 0.205);
	}
}
</style>
