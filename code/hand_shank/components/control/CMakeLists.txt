idf_component_register(
	SRCS "control.cpp"
	REQUIRES arduino-esp32 joystick board_prefs board_config mode
	INCLUDE_DIRS "."
)