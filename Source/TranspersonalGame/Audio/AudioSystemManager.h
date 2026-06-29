#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Adaptive ambient audio, survival feedback sounds,
// and environmental audio zones for the prehistoric world.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenPlain       UMETA(DisplayName = "Open Plain"),
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    NightTime       UMETA(DisplayName = "Night Time"),
    Storm           UMETA(DisplayName = "Storm")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Uneasy      UMETA(DisplayName = "Uneasy"),
    Threat      UMETA(DisplayName = "Threat Nearby"),
    Imminent    UMETA(DisplayName = "Imminent Danger"),
    Combat      UMETA(DisplayName = "In Combat")
};

UENUM(BlueprintType)
enum class EAudio_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Storm"),
    Fog         UMETA(DisplayName = "Fog")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenPlain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* IdleSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AlertSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AttackSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DeathSound = nullptr;

    // Distance at which distant rumble is audible (for large dinos like Brachiosaurus)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DistantAudioRadius = 5000.0f;

    // Screen shake magnitude when this dino walks nearby
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FootstepShakeMagnitude = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_SurvivalFeedback
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* LowHealthHeartbeat = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* HungerGrowl = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ThirstGasp = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* StaminaExhausted = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DamageImpact = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* HealingSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* CraftingStone = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FireIgnite = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FireExtinguish = nullptr;
};

/**
 * UAudio_AudioSystemManager
 * 
 * Central audio management component. Handles:
 * - Adaptive ambient layers by biome zone
 * - Danger-level music tension system
 * - Dinosaur sound profiles with distance falloff
 * - Survival feedback sounds (heartbeat, hunger, thirst)
 * - Weather audio transitions (rain, storm, fog)
 * - Screen shake on heavy dino footsteps
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Audio System Manager")
class TRANSPERSONALGAME_API UAudio_AudioSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AudioSystemManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Biome & Ambient ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    EAudio_BiomeZone GetCurrentBiomeZone() const { return CurrentBiomeZone; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetWeatherState(EAudio_WeatherState NewWeather);

    // ---- Danger / Music Tension ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    void NotifyPredatorNearby(FName SpeciesName, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Tension")
    void NotifyPredatorLost();

    // ---- Dinosaur Sounds ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaurs")
    void PlayDinoSound(FName SpeciesName, FName SoundType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaurs")
    void TriggerDinoFootstep(FName SpeciesName, FVector Location, float Mass);

    // ---- Survival Feedback ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void UpdateSurvivalStats(float Health, float Hunger, float Thirst, float Stamina);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayDamageImpact(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayCraftingSound(FName CraftingType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayFireSound(bool bIgniting);

    // ---- Screen Shake ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerScreenShake(float Magnitude, float Duration);

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinoSoundProfile> DinoSoundProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_SurvivalFeedback SurvivalSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.6f;

    // Danger radius — within this distance, predator triggers tension music
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float PredatorTensionRadius = 2000.0f;

    // Time (seconds) after losing predator before music relaxes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float TensionReleaseDelay = 8.0f;

    // Voice lines from ElevenLabs — wired to dialogue system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString SurvivorNarratorURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782758728022_SurvivorNarrator.mp3";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString ElderHealerURL = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782758746966_ElderHealer.mp3";

    // Freesound references for ambient layers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString CampfireFreesoundID = "681366";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString ForestThunderFreesoundID = "728687";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|References")
    FString JungleRainFreesoundID = "583932";

private:
    UPROPERTY()
    EAudio_BiomeZone CurrentBiomeZone = EAudio_BiomeZone::OpenPlain;

    UPROPERTY()
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY()
    EAudio_WeatherState CurrentWeatherState = EAudio_WeatherState::Clear;

    float TensionReleaseTimer = 0.0f;
    bool bPredatorTracked = false;

    float LastHealthValue = 100.0f;
    float HeartbeatTimer = 0.0f;
    float HeartbeatInterval = 1.5f;

    void TickHeartbeat(float DeltaTime);
    void TickTensionRelease(float DeltaTime);
    void UpdateAmbientLayerVolumes();

    FAudio_DinoSoundProfile* FindDinoProfile(FName SpeciesName);
};
