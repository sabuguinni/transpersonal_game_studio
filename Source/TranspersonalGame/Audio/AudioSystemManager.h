#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EnvironmentState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert — Predator Nearby"),
    Combat      UMETA(DisplayName = "Combat"),
    Night       UMETA(DisplayName = "Night"),
    Rain        UMETA(DisplayName = "Rain"),
    Fire        UMETA(DisplayName = "Fire — Camp Active")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundCue* SoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_EnvironmentState ActiveInState = EAudio_EnvironmentState::Calm;
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float CooldownSeconds = 120.0f;
};

/**
 * AAudio_SystemManager
 * Manages adaptive ambient audio, voice lines, and environmental sound states
 * for the prehistoric survival game. Integrates with MetaSounds via UE5.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === ENVIRONMENT STATE ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SetEnvironmentState(EAudio_EnvironmentState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    EAudio_EnvironmentState GetCurrentEnvironmentState() const { return CurrentEnvironmentState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void TransitionToState(EAudio_EnvironmentState NewState, float TransitionTime = 2.0f);

    // === AMBIENT LAYERS ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void RegisterAmbientLayer(FAudio_AmbientLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateAmbientMix();

    // === VOICE LINES ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(FAudio_VoiceLine VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void TriggerVoiceLine(const FString& CharacterName, const FString& Condition);

    // === DINOSAUR PROXIMITY ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void NotifyDinosaurProximity(float DistanceMeters, FName DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void NotifyDinosaurLost();

    // === SURVIVAL FEEDBACK ===

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayHeartbeatTension(float IntensityNormalized);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayDamageImpact(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayCraftingSound(FName ToolType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Survival")
    void PlayFootstep(FName SurfaceType);

    // === DAY/NIGHT AUDIO ===

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void SetTimeOfDay(float NormalizedTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    float GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
        meta = (AllowPrivateAccess = "true"))
    EAudio_EnvironmentState CurrentEnvironmentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> RegisteredAmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> RegisteredVoiceLines;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|DayNight",
        meta = (AllowPrivateAccess = "true"))
    float CurrentTimeOfDay = 0.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Dinosaur",
        meta = (AllowPrivateAccess = "true"))
    float NearestDinosaurDistance = 9999.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Dinosaur",
        meta = (AllowPrivateAccess = "true"))
    FName NearestDinosaurSpecies = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Survival",
        meta = (AllowPrivateAccess = "true"))
    float HeartbeatIntensity = 0.0f;

    // === VOICE LINE CATALOG (URLs from ElevenLabs TTS) ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|VoiceCatalog")
    TMap<FString, FString> VoiceLineURLCatalog;

private:
    void UpdateDinosaurAudioState(float DeltaTime);
    void UpdateDayNightAudio();
    void UpdateHeartbeat(float DeltaTime);

    float StateTransitionTimer = 0.0f;
    float StateTransitionDuration = 2.0f;
    EAudio_EnvironmentState TargetEnvironmentState;
    bool bIsTransitioning = false;
};
