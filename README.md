# C-CNC-LookAheadProject

### Aim of the course: 
We have to write a SW/CNC controller for controlling a 3 axes machine (machine tool) (of the digital twin (simulator) of the machine itself).
This C-program will talk via ZeroMQ to the simulator in Simulink (Simscape Multibody). We had also seen how to deploy this SW into a micro-controller (CROSS-COMPILATION) so how to compile C on our PCs to be run on a different archictecture (i. e. BeagleBone, Raspberry Pi, embedded system, ...) and that micro-controller has been be also connected to the same simulator via ZeroMQ.
Reagarding the cross-compilation will be used Docker for cross-compiling .exe files able to run on BeagleBone Black.

In particular we had done the following during the course:
- taken the tool properties, calculating the machining parameters (feed and spindle rate) 
- calculated the power needed for making that machining checking whether this power was compatible with the spindle/max power of the spindle we had (if not we had to reduce either f or the cross-section).
- we implemented a piece of SW that:
  1. reads a G-code block file or line of G-code: _off-line_
  2. splits a G-code line into something meaningful, extracting INFO: position, speeds, ... (_parsing a part-program (off-line)_)
  3. _Off-line_ calculation of the (feedrate) velocity profiles (_profiling_) for each command base on the extracted info above
  4. _ON-LINE_ 3D-coordinates motion/position interpolation of the axes for each block (velocity profiles trajectory projection to the axes) from the initial to the final position to        be sent via **MQTT**, time after time, to the digital twin CNC machine axes drivers in run-time.
- in the end we needed a Program Object that was able to:
  1. load the file containing G-code
  2. create an ordered linked list fashion blocks connection
  3. calculate velocity profiles for each and every _block type_ 
  4. calculate the interpolation of the positions so the set points (command signals generated during machining) to be sent to the machine axes in run-time

__N.B.__ For every block the constraints on the velocity profiles were:
  - preservation of block length (distance between initial and final point = area of block)
  - the states of the machine are updated every tick/clock/time step (D.T base clock) and each profile motion needs to start and stop at a tick
  - limitation of acceleration (the maximum physical acceleration available to each axis)
  - NO limitation of jerks (derivative of acceleration): dangerous for vibrating behaviours (jerk = infinite acceleration for corners in velocity profiles)
  - going down to 0 speed at each corner (we cannot follow a corner with constant speed otherwise the required acceleration should be infinity (**mechanically non-feasible**))

We also used an F.S.M APPROACH used for implementing complex logics in controlling HW. 
F.S.M is a design concept when writing SW where the system we are programming is going to lay (at every time) in a given state or configuration and the possible n° of states is LIMITED / finite.

------------------------------------------------------------
# Look-Ahead project:

We need to improve the algorithm for calculating the velocity profiles with a Look-Ahead Approach:
in order to be able to calculate the velocity profiles looking at the blocks coming next (not to stop to zero speed after each block in each corner)
keeping the speed as costant and high as possible without stopping!
- We need to calculate jerk-limited velocity profiles so that each segment is splitted into 7 ranges rather than 3.

- We will need to deal with combinations of segments in order not to slow down to ZERO speed after each block between each step! 
For instance, a trajectory in x-y plane which is a square with sharp corners is actually needed to stop in each corner (JERK-limitation) while for a trajectory which is a set of small segments parallel each other (collinear) there is actually NO need to stop in corners due to collinearity possibly maintaining the block nominal feedrate even in the corners.

Basically the CNC-controller is looking ahead to the next steps in order to understand whether it can keep the velocity up or not:
the strategy is to keep the speed at corners constant if they are perfectly aligned/parallel and, on the other hand, reduce it linearly with the angle between corners/edges down to 0 about 45° or to a minimum feedrate. 

The _main idea_ is as follows:
- If the corner is NOT a corner so the 2 consecutive segments are perfectly collinear, v_corner = feedrate of blocks
- If there is an angle, we have to decrease the velocity in the corner according to the angle: for any angle larger than 45° v_corner = 0 while for angles <= 45°,       v_corner linear increases up to the nominal feedrate

Instead of having sharp corners in the velocity profiles we have rounded corners so to impose a bound on the jerks.

So we can see how many segments we can lump together in a single segment possibly reducing the speed at corner if the angle is not too sharp
and we reduce the speed at corner at 0 above a certain max threshold (45° (i.e)).
![LookAhead](https://user-images.githubusercontent.com/61516812/200801760-17c7c9f4-1e69-451e-974a-6bfd9eb561d0.JPG) 

How to implement this?
### Look-Ahead Approach
  - Calculate the v_i, v_m, v_f for each block considering v = v(s) 
  - Calculate only accelerations from first (1) to last (n) block compatible with final acceleration and with v_i and v_f for each block (so calculate accelerations that allow us to go from (v_i) velocity to the other (v_f))
  - Calculate the decelerations from block 1 to n (so calculate decelerations that allow us to go from (v_i) velocity to the other (v_f))
  __N.B__ for point 2 and 3 we have only to calculate the 4 intersection points [s_i, s_1, s_2, s_f],  and rank them regarding acceleration and decelation and this brings us to find out which is the condition in which we are for every block (then we put all the blocks together and we have calculated the velocity profile keeping the max possible velocity)
  - Change variable from v(s) to v(t) one block at a time: calculate [s_i, s_1], [s_1, s_2] and [s_2, s_f] and the timings t_1, t_2 and t_f
  - Reshaping such that each block is completed in a multiple of tq and v*(t) is the final velocity profile and then we calculate the lambda function for each block by     integrating the velocity profile.
In the block, we need to have the starting and final acceleration time, the initial and final feed (to be computed) and then we can get lambda function as usual.
__N.B.__ We start and stop the sequence of blocks at every G00 (Rapid motion) so we go down to 0

What's matter/important for the test of the LookAhead-approach: __the nominal setpoints profile generated by the controller__

![LookAheadImplementation](https://user-images.githubusercontent.com/61516812/200802657-44d03095-86a1-4025-984c-ad225bf411c3.JPG)
