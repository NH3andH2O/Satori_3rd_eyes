import apiClient from './client';
import type { ModeConfig, ModeConfigRequest, ApiResponse } from '@/types';

/**
 * 模式配置相關 API
 */
export const modeApi = {
	/**
	 * 獲取模式配置
	 */
	async getConfig(): Promise<ModeConfig> {
		const response = await apiClient.get<ModeConfig>('/api/mode_config', {
			timeout: 5000,
		});
		// 後端直接返回數據，不是包在 ApiResponse 中
		return response.data || { mode: 0 };
	},

	/**
	 * 設置模式配置
	 */
	async setConfig(config: ModeConfigRequest): Promise<ApiResponse> {
		const response = await apiClient.post<ApiResponse>('/api/set_mode_config', config);
		return response.data;
	},
};
