import { computed, ref } from 'vue';
import { useRouter } from 'vue-router';
import { modeApi, servoApi } from '@/api';
import { APP_CONFIG, MODES } from '@/config';
import { useWebSocket } from '@/composables/useWebSocket';
import type { ReconnectAttemptEvent, ServoConfig } from '@/types';
import type { SetupConnectionState } from '../types';
import i18n from '@/i18n';

const WS_CONNECT_TIMEOUT = 30000;

export function useSetupInit() {
	const router = useRouter();
	const { connect, disconnect, isOpen, on, off, resetReconnect } = useWebSocket();

	const isLoading = ref(true);
	const loadingText = ref('');
	const connectionState = ref<SetupConnectionState>('idle');
	const servoConfig = ref<ServoConfig | null>(null);
	const errorTitle = ref('');
	const errorMessage = ref('');
	const errorDialogVisible = ref(false);
	const reconnectAttempt = ref(0);
	const reconnectDelay = ref(0);
	const reconnectToken = ref(0);
	const isManualReconnecting = ref(false);

	let waitOpenHandler: (() => void) | null = null;
	let waitOpenTimeout: ReturnType<typeof setTimeout> | null = null;
	let closeHandler: (() => void) | null = null;
	let openHandler: (() => void) | null = null;
	let reconnectAttemptHandler: ((payload: unknown) => void) | null = null;
	let reconnectFailedHandler: (() => void) | null = null;
	let initialized = false;
	let connectionGeneration = 0;

	const canOperate = computed(() => connectionState.value === 'connected' && isOpen());

	function showError(title: string, message: string) {
		errorTitle.value = title;
		errorMessage.value = message;
		errorDialogVisible.value = true;
	}

	async function checkMode(): Promise<boolean> {
		loadingText.value = i18n.global.t('setup.loading.checking_mode');
		try {
			const config = await modeApi.getConfig();
			if (Number(config.mode) !== MODES.SERVO_SETUP) {
				connectionState.value = 'mode-error';
				showError(i18n.global.t('setup.errors.mode_title'), i18n.global.t('setup.mode_error'));
				return false;
			}
			return true;
		} catch {
			connectionState.value = 'error';
			showError(i18n.global.t('setup.errors.connection_title'), i18n.global.t('setup.state.mode_failed'));
			return false;
		}
	}

	async function loadServoConfig(): Promise<boolean> {
		loadingText.value = i18n.global.t('setup.loading.loading_config');
		try {
			servoConfig.value = await servoApi.getConfig();
			return true;
		} catch {
			connectionState.value = 'error';
			showError(i18n.global.t('setup.errors.connection_title'), i18n.global.t('setup.state.get_failed'));
			return false;
		}
	}

	function cleanupWaitForOpen() {
		if (waitOpenHandler) {
			off('open', waitOpenHandler);
			waitOpenHandler = null;
		}
		if (waitOpenTimeout) {
			clearTimeout(waitOpenTimeout);
			waitOpenTimeout = null;
		}
	}

	function waitForWebSocket(): Promise<boolean> {
		loadingText.value = i18n.global.t('setup.loading.connecting_ws');
		if (isOpen()) return Promise.resolve(true);

		return new Promise((resolve) => {
			waitOpenTimeout = setTimeout(() => {
				cleanupWaitForOpen();
				resolve(false);
			}, WS_CONNECT_TIMEOUT);

			waitOpenHandler = () => {
				cleanupWaitForOpen();
				resolve(true);
			};
			on('open', waitOpenHandler);
		});
	}

	async function restoreAfterReconnect() {
		const generation = connectionGeneration;
		connectionState.value = 'reconnecting';
		if (!(await checkMode())) return;
		if (generation !== connectionGeneration || !isOpen()) return;
		reconnectToken.value++;
	}

	function completeReconnect(previewRestored: boolean) {
		if (!previewRestored || !isOpen()) {
			connectionState.value = 'failed';
			isManualReconnecting.value = false;
			return;
		}

		connectionState.value = 'connected';
		reconnectAttempt.value = 0;
		reconnectDelay.value = 0;
		isManualReconnecting.value = false;
		errorDialogVisible.value = false;
	}

	function monitorConnection() {
		closeHandler = () => {
			connectionGeneration++;
			connectionState.value = 'disconnected';
		};
		openHandler = () => {
			if (
				!isLoading.value &&
				(connectionState.value === 'disconnected' || connectionState.value === 'reconnecting' || connectionState.value === 'failed')
			) {
				void restoreAfterReconnect();
			}
		};
		reconnectAttemptHandler = (payload: unknown) => {
			const event = payload as ReconnectAttemptEvent;
			connectionState.value = 'reconnecting';
			reconnectAttempt.value = event.attempt;
			reconnectDelay.value = event.delay;
		};
		reconnectFailedHandler = () => {
			connectionState.value = 'failed';
			isManualReconnecting.value = false;
		};

		on('close', closeHandler);
		on('open', openHandler);
		on('reconnect:attempt', reconnectAttemptHandler);
		on('reconnect:failed', reconnectFailedHandler);
	}

	async function initialize(): Promise<boolean> {
		isLoading.value = true;
		connectionState.value = 'initializing';
		errorDialogVisible.value = false;
		cleanupWaitForOpen();

		if (!(await checkMode()) || !(await loadServoConfig())) {
			isLoading.value = false;
			return false;
		}

		if (!initialized) {
			monitorConnection();
			initialized = true;
		}

		connect();
		if (!(await waitForWebSocket())) {
			connectionState.value = 'failed';
			showError(i18n.global.t('setup.errors.websocket_title'), i18n.global.t('setup.loading.ws_timeout'));
			isLoading.value = false;
			return false;
		}

		connectionState.value = 'connected';
		loadingText.value = '';
		isLoading.value = false;
		return true;
	}

	function manualReconnect() {
		if (isManualReconnecting.value) return;
		isManualReconnecting.value = true;
		reconnectAttempt.value = 0;
		connectionState.value = 'reconnecting';
		if (isOpen()) {
			void restoreAfterReconnect();
			return;
		}
		resetReconnect();
		connect();
	}

	function goHome() {
		void router.replace({ name: 'Home' });
	}

	function cleanup() {
		cleanupWaitForOpen();
		if (closeHandler) off('close', closeHandler);
		if (openHandler) off('open', openHandler);
		if (reconnectAttemptHandler) off('reconnect:attempt', reconnectAttemptHandler);
		if (reconnectFailedHandler) off('reconnect:failed', reconnectFailedHandler);
		closeHandler = null;
		openHandler = null;
		reconnectAttemptHandler = null;
		reconnectFailedHandler = null;
		initialized = false;
		disconnect(1000, 'leave /setup');
	}

	return {
		isLoading,
		loadingText,
		connectionState,
		servoConfig,
		errorTitle,
		errorMessage,
		errorDialogVisible,
		reconnectAttempt,
		reconnectDelay,
		maxReconnectAttempts: APP_CONFIG.websocket.maxReconnectAttempts,
		reconnectToken,
		isManualReconnecting,
		canOperate,
		initialize,
		manualReconnect,
		completeReconnect,
		goHome,
		cleanup,
	};
}
