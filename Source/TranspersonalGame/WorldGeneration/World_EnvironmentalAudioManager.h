#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataAsset.h"
#include "WorldGenerationTypes.h"
#include "World_EnvironmentalAudioManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeAudioType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra")
};

UENUM(BlueprintType)
enum class EWorld_WeatherAudioState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Windy       UMETA(DisplayName = "Windy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EWorld_BiomeAudioType BiomeType = EWorld_BiomeAudioType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TObjectPtr<USoundCue> AmbientSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TObjectPtr<USoundCue> DaySound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TObjectPtr<USoundCue> NightSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    bool bUseRandomVariations = true;

    FWorld_BiomeAudioData()
    {
        BiomeType = EWorld_BiomeAudioType::Forest;
        VolumeMultiplier = 1.0f;
        FadeDistance = 2000.0f;
        bUseRandomVariations = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    EWorld_WeatherAudioState WeatherState = EWorld_WeatherAudioState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    TObjectPtr<USoundCue> WeatherSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    float FadeOutTime = 5.0f;

    FWorld_WeatherAudioData()
    {
        WeatherState = EWorld_WeatherAudioState::Clear;
        IntensityMultiplier = 1.0f;
        FadeInTime = 3.0f;
        FadeOutTime = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_BiomeAudioDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Audio")
    TArray<FWorld_BiomeAudioData> BiomeAudioConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Audio")
    TArray<FWorld_WeatherAudioData> WeatherAudioConfigs;
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

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> BiomeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> WeatherAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> DinosaurAudioComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TObjectPtr<UWorld_BiomeAudioDataAsset> BiomeAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float AudioUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float MaxAudioDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    bool bEnableDynamicWeatherAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    bool bEnableDayNightCycle = true;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWorld_BiomeAudioType CurrentBiome = EWorld_BiomeAudioType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWorld_WeatherAudioState CurrentWeather = EWorld_WeatherAudioState::Clear;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsDay = true;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentAudioIntensity = 1.0f;

private:
    float AudioUpdateTimer = 0.0f;
    float LastBiomeTransitionTime = 0.0f;
    float LastWeatherTransitionTime = 0.0f;

public:
    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetBiomeAudio(EWorld_BiomeAudioType NewBiome, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetWeatherAudio(EWorld_WeatherAudioState NewWeather, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetDayNightState(bool bNewIsDay, float TransitionTime = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void PlayDinosaurAmbientSound(FVector DinosaurLocation, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetMasterAudioVolume(float VolumeMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    EWorld_BiomeAudioType DetectBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateAudioBasedOnPlayerLocation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Audio")
    void OnBiomeAudioChanged(EWorld_BiomeAudioType NewBiome);

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental Audio")
    void OnWeatherAudioChanged(EWorld_WeatherAudioState NewWeather);

protected:
    // Internal Methods
    void UpdateBiomeAudio();
    void UpdateWeatherAudio();
    void UpdateDayNightAudio();
    FWorld_BiomeAudioData* GetBiomeAudioData(EWorld_BiomeAudioType BiomeType);
    FWorld_WeatherAudioData* GetWeatherAudioData(EWorld_WeatherAudioState WeatherState);
    void CrossfadeAudioComponents(UAudioComponent* FromComponent, UAudioComponent* ToComponent, float Duration);
    float CalculateDistanceBasedVolume(FVector SourceLocation, FVector ListenerLocation);
};