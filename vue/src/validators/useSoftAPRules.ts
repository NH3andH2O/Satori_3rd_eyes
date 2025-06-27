export function useSoftAPRules(form: any, t: Function) {
	const printableASCII = /^[\x20-\x7E]+$/;

	return {
		password: [
			{
				validator: (_: any, value: string, callback: Function) => {
					if (!form.is_change_password) return callback();

					if (value.length < 8 && value.length !== 0) {
						return callback(new Error(t('password_length_short')));
					}
					if (value.length > 63) {
						return callback(new Error(t('password_length_long')));
					}
					if (!printableASCII.test(value) && value.length !== 0) {
						return callback(new Error(t('password_invalid_chars')));
					}

					callback();
				},
				trigger: 'blur',
			},
		],
		password_confirm: [
			{
				validator: (_: any, value: string, callback: Function) => {
					if (!form.is_change_password) return callback();

					if (value !== form.password) {
						return callback(new Error(t('password_not_match')));
					}

					callback();
				},
				trigger: 'blur',
			},
		],
	};
}
