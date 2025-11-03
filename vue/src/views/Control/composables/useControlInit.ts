import { ref } from 'vue';
import { useRouter } from 'vue-router';
import { ElMessage } from 'element-plus';
import { modeApi } from '@/api';
import { useWebSocket } from '@/composables/useWebSocket';
import { AppError } from '@/utils/AppError';
import i18n from '@/i18n';

export function useControlInit(connectWebSocket: () => void) {
	const router = useRouter();
	const { isOpen, on, off } = useWebSocket();

	// 加载状态
	const isLoading = ref(true);
	const loadingText = ref('');

	// 错误状态
	const modeErrorDialogVisible = ref(false);
	const errorMessage = ref('');
	const errorTitle = ref('');

	// WebSocket 连接状态追踪
	let wsOpenHandler: (() => void) | null = null;
	let wsCloseHandler: (() => void) | null = null;
	let wsReconnectHandler: (() => void) | null = null;

	/**
	 * 步骤 1: 检查模式配置
	 */
	async function checkMode(): Promise<boolean> {
		loadingText.value = i18n.global.t('controlpage.loading.connecting_ws');

		try {
			const data = await modeApi.getConfig();
			const mode = Number(data.mode);

			if (mode !== 2) {
				// 模式错误
				errorTitle.value = i18n.global.t('controlpage.mode_error.title');
				errorMessage.value = i18n.global.t('controlpage.mode_error.message');
				modeErrorDialogVisible.value = true;
				isLoading.value = false;
				return false;
			}

			return true;
		} catch (error: unknown) {
			// API 连接错误
			if (error instanceof AppError) {
				errorTitle.value = i18n.global.t('controlpage.api_error.title');
				errorMessage.value = error.message;
			} else {
				errorTitle.value = i18n.global.t('controlpage.api_error.title');
				errorMessage.value = i18n.global.t('controlpage.api_error.message');
			}
			modeErrorDialogVisible.value = true;
			isLoading.value = false;
			return false;
		}
	}

	/**
	 * 步骤 2: 等待 WebSocket 连接
	 */
	function waitForWebSocket(): Promise<boolean> {
		loadingText.value = i18n.global.t('controlpage.loading.connecting_ws');

		return new Promise((resolve) => {
			// 如果已经连接，直接返回
			if (isOpen()) {
				resolve(true);
				return;
			}

			// 设置超时（30秒）
			const timeout = setTimeout(() => {
				cleanupWsOpenHandler();
				errorTitle.value = i18n.global.t('controlpage.ws_error.title');
				errorMessage.value = i18n.global.t('controlpage.loading.ws_timeout');
				modeErrorDialogVisible.value = true;
				isLoading.value = false;
				resolve(false);
			}, 30000);

			// 监听 WebSocket 打开事件
			wsOpenHandler = () => {
				clearTimeout(timeout);
				cleanupWsOpenHandler();
				resolve(true);
			};

			on('open', wsOpenHandler);

			// 清理函数
			function cleanupWsOpenHandler() {
				if (wsOpenHandler) {
					off('open', wsOpenHandler);
					wsOpenHandler = null;
				}
			}
		});
	}

	/**
	 * 监听 WebSocket 断开连接（初始化完成后）
	 */
	function monitorWebSocketConnection() {
		// 监听断开
		wsCloseHandler = () => {
			ElMessage.warning(i18n.global.t('controlpage.ws_disconnected'));
		};

		// 监听重连成功
		wsReconnectHandler = () => {
			ElMessage.success(i18n.global.t('controlpage.ws_reconnected'));
		};

		on('close', wsCloseHandler);
		on('open', wsReconnectHandler);
	}

	/**
	 * 初始化 Control 页面
	 */
	async function initialize(): Promise<boolean> {
		isLoading.value = true;

		// 步骤 1: 检查模式
		const modeOk = await checkMode();
		if (!modeOk) {
			return false;
		}

		// 步骤 2: 连接 WebSocket
		connectWebSocket();

		// 步骤 3: 等待 WebSocket 连接
		const wsOk = await waitForWebSocket();
		if (!wsOk) {
			return false;
		}

		// 初始化成功
		isLoading.value = false;
		loadingText.value = '';

		// 开始监听 WebSocket 断开和重连
		monitorWebSocketConnection();

		return true;
	}

	/**
	 * 返回首页
	 */
	function goBack() {
		router.push('/');
	}

	/**
	 * 清理监听器
	 */
	function cleanup() {
		if (wsOpenHandler) {
			off('open', wsOpenHandler);
			wsOpenHandler = null;
		}
		if (wsCloseHandler) {
			off('close', wsCloseHandler);
			wsCloseHandler = null;
		}
		if (wsReconnectHandler) {
			off('open', wsReconnectHandler);
			wsReconnectHandler = null;
		}
	}

	return {
		isLoading,
		loadingText,
		modeErrorDialogVisible,
		errorTitle,
		errorMessage,
		initialize,
		goBack,
		cleanup,
	};
}
