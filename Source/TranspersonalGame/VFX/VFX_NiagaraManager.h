#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/SceneComponent.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    VolcanicEruption    UMETA(DisplayName = "Volcanic Eruption"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    DustCloud           UMETA(DisplayName = "Dust Cloud"),
    FootstepImpact      UMETA(DisplayName = "Footstep Impact"),
    BreathVapor         UMETA(DisplayName = "Breath Vapor"),
    CampfireSmoke       UMETA(DisplayName = "Campfire Smoke"),
    AmbientParticles    UMETA(DisplayName = "Ambient Particles"),
    BloodSplatter       UMETA(DisplayName = "Blood Splatter")
};

USTRUCT(BlueprintType)
struct FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_EffectType EffectType = EVFX_EffectType::DustCloud;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoActivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector SpawnOffset = FVector::ZeroVector;

    FVFX_EffectSettings()
    {
        EffectType = EVFX_EffectType::DustCloud;
        Intensity = 1.0f;
        Duration = 5.0f;
        bAutoActivate = true;
        SpawnOffset = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TArray<FVFX_EffectSettings> EffectSettings;

    // Niagara system assets for different effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* VolcanicEruptionSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* WaterSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* DustCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* FootstepImpactSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* BreathVaporSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    UNiagaraSystem* CampfireSystem;

public:
    virtual void Tick(float DeltaTime) override;

    // VFX Management Functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXEffectWithSettings(const FVFX_EffectSettings& Settings, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    // Dinosaur-specific VFX
    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerFootstepImpact(FVector Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerBreathVapor(FVector Location, bool bIsCold = true);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur VFX")
    void TriggerBloodSplatter(FVector Location, FVector ImpactDirection);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void TriggerVolcanicEruption(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void TriggerWaterSplash(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment VFX")
    void CreateCampfire(FVector Location);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    int32 GetActiveVFXCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredVFX();

private:
    UNiagaraSystem* GetNiagaraSystemForEffect(EVFX_EffectType EffectType);
    void InitializeVFXSystems();
    void UpdateVFXComponents(float DeltaTime);
};