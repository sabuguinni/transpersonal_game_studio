#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — Audio_* prefix to avoid collision with engine types
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee")
};

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseJungle     UMETA(DisplayName = "Dense Jungle"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    CaveDeep        UMETA(DisplayName = "Cave Deep"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    Camp            UMETA(DisplayName = "Camp")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class EAudio_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Roar        UMETA(DisplayName = "Roar"),
    Charge      UMETA(DisplayName = "Charge"),
    Footstep    UMETA(DisplayName = "Footstep"),
    Death       UMETA(DisplayName = "Death")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float DurationSeconds = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasBeenPlayed = false;

    FAudio_VoiceLine()
        : LineID(TEXT("")), AudioURL(TEXT("")), SpeakerName(TEXT("")),
          DurationSeconds(10.0f), bHasBeenPlayed(false) {}
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float NightMultiplier = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float RainMultiplier = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bIsActive = false;

    FAudio_AmbientLayer()
        : BiomeZone(EAudio_BiomeZone::OpenPlains), BaseVolume(0.6f),
          NightMultiplier(1.4f), RainMultiplier(0.8f), bIsActive(false) {}
};

USTRUCT(BlueprintType)
struct FAudio_DinoSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    EAudio_DinosaurSpecies Species = EAudio_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepIntervalSeconds = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ScreenShakeRadiusThreshold = 1500.0f;

    FAudio_DinoSoundProfile()
        : Species(EAudio_DinosaurSpecies::TRex), RoarRadius(5000.0f),
          FootstepRadius(2000.0f), FootstepIntervalSeconds(1.2f),
          ScreenShakeRadiusThreshold(1500.0f) {}
};

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_VoiceLineComponent — attaches to NPC actors, plays TTS voice lines
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "TranspersonalAudio", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_VoiceLineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_VoiceLineComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float TriggerRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float CooldownSeconds = 30.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FString& LineID, const FString& AudioURL,
                           const FString& SpeakerName, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    FAudio_VoiceLine GetNextUnplayedLine() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void MarkLinePlayed(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    bool HasUnplayedLines() const;

protected:
    virtual void BeginPlay() override;

private:
    float LastPlayTimestamp = 0.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_SystemManager — world-level audio orchestrator
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    // ── Threat-level adaptive music ──────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void UpdateMusicLayersForThreat(EAudio_ThreatLevel Level);

    // ── Biome ambient system ─────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeZone CurrentBiomeZone = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateAmbientForTimeOfDay(float NormalizedTimeOfDay);

    // ── Dinosaur audio ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<FAudio_DinoSoundProfile> DinoSoundProfiles;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinoSound(EAudio_DinosaurSpecies Species, EAudio_DinoState State,
                          FVector DinoLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    float GetDistanceToPlayer(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    bool ShouldTriggerScreenShake(EAudio_DinosaurSpecies Species,
                                  FVector DinoLocation) const;

    // ── Voice line registry ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TMap<FString, FString> VoiceLineRegistry;

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLineURL(const FString& LineID, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    FString GetVoiceLineURL(const FString& LineID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void InitializeDefaultVoiceLines();

    // ── Day/night audio transition ───────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DayNight")
    bool bIsNight = false;

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    bool IsNightTime() const { return bIsNight; }

    // ── Screen shake ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerScreenShakeForDino(EAudio_DinosaurSpecies Species, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageFlash(float DamageAmount);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    UAudioComponent* MusicComponent = nullptr;

    float ThreatTransitionTimer = 0.0f;
    float AmbientUpdateInterval = 5.0f;
    float AmbientTimer = 0.0f;

    void InitializeDinoSoundProfiles();
    void InitializeAmbientLayers();
};
