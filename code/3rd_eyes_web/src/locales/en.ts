export default {
	/* 首頁 */
	homepage: {
		page_title: '3rdEyes',
		title: '3rdEyes Settings',
	},

	/* WiFi 設定 */
	wifiset: {
		title: 'WiFi Settings',

		/* 局域網設定 */
		LAN: {
			title: 'LAN Settings',
			title_help: 'Connect 3rdEyes to your home network or mobile hotspot for direct access to 3rdEyes',
			wifi_enable: 'Connect LAN',
			ssid: 'WiFi Name:',
			password: 'WiFi Password:',
			save: 'Save',

			/* 狀態 */
			state: {
				get_failed: 'WiFi get failed',
				set_successfully: 'WiFi set successfully',
				set_failed: 'WiFi set failed',
			},

			/* 錯誤訊息 */
			errors: {
				ssid_empty: 'WiFi cannot be empty',
			},
		},

		/* 熱點模式設定 */
		softAP: {
			title: 'Hotspot Settings',
			title_help: '3rdEyes can be configured as a wireless hotspot, allowing you to connect via mobile or computer.',
			softAP_name: 'Hotspot Name:',
			change_password: 'Change Password',
			password: 'Password:',
			password_info: 'Leave blank for no password',
			password_confirm: 'Confirm Password:',
			password_confirm_info: 'Confirm Password',
			save: 'Save',

			/* 狀態 */
			state: {
				get_failed: 'Hotspot get failed',
				set_successfully: 'Hotspot set successfully',
				set_failed: 'Hotspot set failed',
			},

			/* 錯誤訊息 */
			errors: {
				password_length_short: 'Password must be at least 8 characters long',
				password_length_long: 'Password must not exceed 63 characters',
				password_invalid_chars: 'Password contains invalid characters',
				password_not_match: 'Passwords do not match',
			},
		},
	},

	/* 模式設定 */
	modeset: {
		title: 'Mode Settings',
		mode: 'Mode:',
		mode_info: {
			servo_debug: 'Servo Debug Mode',
			gyroscope_tracks: 'Gyroscope Tracking Mode',
			network_control: 'Network Control Mode',
		},
		enter_network_control: 'Enter Control Interface',
		enter_servo_setup: 'Enter 3rdEyes Setup',
		save: 'Save',

		/* 狀態 */
		state: {
			get_failed: 'Mode get failed',
			set_successfully: 'Mode set successfully',
			set_failed: 'Mode set failed',
		},

		/* 錯誤訊息 */
		errors: {
			mode_empty: 'Mode cannot be empty',
			mode_invalid: 'Mode invalid',
		},
	},

	setup: {
		page_title: '3rdEyes Servo Calibration',
		title: '3rdEyes Setup Wizard',
		mode_error: 'The device is not in Servo Setup mode, so the setup wizard cannot be opened.',
		loading: {
			checking_mode: 'Checking Servo Setup mode...',
			loading_config: 'Loading servo settings...',
			connecting_ws: 'Connecting to the live preview service...',
			ws_timeout: 'The WebSocket connection timed out. Check the device and network connection.',
		},
		state: {
			get_failed: 'Unable to load servo settings',
			mode_failed: 'Unable to read the current mode',
			save_failed: 'Unable to save servo settings. The draft has been retained.',
			mode_switch_failed: 'Calibration was saved, but switching to Network Control mode failed.',
		},
		errors: {
			mode_title: 'Mode Error',
			connection_title: 'Connection Error',
			websocket_title: 'WebSocket Error',
		},
		instruction: {
			current_task: 'Current calibration target',
			assist_note: 'All three servos remain adjustable. Values marked “Assist pose” are only for observation and are not saved for this step.',
		},
		steps: {
			eyelid_open: {
				title: 'Eyelids fully open',
				description: 'Adjust both eyelids to the widest safe opening. Move the eyeball as needed to check for mechanical contact.',
			},
			eyelid_closed: {
				title: 'Eyelids closed',
				description:
					'Adjust both eyelids until they close naturally without pressing against each other. The eyeball may be adjusted for assistance.',
			},
			eyelid_middle: {
				title: 'Eyelids midway',
				description: 'Adjust both eyelids to the midpoint of their opening range and verify that the spacing looks natural.',
			},
			eyeball_center: {
				title: 'Eyeball centered',
				description: 'Center the eyeball. The eyelids may be moved to help identify the center, but only the eyeball value is recorded.',
			},
			eyeball_left: {
				title: 'Eyeball far left',
				description: 'Move the eyeball to the safe left limit without binding the mechanism. Eyelid changes are only for assistance.',
			},
			eyeball_right: {
				title: 'Eyeball far right',
				description: 'Move the eyeball to the safe right limit without binding the mechanism. Eyelid changes are only for assistance.',
			},
		},
		controls: {
			upper_eyelid: 'Upper eyelid servo',
			lower_eyelid: 'Lower eyelid servo',
			eyeball: 'Eyeball servo',
			target: 'Recorded now',
			assist: 'Assist pose',
		},
		preview_status: {
			idle: 'Waiting for adjustment',
			pending: 'Preparing to send the device pose...',
			sent: 'Device pose sent',
			failed: 'Unable to send the pose. Check the WebSocket connection.',
		},
		websocket: {
			title: 'Live preview disconnected',
			disconnected: 'The WebSocket disconnected. All calibration controls are locked while automatic reconnection starts.',
			reconnecting: 'Automatically reconnecting (attempt {attempt} of {max}). Your calibration draft is retained.',
			failed: 'Automatic reconnection failed. Calibration controls remain locked.',
		},
		review: {
			title: 'Review calibration',
			description: 'Review the nine saved calibration values. Finishing saves them to the device and switches to Network Control mode.',
			eyelid_open: 'Eyelids fully open (upper/lower)',
			eyelid_closed: 'Eyelids closed (upper/lower)',
			eyelid_middle: 'Eyelids midway (upper/lower)',
			eyeball_center: 'Eyeball centered',
			eyeball_left: 'Eyeball far left',
			eyeball_right: 'Eyeball far right',
		},
		validation: {
			range: 'Every servo angle must be an integer from 0 through 180.',
			upper_order: 'Upper eyelid values must be: fully open < midway < closed.',
			lower_order: 'Lower eyelid values must be: closed < midway < fully open.',
			eyeball_order: 'Eyeball values must be: far left < centered < far right.',
		},
		actions: {
			previous: 'Previous',
			next: 'Next',
			cancel: 'Cancel',
			complete: 'Save and finish',
			retry: 'Retry initialization',
			reconnect: 'Reconnect manually',
			retry_mode: 'Retry mode switch',
			back_home: 'Back to Home',
		},
	},

	/* 進階設定 */
	advancedset: {
		title: 'Advanced Settings',
		correction_time: 'Static Correction Time:',
		correction_time_help:
			'When 3rdEyes is stationary, the gyroscope resets its initial orientation to correct angle deviations. Setting to 0 means no static correction.',
		save: 'Save',

		/* 狀態 */
		state: {
			get_failed: 'Advanced Settings get failed',
			set_successfully: 'Advanced Settings set successfully',
			set_failed: 'Advanced Settings set failed',
		},

		/* 錯誤訊息 */
		errors: {
			correction_time_invalid_number: 'Static correction time must be a valid number',
			correction_time_negative: 'Static correction time cannot be less than 0',
			correction_time_over_range: 'Static correction time cannot exceed 65535',
		},
	},

	/* 控制界面 */
	controlpage: {
		page_title: '3rdEyes Control',
		title: '3rdEyes Control Interface',

		/* 加載狀態 */
		loading: {
			checking_mode: 'Checking mode settings...',
			connecting_ws: 'Connecting to control service...',
			mode_check_failed: 'Mode check failed',
			ws_timeout: 'WebSocket connection timeout',
		},

		/* API 錯誤 */
		api_error: {
			title: 'Connection Error',
			message: 'Unable to connect to server, please check network connection',
		},

		/* 模式錯誤 */
		mode_error: {
			title: 'Mode Error',
			message: 'Current mode is not Network Control mode. Please switch to Network Control mode first.',
			back_button: 'Back to Home',
		},

		/* WebSocket 錯誤 */
		ws_error: {
			title: 'WebSocket Error',
		},

		/* WebSocket 狀態 */
		ws_disconnected: 'WebSocket connection disconnected',
		ws_reconnected: 'WebSocket reconnected successfully',
	},

	/* 控制設定 */
	control_set: {
		title: 'setting',

		/* 眼球移動翻轉控制 */
		move_inverted: {
			title: 'Invert Eye Movement Controls',
			invert_Xaxis: 'Invert X-Axis',
			invert_Yaxis: 'Invert Y-Axis',
		},

		/* 眼睛張開大小翻轉控制 */
		eyelid_inverted: {
			title: 'Invert Eyelid Controls',
			invert_Yaxis: 'Invert Y-Axis',
		},

		help: {
			title: 'Help',
			tutorial_button: 'Show Tutorial',

			/* 教學 */
			tutorial: {
				title: 'Tutorial',
				desktop: 'Swipe on the screen to control 3rdEyes. The left half controls eyelid size, and the right half controls eye movement.',
				mobile: 'Swipe on the screen to control 3rdEyes. The upper half controls eyelid size, and the lower half controls eye movement.',
			},
		},
	},

	/* 元件 */
	component_setting: 'Settings',
	component_back: 'Back',

	/* 錯誤代碼描述 */
	errors: {
		/** 前端錯誤 **/
		/* Axios 錯誤 */
		'-1': 'Network connection failed',
		'-2': 'Request connection aborted',
		'-3': 'Request timeout',
		'-4': 'Request canceled',
		'-10': 'Invalid request option value',
		'-11': 'Invalid request option',
		'-12': 'Invalid URL',
		'-20': 'Invalid response format',
		'-21': 'Bad request',
		'-30': 'Too many redirects',
		'-40': 'Unsupported feature',
		'-41': 'Deprecated feature',

		/* HTTP 狀態碼 */
		'-100': 'Invalid request parameters',
		'-101': 'Unauthorized, please login again',
		'-103': 'Access denied',
		'-104': 'Requested resource not found',
		'-105': 'Method not allowed',
		'-108': 'Request timeout',
		'-109': 'Request conflict',
		'-122': 'Unprocessable entity',
		'-129': 'Too many requests',
		'-200': 'Internal server error',
		'-201': 'Not implemented',
		'-202': 'Bad gateway',
		'-203': 'Service unavailable',
		'-204': 'Gateway timeout',
		'-999': 'Unknown error',

		/* 模式錯誤 */
		'-10301': 'Invalid mode',

		/** 後端錯誤 **/
		/* 通用錯誤 */
		'10001': 'Invalid JSON format',
		/* WiFi 錯誤 */
		'10101': 'SSID cannot be empty when WiFi is enabled',
		/* SoftAP 錯誤 */
		'10201': 'SoftAP password mismatch',
		'10202': 'Invalid SoftAP password',
		/* 模式錯誤 */
		'10301': 'Invalid mode',
		default: 'Error code: {code}',
	},
};
