// BiomeAudioSystem.h
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_004
// Biome-driven environmental audio system — ties ambient sounds to biome zones

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "BiomeAudioSystem.generated.h"

// Biome type enum — unique prefix World_ to avoid cross-agent collision
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky"),
    River       UMETA(DisplayName = "River"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Unknown     UMETA(DisplayName = "Unknown"),
};

// Weather state enum
UENUM(BlueprintType)
enum class EWorld_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog"),
};

// Per-biome audio configuration
USTRUCT(BlueprintType)
struct FWorld_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    USoundBase* AmbientLoop = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    USoundBase* RainOverlay = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    USoundBase* NightLoop = nullptr;

    // Volume multiplier for this biome's ambient
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float VolumeMultiplier = 1.0f;

    // Crossfade duration when transitioning into this biome (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float CrossfadeDuration = 3.0f;
};

/**
 * ABiomeAudioSystem
 * Placed once in the level. Detects the player's current biome and
 * crossfades ambient audio accordingly. Also responds to weather changes.
 */
UCLASS(ClassGroup = (World), meta = (DisplayName = "Biome Audio System"))
class TRANSPERSONALGAME_API ABiomeAudioSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeAudioSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Configuration ────────────────────────────────────────────────────────

    // Per-biome audio configs — populate in editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FWorld_BiomeAudioConfig> BiomeConfigs;

    // How often (seconds) to re-query the player's biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float BiomeQueryInterval = 1.5f;

    // Radius (cm) around player to sample biome tags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float BiomeSampleRadius = 1000.0f;

    // Current weather — set by weather system or Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EWorld_WeatherState CurrentWeather = EWorld_WeatherState::Clear;

    // Is it night? Set by day/night cycle system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    bool bIsNight = false;

    // ── State (read-only in Blueprint) ───────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Biome Audio", meta = (AllowPrivateAccess = "true"))
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Audio", meta = (AllowPrivateAccess = "true"))
    EWorld_BiomeType PreviousBiome = EWorld_BiomeType::Unknown;

    // ── Audio Components ─────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Audio")
    UAudioComponent* AmbientChannelA = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Audio")
    UAudioComponent* AmbientChannelB = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Audio")
    UAudioComponent* WeatherChannel = nullptr;

    // ── Public API ───────────────────────────────────────────────────────────

    // Force a biome transition (called by world partition stream events)
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetBiome(EWorld_BiomeType NewBiome);

    // Update weather state — triggers audio overlay crossfade
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetWeather(EWorld_WeatherState NewWeather);

    // Set day/night — swaps ambient loop to night variant if available
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetNight(bool bNight);

    // Query biome at a world location using landscape layer weights
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    EWorld_BiomeType QueryBiomeAtLocation(FVector WorldLocation) const;

    // Get config for a given biome type
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    bool GetBiomeConfig(EWorld_BiomeType BiomeType, FWorld_BiomeAudioConfig& OutConfig) const;

private:
    // Internal crossfade logic
    void CrossfadeToNewBiome(const FWorld_BiomeAudioConfig& Config);
    void UpdateWeatherAudio();
    void TickBiomeQuery(float DeltaTime);

    // Crossfade state
    float CrossfadeTimer = 0.0f;
    float CrossfadeDuration = 0.0f;
    bool bCrossfading = false;
    bool bChannelAActive = true;  // Which channel is currently "live"

    // Query timer
    float BiomeQueryTimer = 0.0f;
};
