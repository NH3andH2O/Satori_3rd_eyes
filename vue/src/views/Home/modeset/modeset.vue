<template>
	<div class="title">
		<h2>{{ $t('modeset_title') }}</h2>
	</div>
	<el-form :model="mode_form_model" :rules="mode_rules" ref="mode_form_ref" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('mode')" prop="mode">
				<el-select
					v-model="mode_form_model.mode"
					placeholder="Select Mode"
					:disabled="mode_form_model.isLoading || mode_form_model.isSaving"
					style="width: 170px"
					@change="mode_save"
				>
					<el-option label="" :value="0" v-show="false" />
					<el-option :label="$t('gyroscope_tracks_mode')" :value="1" />
				</el-select>
			</el-form-item>
		</div>
	</el-form>
</template>

<script lang="ts" setup>
import { ref, computed, reactive } from 'vue';
import { useModeRules } from './validators/useModeRules';
import { useModeSetting } from './composables/useModeSetting';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();
const mode_form_ref = ref();
const mode = useModeSetting();
const mode_form_model = reactive({
	mode: mode.mode,
	isLoading: mode.isLoading,
	isSaving: mode.isSaving,
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
