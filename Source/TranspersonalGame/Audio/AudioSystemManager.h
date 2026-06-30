// AudioSystemManager.h
// Audio Agent #16 — PROD_CYCLE_AUTO_20260630_007
// Adaptive audio system: danger-driven music, ambient zones, NPC dialogue voice binding
// Wires into QuestObjectiveSystem delegates from Agent #15

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — EAudio_ prefix (unique across project)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Day     UMETA(DisplayName = "Day"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_QuestStinger : uint8
{
    QuestStarted    UMETA(DisplayName = "Quest Started"),
    QuestCompleted  UMETA(DisplayName = "Quest Completed"),
    QuestFailed     UMETA(DisplayName = "Quest Failed"),
    ObjectiveDone   UMETA(DisplayName = "Objective Done")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs — FAudio_ prefix
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_DangerZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    FName ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float DangerRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Aware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FName LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtDawn  = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtDay   = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtDusk  = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtNight = false;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLineKey
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FName SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    int32 LineIndex = 0;

    bool operator==(const FAudio_VoiceLineKey& Other) const
    {
        return SpeakerName == Other.SpeakerName && LineIndex == Other.LineIndex;
    }
};

FORCEINLINE uint32 GetTypeHash(const FAudio_VoiceLineKey& Key)
{
    return HashCombine(GetTypeHash(Key.SpeakerName), GetTypeHash(Key.LineIndex));
}

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudio_OnDangerLevelChanged,
    EAudio_DangerLevel, OldLevel, EAudio_DangerLevel, NewLevel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnTimeOfDayChanged,
    EAudio_TimeOfDay, NewTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudio_OnDialogueLineStarted,
    FName, SpeakerName, int32, LineIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudio_OnDialogueLineFinished,
    FName, SpeakerName, int32, LineIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAudio_OnPlayerEnteredZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAudio_OnPlayerExitedZone);

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_AdaptiveMusicComponent
// Manages danger-driven music blending and time-of-day ambient layers
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Danger system ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void RegisterDangerZone(const FAudio_DangerZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void UnregisterDangerZone(FName ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void EvaluateDangerLevel();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StartAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopAdaptiveMusic();

    // ── Time of day ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|TimeOfDay")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    // ── State accessors ────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    float GetDangerBlendWeight() const { return CurrentDangerWeight; }

    // ── Delegates ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnDangerLevelChanged OnDangerLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnTimeOfDayChanged OnTimeOfDayChanged;

    // ── Config ─────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DangerUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbienceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DangerBlendSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

private:
    EAudio_DangerLevel CurrentDangerLevel;
    EAudio_TimeOfDay   CurrentTimeOfDay;

    bool  bMusicSystemActive;
    float TargetDangerWeight;
    float CurrentDangerWeight;

    FTimerHandle DangerUpdateTimer;

    UPROPERTY()
    TArray<FAudio_DangerZone> RegisteredDangerZones;
};

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_DialogueVoiceComponent
// Manages NPC voice line playback — binds to ANarr_DialogueTriggerActor delegates
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UAudio_DialogueVoiceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DialogueVoiceComponent();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void RegisterVoiceLine(FName SpeakerName, int32 LineIndex, USoundBase* AudioAsset);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayDialogueLine(FName SpeakerName, int32 LineIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintPure, Category = "Audio|Dialogue")
    bool IsPlayingDialogue() const { return bIsPlayingDialogue; }

    UFUNCTION(BlueprintPure, Category = "Audio|Dialogue")
    FName GetActiveSpeaker() const { return ActiveSpeakerName; }

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnDialogueLineStarted OnDialogueLineStarted;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnDialogueLineFinished OnDialogueLineFinished;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float DialogueVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float DialogueFadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float DialogueFadeOutTime;

private:
    UFUNCTION()
    void OnDialogueFinished();

    TMap<FAudio_VoiceLineKey, USoundBase*> VoiceLineRegistry;

    bool bIsPlayingDialogue;
    FName ActiveSpeakerName;
    int32 ActiveLineIndex;

    UPROPERTY()
    UAudioComponent* ActiveAudioComponent;
};

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_AmbientZoneComponent
// Spatial ambient audio — fades in/out as player enters/exits radius
// Used by NPC dialogue zones from Agent #15
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UAudio_AmbientZoneComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AmbientZoneComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float VolumeBlendSpeed;

    UFUNCTION(BlueprintPure, Category = "Audio|Ambient")
    bool IsPlayerInZone() const { return bPlayerInZone; }

    UFUNCTION(BlueprintPure, Category = "Audio|Ambient")
    float GetCurrentVolume() const { return CurrentVolume; }

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnPlayerEnteredZone OnPlayerEnteredZone;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnPlayerExitedZone OnPlayerExitedZone;

private:
    bool  bPlayerInZone;
    float CurrentVolume;
    float TargetVolume;

    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent;
};
