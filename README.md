# TactileBrush

<!-- TOC depthFrom:2 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Introduction](#introduction)
- [Compilation and installation](#compilation-and-installation)
- [Usage](#usage)

<!-- /TOC -->

## Introduction

*Disclaimer : as this is a very specific topic, and because I am a beginner in haptics, this may be totally useless for most people and/or partially wrong. Nevertheless, I did my best and contributions/issues are welcomed.*

This repo proposes an implementation of the Tactile Brush algorithm proposed by Ali Israr and Ivan Poupyrev in 2011 ([paper](https://dl.acm.org/citation.cfm?id=1979235)). Afaik, there is no other free/libre implementation of this algorithm.

This algorithm gives a general method to create haptic illusions of a 2D motion from a 2D grid of *actuators* (which may be ERMs, LRAs, haptic excitors...). It is based on two known haptic illusions :

* Phantom actuator, or funneling illusion : when you feel an actuator at one place while none is there. Instead, two close actuators are triggered at the same time with a variable intensity, and the brain interpret these signals as a single actuator located in between.
* Apparent motion, when the consecutive activation of two actuators feels like a continuous motion. This illusion is subject to the right choice of duration, frequency, and [SOA](https://en.wikipedia.org/wiki/Stimulus_onset_asynchrony).

The algorithm has a few constraints :

* Single-point motion only, *e.g.* imitation of a finger.
* Straight-line motion only.
* Beginning and end of the motion must be located on the grid formed by the actuators.

So, if you have :

1. A 2D grid of any type of actuators, **equally spaced**
2. A way to control individual actuator's intensity
3. A low-latency actuators' activation
4. The wish to make the user feel a continuous motion

This algorithm may be useful to you.

## Compilation and installation

The library has no external dependencies. To compile and install it as a shared library, just :

```bash
$ make
$ sudo make install
```

You can then use the library by including it in your program.

```c++
#include <TactileBrush.h>
```

Don't forget to compile your program with `-ltactilebrush` flag.

## Usage

There is two concepts in TactileBrush : **grid** and **stroke**. You always start by creating a 2D-grid representing your **physical** actuators, *e.g.* :

```c++
// Creates a 3*4 grid of actuators, spaced vertically and horizontally by 2.5cm.
TactileBrush t(
  3, // Number of lines
  4, // Number of columns
  2.5); // Spacing of the actuators in centimeters
```

Then, you want to know when, for how long and how strong to trigger actuators so that the user feels a given motion. To do that, create a stroke (representing the motion) and compute the motion parameters, *e.g.* :

```c++
Stroke s(
  ActuatorPoint(1, 5), // Start of the stroke
  ActuatorPoint(5, 2), // End of the stroke
  1000, // Total duration in milliseconds
  1); // Total intensity (between 0 and 1)
// Compute actuators' activation parameters
t.computeStroke(s);
```

Note that `ActuatorPoint` is constructed from "real world" coordinates, not "grid units", *i.e.* use centimeters and (x, y) format. This may seem strange as we are mixing grid dimension (4x3) and coordinates in centimeters, but at least in my case, using centimeters to describe motion is more straightforward.

Once the computation is over, you can exploit the results with something like this :

```c++
const auto& motion = t.getMotion();
for(const auto& [key, val] : motion) {
  // key contains time of activation in milliseconds
  // val.line and val.column contains the position of the actuator to trigger
  // val.intensity defines the actuator intensity between 0 and 1 included
  // val.duration tells us the activation duration in milliseconds
}
```

Then you are free to use this data as you want to trigger your actuators, but you should have all that you need.
