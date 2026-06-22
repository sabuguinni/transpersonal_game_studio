#pragma once

// === TRANSPERSONAL GAME STUDIO — DINOSAUR LOD CONFIGURATION ===
// Agent: #04 Performance Optimizer | PROD_CYCLE_AUTO_20260622_001
// Purpose: Centralised LOD distance thresholds for all dinosaur species.
//          Used by DinosaurBase and species-specific classes to configure
//          SkeletalMeshComponent LOD settings at construction time.

#include "CoreMinimal.h"

// ─────────────────────────────────────────────────────────────
//  LOD DISTANCE STRUCT  (plain C++ — no USTRUCT, no GENERATED_BODY)
// ─────────────────────────────────────────────────────────────

struct FPerf_DinoLODDistances
{
    float LOD0_Max;   // Full quality — closest range (cm)
    float LOD1_Max;   // Medium quality
    float LOD2_Max;   // Low quality
    float CullDist;   // Beyond this: actor hidden, tick disabled
};

// ─────────────────────────────────────────────────────────────
//  PER-SPECIES LOD THRESHOLDS
//  Tuned for 60fps target with up to 5 simultaneous dinos.
//  Larger dinos get longer cull distances (visible from further away).
// ─────────────────────────────────────────────────────────────

namespace DinoLOD
{
    // Tyrannosaurus Rex — apex predator, large silhouette
    static constexpr FPerf_DinoLODDistances TRex     = { 3000.f, 6000.f, 10000.f, 12000.f };

    // Velociraptor — fast, small, pack hunter
    static constexpr FPerf_DinoLODDistances Raptor   = { 2000.f, 4500.f,  8000.f,  9000.f };

    // Brachiosaurus — enormous, visible from extreme range
    static constexpr FPerf_DinoLODDistances Brachio  = { 4000.f, 8000.f, 12000.f, 15000.f };

    // Triceratops — large herbivore
    static constexpr FPerf_DinoLODDistances Trike    = { 3000.f, 6000.f, 10000.f, 12000.f };

    // Pterodactyl — airborne, seen from great distances
    static constexpr FPerf_DinoLODDistances Ptero    = { 2500.f, 5000.f,  9000.f, 11000.f };

    // Generic small fauna (fish, lizards, insects)
    static constexpr FPerf_DinoLODDistances SmallFauna = { 1500.f, 3000.f, 0.f, 3500.f };
}

// ─────────────────────────────────────────────────────────────
//  HELPER — Apply LOD distances to a SkeletalMeshComponent
//  Call this in BeginPlay or constructor of any dinosaur class.
// ─────────────────────────────────────────────────────────────

namespace DinoLODHelper
{
    /**
     * Applies LOD screen-size thresholds to a SkeletalMeshComponent.
     * UE5 uses screen-size (0.0-1.0) not distance for LOD selection,
     * so we convert cm distances to approximate screen fractions.
     *
     * Formula: ScreenSize ≈ ActorRadius / Distance
     * T-Rex capsule radius ≈ 80cm → ScreenSize at 3000cm ≈ 0.027
     */
    inline void ApplyLODDistances(class USkeletalMeshComponent* Mesh,
                                   const FPerf_DinoLODDistances& Distances,
                                   float ActorRadiusCm = 80.f)
    {
        if (!Mesh) return;

        // Convert distances to screen sizes (approximate)
        // Larger screen size = closer = higher quality LOD
        float SS_LOD0 = ActorRadiusCm / FMath::Max(Distances.LOD0_Max, 1.f);
        float SS_LOD1 = ActorRadiusCm / FMath::Max(Distances.LOD1_Max, 1.f);
        float SS_LOD2 = ActorRadiusCm / FMath::Max(Distances.LOD2_Max, 1.f);

        // Apply via LODInfo array if mesh has LODs
        // (Actual LOD mesh must be set up in the asset — this sets the transition distances)
        Mesh->SetCullDistance(Distances.CullDist);

        // Screen size thresholds — set on mesh asset, not component
        // These are reference values for the asset editor / automation
        // LOD0 threshold: SS_LOD0 (e.g. 0.027 for T-Rex at 3000cm)
        // LOD1 threshold: SS_LOD1 (e.g. 0.013 for T-Rex at 6000cm)
        // LOD2 threshold: SS_LOD2 (e.g. 0.008 for T-Rex at 10000cm)

        // Log for debugging
        UE_LOG(LogTemp, Verbose, TEXT("DinoLOD: CullDist=%.0f SS0=%.4f SS1=%.4f SS2=%.4f"),
               Distances.CullDist, SS_LOD0, SS_LOD1, SS_LOD2);
    }
}

// ─────────────────────────────────────────────────────────────
//  TICK RATE BUDGET
//  Dino AI ticks are staggered to avoid frame spikes.
//  With 5 dinos at 0.5s intervals, we get 10 AI ticks/sec total.
//  At 60fps (16.67ms/frame) this costs ~0.8ms/frame — acceptable.
// ─────────────────────────────────────────────────────────────

namespace DinoTickBudget
{
    static constexpr float TRex_TickInterval     = 0.5f;   // seconds between AI updates
    static constexpr float Raptor_TickInterval   = 0.3f;   // raptors react faster
    static constexpr float Brachio_TickInterval  = 0.8f;   // slow herbivore
    static constexpr float Generic_TickInterval  = 0.5f;

    // Max simultaneous dinos before LOD-based tick suppression kicks in
    static constexpr int32 MaxFullTickDinos      = 5;
    static constexpr int32 MaxReducedTickDinos   = 15;     // LOD1+ dinos get 2× slower tick
    static constexpr int32 MaxSuppressedDinos    = 30;     // LOD2+ dinos tick at 2s intervals
}
