idf_component_register(
	SRCS "battery_monitor.cpp"
	REQUIRES arduino-esp32 board_config
	INCLUDE_DIRS "."
)