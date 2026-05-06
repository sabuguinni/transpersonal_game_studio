#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"
#include "VFX_FootstepEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_FootstepSurface : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Stone       UMETA(DisplayName = "Stone"),
    Mud         UMETA(DisplayName = "Mud"),
    Sand        UMETA(DisplayName = "Sand"),
    Snow        UMETA(DisplayName = "Snow"),
    Water       UMETA(DisplayName = "Water")
};

USTRUCT(BlueprintType)
struct FVFX_FootstepEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor EffectColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bUseScreenShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float ScreenShakeIntensity = 0.5f;

    FVFX_FootstepEffectData()
    {
        EffectScale = 1.0f;
        EffectDuration = 2.0f;
        EffectColor = FLinearColor::White;
        bUseScreenShake = true;
        ScreenShakeIntensity = 0.5f;
    }
};

/**
 * VFX Footstep Effect Manager
 * Manages footstep particle effects for different creatures and surfaces
 * Syncs with Audio_BiomeAudioManager from Agent #16
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_FootstepEffectManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_FootstepEffectManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === FOOTSTEP VFX SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    TMap<EVFX_FootstepSurface, FVFX_FootstepEffectData> SurfaceEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Footstep")
    TMap<EBiomeType, FVFX_FootstepEffectData> BiomeEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Components")
    class UNiagaraComponent* FootstepParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float MaxFootstepDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float EffectCooldown = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bAutoDetectSurface = true;

    // === AUDIO SYNC SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Audio Sync")
    bool bSyncWithAudioSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Audio Sync")
    float AudioSyncRadius = 2000.0f;

    // === PERFORMANCE SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveEffects = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float LODDistance2 = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float CullDistance = 5000.0f;

    // === PUBLIC METHODS ===

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void TriggerFootstepEffect(const FVector& Location, EVFX_FootstepSurface Surface, float CreatureSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void TriggerBiomeFootstepEffect(const FVector& Location, EBiomeType Biome, float CreatureSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    EVFX_FootstepSurface DetectSurfaceType(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    EBiomeType DetectBiomeType(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Audio Sync")
    void SyncWithAudioManager();

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void UpdateLODSettings(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "VFX|Performance")
    void CleanupOldEffects();

private:
    // === INTERNAL STATE ===

    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    class AActor* AudioManagerReference;

    float LastEffectTime;
    FVector LastEffectLocation;
    int32 ActiveEffectCount;

    // === INTERNAL METHODS ===

    void InitializeSurfaceEffects();
    void InitializeBiomeEffects();
    void FindAudioManager();
    bool CanTriggerEffect(const FVector& Location);
    void SpawnFootstepParticles(const FVector& Location, const FVFX_FootstepEffectData& EffectData, float Scale);
    void ApplyScreenShake(const FVector& Location, float Intensity);
    float CalculateLODScale(float Distance);
    void RemoveOldestEffect();
};