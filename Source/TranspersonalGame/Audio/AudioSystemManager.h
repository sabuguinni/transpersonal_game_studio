// AudioSystemManager.h
// Agent #16 — Audio Agent
// Adaptive audio system for prehistoric survival game
// Manages ambient layers, danger music, and spatial sound cues

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Sound/AmbientSound.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

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
enum class EAudio_BiomeType : uint8
{
    OpenPlains  UMETA(DisplayName = "Open Plains"),
    DenseForest UMETA(DisplayName = "Dense Forest"),
    RiverBasin  UMETA(DisplayName = "River Basin"),
    RockyRidge  UMETA(DisplayName = "Rocky Ridge"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FName DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> DeathSound;

    // Radius at which footsteps cause camera shake
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepShakeRadius = 1500.0f;

    // Intensity multiplier for camera shake
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_NarrativeVoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FName LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FText SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    bool bPlayOnce = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    bool bHasPlayed = false;
};

// ─── AudioSystemManager Actor ─────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Threat / Music State ──────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // ── Biome / Ambient ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void TransitionToBiome(EAudio_BiomeType NewBiome, float CrossfadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    // ── Dinosaur Audio ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(FName Species, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerTRexApproachSequence(float Distance);

    // ── Narrative Voice ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    void PlayNarrativeLine(FName LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrative")
    void RegisterNarrativeLine(FAudio_NarrativeVoiceLine Line);

    // ── Screen Feedback ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageAudioFeedback(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerCraftingAudioFeedback();

    // ── Properties ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float VoiceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    TArray<FAudio_NarrativeVoiceLine> NarrativeLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float ThreatTransitionSpeed = 2.0f;

private:
    UPROPERTY()
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY()
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::OpenPlains;

    UPROPERTY()
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY()
    UAudioComponent* ActiveMusicComponent = nullptr;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAmbientComponents;

    float ThreatBlendAlpha = 0.0f;
    float TimeSinceLastThreatChange = 0.0f;

    void UpdateAdaptiveMusic(float DeltaTime);
    void UpdateAmbientLayers(float DeltaTime);
    void CrossfadeAmbientLayer(FAudio_AmbientLayer& Layer, bool bFadeIn, float Duration);
};
