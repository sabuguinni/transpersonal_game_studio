#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "../Core/SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

// VFX Effect Types for prehistoric environment
UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None = 0,
    DinosaurFootstep,
    CampfireFire,
    AmbientDust,
    WindParticles,
    BloodSplatter,
    ImpactDebris,
    WaterSplash,
    BreathVapor
};

// VFX Intensity levels
UENUM(BlueprintType)
enum class EVFX_Intensity : uint8
{
    Low = 0,
    Medium,
    High,
    Extreme
};

// VFX Effect Settings
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_Intensity Intensity = EVFX_Intensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoDestroy = true;

    FVFX_EffectSettings()
    {
        EffectType = EVFX_EffectType::None;
        Intensity = EVFX_Intensity::Medium;
        Duration = 2.0f;
        Scale = 1.0f;
        TintColor = FLinearColor::White;
        bAutoDestroy = true;
    }
};

// VFX Emitter Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EmitterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVFX_EffectSettings Settings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SpawnRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bIsActive = true;

    FVFX_EmitterData()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        SpawnRate = 1.0f;
        bIsActive = true;
    }
};

/**
 * VFX Niagara Manager Component
 * Manages all visual effects for the prehistoric survival game
 * Handles dinosaur footsteps, environmental effects, combat impacts, etc.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnVFXEffect(EVFX_EffectType EffectType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator, const FVFX_EffectSettings& Settings = FVFX_EffectSettings());

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnDinosaurFootstep(const FVector& Location, float DinosaurSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfire(const FVector& Location, EVFX_Intensity Intensity = EVFX_Intensity::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnImpactEffect(const FVector& Location, const FVector& Normal, EVFX_EffectType ImpactType = EVFX_EffectType::ImpactDebris);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartAmbientDust(const FVector& Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartWindParticles(const FVector& Location, const FVector& WindDirection, float Strength = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplash(const FVector& Location, float SplashSize = 1.0f);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplatter(const FVector& Location, const FVector& Direction, EVFX_Intensity Intensity = EVFX_Intensity::Medium);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBreathVapor(const FVector& Location, const FVector& Direction, float Temperature = 0.0f);

    // VFX Control
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXByType(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQuality(int32 QualityLevel);

    // VFX Query
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX")
    int32 GetActiveVFXCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VFX")
    bool IsVFXTypeActive(EVFX_EffectType EffectType) const;

protected:
    // VFX Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    TArray<FVFX_EmitterData> ActiveEmitters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    TMap<EVFX_EffectType, TSoftObjectPtr<UNiagaraSystem>> VFXAssetRegistry;

    // VFX Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveEmitters = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    float VFXQualityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    bool bEnableVFX = true;

    // Internal Methods
    void InitializeVFXRegistry();
    void CleanupExpiredVFX();
    UNiagaraSystem* GetVFXAsset(EVFX_EffectType EffectType);
    void RegisterVFXEmitter(const FVFX_EmitterData& EmitterData);
};

/**
 * VFX Manager Actor
 * World-level VFX management for environmental effects
 */
UCLASS()
class TRANSPERSONALGAME_API AVFX_ManagerActor : public AActor
{
    GENERATED_BODY()

public:
    AVFX_ManagerActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UVFX_NiagaraManager* VFXManager;

    // Environmental VFX Control
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void UpdateEnvironmentalVFX(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetWeatherVFX(EWeatherType WeatherType, float Intensity);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    float EnvironmentUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (AllowPrivateAccess = "true"))
    float LastEnvironmentUpdate = 0.0f;
};