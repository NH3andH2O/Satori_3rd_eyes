<script lang="ts" setup>
import { useWifiSettings } from './composables/useWifiSettings';
import { useSoftAPSettings } from './composables/useSoftAPSettings';
import { useWiFiRules } from './validators/useWiFiRules';
import { useSoftAPRules } from './validators/useSoftAPRules';
import { useI18n } from 'vue-i18n';
import { ref, computed } from 'vue';

const { t } = useI18n();
const tLanErrors = (key: string) => t(`wifiset.LAN.errors.${key}`);
const tSoftAPErrors = (key: string) => t(`wifiset.softAP.errors.${key}`);

const wifi_form_ref = ref();
const softap_form_ref = ref();
const wifi = useWifiSettings();
const softAP = useSoftAPSettings();
const wifi_rules = computed(() => useWiFiRules(wifi.config.value, tLanErrors));
const softAP_rules = computed(() => useSoftAPRules(softAP.config.value, tSoftAPErrors));

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
		<h2>{{ $t('wifiset.title') }}</h2>
	</div>
	<div class="title2" style="margin-bottom: 0px">
		<h3>{{ $t('wifiset.LAN.title') }}</h3>
		<el-tooltip :content="t('wifiset.LAN.title_help')" placement="top">
			<el-icon size="16" color="slategray">
				<QuestionFilled />
			</el-icon>
		</el-tooltip>
	</div>
	<el-form ref="wifi_form_ref" :model="wifi.config.value" :rules="wifi_rules" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('wifiset.LAN.wifi_enable')">
				<el-switch v-model="wifi.config.value.is_wifi" :disabled="wifi.config.value.isLoading" />
			</el-form-item>
		</div>
		<div class="content">
			<transition name="slide-toggle">
				<div v-show="wifi.config.value.is_wifi">
					<el-form-item :label="$t('wifiset.LAN.ssid')" prop="ssid">
						<el-input
							v-model="wifi.config.value.ssid"
							placeholder="SSID"
							style="width: auto"
							clearable
							:disabled="wifi.config.value.isLoading"
						/>
					</el-form-item>
					<el-form-item :label="$t('wifiset.LAN.password')" prop="password">
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
				<el-button round :loading="wifi.config.value.isSaving" :disabled="wifi.config.value.isLoading" @click="wifi_save">
					{{ $t('wifiset.LAN.save') }}
				</el-button>
			</el-form-item>
		</div>
	</el-form>

	<div class="title2" style="margin-bottom: 0px">
		<h3>{{ $t('wifiset.softAP.title') }}</h3>
		<el-tooltip :content="t('wifiset.softAP.title_help')" placement="top">
			<el-icon size="16" color="slategray">
				<QuestionFilled />
			</el-icon>
		</el-tooltip>
	</div>
	<el-form ref="softap_form_ref" :model="softAP.config.value" :rules="softAP_rules" label-position="right" class="custom-form">
		<div class="content">
			<el-form-item :label="$t('wifiset.softAP.softAP_name')">
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
			<el-form-item :label="$t('wifiset.softAP.change_password')">
				<el-switch v-model="softAP.config.value.is_change_password" :disabled="softAP.config.value.isLoading" />
			</el-form-item>
		</div>
		<div class="content">
			<transition name="slide-toggle">
				<div v-show="softAP.config.value.is_change_password">
					<el-form-item :label="$t('wifiset.softAP.password')" prop="password">
						<el-input
							v-model="softAP.config.value.password"
							type="password"
							:placeholder="t('wifiset.softAP.password_info')"
							style="max-width: 600px"
							clearable
							:disabled="softAP.config.value.isLoading"
						/>
					</el-form-item>

					<el-form-item :label="$t('wifiset.softAP.password_confirm')" prop="password_confirm">
						<el-input
							v-model="softAP.config.value.password_confirm"
							type="password"
							:placeholder="t('wifiset.softAP.password_confirm_info')"
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
				<el-button round :loading="softAP.config.value.isSaving" :disabled="softAP.config.value.isLoading" @click="softAP_save">
					{{ $t('wifiset.softAP.save') }}
				</el-button>
			</el-form-item>
		</div>
	</el-form>
</template>

<style src="@/styles/card.css" scoped></style>
