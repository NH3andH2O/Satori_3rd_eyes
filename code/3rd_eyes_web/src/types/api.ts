/**
 * API GET 請求響應類型（帶 data）
 */
export interface ApiResponse<T = unknown> {
	success: boolean;
	code: number;
	message: string;
	data: T;
}

/**
 * API POST 請求響應類型（不帶 data）
 */
export interface ApiPostResponse {
	success: boolean;
	code: number;
	message: string;
}

/**
 * API 錯誤類型
 */
export interface ApiError {
	message: string;
	code?: string | number;
	status?: number;
	details?: unknown;
}

/**
 * WiFi 配置類型
 */
export interface WifiConfig {
	is_wifi: boolean;
	ssid: string;
	password: string;
}

/**
 * SoftAP 配置類型
 */
export interface SoftAPConfig {
	is_softAP: boolean;
	ssid: string;
	password: string;
}

/**
 * 模式配置類型
 */
export interface ModeConfig {
	mode: number;
}

/**
 * 高級配置類型
 */
export interface AdvancedConfig {
	correction_timer: number;
}

/**
 * WiFi 配置請求類型
 */
export type WifiConfigRequest = Partial<WifiConfig>;

/**
 * SoftAP 配置請求類型
 */
export type SoftAPConfigRequest = Partial<SoftAPConfig>;

/**
 * 模式配置請求類型
 */
export type ModeConfigRequest = Partial<ModeConfig>;

/**
 * 高級配置請求類型
 */
export type AdvancedConfigRequest = Partial<AdvancedConfig>;
