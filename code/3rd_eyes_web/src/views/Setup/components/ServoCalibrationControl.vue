<script setup lang="ts">
import { computed } from 'vue';
import { useI18n } from 'vue-i18n';
import type { ServoName, ServoPose } from '../types';
import { clampServoAngle, SERVO_MAX_ANGLE, SERVO_MIN_ANGLE } from '../types';

const props = defineProps<{
	modelValue: ServoPose;
	targetServos: readonly ServoName[];
	disabled: boolean;
}>();

const emit = defineEmits<{
	'update:modelValue': [value: ServoPose, changedServo: ServoName];
	commit: [];
}>();

const { t } = useI18n();

const controls = computed(() => [
	{ name: 'upper' as const, label: t('setup.controls.upper_eyelid') },
	{ name: 'lower' as const, label: t('setup.controls.lower_eyelid') },
	{ name: 'eyeball' as const, label: t('setup.controls.eyeball') },
]);

function updateServo(name: ServoName, value: number | undefined) {
	if (typeof value !== 'number' || !Number.isFinite(value)) return;
	emit(
		'update:modelValue',
		{
			...props.modelValue,
			[name]: clampServoAngle(value),
		},
		name,
	);
}
</script>

<template>
	<div class="servo-controls">
		<section v-for="control in controls" :key="control.name" class="servo-control" :class="{ 'is-target': targetServos.includes(control.name) }">
			<div class="control-heading">
				<strong>{{ control.label }}</strong>
			</div>
			<div class="control-inputs">
				<el-slider
					:model-value="modelValue[control.name]"
					:min="SERVO_MIN_ANGLE"
					:max="SERVO_MAX_ANGLE"
					:step="1"
					:disabled="disabled"
					show-input-controls
					@update:model-value="updateServo(control.name, $event)"
					@change="emit('commit')"
				/>
				<el-input-number
					:model-value="modelValue[control.name]"
					:min="SERVO_MIN_ANGLE"
					:max="SERVO_MAX_ANGLE"
					:step="1"
					:step-strictly="true"
					:disabled="disabled"
					controls-position="right"
					@update:model-value="updateServo(control.name, $event)"
					@change="emit('commit')"
				/>
			</div>
		</section>
	</div>
</template>

<style scoped>
.servo-controls {
	display: grid;
	gap: 16px;
}

.servo-control {
	padding: 16px;
	border: 2px solid transparent;
	border-radius: 12px;
	background: rgba(127, 127, 127, 0.1);
	transition:
		border-color 0.2s ease,
		background-color 0.2s ease;
}

.servo-control.is-target {
	border-color: var(--el-color-success);
	background: rgba(103, 194, 58, 0.14);
}

.control-heading,
.control-inputs {
	display: flex;
	align-items: center;
	gap: 18px;
}

.control-heading {
	margin-bottom: 12px;
}

.control-inputs :deep(.el-slider) {
	flex: 1;
}

@media (max-width: 768px) {
	.servo-control {
		padding: 14px 12px;
	}

	.control-inputs {
		align-items: stretch;
		flex-direction: column;
		gap: 16px;
	}

	.control-inputs :deep(.el-input-number) {
		align-self: flex-end;
		width: 150px;
	}
}
</style>
