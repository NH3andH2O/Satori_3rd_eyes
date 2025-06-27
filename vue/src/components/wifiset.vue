<script lang="ts" setup>
import { useWifiSettings } from '@/composables/useWifiSettings';
import { useSoftAPSettings } from '@/composables/useSoftAPSettings';
import { useWiFiRules } from '@/validators/useWiFiRules';
import { useSoftAPRules } from '@/validators/useSoftAPRules';
import { useI18n } from 'vue-i18n';
import { ref, computed } from 'vue';

const { t } = useI18n();
const wifi_form_ref = ref();
const softap_form_ref = ref();
const wifi = useWifiSettings();
const softAP = useSoftAPSettings();
const wifi_rules = computed(() => useWiFiRules(wifi.config.value, t));
const softAP_rules = computed(() => useSoftAPRules(softAP.config.value, t));

function softAP_save() {
	softap_form_ref.value?.validate((valid: boolean) => {
		if (valid) {
			softAP.update();
		}
	});
}

function wifi_save() {
	wifi_form_ref.value?.validate((valid: boolean) => {
		if (valid) {
			wifi.update();
		}
	});
}
</script>

<template>
	<div class="title">
		<h2>{{ $t('wifiset_title') }}</h2>
	</div>
	<div class="title2" style="margin-bottom: 0px">
		<h3>{{ $t('LAN_setting') }}</h3>
		<el-tooltip :content="t('LAN_setting_help')" placement="top">
			<el-icon size="16" color="slategray">
				<QuestionFilled />
			</el-icon>
		</el-tooltip>
	</div>
	<el-form :model="wifi.config.value" :rules="wifi_rules" ref="wifi_form_ref" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('enable_wifi')">
				<el-switch v-model="wifi.config.value.is_wifi" :disabled="wifi.config.value.isLoading" />
			</el-form-item>
		</div>
		<div class="content">
			<transition name="slide-toggle">
				<div v-show="wifi.config.value.is_wifi">
					<el-form-item :label="$t('wifi_ssid')" prop="ssid">
						<el-input
							v-model="wifi.config.value.ssid"
							placeholder="SSID"
							style="width: auto"
							clearable
							:disabled="wifi.config.value.isLoading"
						/>
					</el-form-item>
					<el-form-item :label="$t('wifi_password')" prop="password">
						<el-input
							v-model="wifi.config.value.password"
							type="password"
							placeholder="Password"
							style="width: auto"
							clearable
							:disabled="wifi.config.value.isLoading"
						/>
					</el-form-item>
				</div>
			</transition>
		</div>
		<div class="content button">
			<el-form-item>
				<el-button round @click="wifi_save" :loading="wifi.config.value.isSaving" :disabled="wifi.config.value.isLoading">
					{{ $t('save') }}
				</el-button>
			</el-form-item>
		</div>
	</el-form>

	<div class="title2" style="margin-bottom: 0px">
		<h3>{{ $t('softAP_setting') }}</h3>
		<el-tooltip :content="t('softAP_setting_help')" placement="top">
			<el-icon size="16" color="slategray">
				<QuestionFilled />
			</el-icon>
		</el-tooltip>
	</div>
	<el-form :model="softAP.config.value" :rules="softAP_rules" ref="softap_form_ref" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('softAP_name')">
				<el-input
					v-model="softAP.config.value.ssid"
					placeholder="3rd-Eyes"
					style="max-width: 600px"
					clearable
					:disabled="softAP.config.value.isLoading"
				/>
			</el-form-item>
		</div>
		<div class="content">
			<el-form-item :label="$t('change_password')">
				<el-switch v-model="softAP.config.value.is_change_password" :disabled="softAP.config.value.isLoading" />
			</el-form-item>
		</div>
		<div class="content">
			<transition name="slide-toggle">
				<div v-show="softAP.config.value.is_change_password">
					<el-form-item :label="$t('password')" prop="password">
						<el-input
							v-model="softAP.config.value.password"
							type="password"
							:placeholder="t('password_info')"
							style="max-width: 600px"
							clearable
							:disabled="softAP.config.value.isLoading"
						/>
					</el-form-item>

					<el-form-item :label="$t('password_confirm')" prop="password_confirm">
						<el-input
							v-model="softAP.config.value.password_confirm"
							type="password"
							:placeholder="t('password_confirm_no_colon')"
							style="max-width: 600px"
							clearable
							:disabled="softAP.config.value.isLoading"
						/>
					</el-form-item>
				</div>
			</transition>
		</div>
		<div class="content button">
			<el-form-item>
				<el-button round @click="softAP_save" :loading="softAP.config.value.isSaving" :disabled="softAP.config.value.isLoading">
					{{ $t('save') }}
				</el-button>
			</el-form-item>
		</div>
	</el-form>
</template>

<style src="@/styles/card.css" scoped></style>
