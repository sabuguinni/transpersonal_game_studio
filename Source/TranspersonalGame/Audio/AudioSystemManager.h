#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// Audio Agent #16 — AudioSystemManager
// Adaptive audio system for prehistoric survival game
// Manages ambient zones, dialogue audio, quest stingers,
// proximity-based dino audio, and day/night audio transitions
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Camp        UMETA(DisplayName = "Camp Zone"),
    Jungle      UMETA(DisplayName = "Jungle Zone"),
    TRexTerritory UMETA(DisplayName = "T-Rex Territory"),
    RaptorZone  UMETA(DisplayName = "Raptor Zone"),
    River       UMETA(DisplayName = "River Zone"),
    OpenPlain   UMETA(DisplayName = "Open Plain"),
    Cave        UMETA(DisplayName = "Cave Zone")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString AmbientSoundPath = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float FadeOutDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float ThreatMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString SpeakerID = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString LineText = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioURL = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float SubtitleDisplayTime = 6.0f;
};

USTRUCT(BlueprintType)
struct FAudio_ProximityTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    FString DinoSpecies = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float TriggerRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float GroundShakeIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    bool bPlayWarningVoice = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    FString WarningVoiceURL = TEXT("");
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── Zone Management ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void EnterAudioZone(EAudio_ZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ExitAudioZone(EAudio_ZoneType Zone);

    UFUNCTION(BlueprintPure, Category = "Audio|Zone")
    EAudio_ZoneType GetCurrentZone() const { return CurrentZone; }

    // ── Threat & Adaptive Music ──────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintPure, Category = "Audio|Threat")
    EAudio_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void OnDinoProximityEnter(const FString& DinoSpecies, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void OnDinoProximityExit(const FString& DinoSpecies);

    // ── Dialogue Audio ───────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayDialogueLine(const FAudio_DialogueLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintPure, Category = "Audio|Dialogue")
    bool IsDialoguePlaying() const { return bDialoguePlaying; }

    // ── Quest Audio ──────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Quest")
    void PlayQuestCompleteStinger();

    UFUNCTION(BlueprintCallable, Category = "Audio|Quest")
    void PlayQuestFailStinger();

    UFUNCTION(BlueprintCallable, Category = "Audio|Quest")
    void PlayObjectiveCompleteSound();

    // ── Day/Night Audio Transition ───────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintPure, Category = "Audio|DayNight")
    EAudio_TimeOfDay GetTimeOfDay() const { return CurrentTimeOfDay; }

    // ── Screen Shake (Audio-driven) ──────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerGroundShake(float Intensity, float Duration);

    // ── Registered Audio Lines ───────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DialogueLine> RegisteredVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_ProximityTrigger> ProximityTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_ZoneConfig> ZoneConfigs;

    // ── Audio URLs (from ElevenLabs TTS) ────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceURLs")
    FString URL_TRexProximityWarning = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782489913495_Survivor_TRexProximity.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceURLs")
    FString URL_FireWisdom = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782489922214_TribeElder_FireWisdom.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceURLs")
    FString URL_RaptorTactics = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/TribeElder_Combat.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceURLs")
    FString URL_ScoutDiscovery = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/Scout_Discovery.mp3");

    // ── Freesound References ─────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    FString Freesound_Campfire1 = TEXT("https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freesound")
    FString Freesound_Campfire2 = TEXT("https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3");

private:
    EAudio_ZoneType CurrentZone = EAudio_ZoneType::Jungle;
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    bool bDialoguePlaying = false;
    float DialogueTimer = 0.0f;
    float DialogueDuration = 0.0f;

    float ThreatDecayTimer = 0.0f;
    static constexpr float ThreatDecayDelay = 10.0f;

    TMap<FString, float> ActiveDinoProximities;

    void LoadDefaultVoiceLines();
    void LoadDefaultProximityTriggers();
    void LoadDefaultZoneConfigs();
    void UpdateThreatDecay(float DeltaTime);
    void UpdateDialogueTimer(float DeltaTime);
};
