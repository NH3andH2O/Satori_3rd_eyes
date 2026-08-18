<template>
	<div class="title">
		<h2>{{ $t('modeset.title') }}</h2>
	</div>
	<el-form ref="mode_form_ref" :model="mode_form_model" :rules="mode_rules" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('modeset.mode')" prop="mode">
				<el-select
					v-model="mode.mode.value"
					placeholder="Select Mode"
					:disabled="mode.isLoading.value || mode.isSaving.value"
					style="width: 170px"
					@change="mode_save"
				>
					<el-option v-show="false" label=" " :value="-1" />
					<el-option v-show="false" :label="$t('modeset.mode_info.servo_debug')" :value="0" />
					<el-option :label="$t('modeset.mode_info.gyroscope_tracks')" :value="1" />
					<el-option :label="$t('modeset.mode_info.network_control')" :value="2" />
				</el-select>
			</el-form-item>
		</div>
		<transition name="slide-toggle">
			<div v-show="mode.mode.value === MODES.SERVO_SETUP" class="content button">
				<el-form-item>
					<el-button type="warning" round :disabled="mode.isLoading.value || mode.isSaving.value" @click="$router.push('/setup')">
						{{ $t('modeset.enter_servo_setup') }}
					</el-button>
				</el-form-item>
			</div>
		</transition>
		<transition name="slide-toggle">
			<div v-show="mode.mode.value === MODES.NETWORK_CONTROL" class="content button">
				<el-form-item>
					<el-button type="success" round :disabled="mode.isLoading.value || mode.isSaving.value" @click="$router.push('/control')">
						{{ $t('modeset.enter_network_control') }}
					</el-button>
				</el-form-item>
			</div>
		</transition>
	</el-form>
</template>

<script lang="ts" setup>
import { ref, computed, nextTick } from 'vue';
import { useModeRules } from './validators/useModeRules';
import { useModeSetting } from './composables/useModeSetting';
import { useI18n } from 'vue-i18n';
import { MODES } from '@/config';

const emit = defineEmits<{
	loaded: [mode: number];
}>();

const { t } = useI18n();
const tModeErrors = (key: string) => t(`modeset.errors.${key}`);

const mode_form_ref = ref();
const mode = useModeSetting((currentMode) => emit('loaded', currentMode));

// 直接綁定到 mode 的響應式屬性，而不是創建新的 reactive
const mode_form_model = computed({
	get: () => ({
		mode: mode.mode.value,
		isLoading: mode.isLoading.value,
		isSaving: mode.isSaving.value,
	}),
	set: (val) => {
		mode.mode.value = val.mode;
	},
});

const mode_rules = computed(() => useModeRules(tModeErrors));

async function mode_save() {
	// 等待 DOM 更新，確保 v-model 已經更新了值
	await nextTick();
	mode_form_ref.value?.validate((valid: boolean) => {
		if (valid) {
			mode.update();
		}
	});
}
</script>

<style src="@/styles/card.css" scoped></style>
