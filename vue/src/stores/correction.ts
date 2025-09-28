import { defineStore } from 'pinia';

export const useCorrectionStore = defineStore('correction', {
	state: () => ({
		correction_timer: 0,
	}),
	actions: {
		setCorrectionTimer(v: number) {
			this.correction_timer = v;
		},
		patchFromServer(payload: Partial<{ correction_timer: number }>) {
			if (typeof payload.correction_timer === 'number') this.correction_timer = payload.correction_timer;
		},
	},
});
