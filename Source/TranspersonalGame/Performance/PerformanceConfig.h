// PerformanceConfig.h
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260620_006
// Central performance constants for TranspersonalGame.
// All systems reference these values — change here to tune globally.

#pragma once

#include "CoreMinimal.h"

// ─── Tick Rates ────────────────────────────────────────────────────────────
// DinosaurAIController tick interval (seconds). 0.1 = 10 Hz.
static constexpr float PERF_DINO_AI_TICK_CLOSE   = 0.1f;   // < 2000 units
static constexpr float PERF_DINO_AI_TICK_MID      = 0.1f;   // 2000–5000 units
static constexpr float PERF_DINO_AI_TICK_FAR      = 0.2f;   // > 5000 units
static constexpr float PERF_DINO_AI_TICK_CULLED   = 0.5f;   // > 10000 units (off-screen)

// ─── Vision / Detection ────────────────────────────────────────────────────
static constexpr float PERF_DINO_DETECTION_RADIUS = 2000.0f;  // cm — max sight range
static constexpr float PERF_DINO_ATTACK_RADIUS    = 200.0f;   // cm — melee range
static constexpr float PERF_DINO_VISION_HALF_ANGLE = 60.0f;   // degrees — half FOV

// ─── LOD Distances ─────────────────────────────────────────────────────────
static constexpr float PERF_LOD0_MAX_DIST = 1500.0f;   // Full detail
static constexpr float PERF_LOD1_MAX_DIST = 4000.0f;   // Medium detail
static constexpr float PERF_LOD2_MAX_DIST = 8000.0f;   // Low detail
static constexpr float PERF_LOD3_MAX_DIST = 15000.0f;  // Impostor / cull

// ─── Shadow Budget ─────────────────────────────────────────────────────────
static constexpr int32 PERF_SHADOW_MAX_RESOLUTION  = 1024;
static constexpr float PERF_SHADOW_RADIUS_THRESHOLD = 0.03f;

// ─── Navigation ────────────────────────────────────────────────────────────
static constexpr float PERF_NAV_PATROL_RADIUS = 1500.0f;  // cm — random patrol area
static constexpr float PERF_NAV_QUERY_EXTENT  = 500.0f;   // cm — nav query tolerance

// ─── Frame Budget (ms) ─────────────────────────────────────────────────────
static constexpr float PERF_FRAME_BUDGET_PC_MS      = 16.67f;  // 60 fps
static constexpr float PERF_FRAME_BUDGET_CONSOLE_MS = 33.33f;  // 30 fps
static constexpr float PERF_AI_BUDGET_MS            = 2.0f;    // AI allowed per frame
static constexpr float PERF_PHYSICS_BUDGET_MS       = 4.0f;    // Physics allowed per frame
