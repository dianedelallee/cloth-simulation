# cloth-simulation
c++ cloth simulation

Verlet's algorithm 
------
```tex
\sum_{x=0}^n f(x)
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

License
-------
![alt cc](https://licensebuttons.net/l/by/3.0/88x31.png)
This work is licensed under a [Creative Commons Attribution 4.0 International License] (http://creativecommons.org/licenses/by/4.0/)
