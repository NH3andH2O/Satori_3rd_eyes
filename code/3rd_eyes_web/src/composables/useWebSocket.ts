import { wsService } from '@/services/websocket';

export function useWebSocket() {
	return {
		connect: wsService.connect.bind(wsService),
		disconnect: wsService.disconnect.bind(wsService),
		isOpen: wsService.isOpen.bind(wsService),
		on: wsService.on.bind(wsService),
		off: wsService.off.bind(wsService),
		send: wsService.send.bind(wsService),
		resetReconnect: wsService.resetReconnect.bind(wsService),
	};
}
