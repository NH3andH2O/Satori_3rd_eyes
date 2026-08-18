<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, watch, watchEffect } from 'vue';
import { useI18n } from 'vue-i18n';
import { Loading } from '@element-plus/icons-vue';
import ServoCalibrationControl from './components/ServoCalibrationControl.vue';
import { useSetupInit } from './composables/useSetupInit';
import { useSetupWizard } from './composables/useSetupWizard';
import { SETUP_PROGRESS_STEPS } from './types';

const { t } = useI18n();
const setupInit = useSetupInit();
const wizard = useSetupWizard(setupInit.servoConfig, setupInit.canOperate);

const saveErrorMessage = computed(() => (wizard.saveError.value ? t(wizard.saveError.value) : ''));
const validationErrorMessage = computed(() => (wizard.validationError.value ? t(wizard.validationError.value) : ''));
const mobileStepTitle = computed(() => {
	if (wizard.phase.value === 'review') return t('setup.review.step_title');
	return wizard.currentStep.value ? t(wizard.currentStep.value.titleKey) : '';
});
const progressPercentage = computed(() => ((wizard.progressStepIndex.value + 1) / SETUP_PROGRESS_STEPS.length) * 100);
const isCompletionWarning = computed(() => wizard.completionState.value === 'debug-mode-warning');
const discardAction = ref<'cancel' | 'restart' | null>(null);
const discardDialogVisible = ref(false);
const discardDialogTitle = computed(() => (discardAction.value === 'restart' ? t('setup.discard.restart_title') : t('setup.discard.cancel_title')));
const discardDialogMessage = computed(() =>
	discardAction.value === 'restart' ? t('setup.discard.restart_message') : t('setup.discard.cancel_message'),
);

function requestDiscard(action: 'cancel' | 'restart') {
	if (!wizard.hasUserAdjustments.value) {
		if (action === 'restart') wizard.restart();
		else void wizard.cancel();
		return;
	}
	discardAction.value = action;
	discardDialogVisible.value = true;
}

function confirmDiscard() {
	const action = discardAction.value;
	discardDialogVisible.value = false;
	if (action === 'restart') wizard.restart();
	else if (action === 'cancel') void wizard.cancel();
}

function clearDiscardAction() {
	discardAction.value = null;
}

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

watch(
	() => wizard.phase.value,
	(phase) => {
		if (phase === 'complete') setupInit.stopConnection();
	},
);

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

		<main class="wizard-card semi-transparent">
			<section v-if="wizard.phase.value === 'welcome'" class="welcome-content">
				<el-result icon="primary" :title="t('setup.welcome.title')" :sub-title="t('setup.welcome.description')">
					<template #extra>
						<div class="welcome-actions">
							<el-button type="primary" size="large" :disabled="!wizard.canStart.value" @click="wizard.start()">
								{{ t('setup.actions.start') }}
							</el-button>
							<el-button
								v-if="wizard.hasExistingSetup.value"
								type="primary"
								plain
								size="large"
								:disabled="!wizard.canStart.value"
								@click="wizard.startFromExistingConfig"
							>
								{{ t('setup.actions.start_from_existing') }}
							</el-button>
						</div>
					</template>
				</el-result>
			</section>

			<template v-else-if="wizard.phase.value === 'calibration' || wizard.phase.value === 'review'">
				<el-steps :active="wizard.progressStepIndex.value" finish-status="success" align-center class="desktop-steps">
					<el-step v-for="step in SETUP_PROGRESS_STEPS" :key="step.id" :title="t(step.titleKey)" />
				</el-steps>
				<div class="mobile-progress">
					<div class="mobile-progress-heading">
						<span>{{ t('setup.progress', { current: wizard.progressStepIndex.value + 1, total: SETUP_PROGRESS_STEPS.length }) }}</span>
						<strong>{{ mobileStepTitle }}</strong>
					</div>
					<el-progress :percentage="progressPercentage" :show-text="false" />
				</div>
			</template>

			<section v-if="wizard.phase.value === 'calibration' && wizard.currentStep.value" class="step-content">
				<div class="instruction-card">
					<h2>{{ t(wizard.currentStep.value.titleKey) }}</h2>
					<p>{{ t(wizard.currentStep.value.descriptionKey) }}</p>
				</div>

				<ServoCalibrationControl
					:model-value="wizard.previewPose.value"
					:target-servos="wizard.targetServos.value"
					:disabled="!setupInit.canOperate.value || wizard.isSaving.value"
					@update:model-value="wizard.updatePreviewPose"
					@commit="wizard.flushPreview"
				/>

				<el-alert
					v-if="validationErrorMessage"
					:title="validationErrorMessage"
					type="error"
					show-icon
					:closable="false"
					class="validation-error"
				/>
			</section>

			<section v-else-if="wizard.phase.value === 'review' && wizard.calibration.value" class="review-content">
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
				<el-alert v-if="saveErrorMessage" :title="saveErrorMessage" type="error" show-icon :closable="false" class="save-error" />
			</section>

			<section v-else-if="wizard.phase.value === 'saving'" class="saving-content" aria-live="polite">
				<el-icon class="is-loading saving-icon" :size="64">
					<Loading />
				</el-icon>
				<h2>{{ t('setup.saving.title') }}</h2>
				<p>{{ t('setup.saving.description') }}</p>
			</section>

			<section v-else-if="wizard.phase.value === 'complete'" class="complete-content">
				<el-result
					:icon="isCompletionWarning ? 'warning' : 'success'"
					:title="isCompletionWarning ? t('setup.complete.debug_title') : t('setup.complete.title')"
					:sub-title="isCompletionWarning ? t('setup.complete.debug_description') : t('setup.complete.description')"
				>
					<template #extra>
						<el-button type="primary" size="large" @click="wizard.goHome">
							{{ t('setup.actions.back_home') }}
						</el-button>
					</template>
				</el-result>
			</section>

			<div v-if="wizard.phase.value === 'calibration' || wizard.phase.value === 'review'" class="wizard-actions">
				<el-button :disabled="!wizard.canGoPrevious.value" @click="wizard.previous">
					{{ t('setup.actions.previous') }}
				</el-button>
				<div class="right-actions">
					<el-button v-if="wizard.isInitialSetup.value" type="warning" plain @click="requestDiscard('restart')">
						{{ t('setup.actions.restart') }}
					</el-button>
					<el-button v-else @click="requestDiscard('cancel')">
						{{ t('setup.actions.cancel') }}
					</el-button>
					<el-button v-if="wizard.phase.value === 'calibration'" type="primary" :disabled="!wizard.canGoNext.value" @click="wizard.next">
						{{ t('setup.actions.next') }}
					</el-button>
					<el-button v-else type="success" :disabled="!wizard.canComplete.value" @click="wizard.complete">
						{{ t('setup.actions.complete') }}
					</el-button>
				</div>
			</div>
		</main>

		<el-dialog
			v-model="setupInit.errorDialogVisible.value"
			:title="setupInit.errorTitle.value"
			width="min(420px, calc(100vw - 32px))"
			align-center
			:close-on-click-modal="false"
			:close-on-press-escape="false"
			:show-close="false"
		>
			<p class="dialog-message">
				{{ setupInit.errorMessage.value }}
			</p>
			<template #footer>
				<el-button v-if="setupInit.isReconnectFailure.value" type="primary" @click="setupInit.reloadPage">
					{{ t('setup.actions.reload') }}
				</el-button>
				<template v-else>
					<el-button :loading="setupInit.isLoading.value" @click="setupInit.initialize">
						{{ t('setup.actions.retry') }}
					</el-button>
					<el-button type="primary" @click="setupInit.goHome">
						{{ t('setup.actions.back_home') }}
					</el-button>
				</template>
			</template>
		</el-dialog>

		<el-dialog
			v-model="discardDialogVisible"
			:title="discardDialogTitle"
			width="min(420px, calc(100vw - 32px))"
			align-center
			:close-on-click-modal="false"
			@closed="clearDiscardAction"
		>
			<p class="dialog-message">
				{{ discardDialogMessage }}
			</p>
			<template #footer>
				<el-button @click="discardDialogVisible = false">
					{{ t('setup.actions.keep_editing') }}
				</el-button>
				<el-button type="danger" @click="confirmDiscard">
					{{ discardAction === 'restart' ? t('setup.actions.confirm_restart') : t('setup.actions.confirm_cancel') }}
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

