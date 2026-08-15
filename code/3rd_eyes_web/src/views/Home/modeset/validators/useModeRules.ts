export function useModeRules(t: (key: string) => string) {
	return {
		mode: [
			{
				validator: async (_: unknown, value: number) => {
					if (!Number.isInteger(value) || value < 0 || value > 2) throw new Error(t('mode_invalid'));
				},
				trigger: 'change',
			},
		],
	};
}
