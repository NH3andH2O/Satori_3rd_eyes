import apiClient from './client';
import type { ModeConfig, ModeConfigRequest, ApiResponse, ApiPostResponse } from '@/types';

/**
 * 模式配置相關 API
 */
export const modeApi = {
	/**
	 * 獲取模式配置
	 * GET 響應格式: {"success":true,"code":0,"message":"","data":{...}}
	 */
	async getConfig(): Promise<ModeConfig> {
		const response = await apiClient.get<ApiResponse<ModeConfig>>('/api/mode_config', {
			timeout: 5000,
		});
		// 從新的響應格式中提取 data
		return response.data.data;
	},

	/**
	 * 設置模式配置
	 * POST 響應格式: {"success":true,"code":0,"message":""}
	 */
	async setConfig(config: ModeConfigRequest): Promise<ApiPostResponse> {
		const response = await apiClient.post<ApiPostResponse>('/api/set_mode_config', config);
		return response.data;
	},
};
