#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_NiagaraSystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    DinoFootstep    UMETA(DisplayName = "Dinosaur Footstep"),
    WeatherRain     UMETA(DisplayName = "Weather Rain"),
    BloodImpact     UMETA(DisplayName = "Blood Impact"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    DustCloud       UMETA(DisplayName = "Dust Cloud"),
    Sparks          UMETA(DisplayName = "Sparks"),
    Smoke           UMETA(DisplayName = "Smoke")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float LifeTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAutoActivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFXEffect(EVFX_EffectType EffectType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFXEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXIntensity(EVFX_EffectType EffectType, float Intensity);

    // Prehistoric Environment VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnCampfireVFX(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnWeatherRainVFX(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void SpawnWaterSplashVFX(FVector Location, float SplashSize = 1.0f);

    // Dinosaur VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnDinoFootstepVFX(FVector Location, float DinoSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    void SpawnBloodImpactVFX(FVector Location, FVector ImpactDirection);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnDustCloudVFX(FVector Location, float CloudSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void SpawnSparksVFX(FVector Location, FVector Direction);

    // Integration with Audio System
    UFUNCTION(BlueprintCallable, Category = "VFX|Audio Integration")
    void SyncWithAudioThreatLevel(EThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX|Audio Integration")
    void SyncWithBiomeAudio(EBiomeType BiomeType);

protected:
    // VFX Effect Storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    TMap<EVFX_EffectType, FVFX_EffectData> VFXEffectDatabase;

    // Active VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Runtime")
    TMap<EVFX_EffectType, class UNiagaraComponent*> ActiveVFXComponents;

    // VFX Intensity Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float GlobalVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Config")
    bool bVFXEnabled = true;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    int32 MaxActiveVFXSystems = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Performance")
    float VFXCullingDistance = 5000.0f;

    // Audio Integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Audio")
    EThreatLevel CurrentThreatLevel = EThreatLevel::Safe;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX|Audio")
    EBiomeType CurrentBiome = EBiomeType::Forest;

private:
    // Internal VFX Management
    void InitializeVFXDatabase();
    void CleanupExpiredVFX();
    bool IsVFXWithinCullingDistance(FVector VFXLocation) const;
    void UpdateVFXBasedOnThreatLevel();
    void UpdateVFXBasedOnBiome();

    // Timer Handles
    FTimerHandle VFXCleanupTimer;
    FTimerHandle VFXUpdateTimer;
};