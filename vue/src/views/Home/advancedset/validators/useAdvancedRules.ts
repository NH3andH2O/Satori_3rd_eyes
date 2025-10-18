import { unref, type Ref } from 'vue';

export function useAdvancedRules(modeRef: Ref<number>, t: (k: string) => string) {
	return {
		correction_timer: [
			{
				validator: async (_: any, value: number) => {
					if (unref(modeRef) === 1) {
						const n = Number(value);
						if (Number.isNaN(n)) throw new Error(t('correction_time_invalid_number'));
						if (n < 0) throw new Error(t('correction_time_negative'));
						if (n > 65535) throw new Error(t('correction_time_over_range'));
					}
				},
				trigger: 'blur',
			},
		],
	};
}
