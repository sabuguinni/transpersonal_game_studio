#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// EAudio_BiomeType — which biome zone the player is in
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcano     UMETA(DisplayName = "Volcano"),
    Swamp       UMETA(DisplayName = "Swamp"),
};

// ─────────────────────────────────────────────────────────────────────────────
// EAudio_ThreatLevel — current danger level driving adaptive music
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Critical    UMETA(DisplayName = "Critical"),
};

// ─────────────────────────────────────────────────────────────────────────────
// FAudio_AmbientLayer — one layer of the ambient soundscape
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> Sound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

// ─────────────────────────────────────────────────────────────────────────────
// FAudio_FootstepConfig — footstep sound parameters per surface
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    FName SurfaceType = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    float PitchVariance = 0.1f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudioSystemManager — manages all adaptive audio in the world
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Biome & Threat ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Adaptive")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintPure, Category = "Audio|Adaptive")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio|Adaptive")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // ── Footstep ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Footstep")
    void PlayFootstep(FName SurfaceType, FVector Location, float Weight = 1.0f);

    // ── Dinosaur proximity ─────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void NotifyDinosaurNearby(float DistanceMeters, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerScreenShakeFromFootfall(float Intensity);

    // ── Weather ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Weather")
    void SetWeatherIntensity(float RainIntensity, float WindIntensity, float ThunderProbability);

    // ── Ambient layers ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Footstep")
    TArray<FAudio_FootstepConfig> FootstepConfigs;

    // ── Adaptive music crossfade time ──────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MusicCrossfadeSeconds = 2.0f;

    // ── Threat detection radius ────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Adaptive", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
    float ThreatDetectionRadius = 500.0f;

    // ── Screen shake camera ────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float ScreenShakeBaseIntensity = 1.0f;

private:
    UPROPERTY()
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Forest;

    UPROPERTY()
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAmbientComponents;

    float ThreatUpdateTimer = 0.0f;
    float WeatherRainIntensity = 0.0f;
    float WeatherWindIntensity = 0.0f;

    void UpdateAmbientLayers();
    void CrossfadeToThreatLevel(EAudio_ThreatLevel NewLevel);
    float GetVolumeForBiomeLayer(const FAudio_AmbientLayer& Layer) const;
};
