#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "World_EnvironmentalAudioManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeAudioType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Coastal     UMETA(DisplayName = "Coastal"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp")
};

UENUM(BlueprintType)
enum class EWorld_WeatherAudioState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* WeatherSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bUseReverb = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UReverbEffect* BiomeReverb;

    FWorld_BiomeAudioConfig()
    {
        AmbientSound = nullptr;
        WeatherSound = nullptr;
        BaseVolume = 0.7f;
        FadeDistance = 1000.0f;
        bUseReverb = true;
        BiomeReverb = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_EnvironmentalAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_EnvironmentalAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio configuration for each biome type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeAudioType, FWorld_BiomeAudioConfig> BiomeAudioConfigs;

    // Current audio state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    EWorld_BiomeAudioType CurrentBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio State")
    EWorld_WeatherAudioState CurrentWeather;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* DynamicAudioComponent;

    // Player tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Tracking")
    class APawn* TrackedPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AudioUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float BiomeTransitionTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float WeatherTransitionTime = 2.0f;

private:
    float LastAudioUpdateTime;
    bool bIsTransitioning;
    float TransitionStartTime;
    EWorld_BiomeAudioType TargetBiome;

public:
    // Main audio management functions
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdatePlayerBiome(EWorld_BiomeAudioType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherState(EWorld_WeatherAudioState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiomeAudioConfig(EWorld_BiomeAudioType BiomeType, const FWorld_BiomeAudioConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    FWorld_BiomeAudioConfig GetBiomeAudioConfig(EWorld_BiomeAudioType BiomeType) const;

    // Audio zone detection
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    EWorld_BiomeAudioType DetectPlayerBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void RegisterAudioZone(class ATriggerBox* AudioZone, EWorld_BiomeAudioType BiomeType);

    // Dynamic audio effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayDynamicAudioEvent(class USoundCue* SoundCue, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void StopDynamicAudio();

    // Audio volume and mixing
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterEnvironmentalVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void FadeToNewBiome(EWorld_BiomeAudioType NewBiome, float FadeTime = 3.0f);

protected:
    // Internal audio management
    void InitializeAudioComponents();
    void LoadDefaultBiomeConfigs();
    void UpdateAudioBasedOnLocation();
    void ProcessBiomeTransition(float DeltaTime);
    void ApplyBiomeAudioSettings(EWorld_BiomeAudioType BiomeType);
    void ApplyWeatherAudioSettings(EWorld_WeatherAudioState WeatherState);

    // Audio zone tracking
    TMap<class ATriggerBox*, EWorld_BiomeAudioType> RegisteredAudioZones;
    
    // Performance optimization
    bool ShouldUpdateAudio() const;
    void OptimizeAudioPerformance();
};