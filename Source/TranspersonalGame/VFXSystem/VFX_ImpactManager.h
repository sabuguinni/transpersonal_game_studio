#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "VFX_ImpactManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    class UNiagaraSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    class USoundCue* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float VolumeMultiplier;

    FVFX_ImpactData()
    {
        ParticleEffect = nullptr;
        ImpactSound = nullptr;
        EffectScale = 1.0f;
        VolumeMultiplier = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_CampfireData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire VFX")
    class UNiagaraSystem* FlameEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire VFX")
    class UNiagaraSystem* SmokeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire VFX")
    class UNiagaraSystem* SparkEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire VFX")
    class USoundCue* CracklingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire VFX")
    float FlameIntensity;

    FVFX_CampfireData()
    {
        FlameEffect = nullptr;
        SmokeEffect = nullptr;
        SparkEffect = nullptr;
        CracklingSound = nullptr;
        FlameIntensity = 1.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* FootstepVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* CampfireFlameComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* CampfireSmokeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ImpactAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* CampfireAudioComponent;

public:
    virtual void Tick(float DeltaTime) override;

    // T-Rex footstep impact VFX
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerTRexFootstep(FVector ImpactLocation, float FootstepIntensity = 1.0f);

    // Raptor footstep impact VFX (smaller scale)
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerRaptorFootstep(FVector ImpactLocation, float FootstepIntensity = 0.3f);

    // Generic impact VFX for any dinosaur
    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurImpact(FVector ImpactLocation, float DinosaurSize, FString DinosaurType);

    // Campfire VFX management
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StartCampfireVFX(FVector CampfireLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StopCampfireVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void UpdateCampfireIntensity(float NewIntensity);

    // Screen shake effects
    UFUNCTION(BlueprintCallable, Category = "VFX Camera")
    void TriggerScreenShake(float Intensity, float Duration = 1.0f);

    // Dust cloud VFX for running dinosaurs
    UFUNCTION(BlueprintCallable, Category = "VFX Movement")
    void TriggerDustCloud(FVector Location, FVector Velocity, float DinosaurSize);

    // Blood impact VFX for combat
    UFUNCTION(BlueprintCallable, Category = "VFX Combat")
    void TriggerBloodImpact(FVector ImpactLocation, FVector ImpactNormal, float Intensity);

    // Environmental particle effects
    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StartRainVFX(float RainIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StopRainVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX Environment")
    void StartWindVFX(FVector WindDirection, float WindStrength);

protected:
    // VFX configuration data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Data")
    FVFX_ImpactData TRexImpactData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Data")
    FVFX_ImpactData RaptorImpactData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Data")
    FVFX_CampfireData CampfireData;

    // VFX timing and state
    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    bool bCampfireActive;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    bool bRainActive;

    UPROPERTY(BlueprintReadOnly, Category = "VFX State")
    float CurrentCampfireIntensity;

private:
    // Internal VFX helper methods
    void InitializeVFXComponents();
    void LoadVFXAssets();
    UNiagaraSystem* GetFootstepVFXForSize(float DinosaurSize);
    float CalculateVFXIntensity(float Distance, float BaseIntensity);
};