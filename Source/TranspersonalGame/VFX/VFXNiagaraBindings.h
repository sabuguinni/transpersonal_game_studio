// VFXNiagaraBindings.h
// VFX Agent #17 — Transpersonal Game Studio
// Maps EVFX_EffectType enum values to Niagara System asset paths
// All effects are physically realistic prehistoric world VFX — NO magical/spiritual effects

#pragma once

#include "CoreMinimal.h"
#include "VFXManager.h"
#include "VFXNiagaraBindings.generated.h"

// ============================================================
// FVFX_NiagaraBinding — maps one effect type to a Niagara asset
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_NiagaraBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    EVFX_EffectType EffectType = EVFX_EffectType::CampfireFire;

    // Path to the Niagara System asset in Content Browser
    // Example: /Game/VFX/Niagara/NS_Fire_Campfire
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    FString NiagaraAssetPath = TEXT("");

    // LOD variant paths (empty = use main asset at all LODs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    FString NiagaraAssetPath_Mid = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    FString NiagaraAssetPath_Far = TEXT("");

    // Scale multiplier applied at each LOD level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    float LOD_Near_ParticleScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    float LOD_Mid_ParticleScale = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    float LOD_Far_ParticleScale = 0.3f;

    // Max simultaneous instances of this effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    int32 MaxInstances = 8;
};

// ============================================================
// UVFX_NiagaraBindingTable — DataAsset holding all bindings
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_NiagaraBindingTable : public UObject
{
    GENERATED_BODY()

public:
    UVFX_NiagaraBindingTable();

    // All effect-to-Niagara bindings. Configure in editor.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Niagara")
    TArray<FVFX_NiagaraBinding> Bindings;

    // Find binding for a given effect type
    UFUNCTION(BlueprintCallable, Category = "VFX|Niagara")
    bool GetBinding(EVFX_EffectType EffectType, FVFX_NiagaraBinding& OutBinding) const;

    // Get asset path for effect at given LOD level
    UFUNCTION(BlueprintCallable, Category = "VFX|Niagara")
    FString GetAssetPathForLOD(EVFX_EffectType EffectType, EVFX_LODLevel LODLevel) const;

    // Populate default bindings with standard prehistoric VFX paths
    UFUNCTION(CallInEditor, Category = "VFX|Niagara")
    void PopulateDefaultBindings();

    // === DEFAULT NIAGARA ASSET PATHS ===
    // These match the naming convention: NS_[Category]_[Effect]
    // Category: Fire, Dino, Weather, Combat, World, Player
    static const FString PATH_NS_Fire_Campfire;
    static const FString PATH_NS_Fire_CampfireEmbers;
    static const FString PATH_NS_Dino_FootstepDust;
    static const FString PATH_NS_Dino_BreathVapor;
    static const FString PATH_NS_Dino_BloodImpact;
    static const FString PATH_NS_Weather_Rain;
    static const FString PATH_NS_Weather_Snow;
    static const FString PATH_NS_Weather_Fog;
    static const FString PATH_NS_World_VolcanicAsh;
    static const FString PATH_NS_World_WaterfallSpray;
    static const FString PATH_NS_World_PollenDrift;
    static const FString PATH_NS_Combat_WeaponImpact;
    static const FString PATH_NS_Player_Sweat;
};
