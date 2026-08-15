<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, watch, watchEffect } from 'vue';
import { useI18n } from 'vue-i18n';
import { WarningFilled } from '@element-plus/icons-vue';
import ServoCalibrationControl from './components/ServoCalibrationControl.vue';
import { useSetupInit } from './composables/useSetupInit';
import { useSetupWizard } from './composables/useSetupWizard';

const { t } = useI18n();
const setupInit = useSetupInit();
const wizard = useSetupWizard(
	setupInit.servoConfig,
	setupInit.canOperate,
);

const showConnectionWarning = computed(() =>
	['disconnected', 'reconnecting', 'failed'].includes(setupInit.connectionState.value),
);

const reconnectMessage = computed(() => {
	if (setupInit.connectionState.value === 'failed') return t('setup.websocket.failed');
	if (setupInit.connectionState.value === 'reconnecting') {
		return t('setup.websocket.reconnecting', {
			attempt: setupInit.reconnectAttempt.value,
			max: setupInit.maxReconnectAttempts,
		});
	}
	return t('setup.websocket.disconnected');
});

const previewStatusText = computed(() => {
	const key = wizard.previewStatus.value;
	return t(`setup.preview_status.${key}`);
});

const saveErrorMessage = computed(() => (wizard.saveError.value ? t(wizard.saveError.value) : ''));

watchEffect(() => {
	document.title = t('setup.page_title');
});

watch(
	() => setupInit.connectionState.value,
	(state) => {
		if (state === 'disconnected' || state === 'reconnecting' || state === 'failed') {
			wizard.pausePreview();
		}
	},
);

watch(setupInit.reconnectToken, () => {
	setupInit.completeReconnect(wizard.restoreCurrentPreview());
});

onMounted(() => {
	void setupInit.initialize();
});

onBeforeUnmount(() => {
	setupInit.cleanup();
});
</script>

<template>
	<div
		v-loading="setupInit.isLoading.value"
		:element-loading-text="setupInit.loadingText.value"
		element-loading-background="rgba(0, 0, 0, 0.55)"
		class="setup-page"
	>
		<div class="title">
			<h1>{{ t('setup.title') }}</h1>
		</div>

		<div v-if="showConnectionWarning" class="connection-warning" role="alert" aria-live="assertive">
			<el-icon :size="28">
				<WarningFilled />
			</el-icon>
			<div class="connection-message">
				<strong>{{ t('setup.websocket.title') }}</strong>
				<span>{{ reconnectMessage }}</span>
			</div>
			<div v-if="setupInit.connectionState.value === 'failed'" class="warning-actions">
				<el-button
					type="danger"
					plain
					:loading="setupInit.isManualReconnecting.value"
					@click="setupInit.manualReconnect"
				>
					{{ t('setup.actions.reconnect') }}
				</el-button>
				<el-button @click="setupInit.goHome">
					{{ t('setup.actions.back_home') }}
				</el-button>
			</div>
		</div>

		<main class="wizard-card semi-transparent">
			<el-steps :active="wizard.currentStepIndex.value" finish-status="success" align-center class="steps">
				<el-step v-for="step in wizard.steps" :key="step.id" :title="t(step.titleKey)" />
			</el-steps>

			<section v-if="!wizard.isReview.value && wizard.currentStep.value" class="step-content">
				<div class="instruction-card">
					<el-tag type="warning" effect="dark" round>
						{{ t('setup.instruction.current_task') }}
					</el-tag>
					<h2>{{ t(wizard.currentStep.value.titleKey) }}</h2>
					<p>{{ t(wizard.currentStep.value.descriptionKey) }}</p>
					<p class="assist-note">
						{{ t('setup.instruction.assist_note') }}
					</p>
				</div>

				<ServoCalibrationControl
					:model-value="wizard.previewPose.value"
					:target-servos="wizard.targetServos.value"
					:disabled="!setupInit.canOperate.value || wizard.isSaving.value"
					@update:model-value="wizard.updatePreviewPose"
					@commit="wizard.flushPreview"
				/>

				<div class="preview-state" :class="`is-${wizard.previewStatus.value}`">
					{{ previewStatusText }}
				</div>
			</section>

			<section v-else-if="wizard.calibration.value" class="review-content">
				<h2>{{ t('setup.review.title') }}</h2>
				<p>{{ t('setup.review.description') }}</p>
				<el-descriptions :column="1" border>
					<el-descriptions-item :label="t('setup.review.eyelid_open')">
						{{ wizard.calibration.value.min_upper_eyelid }} / {{ wizard.calibration.value.max_lower_eyelid }}
					</el-descriptions-item>
					<el-descriptions-item :label="t('setup.review.eyelid_closed')">
						{{ wizard.calibration.value.max_upper_eyelid }} / {{ wizard.calibration.value.min_lower_eyelid }}
					</el-descriptions-item>
					<el-descriptions-item :label="t('setup.review.eyelid_middle')">
						{{ wizard.calibration.value.mid_upper_eyelid }} / {{ wizard.calibration.value.mid_lower_eyelid }}
					</el-descriptions-item>
					<el-descriptions-item :label="t('setup.review.eyeball_center')">
						{{ wizard.calibration.value.mid_eyeball }}
					</el-descriptions-item>
					<el-descriptions-item :label="t('setup.review.eyeball_left')">
						{{ wizard.calibration.value.min_eyeball }}
					</el-descriptions-item>
					<el-descriptions-item :label="t('setup.review.eyeball_right')">
						{{ wizard.calibration.value.max_eyeball }}
					</el-descriptions-item>
				</el-descriptions>
			</section>

			<el-alert v-if="saveErrorMessage" :title="saveErrorMessage" type="error" show-icon :closable="false" class="save-error" />

			<div class="wizard-actions">
				<el-button :disabled="!wizard.canGoPrevious.value" @click="wizard.previous">
					{{ t('setup.actions.previous') }}
				</el-button>
				<div class="right-actions">
					<el-button :disabled="wizard.isSaving.value" @click="wizard.cancel">
						{{ t('setup.actions.cancel') }}
					</el-button>
					<el-button
						v-if="!wizard.isReview.value"
						type="primary"
						:disabled="!wizard.canGoNext.value"
						@click="wizard.next"
					>
						{{ t('setup.actions.next') }}
					</el-button>
					<el-button
						v-else-if="wizard.modeSwitchFailed.value"
						type="warning"
						:loading="wizard.isSaving.value"
						@click="wizard.retryModeSwitch"
					>
						{{ t('setup.actions.retry_mode') }}
					</el-button>
					<el-button
						v-else
						type="success"
						:loading="wizard.isSaving.value"
						:disabled="!wizard.canComplete.value"
						@click="wizard.complete"
					>
						{{ t('setup.actions.complete') }}
					</el-button>
				</div>
			</div>
		</main>

		<el-dialog
			v-model="setupInit.errorDialogVisible.value"
			:title="setupInit.errorTitle.value"
			width="400"
			align-center
			:close-on-click-modal="false"
			:close-on-press-escape="false"
			:show-close="false"
		>
			<p class="dialog-message">
				{{ setupInit.errorMessage.value }}
			</p>
			<template #footer>
				<el-button :loading="setupInit.isLoading.value" @click="setupInit.initialize">
					{{ t('setup.actions.retry') }}
				</el-button>
				<el-button type="primary" @click="setupInit.goHome">
					{{ t('setup.actions.back_home') }}
				</el-button>
			</template>
		</el-dialog>
	</div>
