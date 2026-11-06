#include "Movement.h"
#include "Hardware.h"
#include "Nudging.h"
#include "RobotConfig.h"

static const unsigned long STOP_SLOWDOWN_MS = 300; // Gradually slow down over the final 300 ms.
static const int STOP_END_SPEED = 80; // Speed just before stopping; raise if the wheels stall.
// Derived movement durations: calibrated speed * time / current speed.
static const unsigned long TURN_90_MS = (850UL * 100 + TURN_SPEED / 2) / TURN_SPEED;
static const unsigned long SINGLE_WHEEL_TURN_90_MS = (TURN_90_MS * 190UL + 50UL) / 100UL;
static const unsigned long ONE_GRID_TIME_MS = (1390UL * 150 + FORWARD_SPEED / 2) / FORWARD_SPEED;

static_assert(STOP_END_SPEED > 0 && STOP_END_SPEED <= 255, "STOP_END_SPEED must be 1 to 255");
static_assert(FORWARD_SPEED > 0 && FORWARD_SPEED <= 255, "FORWARD_SPEED must be 1 to 255");
static_assert(TURN_SPEED > 0 && TURN_SPEED <= 255, "TURN_SPEED must be 1 to 255");

void stopMotor()
{
    motor1.stop();
    motor2.stop();
}

static void applyForwardSpeed(int speed)
{
    if (DEBUG_MODE)
    {
        stopMotor();
        return;
    }

    if (isUltrasonicNudgeActive())
    {
        // Turn in place to the right for the configured angle.
        motor1.run(-speed);
        motor2.run(-speed);
        return;
    }

    if (isIRNudgeActive())
    {
        motor1.run(speed);
        // motor2 is the left wheel; stopping it makes the IR correction turn left.
        motor2.stop();
        return;
    }

    int nudge = forwardNudge(speed);
    motor1.run(constrain(speed - nudge, 0, 255));
    motor2.run(-constrain(speed + nudge, 0, 255));
}

static void moveWithSlowdown(int speed, int direction1, int direction2, unsigned long durationMs)
{
    if (DEBUG_MODE)
    {
        stopMotor();
        return;
    }

    int endSpeed = STOP_END_SPEED < speed ? STOP_END_SPEED : speed;
    unsigned long rampMs = STOP_SLOWDOWN_MS < durationMs ? STOP_SLOWDOWN_MS : durationMs;

    // A linear ramp loses (speed - endSpeed) * rampMs / 2 of speed*time.
    // Add enough full-speed time to compensate, preserving the calibration.
    unsigned long extraMs = (rampMs * (speed - endSpeed) + speed) / (2UL * speed);
    unsigned long totalMs = durationMs + extraMs;
    unsigned long rampStartMs = totalMs - rampMs;
    unsigned long startedMs = millis();
    // Manoeuvres start on a black line, so they must not pause for it.
    while (true)
    {
        unsigned long elapsedMs = millis() - startedMs;
        if (elapsedMs >= totalMs)
            break;

        if (direction1 == 1 && direction2 == -1)
        {
            updateNudging();
        }

        int currentSpeed = speed;
        if (rampMs > 0 && elapsedMs >= rampStartMs)
            currentSpeed -= (unsigned long)(speed - endSpeed) * (elapsedMs - rampStartMs) / rampMs;

        if (direction1 == 1 && direction2 == -1)
            applyForwardSpeed(currentSpeed);
        else
        {
            // Planned turns use their calibrated speeds without wall corrections.
            motor1.run(direction1 * currentSpeed);
            motor2.run(direction2 * currentSpeed);
        }
        delay(1); // Small update interval keeps the slowdown smooth.
    }
    stopMotor();
}

void moveForward()
{
    // loop() handles black lines; this must still drive off a line after a turn.
    applyForwardSpeed(FORWARD_SPEED);
}

void moveForwardGrid()
{
    if (DEBUG_MODE)
    {
        stopMotor();
        return;
    }
    moveWithSlowdown(FORWARD_SPEED, 1, -1, ONE_GRID_TIME_MS);
}

void turnRight()
{
    moveWithSlowdown(TURN_SPEED, -1, -1, TURN_90_MS);
}

void turnLeft()
{
    moveWithSlowdown(TURN_SPEED, 1, 1, TURN_90_MS);
}

void uTurn()
{
    turnLeft();
    delay(100);
    turnLeft();
    delay(100);

    /* Special single-wheel U-turn, retained for reference:
    // First quarter-turn: reverse the left wheel around the stopped right wheel.
    motor1.stop();
    motor2.run(TURN_SPEED);
    delay(SINGLE_WHEEL_TURN_90_MS);
    stopMotor();
    delay(100);

    // Second quarter-turn: drive the right wheel forward around the stopped left
    // wheel, continuing the same turn direction to complete the U-turn.
    motor1.run(TURN_SPEED);
    motor2.stop();
    delay(SINGLE_WHEEL_TURN_90_MS);
    stopMotor();
    delay(100);
    */
}

void doubleLeftTurn()
{
    // TODO: Left, advance one grid, left.
    turnLeft();
    delay(100);
    moveForwardGrid();
    delay(100);
    turnLeft();
    delay(100);
}

void doubleRightTurn()
{
    // TODO: Right, advance one grid, right.
    turnRight();
    delay(100);
    moveForwardGrid();
    delay(100);
    turnRight();
    delay(100);
}
