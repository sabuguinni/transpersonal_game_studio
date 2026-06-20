// PerformanceConfig.h
// Performance Optimizer #04 — PROD_CYCLE_AUTO_20260620_007
// Central performance constants and budget definitions for the prehistoric survival game.
// All systems must stay within these budgets to guarantee 60fps PC / 30fps console.

#pragma once

#include "CoreMinimal.h"

// ============================================================
// FRAME BUDGET (milliseconds per frame)
// ============================================================
// PC High-End target: 60fps = 16.67ms total
//   CPU game thread:   6.0ms
//   CPU render thread: 5.0ms
//   GPU:               5.5ms
//   Overhead/swap:     0.17ms
//
// Console target: 30fps = 33.33ms total
//   CPU game thread:  10.0ms
//   CPU render thread: 8.0ms
//   GPU:              14.0ms
//   Overhead/swap:     1.33ms
// ============================================================

namespace Perf
{
    // --- Frame budgets (ms) ---
    constexpr float PC_FRAME_BUDGET_MS          = 16.67f;
    constexpr float CONSOLE_FRAME_BUDGET_MS     = 33.33f;
    constexpr float PC_CPU_GAME_BUDGET_MS       = 6.0f;
    constexpr float PC_CPU_RENDER_BUDGET_MS     = 5.0f;
    constexpr float PC_GPU_BUDGET_MS            = 5.5f;

    // --- Actor count caps ---
    constexpr int32 MAX_ACTORS_IN_WORLD         = 500;    // Hard cap — above this GetAllActorsOfClass degrades
    constexpr int32 MAX_DINOSAURS_ACTIVE        = 20;     // Simultaneous active dino AI agents
    constexpr int32 MAX_RAPTORS_PER_PACK        = 8;      // Per-pack cap — pack AI scales O(n^2)
    constexpr int32 MAX_PHYSICS_BODIES          = 64;     // Rigid body simulation cap

    // --- Spatial query radii (cm) ---
    // Use SphereOverlapActors() with these radii instead of GetAllActorsOfClass()
    constexpr float RAPTOR_PACK_SCAN_RADIUS_CM  = 3000.0f;  // 30m — replaces O(n) world scan in CallPack()
    constexpr float DINO_PERCEPTION_RADIUS_CM   = 5000.0f;  // 50m — AI perception sphere
    constexpr float CROWD_CULL_RADIUS_CM        = 15000.0f; // 150m — beyond this, crowd agents are culled

    // --- LOD distances (cm) ---
    constexpr float LOD0_MAX_DISTANCE_CM        = 1500.0f;  // Full detail — within 15m
    constexpr float LOD1_MAX_DISTANCE_CM        = 4000.0f;  // Medium detail — 15-40m
    constexpr float LOD2_MAX_DISTANCE_CM        = 8000.0f;  // Low detail — 40-80m
    constexpr float LOD3_MAX_DISTANCE_CM        = 15000.0f; // Impostor/billboard — 80-150m
    constexpr float CULL_DISTANCE_CM            = 20000.0f; // Invisible beyond 200m

    // --- Texture streaming ---
    constexpr int32 TEXTURE_POOL_SIZE_MB        = 1000;
    constexpr int32 TEXTURE_TEMP_MEM_MB         = 50;

    // --- Shadow quality ---
    constexpr int32 SHADOW_MAX_RESOLUTION       = 1024;
    constexpr float SHADOW_RADIUS_THRESHOLD     = 0.03f;
    constexpr float SHADOW_DISTANCE_SCALE       = 0.8f;

    // --- Lumen (Global Illumination) ---
    // Hardware ray tracing disabled — SW Lumen saves 3-5ms/frame on mid-range GPUs
    constexpr bool  LUMEN_HARDWARE_RT_ENABLED   = false;
    constexpr bool  LUMEN_REFLECTIONS_ENABLED   = true;
    constexpr bool  LUMEN_DIFFUSE_INDIRECT      = true;

    // --- Nanite ---
    constexpr float NANITE_MAX_PIXELS_PER_EDGE  = 1.0f;
    constexpr float NANITE_PROXY_TRI_PERCENT    = 100.0f;
}

// ============================================================
// PERFORMANCE RECOMMENDATIONS (for other agents)
// ============================================================
//
// AGENT #03 (Core Systems / RaptorCharacter):
//   ISSUE: CallPack() uses GetAllActorsOfClass() — O(n) over ALL world actors.
//   FIX:   Replace with UKismetSystemLibrary::SphereOverlapActors():
//
//     TArray<AActor*> OverlappingActors;
//     UKismetSystemLibrary::SphereOverlapActors(
//         GetWorld(),
//         GetActorLocation(),
//         Perf::RAPTOR_PACK_SCAN_RADIUS_CM,
//         TArray<TEnumAsByte<EObjectTypeQuery>>(),
//         ARaptorCharacter::StaticClass(),
//         TArray<AActor*>{ this },
//         OverlappingActors
//     );
//
//   IMPACT: Reduces scan from O(500 actors) to O(5-10 raptors in radius) — 50-100x faster.
//
// AGENT #05 (World Generator):
//   - Use World Partition streaming — never load >500 actors simultaneously
//   - Set cull distances on all foliage: Perf::CULL_DISTANCE_CM
//   - Nanite for landscape and large rocks; impostor billboards for distant trees
//
// AGENT #06 (Environment Artist):
//   - Max 3 LOD levels per mesh (LOD0/LOD1/LOD2 + cull)
//   - Texture atlasing for foliage — max 1 draw call per foliage type
//   - No more than 4 dynamic lights in any 50m radius
//
// AGENT #08 (Lighting):
//   - SW Lumen only (no hardware RT) — saves 3-5ms/frame
//   - Max 1 directional light (sun) + 1 sky light
//   - Dynamic point lights: max 4 per 50m radius, shadow-casting: max 2
//
// AGENT #12 (Combat AI):
//   - Dinosaur AI tick rate: 0.1s (10Hz) for non-combat, 0.033s (30Hz) for combat
//   - Use EQS (Environment Query System) for spatial decisions, not per-frame traces
//   - Cap simultaneous combat AI: Perf::MAX_DINOSAURS_ACTIVE
//
// AGENT #13 (Crowd):
//   - Mass AI agents beyond 50m: LOD2 (position-only update, no animation)
//   - Mass AI agents beyond 150m: culled entirely
//   - Max 50,000 agents but only 200 fully simulated at once
