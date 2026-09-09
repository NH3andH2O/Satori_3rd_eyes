#include "unity.h"
#include "pid_controller.hpp"

TEST_CASE("proportional output", "[pid]")
{
	PidController controller(2.0, 0.0, 0.0);
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 12.0, controller.update(10.0, 4.0, 0.5));
}

TEST_CASE("integral accumulates", "[pid]")
{
	PidController controller(0.0, 2.0, 0.0);
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 2.0, controller.update(2.0, 0.0, 0.5));
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 4.0, controller.update(2.0, 0.0, 0.5));
}

TEST_CASE("derivative uses previous error", "[pid]")
{
	PidController controller(0.0, 0.0, 0.25);
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 4.0, controller.update(8.0, 0.0, 0.5));
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.0, controller.update(8.0, 0.0, 0.5));
}

TEST_CASE("legacy motion sequence remains reproducible", "[pid]")
{
	PidController controller(4.0, 0.2, 0.25);
	double state = 0.0;

	state += controller.update(40.0, state, 0.02) * 0.02;
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 13.2032, state);

	state += controller.update(40.0, state, 0.02) * 0.02;
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 12.051487744, state);
}

TEST_CASE("reset clears history but keeps gains", "[pid]")
{
	PidController controller(2.0, 1.0, 0.5);
	static_cast<void>(controller.update(4.0, 0.0, 0.5));
	controller.reset();

	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 14.0, controller.update(4.0, 0.0, 0.5));
}

TEST_CASE("setGains keeps controller history", "[pid]")
{
	PidController controller(0.0, 1.0, 0.0);
	static_cast<void>(controller.update(2.0, 0.0, 1.0));
	controller.setGains(0.0, 2.0, 0.0);

	TEST_ASSERT_DOUBLE_WITHIN(1e-9, 8.0, controller.update(2.0, 0.0, 1.0));
}

TEST_CASE("error sign is preserved without output limiting", "[pid]")
{
	PidController controller(1.0, 0.0, 0.0);
	TEST_ASSERT_DOUBLE_WITHIN(1e-9, -20.0, controller.update(-10.0, 10.0, 1.0));
}
