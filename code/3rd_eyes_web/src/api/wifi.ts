import apiClient from './client';
import type { WifiConfig, WifiConfigRequest, SoftAPConfig, SoftAPConfigRequest, ApiResponse, ApiPostResponse } from '@/types';

/**
 * WiFi 配置相關 API
 */
export const wifiApi = {
	/**
	 * 獲取 WiFi 配置
	 * GET 響應格式: {"success":true,"code":0,"message":"","data":{...}}
	 */
	async getConfig(): Promise<WifiConfig> {
		const response = await apiClient.get<ApiResponse<WifiConfig>>('/api/wifi_config', {
			timeout: 5000,
		});
		// 從新的響應格式中提取 data
		return response.data.data;
	},

	/**
	 * 設置 WiFi 配置
	 * POST 響應格式: {"success":true,"code":0,"message":""}
	 */
	async setConfig(config: WifiConfigRequest): Promise<ApiPostResponse> {
		const response = await apiClient.post<ApiPostResponse>('/api/set_wifi_config', config);
		return response.data;
	},
};

/**
 * SoftAP 配置相關 API
 */
export const softAPApi = {
	/**
	 * 獲取 SoftAP 配置
	 * GET 響應格式: {"success":true,"code":0,"message":"","data":{...}}
	 */
	async getConfig(): Promise<SoftAPConfig> {
		const response = await apiClient.get<ApiResponse<SoftAPConfig>>('/api/softap_config', {
			timeout: 5000,
		});
		// 從新的響應格式中提取 data
		return response.data.data;
	},

	/**
	 * 設置 SoftAP 配置
	 * POST 響應格式: {"success":true,"code":0,"message":""}
	 */
	async setConfig(config: SoftAPConfigRequest): Promise<ApiPostResponse> {
		const response = await apiClient.post<ApiPostResponse>('/api/set_softap_config', config);
		return response.data;
	},
};
