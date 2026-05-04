#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Audio_SpatialAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeAudioType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"), 
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Caution     UMETA(DisplayName = "Caution"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bUseReverb = true;

    FAudio_BiomeAudioConfig()
    {
        BaseVolume = 0.7f;
        AttenuationRadius = 5000.0f;
        bUseReverb = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ThreatAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> ThreatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerDistance = 2000.0f;

    FAudio_ThreatAudioConfig()
    {
        ThreatVolume = 0.5f;
        TriggerDistance = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SpatialAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SpatialAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* BiomeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* ThreatAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WeatherAudioComponent;

    // Biome Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeAudioType, FAudio_BiomeAudioConfig> BiomeAudioConfigs;

    // Threat Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Audio")
    TMap<EAudio_ThreatLevel, FAudio_ThreatAudioConfig> ThreatAudioConfigs;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_BiomeAudioType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float BiomeTransitionProgress;

    // Audio Update Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BiomeTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ThreatDetectionRadius = 3000.0f;

private:
    float LastUpdateTime;
    class APawn* PlayerPawn;

public:
    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateSpatialAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_BiomeAudioType DetectCurrentBiome(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ThreatLevel DetectThreatLevel(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TransitionToBiome(EAudio_BiomeAudioType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayWeatherAudio(TSoftObjectPtr<USoundCue> WeatherSound, float Volume = 0.6f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopWeatherAudio();

protected:
    void InitializeAudioConfigs();
    void UpdateBiomeAudio(float DeltaTime);
    void UpdateThreatAudio();
    void UpdateWeatherAudio();
    
    float CalculateDistanceToNearestThreat(const FVector& Location);
    TArray<class APawn*> GetNearbyDinosaurs(const FVector& Location, float Radius);
};