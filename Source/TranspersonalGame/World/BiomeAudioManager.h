// BiomeAudioManager.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_004
// Manages environmental audio tied to biome zones and weather states.
// Priority P1 — World Generation (environmental audio layer).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeAudioManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Unknown     UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeAudioLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    EWorld_WeatherState WeatherState = EWorld_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float WindIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    bool bPlayInsectChorus = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    bool bPlayBirdCalls = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    bool bPlayWaterFlow = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeAudioManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Current biome the player is in
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BiomeAudio")
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::Unknown;

    // Current weather state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BiomeAudio")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    // Audio layer configurations per biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    TArray<FWorld_BiomeAudioLayer> BiomeAudioLayers;

    // Radius around player to detect biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float BiomeDetectionRadius = 500.0f;

    // How fast audio transitions between biomes (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float AudioTransitionSpeed = 3.0f;

    // Weather transition timer (seconds between weather changes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BiomeAudio")
    float WeatherChangePeriod = 300.0f;

    // Detect which biome the player is currently in
    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    EWorld_BiomeType DetectPlayerBiome() const;

    // Force a weather state change (called by weather system)
    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    void SetWeatherState(EWorld_WeatherState NewWeather);

    // Get the active audio layer for current biome + weather combination
    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    FWorld_BiomeAudioLayer GetActiveAudioLayer() const;

    // Returns true if audio layer was found for current state
    UFUNCTION(BlueprintCallable, Category = "BiomeAudio")
    bool HasAudioLayerForCurrentState() const;

private:
    float WeatherTimer = 0.0f;
    float TransitionAlpha = 0.0f;
    EWorld_BiomeType PreviousBiome = EWorld_BiomeType::Unknown;

    void InitDefaultAudioLayers();
    void UpdateWeatherCycle(float DeltaTime);
    void UpdateBiomeDetection();
};
