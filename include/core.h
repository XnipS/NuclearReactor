#pragma once
#define Debug true
// NIP Engine Config
#define NE_VERSION "v0.8"
#define NE_TARGET_TICKRATE 60
#define NE_TICKRATE_TIME (1000 / NE_TARGET_TICKRATE)
#define NE_DELTATIME (1.0 / NE_TARGET_TICKRATE)
// Nuclear Reactor Structure Config
#define NR_SIZE_X 40
#define NR_SIZE_Y 25
#define NR_ENRICHMENT 0.2
#define NR_WATER_RANGE 1.5
#define NR_WATER_TEMP_OFFSET 20
// Reactor Renderer
#define RR_SCALE 30
#define RR_ATOM_PADDING 5
#define RR_WATER_PADDING 0.1
#define RR_CR_PADDING 1