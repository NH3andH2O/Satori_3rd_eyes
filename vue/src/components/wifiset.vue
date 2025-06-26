<script lang="ts" setup>
import { useWifiSettings } from '@/composables/useWifiSettings';
import { useSoftAPSettings } from '@/composables/useSoftAPSettings';
import { useI18n } from 'vue-i18n';
import { ref, computed } from 'vue';

const { t } = useI18n();

const wifi = useWifiSettings();
const softAP = useSoftAPSettings();

const passwordConfirmBlur = ref(false);

const showPasswordMismatch = computed(() => {
	return !!(
		softAP.config.value.is_change_password &&
		passwordConfirmBlur.value &&
		softAP.config.value.password_confirm &&
		softAP.config.value.password !== softAP.config.value.password_confirm
	);
});
</script>

<template>
	<div class="title">
		<h2>{{ $t('wifiset_title') }}</h2>
	</div>
	<div class="title2">
		<h3>{{ $t('LAN_setting') }}</h3>
		<el-tooltip :content="t('LAN_setting_help')" placement="top">
			<el-icon size="16" color="slategray">
				<QuestionFilled />
			</el-icon>
		</el-tooltip>
	</div>
	<div class="content">
		<p>{{ $t('enable_wifi') }}</p>
		<el-switch v-model="wifi.config.value.iswifi" :disabled="wifi.config.value.isLoading" />
	</div>
	<transition name="slide-toggle">
		<div v-show="wifi.config.value.iswifi">
			<div class="content">
				<p>{{ $t('wifi_ssid') }}</p>
				<el-input
					v-model="wifi.config.value.wifi_ssid"
					placeholder="SSID"
					style="width: auto"
					clearable
					:disabled="wifi.config.value.isLoading"
				/>
			</div>
			<div class="content">
				<p>{{ $t('wifi_password') }}</p>
				<el-input
					v-model="wifi.config.value.wifi_password"
					type="password"
					placeholder="Password"
					style="width: auto"
					clearable
					:disabled="wifi.config.value.isLoading"
				/>
			</div>
		</div>
	</transition>
	<div class="content button">
		<el-button
			round
			@click="wifi.update(wifi.config.value)"
			:loading="wifi.config.value.isSaving"
			:disabled="
				wifi.config.value.isLoading || (wifi.config.value.iswifi && (!wifi.config.value.wifi_ssid || !wifi.config.value.wifi_password))
			"
			>{{ $t('save') }}</el-button
		>
	</div>
	<div class="title2">
		<h3>{{ $t('softAP_setting') }}</h3>
		<el-tooltip :content="t('softAP_setting_help')" placement="top">
			<el-icon size="16" color="slategray">
				<QuestionFilled />
			</el-icon>
		</el-tooltip>
	</div>
	<div class="content">
		<p>{{ $t('softAP_name') }}</p>
		<el-input v-model="softAP.config.value.ssid" placeholder="3rd-Eyes" style="width: auto" clearable :disabled="softAP.config.value.isLoading" />
	</div>
	<div class="content">
		<p>{{ $t('change_password') }}</p>
		<el-switch v-model="softAP.config.value.is_change_password" :disabled="softAP.config.value.isLoading" />
	</div>
	<transition name="slide-toggle">
		<div v-show="softAP.config.value.is_change_password">
			<div class="content">
				<p>{{ $t('password') }}</p>
				<el-input
					v-model="softAP.config.value.password"
					type="password"
					:placeholder="t('password_info')"
					style="width: auto"
					clearable
					:disabled="softAP.config.value.isLoading"
				/>
			</div>
			<div class="content">
				<p>{{ $t('password_confirm') }}：</p>
				<el-input
					v-model="softAP.config.value.password_confirm"
					type="password"
					:placeholder="t('password_confirm')"
					style="width: auto"
					clearable
					:disabled="softAP.config.value.isLoading"
					@blur="passwordConfirmBlur = true"
					@focus="passwordConfirmBlur = false"
				/>
			</div>
			<div v-if="showPasswordMismatch" style="color: red; margin-bottom: 4px">
				{{ $t('password_not_match') }}
			</div>
		</div>
	</transition>
	<div class="content button">
		<el-button
			round
			@click="softAP.update()"
			:loading="softAP.config.value.isSaving"
			:disabled="
				softAP.config.value.isLoading ||
				(softAP.config.value.password !== softAP.config.value.password_confirm && softAP.config.value.is_change_password)
			"
			>{{ $t('save') }}</el-button
		>
	</div>
</template>

<style src="@/styles/card.css" scoped></style>
