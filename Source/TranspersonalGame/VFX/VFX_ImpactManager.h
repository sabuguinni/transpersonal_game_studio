#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<UNiagaraSystem> ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    TSoftObjectPtr<USoundCue> ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float EffectScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float SoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    FVector EffectOffset = FVector::ZeroVector;

    FVFX_ImpactData()
    {
        EffectScale = 1.0f;
        SoundVolume = 1.0f;
        EffectOffset = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_DinosaurFootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    EDinosaurSpecies DinosaurType = EDinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    FVFX_ImpactData FootstepImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    FVFX_ImpactData BreathVapor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    float FootstepForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur VFX")
    float BreathInterval = 3.0f;

    FVFX_DinosaurFootstepData()
    {
        DinosaurType = EDinosaurSpecies::TRex;
        FootstepForce = 1000.0f;
        BreathInterval = 3.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_ImpactManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* ImpactEffectComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* ImpactAudioComponent;

public:
    // Impact VFX Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TMap<EDinosaurSpecies, FVFX_DinosaurFootstepData> DinosaurFootstepEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    FVFX_ImpactData WeaponImpactEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    FVFX_ImpactData BloodSplatterEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    FVFX_ImpactData RockImpactEffect;

    // Environmental VFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    FVFX_ImpactData DustCloudEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    FVFX_ImpactData WaterSplashEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float EnvironmentalEffectRadius = 2000.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEffects = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinTimeBetweenEffects = 0.1f;

    // Impact Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurFootstep(EDinosaurSpecies DinosaurType, const FVector& Location, const FVector& Normal);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerWeaponImpact(const FVector& Location, const FVector& Normal, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerBloodSplatter(const FVector& Location, const FVector& Direction, float BloodAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerEnvironmentalEffect(const FVector& Location, const FString& EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurBreath(EDinosaurSpecies DinosaurType, const FVector& Location, const FVector& Direction);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void SetEffectQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void EnableDisableEffects(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    int32 GetActiveEffectCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX Utility")
    void ClearAllActiveEffects();

private:
    // Internal state
    TArray<UNiagaraComponent*> ActiveEffects;
    float LastEffectTime;
    int32 CurrentQualityLevel;
    bool bEffectsEnabled;

    // Internal functions
    void SpawnImpactEffect(const FVFX_ImpactData& ImpactData, const FVector& Location, const FVector& Normal, float Scale = 1.0f);
    void CleanupExpiredEffects();
    bool CanSpawnNewEffect() const;
    FVector CalculateEffectRotation(const FVector& Normal) const;
    void InitializeDefaultEffects();
};