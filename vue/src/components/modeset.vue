<script lang="ts" setup>
import { ref, computed } from 'vue';
import { useModeRules } from '@/validators/useModeRules';
import { useModeSetting } from '@/composables/useModeSetting';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();
const mode_form_ref = ref();
const mode = useModeSetting();
const mode_rules = computed(() => useModeRules(mode.config.value, t));

function mode_save() {
	mode_form_ref.value?.validate((valid: boolean) => {
		if (valid) {
			mode.update();
		}
	});
}
</script>

<template>
	<div class="title">
		<h2>{{ $t('modeset_title') }}</h2>
	</div>
	<el-form :model="mode.config.value" :rules="mode_rules" ref="mode_form_ref" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('mode')" prop="mode">
				<el-select v-model="mode.config.value.mode" placeholder="Select Mode" :disabled="mode.config.value.isLoading" style="width: 170px">
					<el-option label="" :value="0" v-show="false" />
					<el-option :label="$t('gyroscope_tracks_mode')" :value="1" />
				</el-select>
			</el-form-item>
		</div>
		<div class="content">
			<transition name="slide-toggle">
				<div v-show="mode.config.value.mode === 1">
					<el-form-item prop="correction_timer">
						<template #label>
							<div style="display: flex; align-items: center; gap: 4px">
								<el-tooltip :content="t('correction_time_help')" placement="top">
									<el-icon size="16" color="slategray">
										<QuestionFilled />
									</el-icon>
								</el-tooltip>
								{{ $t('correction_time') }}
							</div>
						</template>
						<el-input-number
							v-model="mode.config.value.correction_timer"
							placeholder="Gyroscope Tracking Mode"
							clearable
							:step="100"
							:min="0"
							:max="65535"
							:disabled="mode.config.value.isLoading"
						>
							<template #suffix>ms</template>
						</el-input-number>
					</el-form-item>
				</div>
			</transition>
		</div>
		<div class="content button">
			<el-form-item>
				<el-button round @click="mode_save" :loading="mode.config.value.isSaving" :disabled="mode.config.value.isLoading">
					{{ $t('save') }}
				</el-button>
			</el-form-item>
		</div>
	</el-form>
</template>

<style src="../styles/card.css" scoped></style>
