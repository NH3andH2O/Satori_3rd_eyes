import { computed, ref, type Ref } from 'vue';
import { useWebSocket } from '../../../composables/useWebSocket';
import type { JoystickMovePayload, EyelidPayload } from '@/types';

interface JoystickControlOptions {
	/** 訊息類型（移動/眼瞼） */
	messageType: string;
	/** 結束訊息類型 */
	endMessageType: string;
	/** X 軸是否反轉 */
	invertX?: Ref<boolean>;
	/** Y 軸是否反轉 */
	invertY?: Ref<boolean>;
	/** 是否為單軸控制（僅 Y 軸） */
	singleAxis?: boolean;
}

/**
 * 搖桿控制邏輯的可組合函數
 * 使用 requestAnimationFrame 優化性能，合併同一幀的多次更新
 */
export function useJoystickControl(options: JoystickControlOptions) {
	const { messageType, endMessageType, invertX, invertY, singleAxis = false } = options;
	const { send, isOpen } = useWebSocket();

	// 日誌記錄（用於調試或顯示）
	const log = ref<JoystickMovePayload | EyelidPayload>(singleAxis ? { vy: 0 } : { vx: 0, vy: 0 });
	const vx = computed(() => ('vx' in log.value ? log.value.vx : 0));
	const vy = computed(() => log.value.vy);

	// 待處理的數據（用於 rAF 批處理）
	let pending: JoystickMovePayload | EyelidPayload | null = null;
	let rafId = 0;
	let isEnded = false;

	/**
	 * 刷新待處理的數據到 WebSocket
	 */
	function flushPending() {
		if (!pending) return;

		const connected = typeof isOpen === 'function' ? isOpen() : true;

		if (connected && !isEnded) {
			send?.(messageType, pending);
		}

		pending = null;
		rafId = 0;
		isEnded = false;
	}

	/**
	 * 處理搖桿移動事件
	 */
	function onMove(payload: JoystickMovePayload) {
		let data: JoystickMovePayload | EyelidPayload;

		if (singleAxis) {
			// 單軸控制（眼瞼）
			const vy = invertY?.value ? payload.vy : -payload.vy;
			data = { vy };
		} else {
			// 雙軸控制（移動）
			const vx = invertX?.value ? -payload.vx : payload.vx;
			const vy = invertY?.value ? payload.vy : -payload.vy;
			data = { vx, vy };
		}

		// 更新日誌
		log.value = data;

		// 使用 rAF 批處理，避免在同一幀內多次發送
		pending = { ...data };
		if (!rafId) {
			rafId = requestAnimationFrame(flushPending);
		}
	}

	/**
	 * 處理搖桿結束事件
	 */
	function onEnd() {
		isEnded = true;
		send?.(endMessageType, {});
	}

	return {
		log,
		vx,
		vy,
		onMove,
		onEnd,
	};
}
