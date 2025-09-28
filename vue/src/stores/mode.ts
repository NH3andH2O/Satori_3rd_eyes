import { defineStore } from 'pinia';

export const useModeStore = defineStore('mode', {
	state: () => ({
		mode: 0,
	}),
	actions: {
		setMode(v: number) {
			this.mode = v;
		},
		patchFromServer(payload: Partial<{ mode: number }>) {
			if (typeof payload.mode === 'number') this.mode = payload.mode;
		},
	},
});
