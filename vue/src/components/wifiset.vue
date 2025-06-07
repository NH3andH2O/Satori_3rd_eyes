<script lang="ts" setup>
import { useEsp32WebSocket } from '../composables/useWebSocket'
const { wifi_ssid, wifi_password, iswifi, sendConfig } = useEsp32WebSocket('ws://${location.host}/ws')
</script>

<template>

	<div class="title">
		<h2>{{ $t('wifiset_title') }}</h2>
	</div>
	<div class="content">
		<p>{{ $t('enable_wifi') }} </p>
		<el-switch v-model="iswifi" />
	</div>
	<transition name="slide-toggle">
		<div v-show="iswifi">
			<div class="content">
				<p>{{ $t('wifi_ssid') }}</p>
				<el-input v-model="wifi_ssid" placeholder="SSID" style="width: auto" clearable />
			</div>
			<div class="content">
				<p>{{ $t('wifi_password') }}</p>
				<el-input v-model="wifi_password" type="password" placeholder="Password" style="width: auto"
					clearable />
			</div>
		</div>
	</transition>
	<div class="content button">
		<el-button round @click="sendConfig">{{ $t('save') }}</el-button>
	</div>
</template>

<style src="../styles/card.css" scoped></style>