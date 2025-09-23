<script setup lang="ts">
import { useI18n } from 'vue-i18n';
import { watchEffect, onMounted, onBeforeUnmount, ref, watch } from 'vue';
import { useWebSocket } from '@/composables/useWebSocket';
import VirtualJoystick from '@/components/VirtualJoystick.vue';
import SettingsBottom from '@/components/SettingsBottom.vue';
import '@/components/BackToPrev.vue';

const { t } = useI18n();
const { connect, disconnect } = useWebSocket();
const log = ref({ vx: 0, vy: 0, x: 0, y: 0 });
const SettingVisible = ref(false);

const show_help = ref(localStorage.getItem('show_help') === null ? true : localStorage.getItem('show_help') === 'true');
const X_invert = ref(localStorage.getItem('X_invert') === 'true');
const Y_invert = ref(localStorage.getItem('Y_invert') === 'true');

watch(X_invert, (newVal) => {
	localStorage.setItem('X_invert', String(newVal));
});

watch(Y_invert, (newVal) => {
	localStorage.setItem('Y_invert', String(newVal));
});

watch(show_help, (newVal) => {
	localStorage.setItem('show_help', String(newVal));
});

watchEffect(() => {
	document.title = t('controlTitle');
});

onMounted(() => {
	connect();
});

onBeforeUnmount(() => {
	disconnect(1000, 'leave /control');
});

function onMove(p: any) {
	log.value = { vx: p.vx, vy: p.vy, x: p.x, y: p.y };
}
</script>

<template>
	<section class="control-full" ref="controlEl">
		<VirtualJoystick
			mode="dynamic"
			:size="140"
			color="#22c55e"
			:deadzone="0.12"
			:throttle="60"
			:scale="100"
			:invertYPositive="true"
			@move="onMove"
		/>
	</section>
	<SettingsBottom :visibility-height="0" :bottom="110" @click="SettingVisible = true" />
	<BackToPrev :visibility-height="0" return-to="/" />
	<el-dialog v-model="show_help" width="500" align-center :title="t('tutorial')">
		<div class="setting-content">
			<p>{{ t('control_help') }}</p>
		</div>
	</el-dialog>
	<el-dialog v-model="SettingVisible" width="500" align-center :title="t('setting')">
		<div class="setting-content">
			<h3>{{ t('inverted_controls') }}</h3>
			<el-row>
				<el-col :span="12">
					<div class="setting-item">
						<p>{{ t('invert_Xaxis') }}</p>
						<el-switch v-model="X_invert" />
					</div>
				</el-col>
				<el-col :span="12">
					<div class="setting-item">
						<p>{{ t('invert_Yaxis') }}</p>
						<el-switch v-model="Y_invert" />
					</div>
				</el-col>
			</el-row>
			<h3>{{ t('help') }}</h3>
			<div class="setting-item">
				<el-button type="primary" round plain @click="show_help = true">{{ t('help_button') }}</el-button>
			</div>
		</div>
	</el-dialog>
</template>

<style scoped>
.control-full {
	position: fixed;
	left: 0;
	right: 0;
	bottom: 0;
	height: 100vh;
	width: 100vw;
	margin: 0;
	padding: 0;
	background: transparent;
	touch-action: none;
	padding-bottom: env(safe-area-inset-bottom); /* iOS 安全区 */
	z-index: 1;
}

.dialog-title {
	text-align: left;
	font-weight: 600;
	color: black;
}

.setting-content {
	display: flexbox;
}

.setting-content h3 {
	text-align: center;
	color: rgb(64, 235, 235);
}

.setting-content p {
	color: black;
}

.setting-item {
	display: flex;
	text-align: center;
	align-items: center;
	justify-content: center;
	gap: 10px;
	margin-bottom: 12px;
}

@media (prefers-color-scheme: dark) {
	.setting-content h3 {
		color: rgb(74, 250, 177);
	}

	.setting-content p {
		color: rgb(255, 255, 255);
	}
}
</style>

<style src="@/styles/page.css" scoped />
