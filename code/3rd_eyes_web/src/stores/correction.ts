import { defineStore } from 'pinia';
import type { AdvancedConfig } from '@/types';

export const useCorrectionStore = defineStore('correction', {
	state: () => ({
		correction_timer: 0,
		gyroscope_eyelid_angle: 45,
	}),
	actions: {
		setCorrectionTimer(v: number) {
			this.correction_timer = v;
		},
		patchFromServer(payload: Partial<AdvancedConfig>) {
			if (typeof payload.correction_timer === 'number') this.correction_timer = payload.correction_timer;
			if (typeof payload.gyroscope_eyelid_angle === 'number') this.gyroscope_eyelid_angle = payload.gyroscope_eyelid_angle;
		},
	},
});
