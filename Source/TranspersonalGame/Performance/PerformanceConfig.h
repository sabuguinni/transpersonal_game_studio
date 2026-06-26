// PerformanceConfig.h — Transpersonal Game Studio
// Agent #4 Performance Optimizer — Cycle 003
// Frame budget constants and LOD thresholds for 60fps PC / 30fps console

#pragma once

#include "CoreMinimal.h"

// ============================================================
// FRAME BUDGET CONSTANTS
// ============================================================

// Target frame times (milliseconds)
#define PERF_TARGET_FPS_PC          60.0f
#define PERF_TARGET_FPS_CONSOLE     30.0f
#define PERF_FRAME_BUDGET_PC_MS     16.67f   // 1000ms / 60fps
#define PERF_FRAME_BUDGET_CONSOLE_MS 33.33f  // 1000ms / 30fps

// Sub-budget allocations (PC, in ms)
#define PERF_BUDGET_RENDER_MS       8.0f     // GPU render thread
#define PERF_BUDGET_GAME_MS         4.0f     // Game thread (logic, AI, physics)
#define PERF_BUDGET_GPU_MS          12.0f    // Total GPU budget
#define PERF_BUDGET_RHITHREAD_MS    2.0f     // RHI submission

// ============================================================
// LOD DISTANCE THRESHOLDS (Unreal Units = cm)
// ============================================================

// Dinosaur LOD distances
#define PERF_DINO_LOD0_DISTANCE     2000.0f   // Full detail
#define PERF_DINO_LOD1_DISTANCE     5000.0f   // Medium detail
#define PERF_DINO_LOD2_DISTANCE     10000.0f  // Low detail
#define PERF_DINO_LOD3_DISTANCE     15000.0f  // Minimal / impostor

// Large dinosaur (Brachiosaurus) extended distances
#define PERF_DINO_LARGE_LOD0        3000.0f
#define PERF_DINO_LARGE_LOD1        8000.0f
#define PERF_DINO_LARGE_MAX_DRAW    20000.0f

// T-Rex (apex predator — always visible further)
#define PERF_DINO_TREX_LOD0         2500.0f
#define PERF_DINO_TREX_LOD1         6000.0f
#define PERF_DINO_TREX_MAX_DRAW     15000.0f

// Small dinosaurs (Raptor, Protoceratops)
#define PERF_DINO_SMALL_LOD0        1500.0f
#define PERF_DINO_SMALL_LOD1        4000.0f
#define PERF_DINO_SMALL_MAX_DRAW    8000.0f

// Foliage LOD distances
#define PERF_FOLIAGE_LOD0_DISTANCE  1000.0f
#define PERF_FOLIAGE_LOD1_DISTANCE  3000.0f
#define PERF_FOLIAGE_LOD2_DISTANCE  6000.0f
#define PERF_FOLIAGE_CULL_DISTANCE  8000.0f

// Rock / prop LOD distances
#define PERF_PROP_LOD0_DISTANCE     1500.0f
#define PERF_PROP_LOD1_DISTANCE     4000.0f
#define PERF_PROP_CULL_DISTANCE     6000.0f

// ============================================================
// DRAW CALL BUDGETS
// ============================================================

#define PERF_MAX_DRAW_CALLS_PC      2000
#define PERF_MAX_DRAW_CALLS_CONSOLE 800
#define PERF_MAX_DYNAMIC_LIGHTS     8       // Dynamic shadow-casting lights
#define PERF_MAX_SKELETAL_MESHES    20      // Animated characters/dinos on screen
#define PERF_MAX_PARTICLES_PC       500     // Niagara particle count
#define PERF_MAX_PARTICLES_CONSOLE  200

// ============================================================
// SURVIVAL COMPONENT TICK INTERVALS
// ============================================================

// SurvivalComponent tick at 1Hz (every 1 second) — validated for 60fps
#define PERF_SURVIVAL_TICK_INTERVAL     1.0f

// AI behavior tree tick interval
#define PERF_AI_TICK_INTERVAL           0.1f    // 10Hz for AI decisions

// Crowd simulation tick interval (Mass AI)
#define PERF_CROWD_TICK_INTERVAL        0.2f    // 5Hz for crowd agents

// World streaming check interval
#define PERF_STREAMING_CHECK_INTERVAL   0.5f    // 2Hz for streaming updates

// ============================================================
// MEMORY BUDGETS
// ============================================================

#define PERF_TEXTURE_POOL_PC_MB         2048    // 2GB texture streaming pool
#define PERF_TEXTURE_POOL_CONSOLE_MB    512     // 512MB for console
#define PERF_MESH_BUDGET_MB             256     // Static/skeletal mesh budget
#define PERF_AUDIO_BUDGET_MB            128     // Audio streaming budget

// ============================================================
// SHADOW QUALITY SETTINGS
// ============================================================

#define PERF_SHADOW_MAX_CASCADES        2       // CSM cascades (2 for perf)
#define PERF_SHADOW_MAX_RESOLUTION      1024    // Shadow map resolution
#define PERF_SHADOW_RADIUS_THRESHOLD    0.03f   // Skip tiny shadow casters

// ============================================================
// SCALABILITY PRESETS
// ============================================================

// PC High-End (target: 60fps @ 1080p/1440p)
#define PERF_PRESET_PC_SHADOW_QUALITY       3
#define PERF_PRESET_PC_TEXTURE_QUALITY      3
#define PERF_PRESET_PC_EFFECTS_QUALITY      3
#define PERF_PRESET_PC_POSTPROCESS_QUALITY  3
#define PERF_PRESET_PC_FOLIAGE_QUALITY      2   // Foliage at medium saves 3-5ms
#define PERF_PRESET_PC_VIEWDIST_QUALITY     3

// Console (target: 30fps @ 1080p)
#define PERF_PRESET_CONSOLE_SHADOW_QUALITY      2
#define PERF_PRESET_CONSOLE_TEXTURE_QUALITY     2
#define PERF_PRESET_CONSOLE_EFFECTS_QUALITY     2
#define PERF_PRESET_CONSOLE_POSTPROCESS_QUALITY 2
#define PERF_PRESET_CONSOLE_FOLIAGE_QUALITY     1
#define PERF_PRESET_CONSOLE_VIEWDIST_QUALITY    2
