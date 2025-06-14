<script lang="ts" setup>

import { useWifiSettings } from '@/composables/useWifiSettings'

const wifi = useWifiSettings()
</script>

<template>

	<div class="title">
		<h2>{{ $t('wifiset_title') }}</h2>
	</div>
	<div class="content">
		<p>{{ $t('enable_wifi') }} </p>
		<el-switch v-model="wifi.config.value.iswifi" :disabled="wifi.config.value.isLoading" />
	</div>
	<transition name="slide-toggle">
		<div v-show="wifi.config.value.iswifi">
			<div class="content">
				<p>{{ $t('wifi_ssid') }}</p>
				<el-input v-model="wifi.config.value.wifi_ssid" placeholder="SSID" style="width: auto" clearable
					:disabled="wifi.config.value.isLoading" />
			</div>
			<div class="content">
				<p>{{ $t('wifi_password') }}</p>
				<el-input v-model="wifi.config.value.wifi_password" type="password" placeholder="Password"
					style="width: auto" clearable :disabled="wifi.config.value.isLoading" />
			</div>
		</div>
	</transition>
	<div class="content button">
		<el-button round @click="wifi.update(wifi.config.value)" :loading="wifi.config.value.isSaving"
			:disabled="wifi.config.value.isLoading || (wifi.config.value.iswifi && (!wifi.config.value.wifi_ssid || !wifi.config.value.wifi_password))">{{
				$t('save')
			}}</el-button>
	</div>
</template>

<style src="../styles/card.css" scoped></style>