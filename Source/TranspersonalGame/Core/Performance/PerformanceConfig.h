// PerformanceConfig.h — Transpersonal Game Studio
// Performance Optimizer Agent #04 — Cycle PROD_CYCLE_AUTO_20260630_001
// Compile-time performance constants and tick budget definitions
// All systems must respect these limits to maintain 60fps on PC / 30fps console

#pragma once

#include "CoreMinimal.h"

// ============================================================
// FRAME BUDGET CONSTANTS (milliseconds @ 60fps = 16.67ms)
// ============================================================
namespace Perf
{
    // Total frame budget
    constexpr float FrameBudget_60fps_ms  = 16.67f;
    constexpr float FrameBudget_30fps_ms  = 33.33f;

    // Per-thread budgets (ms)
    constexpr float GameThread_Budget_ms  = 3.0f;
    constexpr float RenderThread_Budget_ms = 5.0f;
    constexpr float GPU_Budget_ms         = 7.0f;
    constexpr float Physics_Budget_ms     = 1.5f;
    constexpr float AI_Budget_ms          = 1.0f;
    constexpr float Audio_Budget_ms       = 0.5f;

    // ============================================================
    // LOD DISTANCES (Unreal units = centimetres)
    // ============================================================
    constexpr float LOD_Near_cm           = 500.0f;
    constexpr float LOD_Mid_cm            = 2000.0f;
    constexpr float LOD_Far_cm            = 5000.0f;
    constexpr float LOD_Cull_cm           = 8000.0f;

    // ============================================================
    // TICK RATE INTERVALS (seconds)
    // Use SetActorTickInterval() or FTimerHandle with these values
    // NEVER tick these systems every frame — use timers instead
    // ============================================================

    // Character survival stats
    constexpr float StaminaDrain_Interval_s      = 0.1f;   // 10Hz
    constexpr float HungerDecay_Interval_s       = 1.0f;   // 1Hz
    constexpr float ThirstDecay_Interval_s       = 1.0f;   // 1Hz
    constexpr float FearDecay_Interval_s         = 0.5f;   // 2Hz
    constexpr float TemperatureUpdate_Interval_s = 2.0f;   // 0.5Hz

    // AI systems
    constexpr float DinoAI_Perception_Interval_s = 0.25f;  // 4Hz
    constexpr float DinoAI_Decision_Interval_s   = 0.5f;   // 2Hz
    constexpr float CrowdSim_Update_Interval_s   = 0.5f;   // 2Hz
    constexpr float NavMesh_Rebuild_Interval_s   = 5.0f;   // 0.2Hz

    // World systems
    constexpr float Weather_Tick_Interval_s      = 10.0f;  // 0.1Hz
    constexpr float Foliage_Update_Interval_s    = 2.0f;   // 0.5Hz
    constexpr float DayNight_Tick_Interval_s     = 1.0f;   // 1Hz

    // ============================================================
    // ACTOR COUNT LIMITS
    // Exceeding these triggers automatic LOD/culling escalation
    // ============================================================
    constexpr int32 MaxDinosaurActors            = 50;
    constexpr int32 MaxFoliageInstancesPerCell   = 10000;
    constexpr int32 MaxCrowdAgents               = 200;
    constexpr int32 MaxParticleSystemsActive     = 30;
    constexpr int32 MaxDynamicLights             = 16;

    // ============================================================
    // MEMORY LIMITS
    // ============================================================
    constexpr int32 TexturePool_MB               = 1024;
    constexpr int32 MeshPool_MB                  = 512;
    constexpr int32 AudioPool_MB                 = 256;

    // ============================================================
    // SHADOW SETTINGS
    // ============================================================
    constexpr int32 Shadow_MaxResolution         = 2048;
    constexpr int32 Shadow_MaxCascades           = 3;
    constexpr float Shadow_DistanceScale         = 0.8f;
    constexpr float Shadow_RadiusThreshold       = 0.02f;

} // namespace Perf
