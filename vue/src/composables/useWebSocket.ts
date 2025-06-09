// src/composables/useWebSocket.ts
import { wsService } from '@/services/websocket'

export function useWebSocket() {
	return {
		on: wsService.on.bind(wsService),
		off: wsService.off.bind(wsService),
		send: wsService.send.bind(wsService),
	}
}
