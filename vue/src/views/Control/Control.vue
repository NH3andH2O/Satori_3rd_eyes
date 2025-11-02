<script setup lang="ts">
import { useI18n } from 'vue-i18n';
import { watchEffect, onMounted, onBeforeUnmount, ref } from 'vue';
import { useWebSocket } from '@/composables/useWebSocket';
import { useLocalStorage } from '@/composables/useLocalStorage';
import { useJoystickControl } from './composables/useJoystickControl';
import { useResponsive } from './composables/useResponsive';
import VirtualJoystick from '@/components/VirtualJoystick.vue';
import SettingsBottom from '@/components/SettingsBottom.vue';
import '@/components/BackToPrev.vue';
import { StorageKeys, APP_CONFIG, WS_MESSAGE_TYPES } from '@/config';

/*  Composables  */
const { t } = useI18n();
const { connect, disconnect } = useWebSocket();
const { isMobile } = useResponsive();

/* 本地存儲 */
const showHelp = useLocalStorage(StorageKeys.SHOW_HELP, true);
const moveXInvert = useLocalStorage(StorageKeys.MOVE_X_INVERT, false);
const moveYInvert = useLocalStorage(StorageKeys.MOVE_Y_INVERT, false);
const eyelidYInvert = useLocalStorage(StorageKeys.EYELID_Y_INVERT, false);

/* 組件狀態 */
const settingVisible = ref(false);

/** 搖桿控制 **/
/* 移動控制（雙軸） */
const moveControl = useJoystickControl({
	messageType: WS_MESSAGE_TYPES.JOYSTICK_MOVE,
	endMessageType: WS_MESSAGE_TYPES.JOYSTICK_MOVE_END,
	invertX: moveXInvert,
	invertY: moveYInvert,
});

/* 眼瞼控制（單軸） */
const eyelidControl = useJoystickControl({
	messageType: WS_MESSAGE_TYPES.JOYSTICK_EYELID,
	endMessageType: WS_MESSAGE_TYPES.JOYSTICK_EYELID_END,
	invertY: eyelidYInvert,
	singleAxis: true,
});

/* 生命週期 */
watchEffect(() => {
	document.title = t('controlpage.page_title') as string;
});

onMounted(() => {
	connect();
});

onBeforeUnmount(() => {
	disconnect(1000, 'leave /control');
});
</script>

<template>
	<!-- 移動控制區域 -->
	<section class="control-move">
		<VirtualJoystick
			:mode="APP_CONFIG.joystick.move.mode"
			:size="APP_CONFIG.joystick.move.size"
			:color="APP_CONFIG.joystick.move.color"
			:deadzone="APP_CONFIG.joystick.move.deadzone"
			:throttle="APP_CONFIG.joystick.move.throttle"
			:scale="APP_CONFIG.joystick.move.scale"
			:invert-y-positive="true"
			@move="moveControl.onMove"
			@end="moveControl.onEnd"
		/>
	</section>

	<!-- 眼瞼控制區域 -->
	<section class="control-eyelid">
		<VirtualJoystick
			:mode="APP_CONFIG.joystick.eyelid.mode"
			:size="APP_CONFIG.joystick.eyelid.size"
			:color="APP_CONFIG.joystick.eyelid.color"
			:deadzone="APP_CONFIG.joystick.eyelid.deadzone"
			:throttle="APP_CONFIG.joystick.eyelid.throttle"
			:scale="APP_CONFIG.joystick.eyelid.scale"
			:invert-y-positive="true"
			@move="eyelidControl.onMove"
			@end="eyelidControl.onEnd"
		/>
	</section>

	<!-- 設置按鈕 -->
	<SettingsBottom :visibility-height="0" :bottom="110" @click="settingVisible = true" />

	<!-- 返回按鈕 -->
	<BackToPrev :visibility-height="0" return-to="/" />

	<!-- 教程對話框 -->
	<el-dialog v-model="showHelp" width="500" align-center :title="t('control_set.help.tutorial.title')">
		<div class="setting-content">
			<p v-if="isMobile">
				{{ t('control_set.help.tutorial.mobile') }}
			</p>
			<p v-else>
				{{ t('control_set.help.tutorial.desktop') }}
			</p>
		</div>
	</el-dialog>

	<!-- 設置對話框 -->
	<el-dialog v-model="settingVisible" width="500" align-center :title="t('control_set.title')">
		<div class="setting-content">
			<!-- 移動反轉設置 -->
			<h3>{{ t('control_set.move_inverted.title') }}</h3>
			<el-row>
				<el-col :span="12">
					<div class="setting-item">
						<p>{{ t('control_set.move_inverted.invert_Xaxis') }}</p>
						<el-switch v-model="moveXInvert" />
					</div>
				</el-col>
				<el-col :span="12">
					<div class="setting-item">
						<p>{{ t('control_set.move_inverted.invert_Yaxis') }}</p>
						<el-switch v-model="moveYInvert" />
					</div>
				</el-col>
			</el-row>

			<!-- 眼瞼反轉設置 -->
			<h3>{{ t('control_set.eyelid_inverted.title') }}</h3>
			<div class="setting-item">
				<p>{{ t('control_set.eyelid_inverted.invert_Yaxis') }}</p>
				<el-switch v-model="eyelidYInvert" />
			</div>

			<!-- 幫助設置 -->
			<h3>{{ t('control_set.help.title') }}</h3>
			<div class="setting-item">
				<el-button type="primary" round plain @click="showHelp = true">
					{{ t('control_set.help.tutorial_button') }}
				</el-button>
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
