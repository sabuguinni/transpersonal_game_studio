// AudioFeedbackSystem.h
// Agent #16 — Audio Agent — PROD_CYCLE_AUTO_20260630_009
// Game-feel audio feedback system: screen shake, damage flash, ambient soundscape, day/night audio phases

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "AudioFeedbackSystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — Audio_* prefix to avoid collision with other agents
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_FeedbackEvent : uint8
{
    PlayerDamaged       UMETA(DisplayName = "Player Damaged"),
    TRexNearby          UMETA(DisplayName = "T-Rex Nearby"),
    RaptorAttack        UMETA(DisplayName = "Raptor Attack"),
    PlayerDeath         UMETA(DisplayName = "Player Death"),
    CampfireLit         UMETA(DisplayName = "Campfire Lit"),
    DinoKilled          UMETA(DisplayName = "Dinosaur Killed")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDayPhase : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Day     UMETA(DisplayName = "Day"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float MaxDistanceForFullShake = 500.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamageFlashStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamageFlashEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFeedbackEventTriggered, EAudio_FeedbackEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayPhaseChanged, EAudio_TimeOfDayPhase, NewPhase);

// ─────────────────────────────────────────────────────────────────────────────
// UAudioFeedbackSystem — ActorComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), DisplayName = "Audio Feedback System")
class TRANSPERSONALGAME_API UAudioFeedbackSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioFeedbackSystem();

    // ── Delegates ──
    UPROPERTY(BlueprintAssignable, Category = "Audio|Feedback")
    FOnDamageFlashStart OnDamageFlashStart;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Feedback")
    FOnDamageFlashEnd OnDamageFlashEnd;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Feedback")
    FOnFeedbackEventTriggered OnFeedbackEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Audio|TimeOfDay")
    FOnTimeOfDayPhaseChanged OnTimeOfDayPhaseChanged;

    // ── Shake Configs ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig TRexShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig RaptorShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig PlayerDamageShakeConfig;

    // ── Camera Shake Class ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    // ── Ambient Sound Assets ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* CampfireAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* DawnAmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* DayAmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* DuskAmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* NightAmbienceSound;

    // ── Proximity Radii ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float TRexProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float RaptorProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Proximity")
    float CampfireAmbientRadius;

    // ── Damage Flash ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|DamageFlash")
    float DamageFlashDuration;

    // ── Public API ──
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerFeedbackEvent(EAudio_FeedbackEvent Event, float DistanceToSource = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageFlash();

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerCameraShake(const FAudio_ScreenShakeConfig& Config, float DistanceToSource);

    UFUNCTION(BlueprintCallable, Category = "Audio|TimeOfDay")
    void SetTimeOfDayPhase(EAudio_TimeOfDayPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayCampfireAmbience();

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopCampfireAmbience();

    UFUNCTION(BlueprintPure, Category = "Audio|Feedback")
    float GetDamageFlashIntensity() const;

    UFUNCTION(BlueprintPure, Category = "Audio|Feedback")
    bool IsDamageFlashActive() const;

    UFUNCTION(BlueprintPure, Category = "Audio|TimeOfDay")
    EAudio_TimeOfDayPhase GetCurrentTimeOfDayPhase() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    UAudioComponent* AmbientAudioComponent;

    EAudio_TimeOfDayPhase CurrentTimeOfDay;

    bool bDamageFlashActive;
    float DamageFlashElapsed;

    void unreal_log_warning_stub(const FString& Msg);
    void unreal_log_info_stub(const FString& Msg);
};
