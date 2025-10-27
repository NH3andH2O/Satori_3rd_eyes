/**
 * API 響應通用類型
 */
export interface ApiResponse<T = any> {
	success: boolean;
	message?: string;
	data?: T;
}

/**
 * API 錯誤類型
 */
export interface ApiError {
	message: string;
	code?: string | number;
	status?: number;
	details?: any;
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
