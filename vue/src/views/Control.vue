<script setup lang="ts">
import { useI18n } from 'vue-i18n';
import { computed, watchEffect, onMounted, onBeforeUnmount, ref, watch } from 'vue';
import { useWebSocket } from '@/composables/useWebSocket';
import VirtualJoystick from '@/components/VirtualJoystick.vue';
import SettingsBottom from '@/components/SettingsBottom.vue';
import '@/components/BackToPrev.vue';

type MovePayload = { vx: number; vy: number };
type EyelidPayload = { vy: number };

const { t } = useI18n();
const { connect, disconnect, send, isOpen } = useWebSocket();
const moveLog = ref<MovePayload>({ vx: 0, vy: 0 });
const eyelidLog = ref<EyelidPayload>({ vy: 0 });
const SettingVisible = ref(false);

const show_help = ref(localStorage.getItem('show_help') === null ? true : localStorage.getItem('show_help') === 'true');
const move_X_invert = ref(localStorage.getItem('move_X_invert') === 'true');
const move_Y_invert = ref(localStorage.getItem('move_Y_invert') === 'true');
const eyelid_Y_invert = ref(localStorage.getItem('eyelid_Y_invert') === 'true');

const width = ref(window.innerWidth);
function handleResize() {
	width.value = window.innerWidth;
}

const isMobile = computed(() => width.value <= 768);

watch(move_X_invert, (newVal) => {
	localStorage.setItem('move_X_invert', String(newVal));
});

watch(move_Y_invert, (newVal) => {
	localStorage.setItem('move_Y_invert', String(newVal));
});

watch(eyelid_Y_invert, (newVal) => {
	localStorage.setItem('eyelid_Y_invert', String(newVal));
});

watch(show_help, (newVal) => {
	localStorage.setItem('show_help', String(newVal));
});

watchEffect(() => {
	document.title = t('controlTitle');
});

onMounted(() => {
	connect();
	window.addEventListener('resize', handleResize);
});

onBeforeUnmount(() => {
	disconnect(1000, 'leave /control');
	window.removeEventListener('resize', handleResize);
});

// rAF 合併：同一幀只送最新一次
let movePending: MovePayload | null = null;
let moveRafId = 0;
let moveIsEnded = false;

let eyelidPending: EyelidPayload | null = null;
let eyelidRafId = 0;
let eyelidIsEnded = false;

function move_flushPending() {
	if (!movePending) return;
	const ok = typeof isOpen === 'function' ? isOpen() : true;

	if (ok) {
		if (moveIsEnded) {
			// 如果已經結束，則不再發送移動訊號
			moveIsEnded = false;
		} else {
			send?.('joystick/move/move', movePending);
		}
	}
	movePending = null;
	moveRafId = 0;
}

function eyelid_flushPending() {
	if (!eyelidPending) return;
	const ok = typeof isOpen === 'function' ? isOpen() : true;

	if (ok) {
		if (eyelidIsEnded) {
			// 如果已經結束，則不再發送移動訊號
			eyelidIsEnded = false;
		} else {
			send?.('joystick/eyelid/move', eyelidPending);
		}
	}
	eyelidPending = null;
	eyelidRafId = 0;
}

function move_onMove(p: MovePayload) {
	// 反轉處理
	const vx = move_X_invert.value ? -p.vx : p.vx;
	const vy = move_Y_invert.value ? p.vy : -p.vy;

	// 更新 log
	moveLog.value = { vx, vy };

	// 保持原有的 pending / raf 流程
	movePending = { ...moveLog.value };
	if (!moveRafId) {
		moveRafId = requestAnimationFrame(move_flushPending);
	}
}

function eyelid_onMove(p: MovePayload) {
	// 反轉處理
	const vy = eyelid_Y_invert.value ? p.vy : -p.vy;

	// 更新 log
	eyelidLog.value = { vy };

	// 保持原有的 pending / raf 流程
	eyelidPending = { ...eyelidLog.value };
	if (!eyelidRafId) {
		eyelidRafId = requestAnimationFrame(eyelid_flushPending);
	}
}

function move_onEnd() {
	// 送出結束訊號
	moveIsEnded = true;
	send?.('joystick/move/end', {});
}

function eyelid_onEnd() {
	// 送出結束訊號
	eyelidIsEnded = true;
	send?.('joystick/eyelid/end', {});
}
</script>

<template>
	<section class="control-move" ref="controlEl">
		<VirtualJoystick
			mode="dynamic"
			:size="140"
			color="#22c55e"
			:deadzone="0.12"
			:throttle="60"
			:scale="100"
			:invertYPositive="true"
			@move="move_onMove"
			@end="move_onEnd"
		/>
	</section>
	<section class="control-eyelid" ref="controlEl">
		<VirtualJoystick
			mode="dynamic"
			:size="140"
			color="#f548fdc5"
			:deadzone="0.12"
			:throttle="60"
			:scale="100"
			:invertYPositive="true"
			@move="eyelid_onMove"
			@end="eyelid_onEnd"
		/>
	</section>
	<SettingsBottom :visibility-height="0" :bottom="110" @click="SettingVisible = true" />
	<BackToPrev :visibility-height="0" return-to="/" />
	<el-dialog v-model="show_help" width="500" align-center :title="t('tutorial')">
		<div class="setting-content">
			<p v-if="isMobile">{{ t('control_help_mobile') }}</p>
			<p v-else>{{ t('control_help_desktop') }}</p>
		</div>
	</el-dialog>
	<el-dialog v-model="SettingVisible" width="500" align-center :title="t('setting')">
		<div class="setting-content">
			<h3>{{ t('move_inverted_controls') }}</h3>
			<el-row>
				<el-col :span="12">
					<div class="setting-item">
						<p>{{ t('invert_Xaxis') }}</p>
						<el-switch v-model="move_X_invert" />
					</div>
				</el-col>
				<el-col :span="12">
					<div class="setting-item">
						<p>{{ t('invert_Yaxis') }}</p>
						<el-switch v-model="move_Y_invert" />
					</div>
				</el-col>
			</el-row>
			<h3>{{ t('eyelid_inverted_controls') }}</h3>
			<div class="setting-item">
				<p>{{ t('invert_Yaxis') }}</p>
				<el-switch v-model="eyelid_Y_invert" />
			</div>
			<h3>{{ t('help') }}</h3>
			<div class="setting-item">
				<el-button type="primary" round plain @click="show_help = true">{{ t('help_button') }}</el-button>
			</div>
		</div>
	</el-dialog>
</template>

<style scoped>
/* 通用样式 */
.control-move,
.control-eyelid {
	position: fixed;
	margin: 0;
	padding: 0;
	background: transparent;
	touch-action: none;
	padding-bottom: env(safe-area-inset-bottom); /* iOS 安全区 */
	z-index: 1;
}

/* 手机端：上下布局 */
@media (max-width: 768px) {
	.control-eyelid {
		top: 0;
		left: 0;
		width: 100vw;
		height: 50vh;
	}

	.control-move {
		bottom: 0;
		left: 0;
		width: 100vw;
		height: 50vh;
	}
}

/* 平板与电脑端：左右布局 */
@media (min-width: 769px) {
	.control-eyelid {
		top: 0;
		left: 0;
		width: 50vw;
		height: 100vh;
	}

	.control-move {
		top: 0;
		right: 0;
		width: 50vw;
		height: 100vh;
	}
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
