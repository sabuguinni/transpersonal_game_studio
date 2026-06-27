// AudioSystemManager.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260627_005
// Adaptive audio system: ambient loops, survival feedback, dinosaur proximity audio
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_AmbientZone : uint8
{
    Camp        UMETA(DisplayName = "Camp"),
    Forest      UMETA(DisplayName = "Forest"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Plains      UMETA(DisplayName = "Plains"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Combat      UMETA(DisplayName = "Combat")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_AmbientZone Zone = EAudio_AmbientZone::Camp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundAssetPath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_DinoProximityEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString DinoSpecies = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ProximityRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float GroundShakeIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString RoarSoundPath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString FootstepSoundPath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bTriggerScreenShake = true;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SpeakerName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString DialogueText = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bHasBeenPlayed = false;
};

// ─── Main Class ──────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── Ambient System ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_AmbientZone CurrentZone = EAudio_AmbientZone::Camp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    // ── Dinosaur Proximity ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    TArray<FAudio_DinoProximityEvent> DinoProximityEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    float NearestDinoDistance = 99999.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    bool bTRexNearby = false;

    // ── Voice Lines ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> RegisteredVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FAudio_VoiceLine ActiveVoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bVoiceLineActive = false;

    // ── Survival Audio Feedback ─────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    float HeartbeatIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    bool bPlayingDamageRing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Survival")
    float MasterVolume = 1.0f;

    // ── Functions ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetAmbientZone(EAudio_AmbientZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaurs")
    void OnDinoProximityUpdate(const FString& DinoSpecies, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine(const FAudio_VoiceLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void TriggerDamageAudioFeedback(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void UpdateHeartbeatFromHealth(float CurrentHealth, float MaxHealth);

    UFUNCTION(CallInEditor, Category = "Audio|Debug")
    void LogAudioSystemState();

    UFUNCTION(CallInEditor, Category = "Audio|Debug")
    void InitializeDefaultVoiceLines();

private:
    void UpdateAmbientBlend(float DeltaTime);
    void UpdateDinoProximityAudio(float DeltaTime);
    void UpdateSurvivalAudioFeedback(float DeltaTime);
    void InitializeDefaultAmbientLayers();
    void InitializeDefaultDinoEvents();

    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent = nullptr;

    float TimeSinceLastDinoCall = 0.0f;
    float DinoCallInterval = 30.0f;
    float CurrentHeartbeatRate = 60.0f;
};
