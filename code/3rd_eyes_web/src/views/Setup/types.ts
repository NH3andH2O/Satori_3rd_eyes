import type { ServoConfig } from '@/types';

export const SERVO_MIN_ANGLE = 0;
export const SERVO_MAX_ANGLE = 180;
export const SETUP_DRAFT_VERSION = 2;
export const SETUP_DRAFT_TTL_MS = 10 * 60 * 1000;

export type ServoName = 'upper' | 'lower' | 'eyeball';
export type SetupStepId = 'eyelid-open' | 'eyelid-closed' | 'eyelid-middle' | 'eyeball-center' | 'eyeball-left' | 'eyeball-right';
export type SetupProgressStepId = SetupStepId | 'review';
export type SetupWizardPhase = 'welcome' | 'calibration' | 'review' | 'saving' | 'complete';
export type SetupCompletionState = 'success' | 'debug-mode-warning';
export type ServoCalibrationField = Exclude<keyof ServoConfig, 'is_setup'>;

export interface ServoPose {
	upper: number;
	lower: number;
	eyeball: number;
}

export interface SetupStepDefinition {
	id: SetupStepId;
	titleKey: string;
	descriptionKey: string;
	targetFields: Partial<Record<ServoName, ServoCalibrationField>>;
}

export type SetupStepPoseMap = Partial<Record<SetupStepId, ServoPose>>;

export interface SetupDraft {
	version: typeof SETUP_DRAFT_VERSION;
	currentStepId: SetupProgressStepId;
	baseCalibration: ServoConfig;
	calibration: ServoConfig;
	stepPoses: SetupStepPoseMap;
	hasUserAdjustments: boolean;
	updatedAt?: string;
}

export type SetupConnectionState = 'idle' | 'initializing' | 'connected' | 'disconnected' | 'reconnecting' | 'failed' | 'mode-error' | 'error';

export const SETUP_STEPS: readonly SetupStepDefinition[] = [
	{
		id: 'eyelid-open',
		titleKey: 'setup.steps.eyelid_open.title',
		descriptionKey: 'setup.steps.eyelid_open.description',
		targetFields: { upper: 'min_upper_eyelid', lower: 'max_lower_eyelid' },
	},
	{
		id: 'eyelid-closed',
		titleKey: 'setup.steps.eyelid_closed.title',
		descriptionKey: 'setup.steps.eyelid_closed.description',
		targetFields: { upper: 'max_upper_eyelid', lower: 'min_lower_eyelid' },
	},
	{
		id: 'eyelid-middle',
		titleKey: 'setup.steps.eyelid_middle.title',
		descriptionKey: 'setup.steps.eyelid_middle.description',
		targetFields: { upper: 'mid_upper_eyelid', lower: 'mid_lower_eyelid' },
	},
	{
		id: 'eyeball-center',
		titleKey: 'setup.steps.eyeball_center.title',
		descriptionKey: 'setup.steps.eyeball_center.description',
		targetFields: { eyeball: 'mid_eyeball' },
	},
	{
		id: 'eyeball-left',
		titleKey: 'setup.steps.eyeball_left.title',
		descriptionKey: 'setup.steps.eyeball_left.description',
		targetFields: { eyeball: 'min_eyeball' },
	},
	{
		id: 'eyeball-right',
		titleKey: 'setup.steps.eyeball_right.title',
		descriptionKey: 'setup.steps.eyeball_right.description',
		targetFields: { eyeball: 'max_eyeball' },
	},
] as const;

export const SETUP_PROGRESS_STEPS: readonly { id: SetupProgressStepId; titleKey: string }[] = [
	...SETUP_STEPS.map(({ id, titleKey }) => ({ id, titleKey })),
	{ id: 'review', titleKey: 'setup.review.step_title' },
];

export function clampServoAngle(value: number): number {
	return Math.min(SERVO_MAX_ANGLE, Math.max(SERVO_MIN_ANGLE, Math.round(value)));
}
