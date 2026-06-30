#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeAudioManager.generated.h"

/**
 * EWorld_BiomeType — biome classification for audio and environmental systems.
 * Prefix: World_ to avoid conflicts with other agents.
 */
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Prehistoric Forest"),
    Plains      UMETA(DisplayName = "Open Plains"),
    RiverDelta  UMETA(DisplayName = "River Delta"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic Region"),
    Count       UMETA(Hidden)
};

/**
 * FWorld_BiomeAudioConfig — audio configuration per biome.
 * Defines ambient sound layers, weather audio, and creature call frequency.
 */
USTRUCT(BlueprintType)
struct FWorld_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    /** Base ambient volume multiplier for this biome (0.0-1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolumeMultiplier = 1.0f;

    /** Wind intensity for this biome (affects audio and VFX) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindIntensity = 0.3f;

    /** How frequently dinosaur calls occur (calls per minute) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float CreatureCallFrequency = 2.0f;

    /** Water audio presence (0=none, 1=full river) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterAudioPresence = 0.0f;

    /** Whether this biome has active insect/bird ambience */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    bool bHasInsectAmbience = true;

    /** Whether this biome has active geological sounds (rumble, steam vents) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    bool bHasGeologicalSounds = false;
};

/**
 * FWorld_BiomeZone — defines a biome region in world space.
 */
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    /** World-space center of this biome zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FVector WorldCenter = FVector::ZeroVector;

    /** Radius of influence for this biome zone (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone", meta = (ClampMin = "100.0"))
    float Radius = 5000.0f;

    /** Audio config for this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FWorld_BiomeAudioConfig AudioConfig;
};

/**
 * ABiomeAudioManager — manages environmental audio tied to biome zones and weather.
 * Placed in the level, detects player position, blends audio between biomes.
 * Agent #5 — Procedural World Generator.
 */
UCLASS(ClassGroup = "TranspersonalGame|World", meta = (DisplayName = "Biome Audio Manager"))
class TRANSPERSONALGAME_API ABiomeAudioManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeAudioManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === BIOME ZONES ===

    /** All registered biome zones in the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    TArray<FWorld_BiomeZone> BiomeZones;

    /** How often (seconds) to update the active biome detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones", meta = (ClampMin = "0.1"))
    float BiomeUpdateInterval = 0.5f;

    // === WEATHER AUDIO ===

    /** Current weather intensity (0=clear, 1=heavy storm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherIntensity = 0.0f;

    /** Whether it is currently raining */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    bool bIsRaining = false;

    /** Whether a thunderstorm is active */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Audio")
    bool bIsThunderstorm = false;

    // === RUNTIME STATE ===

    /** Currently active biome type (updated each tick interval) */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    EWorld_BiomeType ActiveBiomeType = EWorld_BiomeType::Plains;

    /** Blend weight of the active biome (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float ActiveBiomeBlendWeight = 1.0f;

    /** Secondary biome for crossfade blending */
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    EWorld_BiomeType SecondaryBiomeType = EWorld_BiomeType::Plains;

    // === PUBLIC API ===

    /** Set weather state — called by weather system */
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void SetWeatherState(float Intensity, bool bRaining, bool bThunderstorm);

    /** Get the audio config for the current player biome */
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    FWorld_BiomeAudioConfig GetActiveBiomeAudioConfig() const;

    /** Force a biome override (for cutscenes, scripted events) */
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void OverrideBiome(EWorld_BiomeType NewBiome, float BlendTime = 2.0f);

    /** Register a new biome zone at runtime (called by PCG system) */
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    void RegisterBiomeZone(const FWorld_BiomeZone& NewZone);

    /** Get biome type at a world position */
    UFUNCTION(BlueprintCallable, Category = "Biome Audio")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Debug: log all registered biome zones */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugLogBiomeZones() const;

private:
    /** Timer accumulator for biome update interval */
    float BiomeUpdateTimer = 0.0f;

    /** Whether a biome override is active */
    bool bBiomeOverrideActive = false;

    /** Override blend time remaining */
    float OverrideBlendTimeRemaining = 0.0f;

    /** Cached player pawn reference */
    UPROPERTY()
    APawn* CachedPlayerPawn = nullptr;

    /** Initialize default biome zones for MinPlayableMap */
    void InitializeDefaultBiomeZones();

    /** Update active biome based on player position */
    void UpdateActiveBiome();

    /** Find the dominant biome zone at a given location */
    int32 FindDominantBiomeZoneIndex(const FVector& Location) const;
};
