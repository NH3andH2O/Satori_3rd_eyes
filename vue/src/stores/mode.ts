import { defineStore } from 'pinia';

export const useModeStore = defineStore('mode', {
	state: () => ({
		mode: 0,
		correction_timer: 0,
	}),
	actions: {
		setMode(v: number) {
			this.mode = v;
		},
		setCorrectionTimer(v: number) {
			this.correction_timer = v;
		},
		patchFromServer(payload: Partial<{ mode: number; correction_timer: number }>) {
			if (typeof payload.mode === 'number') this.mode = payload.mode;
			if (typeof payload.correction_timer === 'number') this.correction_timer = payload.correction_timer;
		},
	},
});
