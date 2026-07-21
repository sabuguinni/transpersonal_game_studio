#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/PostProcessVolume.h"
#include "SharedTypes.h"
#include "VFX_DamageScreenSystem.generated.h"

// VFX damage intensity levels
UENUM(BlueprintType)
enum class EVFX_DamageIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Damage"),
    Medium      UMETA(DisplayName = "Medium Damage"), 
    Heavy       UMETA(DisplayName = "Heavy Damage"),
    Critical    UMETA(DisplayName = "Critical Damage")
};

// VFX damage effect data structure
USTRUCT(BlueprintType)
struct FVFX_DamageEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Damage")
    float FlashIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Damage")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Damage")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Damage")
    float VignetteStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Damage")
    float ScreenShakeIntensity = 1.0f;

    FVFX_DamageEffectData()
    {
        FlashIntensity = 1.0f;
        FlashDuration = 0.3f;
        FlashColor = FLinearColor::Red;
        VignetteStrength = 0.5f;
        ScreenShakeIntensity = 1.0f;
    }
};

/**
 * VFX Damage Screen System - Manages visual damage feedback effects
 * Provides red screen flash, blood splatter overlay, and screen distortion
 * Integrates with Audio_ScreenShakeSystem for synchronized feedback
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_DamageScreenSystem : public AActor
{
    GENERATED_BODY()

public:
    AVFX_DamageScreenSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* BloodSplatterEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* DustParticleEffect;

    // Damage Effect Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_DamageIntensity, FVFX_DamageEffectData> DamageEffectPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    class UMaterialInterface* ScreenOverlayMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    class UNiagaraSystem* BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    class UNiagaraSystem* DustCloudSystem;

    // Post Process Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    class APostProcessVolume* DamagePostProcessVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float PostProcessBlendRadius = 1000.0f;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    bool bIsDamageEffectActive;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    float CurrentFlashAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    float DamageEffectTimer;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    EVFX_DamageIntensity CurrentDamageLevel;

    // Dynamic Material Instance
    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    class UMaterialInstanceDynamic* DynamicOverlayMaterial;

public:
    // Main VFX Trigger Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void TriggerDamageEffect(EVFX_DamageIntensity Intensity, FVector ImpactLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void TriggerBloodSplatter(FVector ImpactLocation, FVector ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void TriggerDustCloud(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Damage")
    void StopAllDamageEffects();

    // Screen Overlay Management
    UFUNCTION(BlueprintCallable, Category = "VFX Screen")
    void SetScreenFlashAlpha(float Alpha);

    UFUNCTION(BlueprintCallable, Category = "VFX Screen")
    void SetVignetteStrength(float Strength);

    UFUNCTION(BlueprintCallable, Category = "VFX Screen")
    void ApplyScreenDistortion(float DistortionAmount, float Duration);

    // Integration with Audio System
    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void SynchronizeWithScreenShake(class AAudio_ScreenShakeSystem* ShakeSystem);

    UFUNCTION(BlueprintCallable, Category = "VFX Integration")
    void OnScreenShakeTriggered(float ShakeIntensity, FVector ShakeLocation);

    // Effect Intensity Scaling
    UFUNCTION(BlueprintCallable, Category = "VFX Scaling")
    float CalculateDistanceIntensity(FVector EffectLocation, float MaxDistance = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Scaling")
    FVFX_DamageEffectData GetDamageEffectData(EVFX_DamageIntensity Intensity);

protected:
    // Internal VFX Management
    void InitializeDamageEffectPresets();
    void UpdateDamageEffectTick(float DeltaTime);
    void CreateDynamicMaterials();
    void SetupPostProcessVolume();

    // Particle Effect Helpers
    void SpawnBloodParticles(FVector Location, FVector Direction);
    void SpawnDustParticles(FVector Location, float Scale);
    void CleanupExpiredEffects();

    // Screen Effect Helpers
    void UpdateScreenFlash(float DeltaTime);
    void UpdateVignetteEffect(float DeltaTime);
    void ResetScreenEffects();

private:
    // Internal timers and state
    float FlashFadeTimer;
    float VignetteFadeTimer;
    float DistortionTimer;
    float DistortionDuration;
    
    // Effect pooling for performance
    TArray<class UNiagaraComponent*> ActiveBloodEffects;
    TArray<class UNiagaraComponent*> ActiveDustEffects;
    
    // Player reference for screen effects
    class APlayerController* CachedPlayerController;
    class APawn* CachedPlayerPawn;
};