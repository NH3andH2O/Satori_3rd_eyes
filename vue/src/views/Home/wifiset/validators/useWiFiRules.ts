export function useWiFiRules(form: any, t: Function) {
	return {
		ssid: [
			{
				validator: (_: any, value: string, callback: Function) => {
					if (!form.is_wifi) return callback();
					if (!value) {
						return callback(new Error(t('wifi_ssid_empty')));
					}
					callback();
				},
				trigger: 'blur',
			},
		],
	};
}
