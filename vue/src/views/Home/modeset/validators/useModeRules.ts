export function useModeRules(t: (key: string) => string) {
	return {
		mode: [
			{
				validator: async (_: any, value: number) => {
					if (value === 0) throw new Error(t('mode_empty'));
					if (value > 1) throw new Error(t('mode_invalid'));
				},
				trigger: 'change',
			},
		],
	};
}
