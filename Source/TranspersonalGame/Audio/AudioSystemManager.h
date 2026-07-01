#pragma once

// AudioSystemManager.h
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260701_006
// Adaptive audio system: ambient layers, dinosaur proximity audio, survival feedback sounds
// Integrates with MetaSounds for procedural audio blending

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ─── Enums ──────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_BiomeLayer : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Cave        UMETA(DisplayName = "Cave"),
    OpenPlain   UMETA(DisplayName = "Open Plain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

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
    EAudio_BiomeLayer Biome = EAudio_BiomeLayer::OpenPlain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;
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

    // Footstep rumble radius in cm — larger dinos shake the ground further
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepRumbleRadius = 2000.0f;

    // Camera shake magnitude when this dino walks nearby
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float ProximityShakeMagnitude = 0.5f;
};

USTRUCT(BlueprintType)
struct FAudio_SurvivalFeedback
{
    GENERATED_BODY()

    // Played when player health drops below 30%
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    TSoftObjectPtr<USoundBase> LowHealthHeartbeat;

    // Played when hunger reaches critical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    TSoftObjectPtr<USoundBase> HungerGrowl;

    // Played when thirst reaches critical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    TSoftObjectPtr<USoundBase> ThirstGasp;

    // Played when player takes damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    TSoftObjectPtr<USoundBase> DamageImpact;

    // Played when player crafts a tool
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    TSoftObjectPtr<USoundBase> CraftingSuccess;

    // Played when player lights fire
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    TSoftObjectPtr<USoundBase> FireIgnite;
};

// ─── Main Class ──────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "TranspersonalGame|Audio", meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Ambient Layer Control ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiomeLayer(EAudio_BiomeLayer NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void CrossfadeToLayer(EAudio_BiomeLayer TargetBiome, float FadeDuration = 3.0f);

    // ── Dinosaur Audio ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(FName Species, FVector Location, float Mass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerProximityRumble(FVector DinosaurLocation, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurVocalization(FName Species, FName VocalizationType, FVector Location);

    // ── Survival Feedback ────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void OnPlayerHealthChanged(float NewHealthPercent);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void OnPlayerHungerChanged(float NewHungerPercent);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void OnPlayerThirstChanged(float NewThirstPercent);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void OnPlayerDamaged(float DamageAmount, FVector DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void OnCraftingComplete(FName ItemCrafted);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void OnFireLit(FVector FireLocation);

    // ── Narration Voice Lines ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void PlayNarrationLine(FName LineID, FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void QueueNarrationLine(FName LineID, float DelaySeconds = 0.0f);

    // ── State Queries ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_BiomeLayer GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    float GetMusicIntensity() const { return MusicIntensity; }

protected:
    // ── Ambient Layers ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Ambient",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* PrimaryAmbientComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Ambient",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* SecondaryAmbientComponent;

    // ── Dinosaur Profiles ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    // ── Survival Feedback ────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    FAudio_SurvivalFeedback SurvivalSounds;

    // ── Narration Voice Lines (ElevenLabs URLs) ──────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narration")
    TMap<FName, FString> NarrationVoiceLineURLs;

    // ── Music Intensity ──────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Music",
              meta = (AllowPrivateAccess = "true"))
    float MusicIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicIntensityLerpSpeed = 1.5f;

    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
              meta = (AllowPrivateAccess = "true"))
    EAudio_BiomeLayer CurrentBiome = EAudio_BiomeLayer::OpenPlain;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
              meta = (AllowPrivateAccess = "true"))
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Calm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
              meta = (AllowPrivateAccess = "true"))
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

private:
    void UpdateMusicIntensity(float DeltaTime);
    void UpdateAmbientLayers();
    float GetTargetMusicIntensity() const;
    FAudio_DinosaurSoundProfile* FindDinosaurProfile(FName Species);

    float TimeSinceLastFootstepRumble = 0.0f;
    float HeartbeatTimer = 0.0f;
    bool bHeartbeatActive = false;
};
