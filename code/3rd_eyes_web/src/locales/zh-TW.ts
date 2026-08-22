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
			servo_debug: '伺服器除錯模式',
			gyroscope_tracks: '陀螺儀跟隨模式',
			network_control: '網絡控制模式',
		},
		enter_network_control: '進入控制界面',
		enter_servo_setup: '進入覺之瞳設定',
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

	setup: {
		page_title: '覺之瞳伺服校正',
		title: '覺之瞳設定精靈',
		progress: '第 {current}／{total} 步',
		mode_error: '目前模式不是伺服設定模式，無法進入設定精靈。',
		loading: {
			checking_mode: '正在檢查伺服設定模式...',
			loading_config: '正在載入伺服設定...',
			connecting_ws: '正在連接即時預覽服務...',
			ws_timeout: 'WebSocket 連線逾時，請確認裝置與網路狀態。',
		},
		state: {
			get_failed: '無法取得伺服設定',
			mode_failed: '無法取得目前模式',
			save_failed: '伺服設定儲存失敗，請確認裝置連線後重試。',
		},
		errors: {
			mode_title: '模式錯誤',
			connection_title: '連線錯誤',
			websocket_title: 'WebSocket 錯誤',
		},
		welcome: {
			title: '歡迎使用覺之瞳設定精靈',
			description: '接下來將依序校正眼皮與眼球的活動範圍。',
		},
		steps: {
			eyelid_open: {
				title: '眼皮最大張開',
				description: '調整上、下眼皮至安全的最大張開位置；可移動眼球確認眼皮不會碰撞眼球機構。',
			},
			eyelid_closed: {
				title: '眼皮閉合',
				description: '調整上、下眼皮至自然閉合且不互相擠壓的位置。',
			},
			eyelid_middle: {
				title: '眼皮中間',
				description: '調整上、下眼皮至開合範圍的中間位置，確認兩側間距自然。',
			},
			eyeball_center: {
				title: '眼球置中',
				description: '調整眼球至正中央位置。',
			},
			eyeball_left: {
				title: '眼球最左',
				description: '將眼球調整至安全的最左位置，不要讓機構卡住。',
			},
			eyeball_right: {
				title: '眼球最右',
				description: '將眼球調整至安全的最右位置，不要讓機構卡住。',
			},
		},
		controls: {
			upper_eyelid: '上眼皮伺服馬達',
			lower_eyelid: '下眼皮伺服馬達',
			eyeball: '眼球伺服馬達',
		},
		websocket: {
			reconnecting: '裝置連線中斷，正在重新連接...',
			reconnected: '裝置已重新連線',
			failed: '自動重連失敗，請刷新頁面後重新連線。',
		},
		review: {
			step_title: '確認設定',
			title: '確認校正設定',
			description: '請確認以下校正值，完成後將儲存至裝置。',
			eyelid_open: '眼皮最大張開（上／下）',
			eyelid_closed: '眼皮閉合（上／下）',
			eyelid_middle: '眼皮中間（上／下）',
			eyeball_center: '眼球置中',
			eyeball_left: '眼球最左',
			eyeball_right: '眼球最右',
		},
		saving: {
			title: '正在儲存設定',
			description: '請保持裝置連線，完成前不要關閉頁面。',
		},
		complete: {
			title: '設定完成',
			description: '伺服校正設定已成功儲存，可以返回首頁。',
			debug_title: '設定已完成，目前仍為調試模式',
			debug_description: '伺服校正設定已成功儲存，但模式切換失敗。請手動將裝置切換至需要的運作模式。',
		},
		discard: {
			restart_title: '重新開始設定？',
			restart_message: '目前尚未提交的調整將全部清除，並從後端中間位置重新開始。',
			cancel_title: '取消本次校正？',
			cancel_message: '目前尚未提交的調整將全部清除，並返回首頁。',
		},
		validation: {
			range: '所有伺服角度必須是 0 至 180 的整數。',
			upper_endpoints: '上眼皮最大張開角度必須小於閉合角度。',
			lower_endpoints: '下眼皮閉合角度必須小於最大張開角度。',
			upper_middle: '上眼皮中間角度必須位於最大張開與閉合之間。',
			lower_middle: '下眼皮中間角度必須位於閉合與最大張開之間。',
			eyeball_left_center: '眼球最左位置不能與中心位置相同。',
			eyeball_endpoints: '眼球最左與最右位置不能相同。',
			eyeball_center: '眼球中心位置必須嚴格位於最左與最右位置之間。',
		},
		actions: {
			start: '開始設定',
			start_from_existing: '從現有設定開始',
			previous: '上一步',
			next: '下一步',
			cancel: '取消',
			restart: '重新開始',
			complete: '儲存並完成',
			retry: '重試初始化',
			reload: '刷新頁面',
			keep_editing: '繼續調整',
			confirm_restart: '清除並重新開始',
			confirm_cancel: '清除並取消',
			back_home: '返回首頁',
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
