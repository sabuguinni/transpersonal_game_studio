
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Enums — Audio_* prefix to avoid collision with engine types
// ============================================================

UENUM(BlueprintType)
enum class EAudio_BiomeZone : uint8
{
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    RiverBank       UMETA(DisplayName = "River Bank"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    VolcanicField   UMETA(DisplayName = "Volcanic Field"),
    NightZone       UMETA(DisplayName = "Night Zone")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_SurvivalEvent : uint8
{
    PlayerHurt          UMETA(DisplayName = "Player Hurt"),
    PlayerLowHealth     UMETA(DisplayName = "Player Low Health"),
    PlayerLowHunger     UMETA(DisplayName = "Player Low Hunger"),
    PlayerLowThirst     UMETA(DisplayName = "Player Low Thirst"),
    PlayerLowStamina    UMETA(DisplayName = "Player Low Stamina"),
    FireLit             UMETA(DisplayName = "Fire Lit"),
    CraftingComplete    UMETA(DisplayName = "Crafting Complete"),
    DinosaurNearby      UMETA(DisplayName = "Dinosaur Nearby"),
    DinosaurCharge      UMETA(DisplayName = "Dinosaur Charge"),
    PlayerDeath         UMETA(DisplayName = "Player Death")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeZone BiomeZone = EAudio_BiomeZone::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> DayAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> NightAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float CrossfadeDuration = 3.0f;
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
    TSoftObjectPtr<USoundBase> ChargeSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepGroundShakeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepGroundShakeIntensity = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    TSoftObjectPtr<USoundBase> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float Volume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float TransitionDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    bool bIsPercussive = false;
};

// ============================================================
// Main Audio System Manager Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Ambient Audio ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiomeZone(EAudio_BiomeZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDayNightBlend(float DayBlend);

    UFUNCTION(BlueprintPure, Category = "Audio|Ambient")
    EAudio_BiomeZone GetCurrentBiomeZone() const { return CurrentBiomeZone; }

    // ---- Threat & Music ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintPure, Category = "Audio|Music")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void TransitionToMusicState(EAudio_ThreatLevel TargetThreat, float OverrideDuration = -1.0f);

    // ---- Survival Sound Events ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlaySurvivalEvent(EAudio_SurvivalEvent Event);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayDinosaurFootstep(FName DinosaurSpecies, FVector FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void TriggerScreenShakeFromDinosaur(FVector DinosaurLocation, float DinosaurMass);

    // ---- Campfire Audio ----

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void RegisterCampfire(AActor* CampfireActor, float Radius = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void UnregisterCampfire(AActor* CampfireActor);

    // ---- Freesound Asset References ----
    // Campfire: Freesound ID 394952 — crackling campfire near lake (1805s)
    // Campfire: Freesound ID 157187 — Campfire Crackle 3 (2s loop)
    // Campfire: Freesound ID 729396 — Campfire 02 (267s)

    // ---- Voice Lines (ElevenLabs TTS) ----
    // Narrator_HerdSilence: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782981691767_Narrator_HerdSilence.mp3
    // ElderHunter_NightWarning: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782981709881_ElderHunter_NightWarning.mp3

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DinosaurSoundProfile> DinosaurSoundProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_MusicState> MusicStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float VoiceVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float ThreatTransitionCooldown = 5.0f;

private:
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

    EAudio_BiomeZone CurrentBiomeZone = EAudio_BiomeZone::OpenPlains;
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    float CurrentDayBlend = 1.0f;
    float LastThreatTransitionTime = 0.0f;

    TMap<AActor*, UAudioComponent*> CampfireAudioComponents;

    void UpdateAmbientAudio();
    void UpdateMusicLayer();
    const FAudio_DinosaurSoundProfile* FindDinosaurProfile(FName Species) const;
};
