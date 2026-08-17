import { computed, onBeforeUnmount, ref, watch } from 'vue';
import type { ComputedRef, Ref } from 'vue';
import { useRouter } from 'vue-router';
import { modeApi, servoApi } from '@/api';
import { MODES, StorageKeys, WS_MESSAGE_TYPES } from '@/config';
import { removeLocalStorageItem } from '@/composables/useLocalStorage';
import { useWebSocket } from '@/composables/useWebSocket';
import type { ServoConfig, ServoPreviewPayload } from '@/types';
import {
	SETUP_DRAFT_TTL_MS,
	SETUP_DRAFT_VERSION,
	SETUP_STEPS,
	SERVO_MAX_ANGLE,
	SERVO_MIN_ANGLE,
	clampServoAngle,
	type SetupCompletionState,
	type ServoCalibrationField,
	type ServoName,
	type ServoPose,
	type SetupDraft,
	type SetupStepId,
	type SetupStepPoseMap,
	type SetupWizardPhase,
} from '../types';

const PREVIEW_THROTTLE_MS = 80 as const;

function cloneConfig(config: ServoConfig): ServoConfig {
	return { ...config };
}

function clonePose(pose: ServoPose): ServoPose {
	return { ...pose };
}

function cloneStepPoses(stepPoses: SetupStepPoseMap): SetupStepPoseMap {
	return Object.fromEntries(Object.entries(stepPoses).map(([stepId, pose]) => [stepId, clonePose(pose)])) as SetupStepPoseMap;
}

function isServoAngle(value: unknown): value is number {
	return typeof value === 'number' && Number.isInteger(value) && value >= SERVO_MIN_ANGLE && value <= SERVO_MAX_ANGLE;
}

function isServoConfig(value: unknown): value is ServoConfig {
	if (!value || typeof value !== 'object') return false;
	const config = value as Partial<ServoConfig>;
	return (
		typeof config.is_setup === 'boolean' &&
		isServoAngle(config.max_upper_eyelid) &&
		isServoAngle(config.mid_upper_eyelid) &&
		isServoAngle(config.min_upper_eyelid) &&
		isServoAngle(config.max_lower_eyelid) &&
		isServoAngle(config.mid_lower_eyelid) &&
		isServoAngle(config.min_lower_eyelid) &&
		isServoAngle(config.max_eyeball) &&
		isServoAngle(config.mid_eyeball) &&
		isServoAngle(config.min_eyeball)
	);
}

function isServoPose(value: unknown): value is ServoPose {
	if (!value || typeof value !== 'object') return false;
	const pose = value as Partial<ServoPose>;
	return isServoAngle(pose.upper) && isServoAngle(pose.lower) && isServoAngle(pose.eyeball);
}

function isSetupStepId(value: unknown): value is SetupStepId {
	return typeof value === 'string' && SETUP_STEPS.some((step) => step.id === value);
}

function readStepPoses(value: unknown): SetupStepPoseMap | null {
	if (!value || typeof value !== 'object' || Array.isArray(value)) return null;
	const stepPoses: SetupStepPoseMap = {};
	for (const [stepId, pose] of Object.entries(value)) {
		if (!isSetupStepId(stepId) || !isServoPose(pose)) return null;
		stepPoses[stepId] = clonePose(pose);
	}
	return stepPoses;
}

function configsMatch(left: ServoConfig, right: ServoConfig): boolean {
	return (Object.keys(left) as (keyof ServoConfig)[]).every((key) => left[key] === right[key]);
}

function readDraft(serverConfig: ServoConfig): SetupDraft | null {
	try {
		const raw = localStorage.getItem(StorageKeys.SETUP_DRAFT);
		if (!raw) return null;
		const draft = JSON.parse(raw) as Partial<SetupDraft>;
		const updatedAtText = draft.updatedAt;
		const updatedAt = typeof updatedAtText === 'string' ? Date.parse(updatedAtText) : Number.NaN;
		const draftAge = Date.now() - updatedAt;
		const stepPoses = readStepPoses(draft.stepPoses);
		if (
			draft.version !== SETUP_DRAFT_VERSION ||
			!(draft.currentStepId === 'review' || isSetupStepId(draft.currentStepId)) ||
			!isServoConfig(draft.baseCalibration) ||
			!configsMatch(draft.baseCalibration, serverConfig) ||
			!isServoConfig(draft.calibration) ||
			!stepPoses ||
			typeof draft.hasUserAdjustments !== 'boolean' ||
			typeof updatedAtText !== 'string' ||
			!Number.isFinite(updatedAt) ||
			draftAge < 0 ||
			draftAge > SETUP_DRAFT_TTL_MS
		) {
			removeLocalStorageItem(StorageKeys.SETUP_DRAFT);
			return null;
		}
		return {
			version: SETUP_DRAFT_VERSION,
			currentStepId: draft.currentStepId,
			baseCalibration: cloneConfig(draft.baseCalibration),
			calibration: cloneConfig(draft.calibration),
			stepPoses,
			hasUserAdjustments: draft.hasUserAdjustments,
			updatedAt: updatedAtText,
		};
	} catch {
		removeLocalStorageItem(StorageKeys.SETUP_DRAFT);
		return null;
	}
}

