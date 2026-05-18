#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFX_FootstepEffectManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_FootstepType : uint8
{
    Light       UMETA(DisplayName = "Light Step"),
    Medium      UMETA(DisplayName = "Medium Step"),
    Heavy       UMETA(DisplayName = "Heavy Step"),
    Massive     UMETA(DisplayName = "Massive Step")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Sand        UMETA(DisplayName = "Sand"),
    Rock        UMETA(DisplayName = "Rock"),
    Mud         UMETA(DisplayName = "Mud"),
    Grass       UMETA(DisplayName = "Grass")
};

USTRUCT(BlueprintType)
struct FVFX_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    EVFX_FootstepType StepType = EVFX_FootstepType::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    FVector ImpactNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep VFX")
    float IntensityMultiplier = 1.0f;

    FVFX_FootstepData()
    {
        StepType = EVFX_FootstepType::Medium;
        SurfaceType = EVFX_SurfaceType::Dirt;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        IntensityMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_FootstepEffectManager : public AActor
{
    GENERATED_BODY()

public:
    UVFX_FootstepEffectManager();

protected:
    virtual void BeginPlay() override;

    // Niagara Systems for different surface types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* DustCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* SandCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* RockDebrisSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* MudSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    class UNiagaraSystem* GrassParticleSystem;

    // Audio components for footstep sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* FootstepAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* LightFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* MediumFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* HeavyFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* MassiveFootstepSound;

    // Effect intensity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float LightStepIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MediumStepIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float HeavyStepIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MassiveStepIntensity = 2.0f;

    // Effect duration and cleanup
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectLifetime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 20;

    // Active effect tracking
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveEffects;

    // Timer handles for cleanup
    TArray<FTimerHandle> CleanupTimers;

public:
    // Main footstep effect function
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerFootstepEffect(const FVFX_FootstepData& FootstepData);

    // Individual surface effect functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateDustCloudEffect(const FVector& Location, const FVector& Normal, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateSandCloudEffect(const FVector& Location, const FVector& Normal, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateRockDebrisEffect(const FVector& Location, const FVector& Normal, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateMudSplashEffect(const FVector& Location, const FVector& Normal, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CreateGrassParticleEffect(const FVector& Location, const FVector& Normal, float Intensity);

    // Audio functions
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayFootstepSound(EVFX_FootstepType StepType, const FVector& Location);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    float GetIntensityForStepType(EVFX_FootstepType StepType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraSystem* GetSystemForSurface(EVFX_SurfaceType SurfaceType) const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupOldEffects();

    // T-Rex specific massive footstep effect
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerTRexFootstep(const FVector& Location, const FVector& Normal);

    // Velociraptor pack effects
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void TriggerPackHunterFootsteps(const TArray<FVector>& Locations, const TArray<FVector>& Normals);

private:
    // Internal cleanup function
    void CleanupEffect(class UNiagaraComponent* EffectComponent);

    // Effect pooling
    class UNiagaraComponent* GetPooledEffect();
    void ReturnEffectToPool(class UNiagaraComponent* EffectComponent);

    TArray<class UNiagaraComponent*> EffectPool;
};