import type { AppConfig } from '@/types';

/**
 * 應用配置常量
 */
export const APP_CONFIG: AppConfig = {
	// 虛擬搖桿配置
	joystick: {
		move: {
			mode: 'dynamic',
			size: 140,
			color: '#22c55e',
			deadzone: 0.12,
			throttle: 60,
			scale: 100,
		},
		eyelid: {
			mode: 'dynamic',
			size: 140,
			color: '#f548fdc5',
			deadzone: 0.12,
			throttle: 60,
			scale: 100,
		},
	},

	// WebSocket 配置
	websocket: {
		maxReconnectAttempts: 5,
		reconnectDelay: 1000, // 1 秒
		maxReconnectDelay: 30000, // 30 秒
	},

	// API 配置
	api: {
		timeout: 10000, // 10 秒
		baseURL: import.meta.env.VITE_API_BASE_URL || '',
	},
};

/**
 * 響應式斷點
 */
export const BREAKPOINTS = {
	MOBILE: 768,
	TABLET: 1024,
	DESKTOP: 1280,
} as const;

/**
 * WebSocket 消息類型常量
 */
export const WS_MESSAGE_TYPES = {
	JOYSTICK_MOVE: 'joystick/move/move',
	JOYSTICK_MOVE_END: 'joystick/move/end',
	JOYSTICK_EYELID: 'joystick/eyelid/move',
	JOYSTICK_EYELID_END: 'joystick/eyelid/end',
	SERVO_PREVIEW: 'preview',
} as const;

/**
 * 模式常量
 */
export const MODES = {
	SERVO_SETUP: 0,
	GYROSCOPE: 1,
	NETWORK_CONTROL: 2,
} as const;

/**
 * API 端點
 */
export const API_ENDPOINTS = {
	VERSION: '/api/version',

	// WiFi
	WIFI_CONFIG: '/api/wifi_config',
	SET_WIFI_CONFIG: '/api/set_wifi_config',

	// SoftAP
	SOFTAP_CONFIG: '/api/softap_config',
	SET_SOFTAP_CONFIG: '/api/set_softap_config',

	// 模式
	MODE_CONFIG: '/api/mode_config',
	SET_MODE_CONFIG: '/api/set_mode_config',
	SERVO_CONFIG: '/api/servo_config',
	SET_SERVO_CONFIG: '/api/set_servo_config',

	// 高級
	ADVANCED_CONFIG: '/api/advanced_config',
	SET_ADVANCED_CONFIG: '/api/set_advanced_config',
} as const;
