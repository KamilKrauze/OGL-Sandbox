# OGL-Sandbox

Mostly a personal repo of me experimenting and learning different rendering techniques and usages of OpenGL specification version 4.6.

`/CommonLib` is a basic statically compiled library that contains all the classes, functions and structs for implementing basic renderers. Mostly to reduce **main.cpp** lines of codes since I try to put each example/experiment in one file for simplicity and keeping multiple files as a library instead.

While the application project also do include the `thirdparty` directory but that's mostly for simplicity and flexibility.

I do want to make `CommonLib` an actual library for rendering but that'll come later once I have the basics and advanced concepts jotted down so I can experiment with those in the future to make:
- Optimization
- Flexible and customizable rendering pipelines
- My one node based shader-graph?

Currently covered:
- Basic compute shader texture generation and utilising work groups for better parallelisation.
- Basic texture mapping with basic texture blending.
- Normal mapping
- Usage of OpenGL DSA (direct state access) functions to reduce CPU overhead.
- Basic deferred rendering pipeline using the GBuffer and having samplable depth.
- Shadow mapping with distance-based scaling PCF sampling
