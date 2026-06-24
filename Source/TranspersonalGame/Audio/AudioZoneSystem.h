#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioZoneSystem.generated.h"

// ============================================================
// Audio Zone System — Agent #16
// Wired to ANarr_DialogueTriggerActor (Agent #15)
// ENarr_DialogueTriggerType drives audio state machine
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Playing         UMETA(DisplayName = "Playing"),
    FadingOut       UMETA(DisplayName = "FadingOut"),
    Completed       UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class EAudio_NarrativeHook : uint8
{
    None            UMETA(DisplayName = "None"),
    ElderWarning    UMETA(DisplayName = "Elder Warning — Stampede Rumble"),
    StampedeApproach UMETA(DisplayName = "Stampede Approach — Low Rumble"),
    UrgencyMid      UMETA(DisplayName = "Urgency Mid — Heartbeat Drums"),
    UrgencyFinal    UMETA(DisplayName = "Urgency Final — Accelerating Drums"),
    Victory         UMETA(DisplayName = "Victory — Wind and Birds"),
    Failure         UMETA(DisplayName = "Failure — Crackling Fire")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_NarrativeHook HookType = EAudio_NarrativeHook::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInDuration = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bScaleWithUrgency = false;
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float TriggerRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float MaxShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeFrequency = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeDuration = 0.3f;
};

// ============================================================
// AAudio_NarrativeZone — proximity-triggered ambient audio
// wired to narrative dialogue trigger positions from Agent #15
// ============================================================
UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio Narrative Zone"))
class TRANSPERSONALGAME_API AAudio_NarrativeZone : public AActor
{
    GENERATED_BODY()

public:
    AAudio_NarrativeZone();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    // Audio component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone")
    EAudio_ZoneState CurrentState = EAudio_ZoneState::Inactive;

    // Urgency scalar [0..1] — driven by quest timer from Agent #14
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float UrgencyScalar = 0.0f;

    // Blueprint events — wire to ANarr_DialogueTriggerActor.OnDialogueStarted
    UFUNCTION(BlueprintImplementableEvent, Category = "Audio Zone")
    void OnNarrativeHookTriggered(EAudio_NarrativeHook HookType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Audio Zone")
    void OnUrgencyUpdated(float NewUrgency);

    // Called by quest manager when urgency changes
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetUrgencyScalar(float NewUrgency);

    // Manually trigger this zone's audio hook
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void TriggerAudioHook(EAudio_NarrativeHook Hook);

    // Fade out and stop
    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void StopAudioZone();

private:
    bool bPlayerInRange = false;
    float CurrentVolume = 0.0f;
    FTimerHandle FadeTimerHandle;

    bool IsPlayerInRange() const;
    void UpdateVolumeForUrgency();
};

// ============================================================
// AAudio_TRexShakeSource — screen shake + proximity rumble
// when T-Rex is within range of the player
// ============================================================
UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio TRex Shake Source"))
class TRANSPERSONALGAME_API AAudio_TRexShakeSource : public AActor
{
    GENERATED_BODY()

public:
    AAudio_TRexShakeSource();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    FAudio_ScreenShakeConfig ShakeConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Screen Shake")
    UAudioComponent* RumbleAudioComponent;

    // Blueprint event — fire camera shake from Blueprint
    UFUNCTION(BlueprintImplementableEvent, Category = "Screen Shake")
    void OnTRexShakeTriggered(float Intensity);

    // Query current distance to player (0..1 normalized, 0=far, 1=close)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Screen Shake")
    float GetProximityToPlayer() const;

private:
    float LastShakeTime = 0.0f;
    float ShakeCooldown = 0.25f;
};

// ============================================================
// AAudio_DayNightAmbience — ambient audio that transitions
// with the day/night cycle (driven by DirectionalLight pitch)
// ============================================================
UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio Day Night Ambience"))
class TRANSPERSONALGAME_API AAudio_DayNightAmbience : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DayNightAmbience();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Day ambience volume [0..1]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float DayAmbienceVolume = 1.0f;

    // Night ambience volume [0..1]
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Audio")
    float NightAmbienceVolume = 1.0f;

    // Current time of day [0..1] — 0=midnight, 0.5=noon, 1=midnight
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Audio")
    float TimeOfDay = 0.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Audio")
    UAudioComponent* DayAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Day Night Audio")
    UAudioComponent* NightAudioComponent;

    // Blueprint event — fires when crossing day/night threshold
    UFUNCTION(BlueprintImplementableEvent, Category = "Day Night Audio")
    void OnDayNightTransition(bool bIsDay);

    // Update time of day (called by lighting system)
    UFUNCTION(BlueprintCallable, Category = "Day Night Audio")
    void SetTimeOfDay(float NewTimeOfDay);

private:
    bool bIsCurrentlyDay = true;
    void BlendAmbienceVolumes();
};
