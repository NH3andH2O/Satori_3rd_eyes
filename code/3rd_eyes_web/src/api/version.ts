import apiClient from './client';
import type { ApiResponse, VersionInfo } from '@/types';
import { API_ENDPOINTS } from '@/config';

export const versionApi = {
	async getVersion(): Promise<string> {
		const response = await apiClient.get<ApiResponse<VersionInfo>>(API_ENDPOINTS.VERSION, {
			timeout: 5000,
		});
		return response.data.data.version;
	},
};
