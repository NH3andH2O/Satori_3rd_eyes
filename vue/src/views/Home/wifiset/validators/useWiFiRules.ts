interface WifiForm {
	is_wifi: boolean;
}

type TranslateFunction = (key: string) => string;
type CallbackFunction = (error?: Error) => void;

export function useWiFiRules(form: WifiForm, t: TranslateFunction) {
	return {
		ssid: [
			{
				validator: (_: unknown, value: string, callback: CallbackFunction) => {
					if (!form.is_wifi) return callback();
					if (!value) {
						return callback(new Error(t('ssid_empty')));
					}
					callback();
				},
				trigger: 'blur',
			},
		],
	};
}
