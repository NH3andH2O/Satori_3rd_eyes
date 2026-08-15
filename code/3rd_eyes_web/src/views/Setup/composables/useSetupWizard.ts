import { computed, onBeforeUnmount, ref, watch } from 'vue';
import type { ComputedRef, Ref } from 'vue';
import { useRouter } from 'vue-router';
import { modeApi, servoApi } from '@/api';
import { MODES, StorageKeys, WS_MESSAGE_TYPES } from '@/config';
import { removeLocalStorageItem } from '@/composables/useLocalStorage';
import { useWebSocket } from '@/composables/useWebSocket';
import type { ServoConfig, ServoPreviewPayload } from '@/types';
import {
	SETUP_DRAFT_VERSION,
	SETUP_STEPS,
	SERVO_MAX_ANGLE,
	SERVO_MIN_ANGLE,
	clampServoAngle,
	type ServoCalibrationField,
	type ServoName,
	type ServoPose,
	type SetupDraft,
} from '../types';

const PREVIEW_THROTTLE_MS = 80;

function cloneConfig(config: ServoConfig): ServoConfig {
	return { ...config };
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

function configsMatch(left: ServoConfig, right: ServoConfig): boolean {
	return (Object.keys(left) as (keyof ServoConfig)[]).every((key) => left[key] === right[key]);
}

function readDraft(serverConfig: ServoConfig): SetupDraft | null {
	try {
		const raw = localStorage.getItem(StorageKeys.SETUP_DRAFT);
		if (!raw) return null;
		const draft = JSON.parse(raw) as Partial<SetupDraft>;
		if (
			draft.version !== SETUP_DRAFT_VERSION ||
			!Number.isInteger(draft.currentStep) ||
			(draft.currentStep ?? -1) < 0 ||
			(draft.currentStep ?? SETUP_STEPS.length + 1) > SETUP_STEPS.length ||
			!isServoConfig(draft.baseCalibration) ||
			!configsMatch(draft.baseCalibration, serverConfig) ||
			!isServoConfig(draft.calibration)
		) {
			return null;
		}
		return draft as SetupDraft;
	} catch {
		return null;
	}
}

export function useSetupWizard(serverConfig: Ref<ServoConfig | null>, canOperate: ComputedRef<boolean>) {
	const router = useRouter();
	const { send, isOpen } = useWebSocket();

	const originalConfig = ref<ServoConfig | null>(null);
	const calibration = ref<ServoConfig | null>(null);
	const currentStepIndex = ref(0);
	const previewPose = ref<ServoPose>({ upper: 85, lower: 50, eyeball: 75 });
	const previewStatus = ref<'idle' | 'pending' | 'sent' | 'failed'>('idle');
	const isSaving = ref(false);
	const servoSaved = ref(false);
	const saveError = ref('');
	const modeSwitchFailed = ref(false);

	let previewTimer: ReturnType<typeof setTimeout> | null = null;
	let lastPreviewAt = 0;

	const currentStep = computed(() => SETUP_STEPS[currentStepIndex.value] ?? null);
	const isReview = computed(() => currentStepIndex.value === SETUP_STEPS.length);
	const targetServos = computed<readonly ServoName[]>(() => {
		const fields = currentStep.value?.targetFields;
		if (!fields) return [];
		return (Object.keys(fields) as ServoName[]).filter((name) => fields[name]);
	});
	const canGoPrevious = computed(() => currentStepIndex.value > 0 && canOperate.value && !isSaving.value);
	const canGoNext = computed(
		() => currentStepIndex.value < SETUP_STEPS.length && canOperate.value && !isSaving.value && previewStatus.value !== 'failed',
	);
	const canComplete = computed(
		() => isReview.value && canOperate.value && !isSaving.value && previewStatus.value !== 'failed' && Boolean(calibration.value),
	);

	function persistDraft() {
		if (!calibration.value) return;
		const draft: SetupDraft = {
			version: SETUP_DRAFT_VERSION,
			currentStep: currentStepIndex.value,
			baseCalibration: cloneConfig(originalConfig.value ?? calibration.value),
			calibration: cloneConfig(calibration.value),
			updatedAt: new Date().toISOString(),
		};
		try {
			localStorage.setItem(StorageKeys.SETUP_DRAFT, JSON.stringify(draft));
		} catch (error) {
			console.error('Unable to save setup draft', error);
		}
	}

	function poseForCurrentStep(): ServoPose {
		const config = calibration.value;
		if (!config) return previewPose.value;

		const pose: ServoPose = {
			upper: config.mid_upper_eyelid,
			lower: config.mid_lower_eyelid,
			eyeball: config.mid_eyeball,
		};
		const fields = currentStep.value?.targetFields;
		if (!fields) return pose;
		if (fields.upper) pose.upper = config[fields.upper];
		if (fields.lower) pose.lower = config[fields.lower];
		if (fields.eyeball) pose.eyeball = config[fields.eyeball];
		return pose;
	}

	function clearPreviewTimer() {
		if (previewTimer) {
			clearTimeout(previewTimer);
			previewTimer = null;
		}
	}

	function sendPreview(force = false): boolean {
		clearPreviewTimer();
		if ((!force && !canOperate.value) || !isOpen()) {
			previewStatus.value = 'failed';
			return false;
		}

		const payload: ServoPreviewPayload = {
			upper_eyelid_angle: clampServoAngle(previewPose.value.upper),
			lower_eyelid_angle: clampServoAngle(previewPose.value.lower),
			eyeball_angle: clampServoAngle(previewPose.value.eyeball),
		};
		const sent = send(WS_MESSAGE_TYPES.SERVO_PREVIEW, payload);
		previewStatus.value = sent ? 'sent' : 'failed';
		if (sent) lastPreviewAt = Date.now();
		return sent;
	}

	function pausePreview() {
		clearPreviewTimer();
		previewStatus.value = 'failed';
	}

	function restoreCurrentPreview(): boolean {
		if (isReview.value) {
			previewStatus.value = 'idle';
			return isOpen();
		}
		previewPose.value = poseForCurrentStep();
		return sendPreview(true);
	}

	function schedulePreview() {
		previewStatus.value = 'pending';
		const remaining = PREVIEW_THROTTLE_MS - (Date.now() - lastPreviewAt);
		if (remaining <= 0) {
			sendPreview();
			return;
		}
		clearPreviewTimer();
		previewTimer = setTimeout(sendPreview, remaining);
	}

	function applyTargetValues(pose: ServoPose) {
		if (!calibration.value || !currentStep.value) return;
		for (const [servo, field] of Object.entries(currentStep.value.targetFields) as [ServoName, ServoCalibrationField][]) {
			calibration.value[field] = clampServoAngle(pose[servo]);
		}
	}

	function updatePreviewPose(pose: ServoPose) {
		previewPose.value = {
			upper: clampServoAngle(pose.upper),
			lower: clampServoAngle(pose.lower),
			eyeball: clampServoAngle(pose.eyeball),
		};
		applyTargetValues(previewPose.value);
		persistDraft();
		schedulePreview();
	}

	function validateCalibration(): string | null {
		const config = calibration.value;
		if (!config || !isServoConfig(config)) return 'setup.validation.range';
		if (!(config.min_upper_eyelid < config.mid_upper_eyelid && config.mid_upper_eyelid < config.max_upper_eyelid)) {
			return 'setup.validation.upper_order';
		}
		if (!(config.min_lower_eyelid < config.mid_lower_eyelid && config.mid_lower_eyelid < config.max_lower_eyelid)) {
			return 'setup.validation.lower_order';
		}
		if (!(config.min_eyeball < config.mid_eyeball && config.mid_eyeball < config.max_eyeball)) {
			return 'setup.validation.eyeball_order';
		}
		return null;
	}

	function moveToStep(index: number): boolean {
		if (!sendPreview()) return false;
		currentStepIndex.value = Math.min(SETUP_STEPS.length, Math.max(0, index));
		previewPose.value = poseForCurrentStep();
		previewStatus.value = 'idle';
		persistDraft();
		if (!isReview.value) schedulePreview();
		return true;
	}

	function next() {
		if (!canGoNext.value) return;
		moveToStep(currentStepIndex.value + 1);
	}

	function previous() {
		if (!canGoPrevious.value) return;
		moveToStep(currentStepIndex.value - 1);
	}

	async function switchToNetworkControl(): Promise<boolean> {
		try {
			await modeApi.setConfig({ mode: MODES.NETWORK_CONTROL });
			modeSwitchFailed.value = false;
			await router.replace({ name: 'Home' });
			return true;
		} catch {
			modeSwitchFailed.value = true;
			saveError.value = 'setup.state.mode_switch_failed';
			return false;
		}
	}

	async function complete(): Promise<boolean> {
		if (!calibration.value || !canComplete.value) return false;
		clearPreviewTimer();
		const validationError = validateCalibration();
		if (validationError) {
			saveError.value = validationError;
			return false;
		}

		isSaving.value = true;
		saveError.value = '';
		try {
			if (!servoSaved.value) {
				await servoApi.setConfig({ ...calibration.value, is_setup: true });
				servoSaved.value = true;
				removeLocalStorageItem(StorageKeys.SETUP_DRAFT);
			}
			return await switchToNetworkControl();
		} catch {
			saveError.value = 'setup.state.save_failed';
			return false;
		} finally {
			isSaving.value = false;
		}
	}

	async function retryModeSwitch() {
		if (!servoSaved.value || isSaving.value) return;
		isSaving.value = true;
		try {
			await switchToNetworkControl();
		} finally {
			isSaving.value = false;
		}
	}

	async function cancel() {
		clearPreviewTimer();
		if (servoSaved.value) {
			await router.replace({ name: 'Home' });
			return;
		}
		if (originalConfig.value && canOperate.value) {
			previewPose.value = {
				upper: originalConfig.value.mid_upper_eyelid,
				lower: originalConfig.value.mid_lower_eyelid,
				eyeball: originalConfig.value.mid_eyeball,
			};
			sendPreview();
		}
		await router.replace({ name: 'Home' });
	}

	watch(
		serverConfig,
		(config) => {
			if (!config || calibration.value) return;
			originalConfig.value = cloneConfig(config);
			const draft = readDraft(config);
			calibration.value = draft ? cloneConfig(draft.calibration) : cloneConfig(config);
			currentStepIndex.value = draft?.currentStep ?? 0;
			previewPose.value = poseForCurrentStep();
			if (canOperate.value && !isReview.value) schedulePreview();
		},
		{ immediate: true },
	);

	watch(canOperate, (ready) => {
		if (!ready) {
			clearPreviewTimer();
			previewStatus.value = 'failed';
		} else if (calibration.value && !isReview.value) {
			previewPose.value = poseForCurrentStep();
			schedulePreview();
		}
	});

	onBeforeUnmount(clearPreviewTimer);

	return {
		steps: SETUP_STEPS,
		originalConfig,
		calibration,
		currentStepIndex,
		currentStep,
		previewPose,
		previewStatus,
		targetServos,
		isReview,
		isSaving,
		servoSaved,
		saveError,
		modeSwitchFailed,
		canGoPrevious,
		canGoNext,
		canComplete,
		updatePreviewPose,
		flushPreview: sendPreview,
		pausePreview,
		restoreCurrentPreview,
		next,
		previous,
		complete,
		retryModeSwitch,
		cancel,
		validateCalibration,
	};
}
