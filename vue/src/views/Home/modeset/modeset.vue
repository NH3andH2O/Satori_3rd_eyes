<template>
	<div class="title">
		<h2>{{ $t('modeset_title') }}</h2>
	</div>
	<el-form :model="mode_form_model" :rules="mode_rules" ref="mode_form_ref" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('mode')" prop="mode">
				<el-select
					v-model="mode.mode.value"
					placeholder="Select Mode"
					:disabled="mode.isLoading.value || mode.isSaving.value"
					style="width: 170px"
					@change="mode_save"
				>
					<el-option label="" :value="0" v-show="false" />
					<el-option :label="$t('gyroscope_tracks_mode')" :value="1" />
					<el-option :label="$t('network_control_mode')" :value="2" />
				</el-select>
			</el-form-item>
		</div>
		<transition name="slide-toggle">
			<div v-show="mode.mode.value === 2" class="content button" @click="$router.push('/control')">
				<el-form-item>
					<el-button type="success" round>
						{{ $t('enter_network_control') }}
					</el-button>
				</el-form-item>
			</div>
		</transition>
	</el-form>
</template>

<script lang="ts" setup>
import { ref, computed } from 'vue';
import { useModeRules } from './validators/useModeRules';
import { useModeSetting } from './composables/useModeSetting';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();
const mode_form_ref = ref();
const mode = useModeSetting();

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

const mode_rules = computed(() => useModeRules(t));

function mode_save() {
	mode_form_ref.value?.validate((valid: boolean) => {
		if (valid) {
			mode.update();
		}
	});
}
</script>

<style src="@/styles/card.css" scoped></style>
