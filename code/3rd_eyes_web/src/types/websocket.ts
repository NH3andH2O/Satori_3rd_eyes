/**
 * WebSocket 消息類型
 */
export interface WebSocketMessage<T = unknown> {
	type: string;
	payload: T;
}

/**
 * WebSocket 連接狀態
 */
export const WebSocketState = {
	CONNECTING: 0,
	OPEN: 1,
	CLOSING: 2,
	CLOSED: 3,
} as const;

export type WebSocketStateType = (typeof WebSocketState)[keyof typeof WebSocketState];

/**
 * 搖桿移動數據
 */
export interface JoystickMovePayload {
	vx: number;
	vy: number;
}

/**
 * 眼瞼控制數據
 */
export interface EyelidPayload {
	vy: number;
}

export interface ServoPreviewPayload {
	upper_eyelid_angle: number;
	lower_eyelid_angle: number;
	eyeball_angle: number;
}

/**
 * 搖桿結束事件
 */
export interface JoystickEndPayload {
	// 可以為空對象
}

/**
 * WebSocket 重連嘗試事件
 */
export interface ReconnectAttemptEvent {
	attempt: number;
	maxAttempts: number;
	delay: number;
}

/**
 * WebSocket 重連失敗事件
 */
export interface ReconnectFailedEvent {
	attempts: number;
}

/**
 * WebSocket 事件映射
 */
export interface WebSocketEvents {
	open: undefined;
	close: CloseEvent;
	error: Event | Error;
	'message:*': WebSocketMessage;
	'reconnect:attempt': ReconnectAttemptEvent;
	'reconnect:failed': ReconnectFailedEvent;
	// 應用特定事件
	'joystick/move/move': JoystickMovePayload;
	'joystick/move/end': JoystickEndPayload;
	'joystick/eyelid/move': EyelidPayload;
	'joystick/eyelid/end': JoystickEndPayload;
}

/**
 * WebSocket 配置選項
 */
export interface WebSocketConfig {
	/** 最大重連次數 */
	maxReconnectAttempts?: number;
	/** 初始重連延遲（毫秒） */
	reconnectDelay?: number;
	/** 最大重連延遲（毫秒） */
	maxReconnectDelay?: number;
	/** 是否自動重連 */
	autoReconnect?: boolean;
}
