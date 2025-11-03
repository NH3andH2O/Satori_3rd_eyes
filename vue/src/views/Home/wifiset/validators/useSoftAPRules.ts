interface SoftAPForm {
	is_change_password: boolean;
	password: string;
}

type TranslateFunction = (key: string) => string;
type CallbackFunction = (error?: Error) => void;

export function useSoftAPRules(form: SoftAPForm, t: TranslateFunction) {
	const printableASCII = /^[\x20-\x7E]+$/;

	return {
		password: [
			{
				validator: (_: unknown, value: string, callback: CallbackFunction) => {
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
				validator: (_: unknown, value: string, callback: CallbackFunction) => {
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
