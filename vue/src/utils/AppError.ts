/**
 * 應用程式自定義錯誤類
 * 用於包含錯誤代碼和國際化錯誤信息
 */
export class AppError extends Error {
	public readonly code: number;

	constructor(code: number, message: string) {
		super(message);
		this.code = code;
		this.name = 'AppError';

		// 維護正確的堆棧追蹤（僅在 V8 引擎中可用）
		// eslint-disable-next-line @typescript-eslint/no-explicit-any
		if (typeof (Error as any).captureStackTrace === 'function') {
			// eslint-disable-next-line @typescript-eslint/no-explicit-any
			(Error as any).captureStackTrace(this, AppError);
		}
	}

	/**
	 * 將錯誤轉換為 API 錯誤格式
	 */
	toApiError() {
		return {
			message: this.message,
			code: this.code,
		};
	}
}

/**
 * 創建應用程式錯誤的工廠函數
 */
export function createAppError(code: number, message: string): AppError {
	return new AppError(code, message);
}
