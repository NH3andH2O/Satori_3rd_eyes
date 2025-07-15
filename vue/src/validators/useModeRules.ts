export function useModeRules(form: any, t: Function) {
	return {
		mode: [
			{
				validator: (_: any, value: number, callback: Function) => {
					if (value === 0) {
						return callback(new Error(t('mode_empty')));
					}
					callback();
				},
				trigger: 'change',
			},
		],
		correction_timer: [
			{
				validator: (_: any, value: number, callback: Function) => {
					if (form.mode !== 1) return callback();
					if (value < 0) {
						return callback(new Error(t('correction_time_negative')));
					} else if (value > 65535) {
						return callback(new Error(t('correction_time_over_range')));
					}
					callback();
				},
				trigger: 'blur',
			},
		],
	};
}