</template>

<style src="@/styles/page.css" scoped></style>
<style scoped>
.setup-page {
	min-height: calc(100vh - 4rem);
}

.wizard-card {
	max-width: 920px;
	margin: 0 auto;
	padding: 24px;
}

.steps {
	margin-bottom: 28px;
}

.connection-warning {
	position: sticky;
	top: 12px;
	z-index: 20;
	display: flex;
	align-items: center;
	gap: 14px;
	max-width: 920px;
	box-sizing: border-box;
	margin: 0 auto 18px;
	padding: 16px 18px;
	border: 2px solid #f56c6c;
	border-radius: 12px;
	background: #f56c6c;
	color: #fff;
	box-shadow: 0 8px 24px rgba(245, 108, 108, 0.35);
}

.connection-message {
	display: flex;
	flex: 1;
	flex-direction: column;
	gap: 4px;
}

.warning-actions,
.wizard-actions,
.right-actions {
	display: flex;
	align-items: center;
	gap: 10px;
}

.instruction-card {
	margin-bottom: 20px;
	padding: 18px;
	border-left: 5px solid var(--el-color-warning);
	border-radius: 10px;
	background: rgba(230, 162, 60, 0.12);
}

.instruction-card h2 {
	margin: 12px 0 8px;
}

.instruction-card p {
	line-height: 1.6;
}

.assist-note {
	font-weight: 600;
}

.preview-state {
	margin-top: 14px;
	font-weight: 600;
	text-align: right;
}

.preview-state.is-failed {
	color: var(--el-color-danger);
}

.preview-state.is-sent {
	color: var(--el-color-success);
}

.review-content {
	max-width: 680px;
	margin: 0 auto;
}

.save-error {
	margin-top: 20px;
}

.wizard-actions {
	justify-content: space-between;
	margin-top: 24px;
}

.dialog-message {
	line-height: 1.6;
	text-align: center;
}

@media (max-width: 768px) {
	.setup-page {
		min-height: calc(100vh - 4rem);
	}

	.wizard-card {
		padding: 16px 12px;
	}

	.steps :deep(.el-step__title) {
		font-size: 11px;
	}

	.connection-warning {
		align-items: flex-start;
		flex-wrap: wrap;
	}

	.warning-actions {
		width: 100%;
	}

	.warning-actions .el-button {
		flex: 1;
	}

	.wizard-actions {
		align-items: stretch;
		flex-direction: column-reverse;
	}

	.right-actions {
		align-items: stretch;
		flex-direction: column;
	}
}
</style>
