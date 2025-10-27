<template>
	<div class="title">
		<h2>{{ $t('advancedset_title') }}</h2>
	</div>
	<el-form :model="formData" :rules="advanced_rules" ref="advanced_form_ref" label-position="right" class="custom-form" @submit.prevent novalidate>
		<div class="content">
			<transition name="slide-toggle">
				<div v-show="mode === 1">
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
							v-model="advanced.correction_timer.value"
							placeholder="0"
							:step="100"
							:min="0"
							:max="65535"
							:step-strictly="false"
							:disabled="advanced.isLoading.value"
						>
							<template #suffix>ms</template>
						</el-input-number>
					</el-form-item>
				</div>
			</transition>
		</div>
		<div class="content button">
			<el-form-item>
				<el-button round @click="advanced_save" :loading="advanced.isSaving.value" :disabled="advanced.isLoading.value">
					{{ $t('save') }}
				</el-button>
			</el-form-item>
		</div>
	</el-form>
</template>

<script lang="ts" setup>
import { ref, computed, reactive } from 'vue';
import { storeToRefs } from 'pinia';
import { useAdvancedRules } from './validators/useAdvancedRules';
import { useModeStore } from '@/stores/mode';
import { useAdvancedSetting } from './composables/useAdvancedSetting';
import { useI18n } from 'vue-i18n';

const { t } = useI18n();
const advanced_form_ref = ref();
const advanced = useAdvancedSetting();

// 直接使用 advanced 的響應式屬性
const { mode } = storeToRefs(useModeStore());

// 創建表單數據對象，使用 reactive 包裝來讓 el-form 能夠正確追蹤
const formData = reactive({
	correction_timer: advanced.correction_timer,
});

const advanced_rules = computed(() => useAdvancedRules(mode, t));

function advanced_save() {
	advanced_form_ref.value?.validate((valid: boolean) => {
		if (valid) {
			advanced.update();
		}
	});
}
</script>

<style src="@/styles/card.css" scoped></style>
