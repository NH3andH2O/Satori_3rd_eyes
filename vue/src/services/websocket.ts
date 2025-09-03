import mitt from 'mitt';

type Message = { type: string; payload: any };

class WebSocketService {
	private ws: WebSocket | null = null;
	private emitter = mitt<Record<string, any>>();

	// 根據 http/https 選擇 ws/wss
	private get url() {
		const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
		const host = window.location.host;
		return `${protocol}//${host}/ws`;
	}

	/* 鏈接 */
	connect() {
		// 已连接或正在连接时不重复连
		if (this.ws && (this.ws.readyState === WebSocket.OPEN || this.ws.readyState === WebSocket.CONNECTING)) {
			return;
		}
		this.ws = new WebSocket(this.url);

		this.ws.onopen = () => {
			this.emitter.emit('open', undefined);
		};

		this.ws.onmessage = (e) => {
			try {
				const data: Message = JSON.parse(e.data);
				if (data?.type) {
					this.emitter.emit(data.type, data.payload);
				}
				this.emitter.emit('message:*', data);
			} catch (err) {
				console.error('WS message parse error', err, e.data);
			}
		};

		this.ws.onerror = (e) => {
			this.emitter.emit('error', e);
		};

		this.ws.onclose = (e) => {
			this.emitter.emit('close', e);
			this.ws = null;
		};
	}

	/* 斷開鏈接 */
	disconnect(code?: number, reason?: string) {
		if (this.ws) {
			try {
				this.ws.close(code, reason);
			} finally {
				this.ws = null;
			}
		}
	}

	isOpen() {
		return this.ws?.readyState === WebSocket.OPEN;
	}

	on(type: string, handler: any) {
		this.emitter.on(type, handler);
	}

	off(type: string, handler: any) {
		this.emitter.off(type, handler);
	}

	send(type: string, payload: any) {
		const msg: Message = { type, payload };
		if (this.ws && this.ws.readyState === WebSocket.OPEN) {
			this.ws.send(JSON.stringify(msg)); // 修正 stringify
		} else {
			console.warn('WS not ready, sending skipped', msg);
		}
	}
}

// 导出单例（不自动连接）
export const wsService = new WebSocketService();