.desktop-steps {
	margin-bottom: 28px;
}

:deep(.el-result__title p) {
	color: rgb(255, 255, 255);
}

:deep(.el-result__subtitle p) {
	color: #afafaf;
}

.wizard-actions,
.right-actions {
	display: flex;
	align-items: center;
	gap: 10px;
}

.instruction-card {
	margin-bottom: 20px;
	padding: 18px;
	border-left: 5px solid var(--el-color-success);
	border-radius: 10px;
	background: rgba(103, 194, 58, 0.12);
}

.instruction-card h2 {
	margin: 0 0 8px;
}

.instruction-card p {
	line-height: 1.6;
}

.review-content {
	max-width: 680px;
	margin: 0 auto;
}

.welcome-content,
.complete-content {
	max-width: 720px;
	margin: 0 auto;
}

.welcome-actions {
	display: flex;
	justify-content: center;
	gap: 12px;
}

.welcome-actions > .el-button {
	margin: 0;
}

.welcome-note {
	max-width: 560px;
	margin: 0 auto 24px;
	line-height: 1.7;
}

.saving-content {
	display: flex;
	min-height: 340px;
	align-items: center;
	justify-content: center;
	flex-direction: column;
	text-align: center;
}

.saving-icon {
	color: var(--el-color-primary);
}

.saving-content h2 {
	margin: 24px 0 8px;
}

.saving-content p {
	margin: 0;
	line-height: 1.6;
}

.mobile-progress {
	display: none;
	margin-bottom: 24px;
}

.mobile-progress-heading {
	display: flex;
	justify-content: space-between;
	gap: 12px;
	margin-bottom: 10px;
}

.validation-error,
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

	.title h1 {
		font-size: 2rem;
	}

	.desktop-steps {
		display: none;
	}

	.mobile-progress {
		display: block;
	}

	.welcome-actions {
		align-items: stretch;
		gap: 8px;
	}

	.welcome-actions > .el-button {
		flex: 1;
		height: auto;
		min-width: 0;
		min-height: 40px;
		padding-inline: 8px;
		line-height: 1.3;
		white-space: normal;
	}

	.wizard-actions {
		align-items: stretch;
		flex-direction: row;
		gap: 6px;
	}

	.right-actions {
		flex: 2;
		align-items: stretch;
		flex-direction: row;
		gap: 6px;
		min-width: 0;
	}

	.wizard-actions > .el-button,
	.right-actions > .el-button {
		flex: 1;
		min-width: 0;
		margin: 0;
		padding-inline: 6px;
		font-size: 12px;
	}

	.mobile-progress-heading {
		align-items: flex-start;
		flex-direction: column;
	}
}
</style>
