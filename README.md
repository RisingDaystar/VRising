# VAureaNox
Experimental CPU C++ Pathtracer for Signed Distance Fields

Started as an university project for the Computer Graphics course @ "Sapienza University of Rome" held by professor/researcher Fabio Pellacini , i kept improving it , adding and improving features as i kept learning more about rendering.

It uses the "Yocto/GL" library written by Fabio Pellacini for utility functions (3d transforms, mathematic utilities , matrices etc..) 


VAureaNox is structured in 2 parts:
A "Switch" part, responsable for initialization,Pre-render tasks, thread initialization and switching between implemented renderers and scenes (TODO: scenes as parseable files)
A "Renderer" part : the selected renderer processes the scene


Feauteres an Unidirectional-Pathtracer with MIS , NEE , support for rough and delta surfaces, both transmissives and not.

Surfaces categorizeable between Conductors and Dielectrics with dedicated Fresnel function (not using Schlick's)
Microfacet brdf for specular surfaces using GGX

Diffuse model which accounts for energy conservation, that allows for proper ( i hope...) handling of mixed specular / diffuse surfaces , especially when fresnel effect kicks in.

Refractive materials with fixed or variable ior accounting for ray wavelength (keeps track of wl/speed/frequency[at first shoot]/ior) to simulate dispersion. Material parametrizable with a fixed ior or with Sellmeier coefficients (for dispersion)

Materials can be attached with a mutator , which allows to change the evaluated material at runtime , accounting also for normals and object loc/world pos...

///SDFS

Features a selection of SDF volumes (prefer calling this way...see below) and operators to combine/deform/displace them.

To assemble scene, utility functions are provided to univocally select items in the scene graph and manipolate them.

Sphere tracing: Uses a non naive approach to sphere tracing , using a self crafted technique (influenced by sphere overrelaxing techniques [TODO: quote] ) , which both improves rendering speed and precision.
Expecially it allows to trace against non C1 continuous and non lipschitz continuous distance functions reducing artifacts by a lot.
It can trace inside volumes natively.
Naive sphere tracing doesn't have a good time in this cases ( basically : it gets stuck or it returns a black pixel, depending on implementation).
For this purpose i use a double distance system (dist and vdist) [TODO: explain]