interface SetupValidationIssue {
	key: string;
	stepIndex: number;
}

export function useSetupWizard(serverConfig: Ref<ServoConfig | null>, canOperate: ComputedRef<boolean>) {
	const router = useRouter();
	const { send, isOpen } = useWebSocket();

	const originalConfig = ref<ServoConfig | null>(null);
	const calibration = ref<ServoConfig | null>(null);
	const phase = ref<SetupWizardPhase>('welcome');
	const resumePhase = ref<'calibration' | 'review'>('calibration');
	const currentStepIndex = ref(0);
	const stepPoses = ref<SetupStepPoseMap>({});
	const previewPose = ref<ServoPose>({ upper: 85, lower: 50, eyeball: 75 });
	const previewStatus = ref<'idle' | 'pending' | 'sent' | 'failed'>('idle');
	const servoSaved = ref(false);
	const saveError = ref('');
	const validationError = ref('');
	const hasUserAdjustments = ref(false);
	const completionState = ref<SetupCompletionState | null>(null);

	let previewTimer: ReturnType<typeof setTimeout> | null = null;
	let lastPreviewAt = 0;

	const currentStep = computed(() => SETUP_STEPS[currentStepIndex.value] ?? null);
	const isReview = computed(() => phase.value === 'review');
	const isSaving = computed(() => phase.value === 'saving');
	const isComplete = computed(() => phase.value === 'complete');
	const isInitialSetup = computed(() => originalConfig.value?.is_setup === false);
	const progressStepIndex = computed(() => (isReview.value ? SETUP_STEPS.length : currentStepIndex.value));
	const targetServos = computed<readonly ServoName[]>(() => {
		if (phase.value !== 'calibration') return [];
		const fields = currentStep.value?.targetFields;
		if (!fields) return [];
		return (Object.keys(fields) as ServoName[]).filter((name) => fields[name]);
	});
	const canStart = computed(() => phase.value === 'welcome' && canOperate.value && Boolean(calibration.value));
	const canGoPrevious = computed(
		() => canOperate.value && !isSaving.value && ((phase.value === 'calibration' && currentStepIndex.value > 0) || phase.value === 'review'),
	);
	const canGoNext = computed(() => phase.value === 'calibration' && canOperate.value && !isSaving.value && previewStatus.value !== 'failed');
	const canComplete = computed(
		() => phase.value === 'review' && canOperate.value && !isSaving.value && previewStatus.value !== 'failed' && Boolean(calibration.value),
	);

	function persistDraft() {
		if (!calibration.value || !originalConfig.value || !['calibration', 'review'].includes(phase.value)) return;
		const draft: SetupDraft = {
			version: SETUP_DRAFT_VERSION,
			currentStepId: phase.value === 'review' ? 'review' : (currentStep.value?.id ?? SETUP_STEPS[0].id),
			baseCalibration: cloneConfig(originalConfig.value),
			calibration: cloneConfig(calibration.value),
			stepPoses: cloneStepPoses(stepPoses.value),
			hasUserAdjustments: hasUserAdjustments.value,
			updatedAt: new Date().toISOString(),
		};
		try {
			localStorage.setItem(StorageKeys.SETUP_DRAFT, JSON.stringify(draft));
		} catch (error) {
			console.error('Unable to save setup draft', error);
		}
	}

	function middlePose(config = originalConfig.value ?? calibration.value): ServoPose {
		if (!config) return clonePose(previewPose.value);
		return {
			upper: config.mid_upper_eyelid,
			lower: config.mid_lower_eyelid,
			eyeball: config.mid_eyeball,
		};
	}

	function poseForStep(index: number, inheritedPose?: ServoPose): ServoPose {
		const step = SETUP_STEPS[index];
		if (!step) return inheritedPose ? clonePose(inheritedPose) : middlePose();
		const savedPose = stepPoses.value[step.id];
		if (savedPose) return clonePose(savedPose);
		if (index === 0) return middlePose();
		const previousStep = SETUP_STEPS[index - 1];
		const previousPose = previousStep ? stepPoses.value[previousStep.id] : undefined;
		return clonePose(inheritedPose ?? previousPose ?? middlePose());
	}

	function saveCurrentStepPose(pose: ServoPose) {
		const step = currentStep.value;
		if (!step) return;
		stepPoses.value = {
			...stepPoses.value,
			[step.id]: clonePose(pose),
		};
	}

	function clearPreviewTimer() {
		if (previewTimer) {
			clearTimeout(previewTimer);
			previewTimer = null;
		}
	}

	function sendPose(pose: ServoPose, force = false): boolean {
		clearPreviewTimer();
		if ((!force && !canOperate.value) || !isOpen()) {
			previewStatus.value = 'failed';
			return false;
		}

		const payload: ServoPreviewPayload = {
			upper_eyelid_angle: clampServoAngle(pose.upper),
			lower_eyelid_angle: clampServoAngle(pose.lower),
			eyeball_angle: clampServoAngle(pose.eyeball),
		};
		const sent = send(WS_MESSAGE_TYPES.SERVO_PREVIEW, payload);
		previewStatus.value = sent ? 'sent' : 'failed';
		if (sent) lastPreviewAt = Date.now();
		return sent;
	}

	function sendPreview(force = false): boolean {
		if (phase.value !== 'calibration') return isOpen();
		return sendPose(previewPose.value, force);
	}

	function pausePreview() {
		clearPreviewTimer();
		previewStatus.value = 'failed';
	}

	function restoreCurrentPreview(): boolean {
		if (phase.value !== 'calibration') {
			previewStatus.value = 'idle';
			return isOpen();
		}
		previewPose.value = poseForStep(currentStepIndex.value, previewPose.value);
		return sendPreview(true);
	}

	function schedulePreview() {
		if (phase.value !== 'calibration') return;
		previewStatus.value = 'pending';
		const remaining = PREVIEW_THROTTLE_MS - (Date.now() - lastPreviewAt);
		if (remaining <= 0) {
			sendPreview();
			return;
		}
		clearPreviewTimer();
		previewTimer = setTimeout(sendPreview, remaining);
	}

	function start(): boolean {
		if (!canStart.value) return false;
		validationError.value = '';
		saveError.value = '';
		if (resumePhase.value === 'review') {
			phase.value = 'review';
			previewStatus.value = 'idle';
			persistDraft();
			return true;
		}

		phase.value = 'calibration';
		previewPose.value = poseForStep(currentStepIndex.value);
		saveCurrentStepPose(previewPose.value);
		persistDraft();
		schedulePreview();
		return true;
	}

	function updatePreviewPose(pose: ServoPose, changedServo: ServoName) {
		if (phase.value !== 'calibration') return;
		previewPose.value = {
			upper: clampServoAngle(pose.upper),
			lower: clampServoAngle(pose.lower),
			eyeball: clampServoAngle(pose.eyeball),
		};
		saveCurrentStepPose(previewPose.value);
		const targetField: ServoCalibrationField | undefined = currentStep.value?.targetFields[changedServo];
		if (calibration.value && targetField) {
			calibration.value[targetField] = previewPose.value[changedServo];
		}
		hasUserAdjustments.value = true;
		validationError.value = '';
		persistDraft();
		schedulePreview();
	}

	function issue(key: string, stepIndex: number): SetupValidationIssue {
		return { key, stepIndex };
	}

	function validateStep(index: number): SetupValidationIssue | null {
		const config = calibration.value;
		if (!config || !isServoConfig(config) || !isServoPose(previewPose.value)) return issue('setup.validation.range', index);
		const step = SETUP_STEPS[index];
		if (!step) return issue('setup.validation.range', 0);

		switch (step.id) {
			case 'eyelid-closed':
				if (!(config.min_upper_eyelid < config.max_upper_eyelid)) return issue('setup.validation.upper_endpoints', index);
				if (!(config.min_lower_eyelid < config.max_lower_eyelid)) return issue('setup.validation.lower_endpoints', index);
				break;
			case 'eyelid-middle':
				if (!(config.min_upper_eyelid < config.mid_upper_eyelid && config.mid_upper_eyelid < config.max_upper_eyelid)) {
					return issue('setup.validation.upper_middle', index);
				}
				if (!(config.min_lower_eyelid < config.mid_lower_eyelid && config.mid_lower_eyelid < config.max_lower_eyelid)) {
					return issue('setup.validation.lower_middle', index);
				}
				break;
			case 'eyeball-left':
				if (config.min_eyeball === config.mid_eyeball) return issue('setup.validation.eyeball_left_center', index);
				break;
			case 'eyeball-right': {
				const minEndpoint = Math.min(config.min_eyeball, config.max_eyeball);
				const maxEndpoint = Math.max(config.min_eyeball, config.max_eyeball);
				if (minEndpoint === maxEndpoint) return issue('setup.validation.eyeball_endpoints', index);
				if (!(minEndpoint < config.mid_eyeball && config.mid_eyeball < maxEndpoint)) {
					return issue('setup.validation.eyeball_center', index);
				}
				break;
			}
		}
		return null;
	}

	function validateCalibration(): SetupValidationIssue | null {
		const config = calibration.value;
		if (!config || !isServoConfig(config)) return issue('setup.validation.range', 0);
		if (!(config.min_upper_eyelid < config.max_upper_eyelid)) return issue('setup.validation.upper_endpoints', 1);
		if (!(config.min_upper_eyelid < config.mid_upper_eyelid && config.mid_upper_eyelid < config.max_upper_eyelid)) {
			return issue('setup.validation.upper_middle', 2);
		}
		if (!(config.min_lower_eyelid < config.max_lower_eyelid)) return issue('setup.validation.lower_endpoints', 1);
		if (!(config.min_lower_eyelid < config.mid_lower_eyelid && config.mid_lower_eyelid < config.max_lower_eyelid)) {
			return issue('setup.validation.lower_middle', 2);
		}
		const minEndpoint = Math.min(config.min_eyeball, config.max_eyeball);
		const maxEndpoint = Math.max(config.min_eyeball, config.max_eyeball);
		if (minEndpoint === maxEndpoint) return issue('setup.validation.eyeball_endpoints', 5);
		if (!(minEndpoint < config.mid_eyeball && config.mid_eyeball < maxEndpoint)) {
			return issue('setup.validation.eyeball_center', 5);
		}
		return null;
	}

	function showValidationIssue(validationIssue: SetupValidationIssue) {
		validationError.value = validationIssue.key;
		saveError.value = '';
		if (phase.value !== 'calibration' || currentStepIndex.value !== validationIssue.stepIndex) {
			phase.value = 'calibration';
			resumePhase.value = 'calibration';
			currentStepIndex.value = validationIssue.stepIndex;
			previewPose.value = poseForStep(validationIssue.stepIndex, previewPose.value);
			saveCurrentStepPose(previewPose.value);
			persistDraft();
			if (canOperate.value) schedulePreview();
		}
	}

	function moveToStep(index: number, inheritedPose?: ServoPose): boolean {
		if (!canOperate.value) return false;
		phase.value = 'calibration';
		resumePhase.value = 'calibration';
		currentStepIndex.value = Math.min(SETUP_STEPS.length - 1, Math.max(0, index));
		previewPose.value = poseForStep(currentStepIndex.value, inheritedPose);
		saveCurrentStepPose(previewPose.value);
		previewStatus.value = 'idle';
		validationError.value = '';
		persistDraft();
		schedulePreview();
		return true;
	}

	function next(): boolean {
		if (!canGoNext.value) return false;
		const validationIssue = validateStep(currentStepIndex.value);
		if (validationIssue) {
			showValidationIssue(validationIssue);
			return false;
		}

		if (currentStepIndex.value === SETUP_STEPS.length - 1) {
			const finalIssue = validateCalibration();
			if (finalIssue) {
				showValidationIssue(finalIssue);
				return false;
			}
			if (!sendPreview()) return false;
			phase.value = 'review';
			resumePhase.value = 'review';
			previewStatus.value = 'idle';
			persistDraft();
			return true;
		}

		return moveToStep(currentStepIndex.value + 1, previewPose.value);
	}

	function previous(): boolean {
		if (!canGoPrevious.value) return false;
		if (phase.value === 'review') return moveToStep(SETUP_STEPS.length - 1, previewPose.value);
		return moveToStep(currentStepIndex.value - 1, previewPose.value);
	}

	function buildSubmissionConfig(): ServoConfig {
		const config = cloneConfig(calibration.value as ServoConfig);
		if (config.max_eyeball < config.min_eyeball) {
			[config.min_eyeball, config.max_eyeball] = [config.max_eyeball, config.min_eyeball];
		}
		config.is_setup = true;
		return config;
	}

	async function complete(): Promise<boolean> {
		if (!calibration.value || !canComplete.value) return false;
		clearPreviewTimer();
		const validationIssue = validateCalibration();
		if (validationIssue) {
			showValidationIssue(validationIssue);
			return false;
		}

		phase.value = 'saving';
		saveError.value = '';
		completionState.value = null;
		try {
			await servoApi.setConfig(buildSubmissionConfig());
			servoSaved.value = true;
			removeLocalStorageItem(StorageKeys.SETUP_DRAFT);
		} catch {
			phase.value = 'review';
			resumePhase.value = 'review';
			saveError.value = 'setup.state.save_failed';
			return false;
		}

		if (originalConfig.value?.is_setup === false) {
			try {
				await modeApi.setConfig({ mode: MODES.NETWORK_CONTROL });
				completionState.value = 'success';
			} catch {
				completionState.value = 'debug-mode-warning';
			}
		} else {
			completionState.value = 'success';
		}

		phase.value = 'complete';
		return true;
	}

	function restart() {
		if (!originalConfig.value) return;
		clearPreviewTimer();
		removeLocalStorageItem(StorageKeys.SETUP_DRAFT);
		calibration.value = cloneConfig(originalConfig.value);
		stepPoses.value = {};
		currentStepIndex.value = 0;
		phase.value = 'calibration';
		resumePhase.value = 'calibration';
		previewPose.value = middlePose(originalConfig.value);
		saveCurrentStepPose(previewPose.value);
		previewStatus.value = 'idle';
		servoSaved.value = false;
		saveError.value = '';
		validationError.value = '';
		hasUserAdjustments.value = false;
		completionState.value = null;
		if (canOperate.value) schedulePreview();
	}

	async function cancel() {
		clearPreviewTimer();
		removeLocalStorageItem(StorageKeys.SETUP_DRAFT);
		if (originalConfig.value && canOperate.value) {
			previewPose.value = middlePose(originalConfig.value);
			sendPose(previewPose.value, true);
		}
		await router.replace({ name: 'Home' });
	}

	async function goHome() {
		await router.replace({ name: 'Home' });
	}

	watch(
		serverConfig,
		(config) => {
			if (!config) return;
			originalConfig.value = cloneConfig(config);
			const draft = readDraft(config);
			calibration.value = draft ? cloneConfig(draft.calibration) : cloneConfig(config);
			stepPoses.value = draft ? cloneStepPoses(draft.stepPoses) : {};
			hasUserAdjustments.value = draft?.hasUserAdjustments ?? false;
			servoSaved.value = false;
			saveError.value = '';
			validationError.value = '';
			completionState.value = null;
			phase.value = 'welcome';
			previewStatus.value = 'idle';

			if (draft?.currentStepId === 'review') {
				currentStepIndex.value = SETUP_STEPS.length - 1;
				resumePhase.value = 'review';
			} else {
				const restoredIndex = draft ? SETUP_STEPS.findIndex((step) => step.id === draft.currentStepId) : 0;
				currentStepIndex.value = restoredIndex >= 0 ? restoredIndex : 0;
				resumePhase.value = 'calibration';
			}
			previewPose.value = poseForStep(currentStepIndex.value);
		},
		{ immediate: true },
	);

	watch(canOperate, (ready) => {
		if (!ready) {
			clearPreviewTimer();
			previewStatus.value = 'failed';
		} else if (calibration.value && phase.value === 'calibration') {
			previewPose.value = poseForStep(currentStepIndex.value, previewPose.value);
			schedulePreview();
		}
	});

	onBeforeUnmount(clearPreviewTimer);

	return {
		steps: SETUP_STEPS,
		originalConfig,
		calibration,
		phase,
		currentStepIndex,
		currentStep,
		progressStepIndex,
		stepPoses,
		previewPose,
		previewStatus,
		targetServos,
		isReview,
		isSaving,
		isComplete,
		isInitialSetup,
		servoSaved,
		saveError,
		validationError,
		hasUserAdjustments,
		completionState,
		canStart,
		canGoPrevious,
		canGoNext,
		canComplete,
		start,
		updatePreviewPose,
		flushPreview: sendPreview,
		pausePreview,
		restoreCurrentPreview,
		next,
		previous,
		complete,
		restart,
		cancel,
		goHome,
		validateCalibration,
	};
}
