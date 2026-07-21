#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "VFX_FootstepEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_FootstepSurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt/Earth"),
    Rock        UMETA(DisplayName = "Rock/Stone"), 
    Grass       UMETA(DisplayName = "Grass/Vegetation"),
    Water       UMETA(DisplayName = "Water/Mud"),
    Sand        UMETA(DisplayName = "Sand/Gravel")
};

UENUM(BlueprintType)
enum class EVFX_CreatureSize : uint8
{
    Small       UMETA(DisplayName = "Small (Compsognathus)"),
    Medium      UMETA(DisplayName = "Medium (Raptor)"),
    Large       UMETA(DisplayName = "Large (T-Rex)"),
    Massive     UMETA(DisplayName = "Massive (Brachiosaurus)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_FootstepEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> DustEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> DebrisEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float DustIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor DustColor = FLinearColor::White;

    FVFX_FootstepEffectData()
    {
        EffectScale = 1.0f;
        DustIntensity = 1.0f;
        DustColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Earth brown
    }
};

/**
 * VFX Footstep Effect Manager
 * Handles particle effects for dinosaur and character footsteps
 * Integrates with Audio system for synchronized audio-visual feedback
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_FootstepEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_FootstepEffectManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core footstep effect functions
    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void TriggerFootstepEffect(const FVector& Location, EVFX_FootstepSurfaceType SurfaceType, EVFX_CreatureSize CreatureSize);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void TriggerDinosaurFootstep(const FVector& Location, const FString& DinosaurType, float SizeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Footstep")
    void TriggerPlayerFootstep(const FVector& Location, EVFX_FootstepSurfaceType SurfaceType);

    // Effect customization
    UFUNCTION(BlueprintCallable, Category = "VFX|Configuration")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Configuration")
    void SetSurfaceEffectData(EVFX_FootstepSurfaceType SurfaceType, const FVFX_FootstepEffectData& EffectData);

    // Integration with audio system
    UFUNCTION(BlueprintCallable, Category = "VFX|Audio")
    void SyncWithScreenShake(bool bEnableSync);

protected:
    // Effect data for different surface types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Effects")
    TMap<EVFX_FootstepSurfaceType, FVFX_FootstepEffectData> SurfaceEffects;

    // Creature size multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Scaling")
    TMap<EVFX_CreatureSize, float> CreatureSizeMultipliers;

    // Default Niagara systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> DefaultDustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> DefaultDebrisSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> BloodImpactSystem;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float MaxEffectDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float EffectCooldown = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxConcurrentEffects = 20;

private:
    // Internal state
    float LastEffectTime = 0.0f;
    int32 ActiveEffectCount = 0;
    bool bSyncWithAudio = true;

    // Helper functions
    FVFX_FootstepEffectData GetEffectDataForSurface(EVFX_FootstepSurfaceType SurfaceType) const;
    float GetCreatureSizeMultiplier(EVFX_CreatureSize CreatureSize) const;
    EVFX_FootstepSurfaceType DetectSurfaceType(const FVector& Location) const;
    void SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FVFX_FootstepEffectData& EffectData);
    bool CanSpawnEffect() const;
};