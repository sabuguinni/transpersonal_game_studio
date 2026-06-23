// PerformanceBudgetConfig.h
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260623_007
// Defines compile-time performance budget constants for the prehistoric survival game.
// All systems must respect these limits to hit 60fps PC / 30fps console targets.

#pragma once

#include "CoreMinimal.h"

// ============================================================
// FRAME BUDGET TARGETS
// ============================================================

/** Target frame time on high-end PC (ms) — 60 fps */
static constexpr float PERF_TARGET_FRAME_MS_PC = 16.67f;

/** Target frame time on console (ms) — 30 fps */
static constexpr float PERF_TARGET_FRAME_MS_CONSOLE = 33.33f;

/** Maximum allowed GPU frame time before quality is reduced (ms) */
static constexpr float PERF_GPU_BUDGET_MS = 12.0f;

/** Maximum allowed CPU game-thread time per frame (ms) */
static constexpr float PERF_CPU_GAME_BUDGET_MS = 6.0f;

/** Maximum allowed render-thread time per frame (ms) */
static constexpr float PERF_CPU_RENDER_BUDGET_MS = 6.0f;

// ============================================================
// ACTOR / DRAW CALL BUDGET
// ============================================================

/** Maximum number of actors visible in any single frame */
static constexpr int32 PERF_MAX_VISIBLE_ACTORS = 500;

/** Maximum draw calls per frame (PC) */
static constexpr int32 PERF_MAX_DRAW_CALLS_PC = 2000;

/** Maximum draw calls per frame (console) */
static constexpr int32 PERF_MAX_DRAW_CALLS_CONSOLE = 1200;

/** Maximum simultaneous dynamic lights */
static constexpr int32 PERF_MAX_DYNAMIC_LIGHTS = 8;

/** Maximum simultaneous shadow-casting lights */
static constexpr int32 PERF_MAX_SHADOW_LIGHTS = 4;

// ============================================================
// DINOSAUR AI BUDGET
// ============================================================

/** Maximum dinosaurs with full AI ticking simultaneously */
static constexpr int32 PERF_MAX_DINOS_FULL_AI = 12;

/** Maximum dinosaurs in scene total (beyond this, use LOD AI) */
static constexpr int32 PERF_MAX_DINOS_TOTAL = 40;

/** Distance at which dinosaur AI switches to simplified tick (cm) */
static constexpr float PERF_DINO_AI_LOD_DISTANCE = 5000.0f;

/** Distance at which dinosaur AI is fully dormant (cm) */
static constexpr float PERF_DINO_AI_SLEEP_DISTANCE = 15000.0f;

// ============================================================
// CROWD SIMULATION BUDGET (Mass AI)
// ============================================================

/** Maximum Mass AI agents simultaneously simulated */
static constexpr int32 PERF_MAX_MASS_AGENTS = 50000;

/** Maximum Mass AI agents with full behaviour trees */
static constexpr int32 PERF_MAX_MASS_BT_AGENTS = 200;

// ============================================================
// PHYSICS BUDGET
// ============================================================

/** Maximum physics sub-steps per frame */
static constexpr int32 PERF_MAX_PHYSICS_SUBSTEPS = 4;

/** Maximum physics delta time (seconds) */
static constexpr float PERF_MAX_PHYSICS_DELTA = 0.033f;

/** Maximum simultaneous ragdolls */
static constexpr int32 PERF_MAX_RAGDOLLS = 6;

/** Maximum simultaneous destructible actors */
static constexpr int32 PERF_MAX_DESTRUCTIBLES = 20;

// ============================================================
// TEXTURE / STREAMING BUDGET
// ============================================================

/** Texture streaming pool size (MB) */
static constexpr int32 PERF_TEXTURE_POOL_MB = 1024;

/** Maximum texture anisotropy level */
static constexpr int32 PERF_MAX_ANISOTROPY = 8;

// ============================================================
// PARTICLE / VFX BUDGET
// ============================================================

/** Maximum CPU particle time per frame (ms) */
static constexpr float PERF_PARTICLE_CPU_BUDGET_MS = 2.0f;

/** Maximum GPU particle time per frame (ms) */
static constexpr float PERF_PARTICLE_GPU_BUDGET_MS = 2.0f;

/** Maximum simultaneous Niagara systems */
static constexpr int32 PERF_MAX_NIAGARA_SYSTEMS = 64;

// ============================================================
// SHADOW CASCADE CONFIG
// ============================================================

/** Number of CSM shadow cascades */
static constexpr int32 PERF_SHADOW_CASCADE_COUNT = 3;

/** Shadow radius threshold — meshes smaller than this skip shadows */
static constexpr float PERF_SHADOW_RADIUS_THRESHOLD = 0.03f;

/** Shadow distance scale factor */
static constexpr float PERF_SHADOW_DISTANCE_SCALE = 0.7f;

// ============================================================
// LOD DISTANCES (cm)
// ============================================================

/** LOD0→LOD1 transition distance */
static constexpr float PERF_LOD1_DISTANCE = 1500.0f;

/** LOD1→LOD2 transition distance */
static constexpr float PERF_LOD2_DISTANCE = 4000.0f;

/** LOD2→LOD3 (lowest) transition distance */
static constexpr float PERF_LOD3_DISTANCE = 8000.0f;

/** Cull distance — actors beyond this are not rendered */
static constexpr float PERF_CULL_DISTANCE = 20000.0f;

// ============================================================
// ANIMATION BUDGET
// ============================================================

/** Enable Update Rate Optimisation (URO) for distant characters */
static constexpr bool PERF_URO_ENABLED = true;

/** Enable parallel animation evaluation */
static constexpr bool PERF_PARALLEL_ANIM = true;

/** Distance at which URO kicks in (cm) */
static constexpr float PERF_URO_DISTANCE = 3000.0f;
