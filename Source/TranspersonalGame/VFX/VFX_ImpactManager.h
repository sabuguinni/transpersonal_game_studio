#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    DinosaurFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    WeaponHit          UMETA(DisplayName = "Weapon Hit"),
    BloodSplatter      UMETA(DisplayName = "Blood Splatter"),
    RockCrush          UMETA(DisplayName = "Rock Crush"),
    WaterSplash        UMETA(DisplayName = "Water Splash")
};

UENUM(BlueprintType)
enum class EVFX_SurfaceType : uint8
{
    Dirt               UMETA(DisplayName = "Dirt"),
    Stone              UMETA(DisplayName = "Stone"),
    Grass              UMETA(DisplayName = "Grass"),
    Water              UMETA(DisplayName = "Water"),
    Sand               UMETA(DisplayName = "Sand"),
    Mud                UMETA(DisplayName = "Mud")
};

USTRUCT(BlueprintType)
struct FVFX_ImpactSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float DustIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float ParticleLifetime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    FVector ParticleVelocity = FVector(0, 0, 100);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    FLinearColor ParticleColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    int32 ParticleCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact VFX")
    float ScreenShakeIntensity = 0.5f;

    FVFX_ImpactSettings()
    {
        DustIntensity = 1.0f;
        ParticleLifetime = 3.0f;
        ParticleVelocity = FVector(0, 0, 100);
        ParticleColor = FLinearColor::White;
        ParticleCount = 50;
        ScreenShakeIntensity = 0.5f;
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
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* DustParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* BloodParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* DebrisParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_SurfaceType, FVFX_ImpactSettings> SurfaceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_ImpactType, FVFX_ImpactSettings> ImpactTypeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* DustExplosionSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* DebrisSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara Systems")
    UNiagaraSystem* WaterSplashSystem;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpact(EVFX_ImpactType ImpactType, EVFX_SurfaceType SurfaceType, FVector Location, FVector Normal = FVector::UpVector);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDinosaurFootstep(FVector Location, float DinosaurSize = 1.0f, EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Dirt);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerWeaponImpact(FVector Location, FVector HitDirection, EVFX_SurfaceType SurfaceType = EVFX_SurfaceType::Stone);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerBloodEffect(FVector Location, FVector BloodDirection, float BloodAmount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerEnvironmentalEffect(FVector Location, EVFX_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX Settings")
    void SetImpactSettings(EVFX_ImpactType ImpactType, const FVFX_ImpactSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Settings")
    void SetSurfaceSettings(EVFX_SurfaceType SurfaceType, const FVFX_ImpactSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void ClearAllActiveEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetEffectIntensityMultiplier(float Multiplier);

private:
    void InitializeDefaultSettings();
    void PlayParticleEffect(UNiagaraSystem* System, FVector Location, const FVFX_ImpactSettings& Settings);
    void ApplyScreenShake(FVector Location, float Intensity);

    UPROPERTY()
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;
};