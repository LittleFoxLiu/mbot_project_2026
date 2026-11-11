# Robot Control Flow and Algorithms

This guide describes how the main functions call one another and how the robot handles waypoints, colour readings, movement, and nudges.

## Main call flow

```text
setup()
  stopMotor()
  setupLights()
  setupColourSensor()
  setupIRReference()

loop()
  handleWaypoint()
    onBlackLine()
    readWaypointColour()
      detectColour()
        readColourRaw()
          readColourChannel() for red, green, and blue
        normalize RGB and vote among calibration points
    actOnColour()
      turnLeft() / turnRight() / uTurn()
      doubleLeftTurn() / doubleRightTurn()
      celebrate() when the finish colour is found
  updateNudging()
  moveForward()
    applyForwardSpeed()
```

`setup()` configures the shared LED decoder, colour sensor input, and IR reference. In normal operation, each `loop()` checks for a waypoint, refreshes the nudge state, and applies forward drive. A waypoint action can briefly run a timed movement routine before the loop resumes.

## Waypoint and colour flow

`handleWaypoint()` checks the line follower. When it first detects a black line, it stops the motors, asks `readWaypointColour()` for a known colour, and passes that colour to `actOnColour()`. The `leavingWaypoint` flag prevents the same line from triggering repeatedly. After the line has been clear for `WAYPOINT_REARM_MS`, the next waypoint can trigger.

`detectColour()` reads red, green, and blue in sequence. For each channel, `readColourChannel()` selects the matching LED, waits for the sensor to settle while continuing line monitoring, then averages `COLOUR_SAMPLES` LDR readings. `readColourRaw()` restores the IR emitter after the three channels have been read.

Each raw channel is normalized against its black and white calibration values, putting the three channels on a comparable scale. The algorithm computes squared RGB distance from the reading to every calibration point. It repeatedly selects the nearest unused points, counts their colour votes, and returns the colour with the most votes. With the current table, there are three points per class and K is two. If even the nearest point is farther than `MAX_COLOUR_DISTANCE`, the result is `COLOUR_UNKNOWN`; `readWaypointColour()` waits `COLOUR_RETRY_MS` and tries again.

`actOnColour()` maps red and green to quarter-turns, orange to two left turns, pink and light blue to a turn plus one grid plus another turn, and white to the finish routine.

## Movement and slowdown

`turnLeft()`, `turnRight()`, and `moveForwardGrid()` call `moveWithSlowdown()`. It drives at the requested speed for most of the calibrated duration, then reduces the speed linearly to `STOP_END_SPEED` over at most `STOP_SLOWDOWN_MS`. At the end it stops both motors. Forward grid movement can refresh nudging during its timed run; planned turns use their configured motor directions without wall corrections.

`moveForward()` is the continuous drive used by the main loop. `applyForwardSpeed()` checks the correction flags before ordinary forward drive: an active ultrasonic nudge commands the configured right turn, an active IR nudge stops the left wheel to turn left, and otherwise the base speed is applied with the scaled correction from `forwardNudge()`.

## Nudging algorithm

`updateNudging()` first expires any timed correction pulses, so a pulse can end promptly even between sensor checks. It then samples according to `NUDGE_INTERVAL_MS`. The ultrasonic sensor is read on every check. A valid distance at or below `ULTRASONIC_PRIORITY_DISTANCE_CM` suppresses the IR check and cancels an active IR pulse. If the distance is also below `ULTRASONIC_RIGHT_NUDGE_DISTANCE_CM`, `nudgeRight()` starts the timed right-turn pulse.

When the ultrasonic reading is outside the priority range, IR checks run on alternating slots. `nudgeLeft()` compares the illuminated reading with the fixed startup reference and starts a left pulse when the drop reaches `IR_DROP_THRESHOLD`. If startup was too bright to establish a usable reference, the first live reading at or below 330 becomes the reference. Both correction types have separate pulse durations and pause intervals, preventing a correction from being retriggered continuously.

In the current implementation, `currentNudge` is reset to zero on each sensor check and is not assigned a nonzero value. Therefore `forwardNudge()` currently returns zero; steering corrections are produced by the timed IR and ultrasonic active flags.

## Appendix: slowdown compensation formula

The final speed ramp reduces the total commanded speed-time compared with running at full speed for the calibrated duration. To retain that calibration, `moveWithSlowdown()` adds a short full-speed interval before the ramp.

Let:

- `speed` be the requested starting speed;
- `endSpeed` be the speed at the end of the ramp;
- `rampMs` be the ramp duration.

The speed-time lost to a linear ramp is the triangle area:

```text
lost speed-time = (speed - endSpeed) × rampMs / 2
```

Dividing that loss by the requested speed gives the extra full-speed time. The code rounds this value to the nearest whole millisecond using integer arithmetic:

```text
extraMs = (rampMs × (speed - endSpeed) + speed) / (2 × speed)
totalMs = calibrated duration + extraMs
ramp starts at totalMs - rampMs
```

The actual ramp duration is the smaller of `STOP_SLOWDOWN_MS` and the requested movement duration. The actual end speed is the smaller of `STOP_END_SPEED` and the requested speed. See `moveWithSlowdown()` in `Movement.cpp` for the implementation.
