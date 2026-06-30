// NiagaraVFXLibrary.cpp
// VFX Agent #17 — Transpersonal Game Studio
// Full implementation of UVFX_NiagaraLibrary utility methods.

#include "NiagaraVFXLibrary.h"
#include "NiagaraSystem.h"

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
    // Default constructor — soft references are unset until assigned in editor
    // All TSoftObjectPtr members default to null (no asset assigned)
}

UNiagaraSystem* UVFX_NiagaraLibrary::GetSystemForDistance(
    TSoftObjectPtr<UNiagaraSystem> HighLOD,
    float DistanceCm) const
{
    // Compute LOD level from distance
    EVFX_LODLevel LOD = ComputeLODLevel(DistanceCm);

    // Culled — do not spawn
    if (LOD == EVFX_LODLevel::Culled)
    {
        return nullptr;
    }

    // For now all LOD levels use the same system reference.
    // When LOD-specific variants are created (NS_Fire_Campfire_LOD1 etc.),
    // this method will select the appropriate variant.
    if (HighLOD.IsValid())
    {
        return HighLOD.Get();
    }

    // Attempt to load if not yet in memory
    if (!HighLOD.IsNull())
    {
        return HighLOD.LoadSynchronous();
    }

    return nullptr;
}

EVFX_LODLevel UVFX_NiagaraLibrary::ComputeLODLevel(float DistanceCm)
{
    // LOD thresholds in centimetres (UE5 default unit = 1cm)
    // High:   0 – 2000cm  (0 – 20m)
    // Medium: 2000 – 6000cm  (20 – 60m)
    // Low:    6000 – 15000cm (60 – 150m)
    // Culled: > 15000cm (> 150m)

    if (DistanceCm <= 2000.0f)
    {
        return EVFX_LODLevel::High;
    }
    else if (DistanceCm <= 6000.0f)
    {
        return EVFX_LODLevel::Medium;
    }
    else if (DistanceCm <= 15000.0f)
    {
        return EVFX_LODLevel::Low;
    }
    else
    {
        return EVFX_LODLevel::Culled;
    }
}

int32 UVFX_NiagaraLibrary::GetBudgetForCategory(EVFX_Category Category)
{
    // Per-category simultaneous instance limits.
    // Tuned for 60fps PC / 30fps console target (GDD performance spec).
    switch (Category)
    {
        case EVFX_Category::Environment:
            return 8;   // Campfires, wind — moderate count, high visual importance

        case EVFX_Category::Dinosaur:
            return 16;  // Footstep dust, breath, blood — many dinos can be visible

        case EVFX_Category::Combat:
            return 24;  // Weapon impacts — burst events, short-lived

        case EVFX_Category::World:
            return 4;   // Waterfall, volcano — large effects, low count

        case EVFX_Category::Weather:
            return 2;   // Rain, snow — full-screen, only 1-2 active at a time

        default:
            return 8;
    }
}
