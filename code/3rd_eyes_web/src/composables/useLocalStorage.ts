import { ref, watch } from 'vue';
import type { Ref } from 'vue';

/**
 * localStorage 封裝 composable
 * @param key 存儲鍵名
 * @param defaultValue 默認值
 * @returns 響應式引用
 */
export function useLocalStorage<T>(key: string, defaultValue: T): Ref<T> {
	// 從 localStorage 讀取初始值
	const getStoredValue = (): T => {
		try {
			const item = localStorage.getItem(key);
			if (item === null) {
				return defaultValue;
			}
			// 嘗試解析 JSON
			return JSON.parse(item) as T;
		} catch (error) {
			console.warn(`無法從 localStorage 讀取 ${key}:`, error);
			return defaultValue;
		}
	};

	// 創建響應式引用
	const storedValue = ref<T>(getStoredValue()) as Ref<T>;

	// 監聽變化並同步到 localStorage
	watch(
		storedValue,
		(newValue) => {
			try {
				localStorage.setItem(key, JSON.stringify(newValue));
			} catch (error) {
				console.error(`無法寫入 localStorage ${key}:`, error);
			}
		},
		{ deep: true },
	);

	return storedValue;
}

/**
 * 刪除 localStorage 項目
 */
export function removeLocalStorageItem(key: string): void {
	try {
		localStorage.removeItem(key);
	} catch (error) {
		console.error(`無法刪除 localStorage ${key}:`, error);
	}
}

/**
 * 清空所有 localStorage
 */
export function clearLocalStorage(): void {
	try {
		localStorage.clear();
	} catch (error) {
		console.error('無法清空 localStorage:', error);
	}
}
