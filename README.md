# cloth-simulation
c++ cloth simulation

Verlet's algorithm 
------
```
acc = force/mass
newPos = 2 * (pos - oldPos) + acc * (Δt*Δt)
oldPos = pos
pos = newPos
```

Building on Mac
-------
Open a terminal
Move to your repository<br/>
and execute these command lines
```{r, engine='bash', count_lines}
g++ -framework GLUT -framework OpenGL -framework Cocoa fileName.cpp -o execName
./execName
```

Commands 
-------
* x/X : move on X axis
* y/Y : move on Y axis
* z/Z : move on Z axis
* r/R : rotation scene on Y axis
* m/M : run and stop the animation
* b : add/delete geometrics object in the scene
* s : add/delete the smog effect
* f : draw the scene with all surfaces
* l : draw the scene with lines
* p : draw the scene with points
* ↑ : fullscreen
* ↓ : escape fullscreen
* q : quit

Images
------
<img src="http://diane-delallee.fr/assets/images/openGL.png" width="50%">
<img src="http://diane-delallee.fr/assets/images/cube.png" width="50%">
<img src="http://diane-delallee.fr/assets/images/sphere.png" width="50%">
<img src="http://diane-delallee.fr/assets/images/flag.png" width="50%">

License
-------
![alt cc](https://licensebuttons.net/l/by/3.0/88x31.png)
This work is licensed under a [Creative Commons Attribution 4.0 International License] (http://creativecommons.org/licenses/by/4.0/)
