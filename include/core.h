#pragma once
#define Debug true
// NIP Engine Config
#define NE_VERSION "v0.7"
#define NE_TARGET_TICKRATE 60
#define NE_TICKRATE_TIME (1000 / NE_TARGET_TICKRATE)
#define NE_DELTATIME (1.0 / NE_TARGET_TICKRATE)
// Nuclear Reactor Structure Config
#define NR_SIZE_X 20
#define NR_SIZE_Y 20
#define NR_ENRICHMENT 0.1
// Reactor Renderer
#define RR_SCALE 30
#define RR_PADDING 3