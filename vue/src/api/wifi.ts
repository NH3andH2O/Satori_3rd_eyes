import apiClient from './client';
import type { WifiConfig, WifiConfigRequest, ApiResponse } from '@/types';

/**
 * WiFi 配置相關 API
 */
export const wifiApi = {
	/**
	 * 獲取 WiFi 配置
	 */
	async getConfig(): Promise<WifiConfig> {
		const response = await apiClient.get<WifiConfig>('/api/wifi_config', {
			timeout: 5000,
		});
		// 後端直接返回數據，不是包在 ApiResponse 中
		return response.data || { is_wifi: false, ssid: '', password: '' };
	},

	/**
	 * 設置 WiFi 配置
	 */
	async setConfig(config: WifiConfigRequest): Promise<ApiResponse> {
		const response = await apiClient.post<ApiResponse>('/api/set_wifi_config', config);
		return response.data;
	},
};

/**
 * SoftAP 配置相關 API
 */
export const softAPApi = {
	/**
	 * 獲取 SoftAP 配置
	 */
	async getConfig(): Promise<any> {
		const response = await apiClient.get<any>('/api/softap_config', {
			timeout: 5000,
		});
		// 後端直接返回數據，不是包在 ApiResponse 中
		return response.data || { is_softAP: false, ssid: '', password: '' };
	},

	/**
	 * 設置 SoftAP 配置
	 */
	async setConfig(config: any): Promise<ApiResponse> {
		const response = await apiClient.post<ApiResponse>('/api/set_softap_config', config);
		return response.data;
	},
};
