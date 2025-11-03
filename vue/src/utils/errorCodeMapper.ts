/**
 * 錯誤代碼映射工具
 * 將 Axios 的字符串錯誤代碼轉換為數字代碼
 */

import i18n from '@/i18n';

/**
 * Axios 錯誤代碼映射表
 */
export const AXIOS_ERROR_CODE_MAP: Record<string, number> = {
	// 網絡錯誤
	ERR_NETWORK: -1,

	// 請求超時
	ECONNABORTED: -2,
	ETIMEDOUT: -3,

	// 請求取消
	ERR_CANCELED: -4,

	// 請求配置錯誤
	ERR_BAD_OPTION_VALUE: -10,
	ERR_BAD_OPTION: -11,
	ERR_INVALID_URL: -12,

	// 響應錯誤
	ERR_BAD_RESPONSE: -20,
	ERR_BAD_REQUEST: -21,

	// 重定向錯誤
	ERR_FR_TOO_MANY_REDIRECTS: -30,

	// 不支持的功能
	ERR_NOT_SUPPORT: -40,
	ERR_DEPRECATED: -41,
};

/**
 * HTTP 狀態碼映射（如果需要也可以映射 HTTP 狀態碼）
 */
export const HTTP_STATUS_CODE_MAP: Record<number, number> = {
	// 4xx 客戶端錯誤
	400: -100, // Bad Request
	401: -101, // Unauthorized
	403: -103, // Forbidden
	404: -104, // Not Found
	405: -105, // Method Not Allowed
	408: -108, // Request Timeout
	409: -109, // Conflict
	422: -122, // Unprocessable Entity
	429: -129, // Too Many Requests

	// 5xx 服務器錯誤
	500: -200, // Internal Server Error
	501: -201, // Not Implemented
	502: -202, // Bad Gateway
	503: -203, // Service Unavailable
	504: -204, // Gateway Timeout
};

/**
 * 將 Axios 錯誤代碼轉換為數字
 * @param code - Axios 錯誤代碼（字符串或數字）
 * @param httpStatus - HTTP 狀態碼（可選）
 * @returns 數字錯誤代碼
 */
export function mapErrorCodeToNumber(code?: string | number, httpStatus?: number): number {
	// 如果已經是數字，直接返回
	if (typeof code === 'number') {
		return code;
	}

	// 如果是字符串，查找映射表
	if (typeof code === 'string' && code in AXIOS_ERROR_CODE_MAP) {
		return AXIOS_ERROR_CODE_MAP[code] ?? -999;
	}

	// 如果有 HTTP 狀態碼，使用狀態碼映射
	if (httpStatus && httpStatus in HTTP_STATUS_CODE_MAP) {
		return HTTP_STATUS_CODE_MAP[httpStatus] ?? -httpStatus;
	}

	// 如果有 HTTP 狀態碼但不在映射表中，使用負的狀態碼
	if (httpStatus) {
		return -httpStatus;
	}

	// 默認未知錯誤
	return -999;
}

/**
 * 獲取錯誤代碼的描述信息
 * @param numericCode - 數字錯誤代碼
 * @returns 錯誤描述
 */
export function getErrorDescription(numericCode: number): string {
	const { t } = i18n.global;
	const errorKey = `errors.${numericCode}`;

	// 嘗試獲取翻譯，如果不存在則使用默認格式
	if (t(errorKey) !== errorKey) {
		return t(errorKey);
	}

	// 如果沒有對應的翻譯，使用默認格式
	return t('errors.default', { code: numericCode });
}

/**
 * 反向映射：從數字代碼獲取原始 Axios 代碼（用於調試）
 */
export function getOriginalAxiosCode(numericCode: number): string | undefined {
	for (const [key, value] of Object.entries(AXIOS_ERROR_CODE_MAP)) {
		if (value === numericCode) {
			return key;
		}
	}
	return undefined;
}
