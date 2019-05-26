# Visualization - 3D Vector Visualization

This project is derivation implementation of the paper "An Illustrative Visualization Framework for 3D Vector Fields".

For technical details, please refer to the [[paper]](https://github.com/michaeltran/V-3D-Vector-Visualization/blob/master/Final%20Project%20Report.pdf) and final [[presentation]](https://github.com/michaeltran/V-3D-Vector-Visualization/blob/master/Final%20Project%20Slides.pdf).

![Seed](https://github.com/michaeltran/V-3D-Vector-Visualization/blob/master/Pictures/seed.png)
![Stage1](https://github.com/michaeltran/V-3D-Vector-Visualization/blob/master/Pictures/stage1.png)
![Stage2](https://github.com/michaeltran/V-3D-Vector-Visualization/blob/master/Pictures/stage2.png)

## Created By

### Michael Tran

Email: mhtran4@uh.edu

---

## Requirements

### General

```Rich Text Format
Visual Studio 2017
```

---

## How to Run

Instructions on how to run this project: (if you are on 64 bit):

1. Open .sln in Visual Studio.
2. BEFORE you build or run the program, change from x64 to x86 on the top of visual studio.
3. Change from Debug to Release. (To ensure best performance)
4. Re-target the solution to your windows version. (Right-click solution->Retarget Solution->Windows SDK Version)
5. Rebuild entire solution.
6. Run.

---

## Controls

### GUI Controls

* Smin/Smax - Entropy Volume Rendering control
* Volume Render Opacity - Controls the Entropy Volume Rendering Opacity
* Field Type - Vector Field Type
* Numerical Integration Type - Streamline Calculation control
* Direct - Toggles on the vector field arrows
* Direct Arrow Opacity - Controls the opacity of the arrows
* Streamlines - Assignment 7 streamlines for example
* P Value - # of stage 1 clusters
* Q Value - # of stage 2 clusters
* Entropy Streamlines - Toggles Displays the seeded entropy streamlines
* Spatial Streamline Clusters - Toggles Stage 1 Clusters
* Shape Streamline Clusters - Toggles Stage 2 Clusters
* Final Streamlines - Toggles Representative Streamlines from Stage 2
* Final Streamtapes - Toggles Black Streamtapes
* Final Streamtapes (Color) - Toggles Colored Streamtapes that match the color of their cluster (Requires Final Streamtapes to be toggled on)
* Final Streamtapes (Torsion) - Toggles Torsion applied streamtapes (Requires Final Streamtapes to be toggled on)
* Recalculate - Re-runs all calculations from scratch to render again