import { unref, type Ref } from 'vue';

export function useAdvancedRules(modeRef: Ref<number>, t: (k: string) => string) {
	return {
		correction_timer: [
			{
				validator: async (_: unknown, value: unknown) => {
					// 只在模式 1 時進行校驗
					if (unref(modeRef) !== 1) {
						return Promise.resolve();
					}

					// 如果值為空、undefined 或 null，允許通過（會使用後端默認值）
					if (value === undefined || value === null || value === '') {
						return Promise.resolve();
					}

					const n = Number(value);

					// 檢查是否為有效數字
					if (Number.isNaN(n)) {
						return Promise.reject(new Error(t('correction_time_invalid_number')));
					}

					// 檢查範圍
					if (n < 0) {
						return Promise.reject(new Error(t('correction_time_negative')));
					}

					if (n > 65535) {
						return Promise.reject(new Error(t('correction_time_over_range')));
					}

					return Promise.resolve();
				},
				trigger: 'blur',
			},
		],
	};
}
