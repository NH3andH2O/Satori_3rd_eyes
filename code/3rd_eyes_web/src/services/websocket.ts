import mitt from 'mitt';
import type { Emitter } from 'mitt';
import type { WebSocketMessage, WebSocketConfig } from '@/types';
import { APP_CONFIG } from '@/config';

// 靈活的事件映射類型
type EventMap = Record<string, unknown>;

class WebSocketService {
	private ws: WebSocket | null = null;
	private emitter: Emitter<EventMap> = mitt<EventMap>();

	// 重連配置
	private reconnectAttempts = 0;
	private maxReconnectAttempts: number;
	private reconnectDelay: number;
	private maxReconnectDelay: number;
	private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
	private shouldReconnect = false;
	private manualClose = false;

	constructor(config?: WebSocketConfig) {
		const wsConfig = { ...APP_CONFIG.websocket, ...config };
		this.maxReconnectAttempts = wsConfig.maxReconnectAttempts;
		this.reconnectDelay = wsConfig.reconnectDelay;
		this.maxReconnectDelay = wsConfig.maxReconnectDelay;
	}

	// 根據環境變量或 http/https 選擇 ws URL
	private get url(): string {
		// 開發環境優先使用環境變量配置的 WebSocket URL
		const envWsUrl = import.meta.env.VITE_WS_URL;
		if (envWsUrl) {
			return envWsUrl;
		}

		// 生產環境根據當前協議自動選擇 ws/wss
		const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
		const host = window.location.host;
		return `${protocol}//${host}/ws`;
	}

	/* 清除重連定時器 */
	private clearReconnectTimer() {
		if (this.reconnectTimer) {
			clearTimeout(this.reconnectTimer);
			this.reconnectTimer = null;
		}
	}

	/* 計算重連延遲（指數退避） */
	private getReconnectDelay(): number {
		const delay = this.reconnectDelay * Math.pow(2, this.reconnectAttempts);
		return Math.min(delay, this.maxReconnectDelay);
	}

	/* 嘗試重連 */
	private attemptReconnect(): void {
		if (!this.shouldReconnect || this.manualClose) {
			return;
		}

		if (this.reconnectAttempts >= this.maxReconnectAttempts) {
			console.error('WS: The maximum reconnection attempts have been reached.');
			this.emitter.emit('reconnect:failed', { attempts: this.reconnectAttempts });
			return;
		}

		const delay = this.getReconnectDelay();
		console.log(`WS: ${delay}ms attempt to reconnect (${this.reconnectAttempts + 1}/${this.maxReconnectAttempts})`);

		this.emitter.emit('reconnect:attempt', {
			attempt: this.reconnectAttempts + 1,
			maxAttempts: this.maxReconnectAttempts,
			delay,
		});

		this.reconnectTimer = setTimeout(() => {
			this.reconnectAttempts++;
			this.connect();
		}, delay);
	}

	/* 鏈接 */
	connect(): void {
		// 已连接或正在连接时不重复连
		if (this.ws && (this.ws.readyState === WebSocket.OPEN || this.ws.readyState === WebSocket.CONNECTING)) {
			return;
		}

		this.clearReconnectTimer();
		this.manualClose = false;
		this.shouldReconnect = true;

		try {
			this.ws = new WebSocket(this.url);

			this.ws.onopen = () => {
				console.log('WS: connected');
				this.reconnectAttempts = 0; // 重置重連計數
				this.emitter.emit('open', undefined);
			};

			this.ws.onmessage = (e: MessageEvent) => {
				try {
					const data: WebSocketMessage = JSON.parse(e.data);
					if (data?.type) {
						this.emitter.emit(data.type, data.payload);
					}
					this.emitter.emit('message:*', data);
				} catch (err) {
					console.error('WS message parse error', err, e.data);
				}
			};

			this.ws.onerror = (e: Event) => {
				console.error('WS: connection error', e);
				this.emitter.emit('error', e);
			};

			this.ws.onclose = (e: CloseEvent) => {
				console.log('WS: close', e.code, e.reason);
				this.emitter.emit('close', e);
				this.ws = null;

				// 非正常關閉且未手動關閉時，嘗試重連
				if (e.code !== 1000 && !this.manualClose && this.shouldReconnect) {
					this.attemptReconnect();
				}
			};
		} catch (err) {
			console.error('WS: connection failed', err);
			this.emitter.emit('error', err);
			// 創建失敗也嘗試重連
			if (this.shouldReconnect && !this.manualClose) {
				this.attemptReconnect();
			}
		}
	}

	/* 斷開鏈接 */
	disconnect(code?: number, reason?: string): void {
		this.manualClose = true;
		this.shouldReconnect = false;
		this.clearReconnectTimer();

		if (this.ws) {
			try {
				this.ws.close(code, reason);
			} finally {
				this.ws = null;
			}
		}
	}

	/* 重置重連狀態（用於手動觸發重連） */
	resetReconnect(): void {
		this.clearReconnectTimer();
		this.reconnectAttempts = 0;
		this.shouldReconnect = true;
		this.manualClose = false;
	}

	isOpen(): boolean {
		return this.ws?.readyState === WebSocket.OPEN;
	}

	on(type: string, handler: (payload: unknown) => void): void {
		this.emitter.on(type, handler);
	}

	off(type: string, handler: (payload: unknown) => void): void {
		this.emitter.off(type, handler);
	}

	send<T = unknown>(type: string, payload: T): boolean {
		const msg: WebSocketMessage<T> = { type, payload };
		if (this.ws && this.ws.readyState === WebSocket.OPEN) {
			try {
				this.ws.send(JSON.stringify(msg));
				return true;
			} catch (error) {
				console.error('WS send failed', error);
				return false;
			}
		}

		console.warn('WS not ready, sending skipped', msg);
		return false;
	}
}

// 导出单例（不自动连接）
export const wsService = new WebSocketService();
