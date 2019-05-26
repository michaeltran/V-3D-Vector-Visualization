Smin and Smax: when creating the colored quads in the planes, if a vertex has a T value < Smin or > Smin, then we don't render that vertex. In some cases, quads become triangles.
XY index: moves the XY plane in either Z direction. XY/XZ/YZ index correspond to the (default) 50 splitted planes.
XZ index: moves XZ plane in either Y direction.
YZ index: moves YZ plane in either X direction.
Gradient Min/Max: similar to Smin/Smax, if a vertex has a gradient value < Gmin or > Gmin, then we don't render that vertex. In some cases, quads become triangles.
XY checkbox: removes XY plane.
XZ checkbox: removes XZ plane.
YZ checkbox: removes YZ plane.
Color: Behaves similarly to previous assignments.
Iso-Contours Scalar Value: Affects wireframe iso-surfaces when # of contour = 1.
Iso-Contours Number of Contours: Affects the wireframe iso-surfaces and splits the wireframe iso-surfaces uniformly based on the #.


Instructions on how to run this project: (if you're on 64 bit) 
1. Change from x64 to x86 
2. Re-target solution 
3. Rebuild solution 
4. Run