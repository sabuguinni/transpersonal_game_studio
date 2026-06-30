#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "VFXManager.generated.h"

// ============================================================
// VFX ENUMS — all prefixed EVFX_ to avoid collisions
// ============================================================

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fire_Campfire   UMETA(DisplayName = "Fire_Campfire"),
    Dust_Footstep   UMETA(DisplayName = "Dust_Footstep"),
    Weather_Rain    UMETA(DisplayName = "Weather_Rain"),
    Weather_Snow    UMETA(DisplayName = "Weather_Snow"),
    Dino_Breath     UMETA(DisplayName = "Dino_Breath"),
    Dino_BloodImpact UMETA(DisplayName = "Dino_BloodImpact"),
    Combat_SpearImpact UMETA(DisplayName = "Combat_SpearImpact"),
    Ambient_Insects UMETA(DisplayName = "Ambient_Insects"),
    Ambient_VolcanicAsh UMETA(DisplayName = "Ambient_VolcanicAsh"),
    Water_Splash    UMETA(DisplayName = "Water_Splash"),
    Dust_DinoRun    UMETA(DisplayName = "Dust_DinoRun")
};

// ============================================================
// VFX STRUCTS — all prefixed FVFX_ to avoid collisions
// ============================================================

USTRUCT(BlueprintType)
struct FVFX_EffectEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DefaultScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifetimeSeconds = 3.0f;
};

USTRUCT(BlueprintType)
struct FVFX_SpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    AActor* AttachTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FName AttachSocket = NAME_None;
};

// ============================================================
// AVFX_Manager — World Actor managing all VFX spawning
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = "VFX")
class TRANSPERSONALGAME_API AVFX_Manager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_Manager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Effect Library ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Library")
    TArray<FVFX_EffectEntry> EffectLibrary;

    // ---- LOD Settings ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD0_Distance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD1_Distance = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|LOD")
    float LOD2_Distance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveEffects = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    bool bEnableLODCulling = true;

    // ---- Weather State ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
    bool bIsStorming = false;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(const FVFX_SpawnRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffectAtLocation(EVFX_EffectType EffectType, FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetWeatherState(float InRainIntensity, float InFogDensity, bool bInIsStorming);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinoFootstepDust(FVector ImpactLocation, float DinoMassKg);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodImpact(FVector HitLocation, FVector HitNormal, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector CampfireLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "VFX|Debug")
    void DebugSpawnAllEffects();

protected:
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    const FVFX_EffectEntry* FindEffectEntry(EVFX_EffectType EffectType) const;
    void PruneDeadEffects();
    float GetLODScaleForDistance(float Distance) const;
};
