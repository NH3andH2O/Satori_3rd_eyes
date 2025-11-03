export default {
	/* 首頁 */
	homepage: {
		page_title: '覺之瞳',
		title: '覺之瞳設定',
	},

	/* WiFi 設定 */
	wifiset: {
		title: 'WiFi 設定',

		/* 局域網設定 */
		LAN: {
			title: '局域網設定',
			title_help: '將覺之瞳連接到家庭網路或者手機熱點，以便直接訪問覺之瞳',
			wifi_enable: '連接局域網',
			ssid: 'WiFi 名稱：',
			password: 'WiFi 密碼：',
			save: '儲存',

			/* 狀態 */
			state: {
				get_failed: 'WiFi 獲取失敗',
				set_successfully: 'WiFi 設定成功',
				set_failed: 'WiFi 設定失敗',
			},
			/* 錯誤訊息 */
			errors: {
				ssid_empty: 'WiFi 名稱不能為空',
			},
		},

		/* 熱點模式設定 */
		softAP: {
			title: '熱點模式設定',
			title_help: '覺之瞳作為無線熱點，可通過手機或電腦連接到此熱點進行配置。',
			softAP_name: '熱點名稱：',
			change_password: '更改密碼',
			password: '密碼設定：',
			password_info: '留空表示無密碼',
			password_confirm: '確認密碼：',
			password_confirm_info: '確認密碼',
			save: '儲存',

			/* 狀態 */
			state: {
				get_failed: '熱點獲取失敗',
				set_successfully: '熱點設定成功',
				set_failed: '熱點設定失敗',
			},

			/* 錯誤訊息 */
			errors: {
				password_length_short: '密碼長度不能少於 8 個字符',
				password_length_long: '密碼長度不能超過 63 個字符',
				password_invalid_chars: '密碼中包含非法字符',
				password_not_match: '兩次輸入的密碼不一致',
			},
		},
	},

	/* 模式設定 */
	modeset: {
		title: '模式設定',
		mode: '模式：',
		mode_info: {
			gyroscope_tracks: '陀螺儀跟隨模式',
			network_control: '網絡控制模式',
		},
		enter_network_control: '進入控制界面',
		save: '儲存',

		/* 狀態 */
		state: {
			get_failed: '模式獲取失敗',
			set_successfully: '模式設定成功',
			set_failed: '模式設定失敗',
		},

		/* 錯誤訊息 */
		errors: {
			mode_empty: '模式不能為空',
			mode_invalid: '模式非法',
		},
	},

	/* 進階設定 */
	advancedset: {
		title: '進階設定',
		correction_time: '靜態校正時間：',
		correction_time_help: '當覺之瞳處於靜止狀態時，陀螺儀會重置初始姿態角度以矯正角度偏差。設置為0表示不進行靜態校正。',
		save: '儲存',

		/* 狀態 */
		state: {
			get_failed: '進階設定獲取失敗',
			set_successfully: '進階設定成功',
			set_failed: '進階設定失敗',
		},

		/* 錯誤訊息 */
		errors: {
			correction_time_invalid_number: '靜態校正時間必須是有效的數字',
			correction_time_negative: '靜態校正時間不能小於 0',
			correction_time_over_range: '靜態校正時間不能超過 65535',
		},
	},

	/* 控制界面 */
	controlpage: {
		page_title: '覺之瞳控制',
		title: '覺之瞳控制界面',

		/* 加載狀態 */
		loading: {
			checking_mode: '正在檢查模式設定...',
			connecting_ws: '正在連接控制服務...',
			mode_check_failed: '模式檢查失敗',
			ws_timeout: 'WebSocket 連接超時',
		},

		/* API 錯誤 */
		api_error: {
			title: '連接錯誤',
			message: '無法連接到伺服器，請檢查網絡連接',
		},

		/* 模式錯誤 */
		mode_error: {
			title: '模式錯誤',
			message: '當前模式不是網絡控制模式，請先切換到網絡控制模式',
			back_button: '返回首頁',
		},

		/* WebSocket 錯誤 */
		ws_error: {
			title: 'WebSocket 錯誤',
		},

		/* WebSocket 狀態 */
		ws_disconnected: 'WebSocket 連接已斷開',
		ws_reconnected: 'WebSocket 重新連接成功',
	},

	/* 控制設定 */
	control_set: {
		title: '設定',

		/* 眼球移動翻轉控制 */
		move_inverted: {
			title: '眼球移動翻轉控制',
			invert_Xaxis: '水平翻轉',
			invert_Yaxis: '垂直翻轉',
		},

		/* 眼睛張開大小翻轉控制 */
		eyelid_inverted: {
			title: '眼睛張開大小翻轉控制',
			invert_Yaxis: '垂直翻轉',
		},

		help: {
			title: '幫助',
			tutorial_button: '顯示教學',

			/* 教學 */
			tutorial: {
				title: '教學',
				desktop: '滑動屏幕來控制覺之瞳，左半部分控制眼睛張開大小，右半部分控制眼球轉動',
				mobile: '滑動屏幕來控制覺之瞳，上半部分控制眼睛張開大小，下半部分控制眼球轉動',
			},
		},
	},

	/* 元件 */
	component_setting: '設定',
	component_back: '返回',

	/* 錯誤代碼描述 */
	errors: {
		/** 前端錯誤 **/
		/* Axios 錯誤 */
		'-1': '網絡連接失敗',
		'-2': '請求連接中止',
		'-3': '請求超時',
		'-4': '請求已取消',
		'-10': '請求選項值錯誤',
		'-11': '請求選項錯誤',
		'-12': '無效的 URL',
		'-20': '響應格式錯誤',
		'-21': '錯誤的請求',
		'-30': '重定向次數過多',
		'-40': '不支持的功能',
		'-41': '已棄用的功能',

		/* HTTP 狀態碼 */
		'-100': '請求參數錯誤',
		'-101': '未授權，請重新登入',
		'-103': '拒絕訪問',
		'-104': '請求的資源不存在',
		'-105': '不允許的請求方法',
		'-108': '請求超時',
		'-109': '請求衝突',
		'-122': '無法處理的實體',
		'-129': '請求次數過多',
		'-200': '服務器內部錯誤',
		'-201': '功能未實現',
		'-202': '網關錯誤',
		'-203': '服務不可用',
		'-204': '網關超時',
		'-999': '未知錯誤',

		/* 模式錯誤 */
		'-10301': '無效的模式',

		/** 後端錯誤 **/
		/* 通用錯誤 */
		'10001': '無效的 JSON 格式',
		/* WiFi 錯誤 */
		'10101': '啟用 WiFi 時 SSID 不能為空',
		/* SoftAP 錯誤 */
		'10201': 'SoftAP 密碼不匹配',
		'10202': '無效的 SoftAP 密碼',
		/* 模式錯誤 */
		'10301': '無效的模式',
		default: '錯誤代碼: {code}',
	},
};
