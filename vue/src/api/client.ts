import axios, { AxiosError } from 'axios';
import type { AxiosInstance, InternalAxiosRequestConfig, AxiosResponse } from 'axios';
import type { ApiError } from '@/types';
import { ElMessage } from 'element-plus';

/**
 * 創建 Axios 實例
 */
const apiClient: AxiosInstance = axios.create({
	baseURL: import.meta.env.VITE_API_BASE_URL || '',
	timeout: 10000,
	headers: {
		'Content-Type': 'application/json',
	},
});

/**
 * 請求攔截器
 */
apiClient.interceptors.request.use(
	(config: InternalAxiosRequestConfig) => {
		// 可以在這裡添加 token 等認證信息
		// const token = localStorage.getItem('token');
		// if (token && config.headers) {
		//   config.headers.Authorization = `Bearer ${token}`;
		// }

		console.log(`API 請求: ${config.method?.toUpperCase()} ${config.url}`);
		return config;
	},
	(error: AxiosError) => {
		console.error('請求攔截器錯誤:', error);
		return Promise.reject(error);
	},
);

/**
 * 響應攔截器
 */
apiClient.interceptors.response.use(
	(response: AxiosResponse) => {
		console.log(`API 響應: ${response.config.url}`, response.data);

		// 統一處理業務邏輯錯誤（僅針對有 success 字段的響應）
		if (response.data && typeof response.data === 'object' && 'success' in response.data) {
			if (!response.data.success) {
				const errorMessage = response.data.message || '請求失敗';
				console.error('API 業務錯誤:', errorMessage);
				// 不在這裡彈出錯誤，讓調用者處理
				return Promise.reject({
					message: errorMessage,
					code: 'BUSINESS_ERROR',
					status: response.status,
					details: response.data,
				} as ApiError);
			}
		}

		return response;
	},
	(error: AxiosError) => {
		console.error('API 響應錯誤:', error);

		const apiError: ApiError = {
			message: '網絡錯誤',
			status: error.response?.status,
			code: error.code,
			details: error.response?.data,
		};

		// 根據不同的錯誤狀態碼處理
		if (error.response) {
			switch (error.response.status) {
				case 400:
					apiError.message = '請求參數錯誤';
					break;
				case 401:
					apiError.message = '未授權，請重新登入';
					break;
				case 403:
					apiError.message = '拒絕訪問';
					break;
				case 404:
					apiError.message = '請求的資源不存在';
					break;
				case 500:
					apiError.message = '服務器錯誤';
					break;
				case 503:
					apiError.message = '服務不可用';
					break;
				default:
					apiError.message = (error.response.data as any)?.message || `請求失敗 (${error.response.status})`;
			}
		} else if (error.request) {
			// 請求已發送但沒有收到響應
			if (error.code === 'ECONNABORTED') {
				apiError.message = '請求超時，請檢查網絡連接';
			} else if (error.message === 'Network Error') {
				apiError.message = '網絡連接失敗，請檢查網絡';
			} else {
				apiError.message = '無法連接到服務器';
			}
		} else {
			apiError.message = error.message || '未知錯誤';
		}

		return Promise.reject(apiError);
	},
);

/**
 * 統一錯誤處理函數
 */
export function handleApiError(error: ApiError | Error | any, customMessage?: string): void {
	const errorMessage = customMessage || (error as ApiError)?.message || error?.message || '未知錯誤';

	console.error('API 錯誤處理:', errorMessage, error);

	// 使用 Element Plus 顯示錯誤消息
	ElMessage.error({
		message: errorMessage,
		duration: 3000,
		showClose: true,
	});
}

/**
 * 統一成功處理函數
 */
export function handleApiSuccess(message?: string): void {
	if (message) {
		ElMessage.success({
			message,
			duration: 2000,
			showClose: true,
		});
	}
}

export default apiClient;
