#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioFeedbackSystem.generated.h"

/**
 * EAudio_FeedbackType — types of audio feedback events in the survival game.
 * Placed at global scope per UE5 compilation rules.
 */
UENUM(BlueprintType)
enum class EAudio_FeedbackType : uint8
{
    None            UMETA(DisplayName = "None"),
    PlayerDamage    UMETA(DisplayName = "Player Damage"),
    PlayerDeath     UMETA(DisplayName = "Player Death"),
    DinoNearby      UMETA(DisplayName = "Dinosaur Nearby"),
    DinoAttack      UMETA(DisplayName = "Dinosaur Attack"),
    CraftingSuccess UMETA(DisplayName = "Crafting Success"),
    FireLit         UMETA(DisplayName = "Fire Lit"),
    LowHealth       UMETA(DisplayName = "Low Health Warning"),
    LowHunger       UMETA(DisplayName = "Low Hunger Warning"),
    StealthSuccess  UMETA(DisplayName = "Stealth Success"),
    TRexRoar        UMETA(DisplayName = "T-Rex Roar"),
    HerdStampede    UMETA(DisplayName = "Herd Stampede")
};

/**
 * FAudio_FeedbackEvent — data for a single audio feedback event.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FeedbackEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    EAudio_FeedbackType FeedbackType = EAudio_FeedbackType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    bool bIs3DSound = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Feedback")
    float TriggerRadius = 1500.0f;
};

/**
 * FAudio_ScreenShakeConfig — configuration for camera shake triggered by audio events.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    bool bFalloffWithDistance = true;
};

/**
 * AAudioFeedbackSystem — manages audio feedback events including screen shake,
 * damage audio, dinosaur proximity sounds, and environmental audio cues.
 * 
 * Placed in the world to handle all reactive audio for the survival game.
 * Works with the MetaSounds system and UE5 audio engine.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Feedback System"))
class TRANSPERSONALGAME_API AAudioFeedbackSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudioFeedbackSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ======= SCREEN SHAKE =======

    /** Trigger screen shake when T-Rex footstep detected nearby */
    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerTRexFootstepShake(float DistanceToTRex);

    /** Trigger screen shake for herd stampede */
    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerStampedeShake(float Intensity);

    /** Screen shake configuration for T-Rex proximity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig TRexShakeConfig;

    // ======= DAMAGE FEEDBACK =======

    /** Play damage audio feedback (hit sound + heartbeat if low health) */
    UFUNCTION(BlueprintCallable, Category = "Audio|Damage")
    void PlayDamageFeedback(float DamageAmount, float CurrentHealth, float MaxHealth);

    /** Play death audio sequence */
    UFUNCTION(BlueprintCallable, Category = "Audio|Damage")
    void PlayDeathAudio();

    // ======= ENVIRONMENTAL AUDIO =======

    /** Trigger a feedback event by type */
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerFeedbackEvent(FAudio_FeedbackEvent Event);

    /** Check if player is in T-Rex proximity zone and apply shake */
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void UpdateDinoProximityAudio(float DeltaTime);

    // ======= AUDIO ASSETS =======

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* DamageHitSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* LowHealthHeartbeatSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* TRexFootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* RaptorScreechSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* CampfireCrackleSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets")
    USoundBase* StampedeRumbleSound = nullptr;

    // ======= STATE =======

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    bool bIsLowHealthWarningActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    float LastShakeTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    float ShakeCooldown = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    float DinoProximityCheckInterval = 0.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State")
    float TimeSinceLastProximityCheck = 0.0f;

private:
    UPROPERTY()
    UAudioComponent* HeartbeatAudioComponent = nullptr;

    UPROPERTY()
    UAudioComponent* CampfireAudioComponent = nullptr;

    float GetShakeIntensityByDistance(float Distance, float MaxRadius) const;
    void ApplyCameraShake(float Intensity, float Duration);
};
