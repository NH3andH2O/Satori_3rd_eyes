import apiClient from './client';
import type { ApiPostResponse, ApiResponse, ServoConfig, ServoConfigRequest } from '@/types';
import { API_ENDPOINTS } from '@/config';

export const servoApi = {
	async getConfig(): Promise<ServoConfig> {
		const response = await apiClient.get<ApiResponse<ServoConfig>>(API_ENDPOINTS.SERVO_CONFIG, {
			timeout: 5000,
		});
		return response.data.data;
	},

	async setConfig(config: ServoConfigRequest): Promise<ApiPostResponse> {
		const response = await apiClient.post<ApiPostResponse>(API_ENDPOINTS.SET_SERVO_CONFIG, config);
		return response.data;
	},
};
