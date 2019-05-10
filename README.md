# RollerCoaster

### 0. Running the Program

**For Mac:**

```
> cd hw2
> make
> ./hw1 track.txt (modify for add or change spline(s))
```

**For Windows (VS 2017):**

```
> Run the .sln
> Select Project > hw1 Properties > Debugging > Command Arguments
> or
> F5 to Run
```

### 1. Splines Import 

This program supports:

- Single Spline File Input

  ```
  1
splines/rollerCoaster.sp
  ```
  
- Multiple Spline Files Input

  ```
  2
splines/rollerCoaster.sp
  splines/goodRide.sp
  ```
  

### 2. Mode Selection

Keyboard ``` 1``` -> ```Color Shader``` Mode

Keyboard ``` 2``` -> ```Phong Shader``` Mode (Default)

Keyboard ```a or d```-> **```Look Around```** on the Track

Keyboard ```w or s``` -> **```Move Forward/Backwards```** on the Track

Keyboard ```q or e``` -> **```Switch Track```** if exists

Keyboard ```space``` -> **```Run/Pause```** the Ride

Keyboard ```x``` -> Take a Screenshot and Save to "screenshot.jpg"

Keyboard ```esc``` -> Exit the Program

### 3. Texture Mapping

located within ```/textures```

- SkyBox Textures: ```SkyBack.jpg, SkyBottom.jpg, SkyFront.jpg, SkyLeft.jpg, SkyRight.jpg, SkyTop.jpg```

- CrossBar Texture: ```Wood.jpg```

### 4. Modified Files

- hw1.cpp

- basicPipelineProgram.cpp 

- basicPipelineProgram.h

- basic.phongFragmentShader.glsl

- basic.phongVertexShader.glsl

- basic.textureFragmentShader.glsl

- basic.textureVertexShader.glsl

### 5. Velocity Value

```u_new = u_old + (dt) * (sqrt(2g(h_max - h)) / mag(dp/du))```

**Currently the velocity's MAX_HEIGHT is set to 6.2 for better performance for rollerCoaster.sp. **

**Set it to a closer value to the max height (z coordinate) in the spline file. Such as 8.0 for greenLantern.sp**