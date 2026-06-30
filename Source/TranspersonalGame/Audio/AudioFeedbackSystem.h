
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "AudioFeedbackSystem.generated.h"

// ─────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_FeedbackEvent : uint8
{
    None            UMETA(DisplayName = "None"),
    PlayerDamaged   UMETA(DisplayName = "Player Damaged"),
    TRexNearby      UMETA(DisplayName = "T-Rex Nearby"),
    RaptorAttack    UMETA(DisplayName = "Raptor Attack"),
    PlayerDeath     UMETA(DisplayName = "Player Death"),
    CampfireLit     UMETA(DisplayName = "Campfire Lit"),
    ItemCrafted     UMETA(DisplayName = "Item Crafted"),
    DinoKilled      UMETA(DisplayName = "Dino Killed")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDayPhase : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Day     UMETA(DisplayName = "Day"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

// ─────────────────────────────────────────────
//  Structs
// ─────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float ShakeScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float ShakeDuration = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float ShakeRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    bool bFalloffByDistance = true;
};

USTRUCT(BlueprintType)
struct FAudio_DamageFlashConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    FLinearColor FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.6f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float FlashDuration = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float FadeOutTime = 0.5f;
};

USTRUCT(BlueprintType)
struct FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float DustParticleScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float FootstepVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    bool bSpawnDustOnSoftGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    bool bSpawnDustOnHardGround = false;
};

// ─────────────────────────────────────────────
//  AudioFeedbackSystem Component
// ─────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent), DisplayName = "Audio Feedback System")
class TRANSPERSONALGAME_API UAudioFeedbackSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioFeedbackSystem();

    // ── Screen Shake ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerTRexShake(float DistanceToTRex);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageShake(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerCustomShake(FAudio_ScreenShakeConfig Config);

    // ── Damage Flash ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageFlash(float DamageNormalized);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDeathFlash();

    // ── Footstep Dust ─────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void SpawnFootstepDust(FVector FootLocation, bool bIsHeavyCreature);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void SpawnTRexFootstepDust(FVector FootLocation, float TRexMassKg);

    // ── Day/Night Audio Transition ────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void OnTimeOfDayChanged(EAudio_TimeOfDayPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void UpdateAmbientMix(float DayNightAlpha);

    // ── Feedback Event Dispatcher ─────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void DispatchFeedbackEvent(EAudio_FeedbackEvent Event, float Intensity = 1.0f);

    // ── Config ────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_ScreenShakeConfig TRexShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_DamageFlashConfig DamageFlashConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_FootstepConfig FootstepConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float TRexShakeTriggerDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float NightAmbientVolumeMultiplier = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DayAmbientVolumeMultiplier = 0.8f;

    // ── State ─────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    EAudio_TimeOfDayPhase CurrentTimePhase;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    bool bIsInDanger;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State")
    float CurrentDangerIntensity;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float DamageFlashAlpha;
    float DamageFlashTimer;
    bool bFlashActive;

    void UpdateDamageFlash(float DeltaTime);
};
