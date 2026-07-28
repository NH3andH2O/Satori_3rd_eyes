import apiClient from './client';
import type { AdvancedConfig, AdvancedConfigRequest, ApiResponse, ApiPostResponse } from '@/types';

/**
 * 高級配置相關 API
 */
export const advancedApi = {
	/**
	 * 獲取高級配置
	 * GET 響應格式: {"success":true,"code":0,"message":"","data":{...}}
	 */
	async getConfig(): Promise<AdvancedConfig> {
		const response = await apiClient.get<ApiResponse<AdvancedConfig>>('/api/advanced_config', {
			timeout: 5000,
		});
		// 從新的響應格式中提取 data
		return response.data.data;
	},

	/**
	 * 設置高級配置
	 * POST 響應格式: {"success":true,"code":0,"message":""}
	 */
	async setConfig(config: AdvancedConfigRequest): Promise<ApiPostResponse> {
		const response = await apiClient.post<ApiPostResponse>('/api/set_advanced_config', config);
		return response.data;
	},
};
