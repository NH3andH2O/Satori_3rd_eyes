export default {
	/* 首頁 */
	homepage: {
		page_title: '觉之瞳',
		title: '觉之瞳设置',
	},

	/* WiFi 設定 */
	wifiset: {
		title: 'WiFi 设置',

		/* 局域網設定 */
		LAN: {
			title: '局域网设置',
			title_help: '将觉之瞳连接到家庭网络或者手机热点，以便直接访问觉之瞳',
			wifi_enable: '连接局域网',
			ssid: 'WiFi 名称：',
			password: 'WiFi 密码：',
			save: '保存',

			/* 狀態 */
			state: {
				get_failed: 'WiFi 获取失败',
				set_successfully: 'WiFi 设置成功',
				set_failed: 'WiFi 设置失败',
			},

			/* 错误讯息 */
			errors: {
				ssid_empty: 'WiFi 名称不能为空',
			},
		},

		/* 熱點模式設定 */
		softAP: {
			title: '热点模式设置',
			title_help: '觉之瞳作为无线热点，可通过手机或电脑连接到此热点进行配置。',
			softAP_name: '热点名称：',
			change_password: '更改密码',
			password: '密码设置：',
			password_info: '留空表示无密码',
			password_confirm: '确认密码：',
			password_confirm_info: '确认密码',
			save: '保存',

			/* 狀態 */
			state: {
				get_failed: '热点获取失败',
				set_failed: '热点设置失败',
				set_successfully: '热点设置成功',
			},
			/* 錯誤訊息 */
			errors: {
				password_length_short: '密码长度不能少于 8 个字符',
				password_length_long: '密码长度不能超过 63 个字符',
				password_invalid_chars: '密码中包含非法字符',
				password_not_match: '两次输入的密码不一致',
			},
		},
	},

	/* 模式設定 */
	modeset: {
		title: '模式设置',
		mode: '模式：',
		mode_info: {
			servo_debug: '舵机调试模式',
			gyroscope_tracks: '陀螺仪跟随模式',
			network_control: '网络控制模式',
		},
		enter_network_control: '进入控制界面',
		enter_servo_setup: '进入觉之瞳设置',
		save: '保存',

		/* 狀態 */
		state: {
			get_failed: '模式获取失败',
			set_successfully: '模式设置成功',
			set_failed: '模式设置失败',
		},

		/* 錯誤訊息 */
		errors: {
			mode_empty: '模式不能为空',
			mode_invalid: '模式非法',
		},
	},

	setup: {
		page_title: '觉之瞳舵机校正',
		title: '觉之瞳设置向导',
		mode_error: '当前模式不是舵机设置模式，无法进入设置向导。',
		loading: {
			checking_mode: '正在检查舵机设置模式...',
			loading_config: '正在加载舵机设置...',
			connecting_ws: '正在连接实时预览服务...',
			ws_timeout: 'WebSocket 连接超时，请确认设备与网络状态。',
		},
		state: {
			get_failed: '无法获取舵机设置',
			mode_failed: '无法获取当前模式',
			save_failed: '舵机设置保存失败，草稿已保留。',
			mode_switch_failed: '校正已保存，但切换至网络控制模式失败。',
		},
		errors: {
			mode_title: '模式错误',
			connection_title: '连接错误',
			websocket_title: 'WebSocket 错误',
		},
		instruction: {
			current_task: '当前校正目标',
			assist_note: '三个舵机都可以调整；标记为“辅助姿态”的数值仅用于本步骤观察，不会写入本步骤校正值。',
		},
		steps: {
			eyelid_open: {
				title: '眼皮最大张开',
				description: '调整上、下眼皮至安全的最大张开位置；可移动眼球确认眼皮不会碰撞眼球机构。',
			},
			eyelid_closed: {
				title: '眼皮闭合',
				description: '调整上、下眼皮至自然闭合且不互相挤压的位置；眼球可作辅助调整。',
			},
			eyelid_middle: {
				title: '眼皮中间',
				description: '调整上、下眼皮至开合范围的中间位置，确认两侧间距自然。',
			},
			eyeball_center: {
				title: '眼球置中',
				description: '调整眼球至正中央；可移动上下眼皮辅助辨识中心，但本步骤只记录眼球值。',
			},
			eyeball_left: {
				title: '眼球最左',
				description: '将眼球调整至安全的最左位置，不要让机构卡住；眼皮只作观察辅助。',
			},
			eyeball_right: {
				title: '眼球最右',
				description: '将眼球调整至安全的最右位置，不要让机构卡住；眼皮只作观察辅助。',
			},
		},
		controls: {
			upper_eyelid: '上眼皮舵机',
			lower_eyelid: '下眼皮舵机',
			eyeball: '眼球舵机',
			target: '本步记录',
			assist: '辅助姿态',
		},
		preview_status: {
			idle: '等待调整',
			pending: '准备发送设备姿态...',
			sent: '设备姿态已发送',
			failed: '姿态发送失败，请确认 WebSocket 连接',
		},
		websocket: {
			title: '实时预览连接中断',
			disconnected: 'WebSocket 已断开，所有校正操作已锁定，系统将自动重连。',
			reconnecting: '正在自动重连（第 {attempt}/{max} 次），校正草稿已保留。',
			failed: '自动重连失败，校正操作仍处于锁定状态。',
		},
		review: {
			title: '确认校正设置',
			description: '请确认以下九个正式校正值。完成后将保存至设备并切换至网络控制模式。',
			eyelid_open: '眼皮最大张开（上／下）',
			eyelid_closed: '眼皮闭合（上／下）',
			eyelid_middle: '眼皮中间（上／下）',
			eyeball_center: '眼球置中',
			eyeball_left: '眼球最左',
			eyeball_right: '眼球最右',
		},
		validation: {
			range: '所有舵机角度必须是 0 到 180 的整数。',
			upper_order: '上眼皮必须符合：最大张开 < 中间 < 闭合。',
			lower_order: '下眼皮必须符合：闭合 < 中间 < 最大张开。',
			eyeball_order: '眼球必须符合：最左 < 置中 < 最右。',
		},
		actions: {
			previous: '上一步',
			next: '下一步',
			cancel: '取消',
			complete: '保存并完成',
			retry: '重试初始化',
			reconnect: '手动重连',
			retry_mode: '重试切换模式',
			back_home: '返回首页',
		},
	},

	/* 進階設定 */
	advancedset: {
		title: '进阶设置',
		correction_time: '静态校正时间：',
		correction_time_help: '当觉之瞳处于静止状态时，陀螺仪会重置初始姿态角度以矫正角度偏差。设置为0表示不进行静态校正。',
		save: '保存',

		/* 狀態 */
		state: {
			get_failed: '进阶设置获取失败',
			set_successfully: '进阶设置成功',
			set_failed: '进阶设置失败',
		},

		/* 錯誤訊息 */
		errors: {
			correction_time_invalid_number: '静态校正时间必须是有效的数字',
			correction_time_negative: '静态校正时间不能小于 0',
			correction_time_over_range: '静态校正时间不能超过 65535',
		},
	},

	/* 控制界面 */
	controlpage: {
		page_title: '觉之瞳控制',
		title: '觉之瞳控制界面',

		/* 加载状态 */
		loading: {
			checking_mode: '正在检查模式设置...',
			connecting_ws: '正在连接控制服务...',
			mode_check_failed: '模式检查失败',
			ws_timeout: 'WebSocket 连接超时',
		},

		/* API 错误 */
		api_error: {
			title: '连接错误',
			message: '无法连接到服务器，请检查网络连接',
		},

		/* 模式错误 */
		mode_error: {
			title: '模式错误',
			message: '当前模式不是网络控制模式，请先切换到网络控制模式',
			back_button: '返回首页',
		},

		/* WebSocket 错误 */
		ws_error: {
			title: 'WebSocket 错误',
		},

		/* WebSocket 状态 */
		ws_disconnected: 'WebSocket 连接已断开',
		ws_reconnected: 'WebSocket 重新连接成功',
	},

	/* 控制設定 */
	control_set: {
		title: '设置',

		/* 眼球移動翻轉控制 */
		move_inverted: {
			title: '眼球移动翻转控制',
			invert_Xaxis: '水平翻转',
			invert_Yaxis: '垂直翻转',
		},

		/* 眼睛張開大小翻轉控制 */
		eyelid_inverted: {
			title: '眼睛张开大小翻转控制',
			invert_Yaxis: '垂直翻转',
		},

		help: {
			title: '帮助',
			tutorial_button: '显示教程',

			/* 教學 */
			tutorial: {
				title: '教程',
				desktop: '滑动屏幕来控制觉之瞳，左半部分控制眼睛张开大小，右半部分控制眼球转动',
				mobile: '滑动屏幕来控制觉之瞳，上半部分控制眼睛张开大小，下半部分控制眼球转动',
			},
		},
	},

	/* 元件 */
	component_setting: '设置',
	component_back: '返回',

	/* 錯誤代碼描述 */
	errors: {
		/** 前端錯誤 **/
		/* Axios 錯誤 */
		'-1': '网络连接失败',
		'-2': '请求连接中止',
		'-3': '请求超时',
		'-4': '请求已取消',
		'-10': '请求选项值错误',
		'-11': '请求选项错误',
		'-12': '无效的 URL',
		'-20': '响应格式错误',
		'-21': '错误的请求',
		'-30': '重定向次数过多',
		'-40': '不支持的功能',
		'-41': '已弃用的功能',

		/* HTTP 狀態碼 */
		'-100': '请求参数错误',
		'-101': '未授权，请重新登录',
		'-103': '拒绝访问',
		'-104': '请求的资源不存在',
		'-105': '不允许的请求方法',
		'-108': '请求超时',
		'-109': '请求冲突',
		'-122': '无法处理的实体',
		'-129': '请求次数过多',
		'-200': '服务器内部错误',
		'-201': '功能未实现',
		'-202': '网关错误',
		'-203': '服务不可用',
		'-204': '网关超时',
		'-999': '未知错误',

		/* 模式錯誤 */
		'-10301': '无效的模式',

		/** 後端錯誤 **/
		/* 通用錯誤 */
		'10001': '无效的 JSON 格式',
		/* WiFi 錯誤 */
		'10101': '启用 WiFi 时 SSID 不能为空',
		/* SoftAP 錯誤 */
		'10201': 'SoftAP 密码不匹配',
		'10202': '无效的 SoftAP 密码',
		/* 模式錯誤 */
		'10301': '无效的模式',
		default: '错误代码: {code}',
	},
};
