#include "Movement.h"
#include "Hardware.h"
#include "Nudging.h"
#include "RobotConfig.h"

static const unsigned long STOP_SLOWDOWN_MS = 300; // Gradually slow down over the final 300 ms.
static const int STOP_END_SPEED = 80; // Speed just before stopping; raise if the wheels stall.
// Derived movement durations: calibrated speed * time / current speed.
static const unsigned long TURN_90_MS = (870UL * 100 + TURN_SPEED / 2) / TURN_SPEED;
static const unsigned long ONE_GRID_TIME_MS = (1380UL * 150 + FORWARD_SPEED / 2) / FORWARD_SPEED;

static_assert(STOP_END_SPEED > 0 && STOP_END_SPEED <= 255, "STOP_END_SPEED must be 1 to 255");
static_assert(FORWARD_SPEED > 0 && FORWARD_SPEED <= 255, "FORWARD_SPEED must be 1 to 255");
static_assert(TURN_SPEED > 0 && TURN_SPEED <= 255, "TURN_SPEED must be 1 to 255");

// Stop both drive motors immediately.
void stopMotor()
{
    motor1.stop();
    motor2.stop();
}

// Apply forward drive, giving active ultrasonic and IR corrections priority.
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

// Run a timed manoeuvre with a final linear ramp down to reduce abrupt stops.
static void moveWithSlowdown(int speed, int direction1, int direction2, unsigned long durationMs)
{
    if (DEBUG_MODE)
    {
        stopMotor();
        return;
    }

    int endSpeed = STOP_END_SPEED < speed ? STOP_END_SPEED : speed;
    unsigned long rampMs = STOP_SLOWDOWN_MS < durationMs ? STOP_SLOWDOWN_MS : durationMs;

    // A linear ramp loses (speed - endSpeed) * rampMs / 2 speed-time units.
    // Convert that loss to full-speed milliseconds and round to the nearest millisecond.
    unsigned long extraMs = (rampMs * (speed - endSpeed) + speed) / (2UL * speed);
    unsigned long totalMs = durationMs + extraMs;
    // Keep the calibrated full-speed portion, then place the ramp at the end.
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
        // Interpolate linearly from speed to endSpeed throughout the ramp.
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

// Drive forward for one loop iteration using any active nudge correction.
void moveForward()
{
    // loop() handles black lines; this must still drive off a line after a turn.
    applyForwardSpeed(FORWARD_SPEED);
}

// Drive forward for the calibrated duration of one grid, then slow to a stop.
void moveForwardGrid()
{
    if (DEBUG_MODE)
    {
        stopMotor();
        return;
    }
    moveWithSlowdown(FORWARD_SPEED, 1, -1, ONE_GRID_TIME_MS);
}

// Turn right for the calibrated quarter-turn duration.
void turnRight()
{
    moveWithSlowdown(TURN_SPEED, -1, -1, TURN_90_MS);
}

// Turn left for the calibrated quarter-turn duration.
void turnLeft()
{
    moveWithSlowdown(TURN_SPEED, 1, 1, TURN_90_MS);
}

// Complete a U-turn as two left quarter-turns.
void uTurn()
{
    turnLeft();
    delay(100);
    turnLeft();
    delay(100);

}

// Turn left, cross one grid, then turn left again.
void doubleLeftTurn()
{
    turnLeft();
    delay(100);
    moveForwardGrid();
    delay(100);
    turnLeft();
    delay(100);
}

// Turn right, cross one grid, then turn right again.
void doubleRightTurn()
{
    turnRight();
    delay(100);
    moveForwardGrid();
    delay(100);
    turnRight();
    delay(100);
}
