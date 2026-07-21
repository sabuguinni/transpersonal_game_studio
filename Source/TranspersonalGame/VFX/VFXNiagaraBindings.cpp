// VFXNiagaraBindings.cpp
// QA Agent #18 — Transpersonal Game Studio
// Stub .cpp for UVFX_NiagaraBindingTable — required by UHT for UCLASS with UFUNCTION methods
// QA FLAG: VFXNiagaraBindings.h was header-only — this .cpp completes the mandatory .h/.cpp pair

#include "VFXNiagaraBindings.h"
#include "Engine/Engine.h"

// ============================================================
// UVFX_NiagaraBindingTable Implementation
// ============================================================

UVFX_NiagaraBindingTable::UVFX_NiagaraBindingTable()
{
    // CDO construction — safe, no world access
    PopulateDefaultBindings();
}

FString UVFX_NiagaraBindingTable::GetBinding(EVFX_EffectType EffectType) const
{
    const FString* Found = EffectBindings.Find(EffectType);
    if (Found)
    {
        return *Found;
    }
    return FString(TEXT(""));
}

FString UVFX_NiagaraBindingTable::GetAssetPathForLOD(EVFX_EffectType EffectType, EVFX_LODLevel LODLevel) const
{
    // Build LOD-specific key
    const int32 LODIndex = static_cast<int32>(LODLevel);

    // Try LOD-specific binding first
    if (LODIndex < LODBindings.Num())
    {
        const TMap<EVFX_EffectType, FString>& LODMap = LODBindings[LODIndex];
        const FString* Found = LODMap.Find(EffectType);
        if (Found && !Found->IsEmpty())
        {
            return *Found;
        }
    }

    // Fallback to base binding
    return GetBinding(EffectType);
}

void UVFX_NiagaraBindingTable::PopulateDefaultBindings()
{
    // Base bindings — Niagara system asset paths
    // Convention: NS_[Category]_[Effect]
    EffectBindings.Add(EVFX_EffectType::Campfire,       TEXT("/Game/VFX/Niagara/NS_Fire_Campfire"));
    EffectBindings.Add(EVFX_EffectType::DinoFootstep,   TEXT("/Game/VFX/Niagara/NS_Dust_DinoFootstep"));
    EffectBindings.Add(EVFX_EffectType::BloodImpact,    TEXT("/Game/VFX/Niagara/NS_Blood_Impact"));
    EffectBindings.Add(EVFX_EffectType::Rain,           TEXT("/Game/VFX/Niagara/NS_Weather_Rain"));
    EffectBindings.Add(EVFX_EffectType::Snow,           TEXT("/Game/VFX/Niagara/NS_Weather_Snow"));
    EffectBindings.Add(EVFX_EffectType::VolcanicAsh,    TEXT("/Game/VFX/Niagara/NS_Volcanic_Ash"));
    EffectBindings.Add(EVFX_EffectType::DinoBreath,     TEXT("/Game/VFX/Niagara/NS_Dino_Breath"));
    EffectBindings.Add(EVFX_EffectType::Explosion,      TEXT("/Game/VFX/Niagara/NS_Impact_Explosion"));
    EffectBindings.Add(EVFX_EffectType::WaterSplash,    TEXT("/Game/VFX/Niagara/NS_Water_Splash"));
    EffectBindings.Add(EVFX_EffectType::LeafBurst,      TEXT("/Game/VFX/Niagara/NS_Foliage_LeafBurst"));

    // LOD Near bindings (full quality)
    TMap<EVFX_EffectType, FString> NearBindings;
    NearBindings.Add(EVFX_EffectType::Campfire,     TEXT("/Game/VFX/Niagara/LOD/NS_Fire_Campfire_Near"));
    NearBindings.Add(EVFX_EffectType::DinoFootstep, TEXT("/Game/VFX/Niagara/LOD/NS_Dust_DinoFootstep_Near"));
    NearBindings.Add(EVFX_EffectType::BloodImpact,  TEXT("/Game/VFX/Niagara/LOD/NS_Blood_Impact_Near"));
    NearBindings.Add(EVFX_EffectType::Rain,         TEXT("/Game/VFX/Niagara/LOD/NS_Weather_Rain_Near"));
    NearBindings.Add(EVFX_EffectType::VolcanicAsh,  TEXT("/Game/VFX/Niagara/LOD/NS_Volcanic_Ash_Near"));

    // LOD Mid bindings (reduced particles)
    TMap<EVFX_EffectType, FString> MidBindings;
    MidBindings.Add(EVFX_EffectType::Campfire,     TEXT("/Game/VFX/Niagara/LOD/NS_Fire_Campfire_Mid"));
    MidBindings.Add(EVFX_EffectType::DinoFootstep, TEXT("/Game/VFX/Niagara/LOD/NS_Dust_DinoFootstep_Mid"));
    MidBindings.Add(EVFX_EffectType::BloodImpact,  TEXT("/Game/VFX/Niagara/LOD/NS_Blood_Impact_Mid"));
    MidBindings.Add(EVFX_EffectType::Rain,         TEXT("/Game/VFX/Niagara/LOD/NS_Weather_Rain_Mid"));
    MidBindings.Add(EVFX_EffectType::VolcanicAsh,  TEXT("/Game/VFX/Niagara/LOD/NS_Volcanic_Ash_Mid"));

    // LOD Far bindings (minimal, billboard-style)
    TMap<EVFX_EffectType, FString> FarBindings;
    FarBindings.Add(EVFX_EffectType::Campfire,     TEXT("/Game/VFX/Niagara/LOD/NS_Fire_Campfire_Far"));
    FarBindings.Add(EVFX_EffectType::DinoFootstep, TEXT(""));  // Culled at far distance
    FarBindings.Add(EVFX_EffectType::BloodImpact,  TEXT(""));  // Culled at far distance
    FarBindings.Add(EVFX_EffectType::Rain,         TEXT("/Game/VFX/Niagara/LOD/NS_Weather_Rain_Far"));
    FarBindings.Add(EVFX_EffectType::VolcanicAsh,  TEXT("/Game/VFX/Niagara/LOD/NS_Volcanic_Ash_Far"));

    LODBindings.Empty();
    LODBindings.Add(NearBindings);  // Index 0 = Near
    LODBindings.Add(MidBindings);   // Index 1 = Mid
    LODBindings.Add(FarBindings);   // Index 2 = Far
}
