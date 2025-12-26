idf_component_register(
	SRCS "joystick.cpp"
	REQUIRES arduino-esp32 board_config board_prefs mode
	INCLUDE_DIRS "."
)