import mitt from 'mitt'

type Message = { type: string; payload: any }

class WebSocketService {
	private ws!: WebSocket
	private emitter = mitt<{ [key: string]: any }>()
	private hostName = window.location.host
	private url = "ws://" + this.hostName + "/ws"

	constructor() {
		this.init()
	}

	private init() {
		this.ws = new WebSocket(this.url)
		this.ws.addEventListener('open', () => {
			console.log('WS is connected', this.url)
		})
		this.ws.addEventListener('message', (evt) => {
			try {
				const msg: Message = JSON.parse(evt.data)
				this.emitter.emit(msg.type, msg.payload)
			} catch (e) {
				console.warn('WS non-JSON messages', evt.data)
			}
		})
		this.ws.addEventListener('close', () => {
			console.log('WS disconnected, attempting to reconnect...')
			setTimeout(() => this.init(), 2000)
		})
	}

	on<T>(type: string, handler: (payload: T) => void) {
		this.emitter.on(type, handler)
	}

	off(type: string, handler: any) {
		this.emitter.off(type, handler)
	}

	send(type: string, payload: any) {
		const msg: Message = { type, payload }
		this.ws.readyState === WebSocket.OPEN
			? this.ws.send(JSON.stringify(msg))
			: console.error('WS not ready, sending failed', msg)
	}
}

// 导出单例
export const wsService = new WebSocketService()
