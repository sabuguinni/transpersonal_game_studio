#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/AmbientSound.h"
#include "Engine/AudioVolume.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "World_PerformanceIntegratedBiomeAudioSystem.generated.h"

// Performance-integrated biome audio configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TSoftObjectPtr<USoundCue> AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float MaxAudioDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimization = true;

    FWorld_BiomeAudioConfig()
    {
        BiomeType = EWorld_BiomeType::Forest;
        MaxAudioDistance = 5000.0f;
        VolumeMultiplier = 1.0f;
        PerformanceThreshold = 60.0f;
        bEnablePerformanceOptimization = true;
    }
};

// Biome audio zone with performance monitoring
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeAudioZone : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeAudioZone();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FWorld_BiomeAudioConfig BiomeAudioConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceMonitoringInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AudioPerformanceImpact = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceOptimized = false;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetBiomeType(EWorld_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdateAmbientSound(USoundCue* NewSoundCue);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceImpact() const { return AudioPerformanceImpact; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceOptimized() const { return bIsPerformanceOptimized; }

private:
    float PerformanceTimer = 0.0f;
    
    void MonitorPerformance();
    void OptimizeAudioPerformance();
    void UpdateAudioSettings();
};

// Performance-integrated biome audio subsystem
UCLASS()
class TRANSPERSONALGAME_API UWorld_PerformanceIntegratedBiomeAudioSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

protected:
    UPROPERTY()
    TArray<AWorld_BiomeAudioZone*> RegisteredAudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float GlobalPerformanceThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AudioOptimizationFactor = 0.8f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float GlobalAudioPerformanceImpact = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 ActiveAudioZones = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bGlobalPerformanceOptimizationEnabled = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void RegisterAudioZone(AWorld_BiomeAudioZone* AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UnregisterAudioZone(AWorld_BiomeAudioZone* AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    TArray<AWorld_BiomeAudioZone*> GetAudioZonesInRange(FVector Location, float Range);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalPerformanceThreshold(float NewThreshold);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableGlobalPerformanceOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetGlobalAudioPerformanceImpact() const { return GlobalAudioPerformanceImpact; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveAudioZoneCount() const { return ActiveAudioZones; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeAllAudioZones();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceOptimization();

private:
    float PerformanceUpdateTimer = 0.0f;
    float PerformanceUpdateInterval = 2.0f;

    void UpdateGlobalPerformanceMetrics();
    void ApplyGlobalPerformanceOptimization();
    void ValidateAudioZones();
};

// Performance-integrated biome audio manager
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PerformanceIntegratedBiomeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PerformanceIntegratedBiomeAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FWorld_BiomeAudioConfig> BiomeAudioConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceMonitoringFrequency = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdaptivePerformanceOptimization = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float SystemPerformanceImpact = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalManagedAudioZones = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void InitializeBiomeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void CreateBiomeAudioZone(EWorld_BiomeType BiomeType, FVector Location, FVector Scale);

    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void UpdateBiomeAudioConfiguration(EWorld_BiomeType BiomeType, const FWorld_BiomeAudioConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceIntegration(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSystemPerformanceImpact() const { return SystemPerformanceImpact; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceMetrics();

private:
    float PerformanceTimer = 0.0f;
    
    void MonitorSystemPerformance();
    void ApplyAdaptiveOptimization();
    void UpdatePerformanceMetrics();
    FWorld_BiomeAudioConfig* GetBiomeAudioConfig(EWorld_BiomeType BiomeType);
};