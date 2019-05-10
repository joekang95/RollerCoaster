Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: Joe Yu-Ho Chang
USC ID 		: 8800444224

Description: In this assignment, we use Catmull-Rom splines along with OpenGL core profile shader-based texture mapping and Phong shading to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - Y

2. Completed all Levels:
  Level 1 : - Y
  level 2 : - Y
  Level 3 : - Y
  Level 4 : - Y
  Level 5 : - Y

3. Rendered the camera at a reasonable speed in a continuous path/orientation - Y

4. Run at interactive frame rate (>15fps at 1280 x 720) - Y

5. Understandably written, well commented code - Y

6. Attached an Animation folder containing not more than 1000 screenshots - Y

7. Attached this ReadMe File - Y

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section - Y

2. Render a Double Rail - Y

3. Made the track circular and closed it with C1 continuity - N

4. Any Additional Scene Elements? (list them here) - Y

   (1) Wooden crossbars

5. Render a sky-box - Y

6. Create tracks that mimic real world roller coaster - Y (greenLantern.sp)

7. Generate track from several sequences of splines - Y

8. Draw splines using recursive subdivision - Y

9. Render environment in a better manner - N

10. Improved coaster normals - N

11. Modify velocity with which the camera moves - Y

12. Derive the steps that lead to the physically realistic equation of updating u - N

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
1. 
2.
3.

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)
1.
2.
3.

Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
1.Keyboard 'w' 's' -> Move Forward/Backwards on the Track
2.Keyboard 'a' 'd' -> Look Around on the Track
3.Keyboard 'q' 'e' -> Switch Track
4.Keyboard 'r' -> Reset to Position the Beginning of the Ride
5.Keyboard '1' -> Color Shader Mode
6.Keyboard '2' -> Phong Shader Mode
7.Keyboard 'x' -> Take a Screenshot and Save to "screenshot.jpg"
8.Keyborad 'esc' -> Exit the Program
9.Keyborad 'space' -> Run/Pause the Ride

Names of the .cpp files you made changes to:
1.hw1.cpp				(Modified)
2.basicPipelineProgram.cpp 		(Modified)
3.basicPipelineProgram.h		(Modified)
4.basic.phongFragmentShader.glsl	(Added)
5.basic.phongVertexShader.glsl		(Added)
6.basic.textureFragmentShader.glsl	(Added)
7.basic.textureVertexShader.glsl 	(Added)

Comments : (If any)
1. To Run Other Spline(s), Modify the File track.txt
2. For Extra Credit 6, the self-made Spline is greenLantern.sp mimiced the "Track" from Green Lantern Ride.
3. Currently the velocity's MAX_HEIGHT is set to 6.2 for better performance for rollerCoaster.sp. Set it to a closer value to the max height (z coordinate) in the spline file. Such as 8.0 for greenLantern.sp
