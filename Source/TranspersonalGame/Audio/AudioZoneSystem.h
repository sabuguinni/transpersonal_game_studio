// AudioZoneSystem.h
// Agent #16 — Audio Agent
// Proximity-based adaptive audio zone system for TranspersonalGame
// Triggers ambient sounds, danger cues, and environmental audio based on player position
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "AudioZoneSystem.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    None            UMETA(DisplayName = "None"),
    TRexProximity   UMETA(DisplayName = "T-Rex Proximity"),
    RaptorPatrol    UMETA(DisplayName = "Raptor Patrol"),
    NightAmbience   UMETA(DisplayName = "Night Ambience"),
    RiverAmbience   UMETA(DisplayName = "River Ambience"),
    WindPlains      UMETA(DisplayName = "Wind Plains"),
    CaveInterior    UMETA(DisplayName = "Cave Interior"),
    CampSafety      UMETA(DisplayName = "Camp Safety"),
    StormApproach   UMETA(DisplayName = "Storm Approaching")
};

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical — Predator Imminent")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundBase> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundBase> DangerStinger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float TriggerRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bIs3D = true;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    TSoftObjectPtr<USoundBase> VoiceAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    EAudio_DangerLevel TriggerDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float CooldownSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bPlayedThisSession = false;
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeFrequency = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float TriggerDistanceMeters = 30.0f;
};

// ─── Component ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Audio Zone System")
class TRANSPERSONALGAME_API UAudio_ZoneSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ZoneSystem();

    // ── Configuration ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    TArray<FAudio_ZoneLayer> AudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    float ZoneCheckIntervalSeconds = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zones")
    bool bEnableAdaptiveMusic = true;

    // ── Screen Shake (T-Rex footsteps) ───────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeConfig TRexShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeConfig BrachiosaurusShakeConfig;

    // ── Voice Lines ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Lines")
    TArray<FAudio_VoiceLine> NPCVoiceLines;

    // ── Runtime State ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    EAudio_DangerLevel CurrentDangerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    EAudio_ZoneType ActiveZoneType;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    float DistanceToNearestPredator;

    // ── Public API ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void TriggerDangerStinger(EAudio_ZoneType ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void PlayVoiceLine(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void TriggerScreenShake(float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetActiveZoneType() const { return ActiveZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void UpdateNearestPredatorDistance(float Distance);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float ZoneCheckTimer = 0.0f;
    float LastVoiceLineTimes_TRex = -999.0f;
    float LastVoiceLineTimes_Raptor = -999.0f;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    void EvaluateZones();
    void TransitionToZone(EAudio_ZoneType NewZone);
    void FadeOutAllAmbient(float FadeTime);
    EAudio_DangerLevel CalculateDangerFromDistance(float Distance) const;
};
