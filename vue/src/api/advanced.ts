import apiClient from './client';
import type { AdvancedConfig, AdvancedConfigRequest, ApiResponse } from '@/types';

/**
 * 高級配置相關 API
 */
export const advancedApi = {
	/**
	 * 獲取高級配置
	 */
	async getConfig(): Promise<AdvancedConfig> {
		const response = await apiClient.get<AdvancedConfig>('/api/advanced_config', {
			timeout: 5000,
		});
		// 後端直接返回數據，不是包在 ApiResponse 中
		return response.data || { correction_timer: 0 };
	},

	/**
	 * 設置高級配置
	 */
	async setConfig(config: AdvancedConfigRequest): Promise<ApiResponse> {
		const response = await apiClient.post<ApiResponse>('/api/set_advanced_config', config);
		return response.data;
	},
};
