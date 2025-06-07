import { ref, onMounted, onBeforeUnmount } from 'vue'

export function useEsp32WebSocket(url: string) {
	// 定義響應式變數
	const wifi_ssid = ref('')
	const wifi_password = ref('')
	const iswifi = ref(false)
	const socket = ref<WebSocket | null>(null)

	// 連線與事件
	function connectWebSocket() {
		socket.value = new WebSocket(url)

		socket.value.onopen = () => {
			console.log('WebSocket connected')
			socket.value?.send(JSON.stringify({ type: 'get_config' }))
		}

		socket.value.onerror = err => {
			console.error('WebSocket error', err)
		}

		socket.value.onmessage = event => {
			try {
				const data = JSON.parse(event.data)
				if (data.type === 'config') {
					iswifi.value = data.iswifi
					wifi_ssid.value = data.wifi_ssid
					wifi_password.value = data.wifi_password
				}
			} catch (e) {
				console.error('Failed to parse message', e)
			}
		}

		socket.value.onclose = () => {
			console.warn('WebSocket closed, retrying in 3 s')
			setTimeout(connectWebSocket, 3000)
		}
	}

	function sendConfig() {
		if (socket.value && socket.value.readyState === WebSocket.OPEN) {
			socket.value.send(
				JSON.stringify({
					type: 'set_config',
					iswifi: iswifi.value,
					wifi_ssid: wifi_ssid.value,
					wifi_password: wifi_password.value
				})
			)
		} else {
			console.warn('WebSocket not ready; config not sent')
		}
	}

	onMounted(connectWebSocket)
	onBeforeUnmount(() => socket.value?.close())

	// 回傳響應式變數與方法
	return {
		wifi_ssid,
		wifi_password,
		iswifi,
		sendConfig
	}
}
