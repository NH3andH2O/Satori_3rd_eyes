/**
 * 虛擬搖桿配置
 */
export interface VirtualJoystickConfig {
	/** 搖桿模式 */
	mode: 'dynamic' | 'static' | 'semi';
	/** 搖桿大小（像素） */
	size: number;
	/** 搖桿顏色 */
	color: string;
	/** 死區（0-1） */
	deadzone: number;
	/** 節流時間（毫秒） */
	throttle: number;
	/** 縮放倍數 */
	scale: number;
}

/**
 * 應用配置常量
 */
export interface AppConfig {
	/** 虛擬搖桿配置 */
	joystick: {
		move: VirtualJoystickConfig;
		eyelid: VirtualJoystickConfig;
	};
	/** WebSocket 配置 */
	websocket: {
		maxReconnectAttempts: number;
		reconnectDelay: number;
		maxReconnectDelay: number;
	};
	/** API 配置 */
	api: {
		timeout: number;
		baseURL?: string;
	};
}

/**
 * 本地存儲鍵
 */
export const StorageKeys = {
	SHOW_HELP: 'show_help',
	MOVE_X_INVERT: 'move_X_invert',
	MOVE_Y_INVERT: 'move_Y_invert',
	EYELID_Y_INVERT: 'eyelid_Y_invert',
	LANGUAGE: 'language',
} as const;

export type StorageKey = (typeof StorageKeys)[keyof typeof StorageKeys];
