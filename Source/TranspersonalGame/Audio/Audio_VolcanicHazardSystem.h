#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "AudioTypes.h"
#include "Audio_VolcanicHazardSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_VolcanicThreatLevel : uint8
{
    Safe = 0        UMETA(DisplayName = "Safe"),
    Warning = 1     UMETA(DisplayName = "Warning"),
    Danger = 2      UMETA(DisplayName = "Danger"),
    Critical = 3    UMETA(DisplayName = "Critical"),
    Eruption = 4    UMETA(DisplayName = "Eruption")
};

USTRUCT(BlueprintType)
struct FAudio_VolcanicAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    USoundCue* RumbleSoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    USoundCue* EruptionSoundCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    USoundCue* WarningAlertCue = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    float MaxDistance = 5000.0f;

    FAudio_VolcanicAudioData()
    {
        BaseVolume = 1.0f;
        MaxDistance = 5000.0f;
    }
};

/**
 * Volcanic Hazard Audio System
 * Manages environmental audio for volcanic activity including:
 * - Seismic rumble progression
 * - Emergency alert systems
 * - Environmental audio transitions
 * - Player proximity warnings
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_VolcanicHazardSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_VolcanicHazardSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === VOLCANIC AUDIO MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void SetVolcanicThreatLevel(EAudio_VolcanicThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void TriggerEmergencyAlert(const FString& AlertMessage, float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void StartSeismicRumble(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void StopSeismicRumble();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void TriggerVolcanicEruption();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void UpdatePlayerProximityToVolcano(float DistanceToVolcano);

    // === AUDIO CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio Config")
    FAudio_VolcanicAudioData VolcanicAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio Config")
    float SeismicUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio Config")
    float MaxVolcanicDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio Config")
    TSubclassOf<UCameraShakeBase> VolcanicShakeClass;

    // === GETTERS ===

    UFUNCTION(BlueprintPure, Category = "Volcanic Audio")
    EAudio_VolcanicThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Volcanic Audio")
    bool IsSeismicActive() const { return bSeismicActive; }

    UFUNCTION(BlueprintPure, Category = "Volcanic Audio")
    float GetCurrentSeismicIntensity() const { return CurrentSeismicIntensity; }

protected:
    // === INTERNAL STATE ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic State")
    EAudio_VolcanicThreatLevel CurrentThreatLevel = EAudio_VolcanicThreatLevel::Safe;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic State")
    bool bSeismicActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic State")
    float CurrentSeismicIntensity = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic State")
    float PlayerDistanceToVolcano = 10000.0f;

    // === AUDIO COMPONENTS ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* RumbleAudioComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AlertAudioComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* EruptionAudioComponent = nullptr;

    // === TIMERS ===

    FTimerHandle SeismicUpdateTimer;
    FTimerHandle AlertTimer;

    // === INTERNAL METHODS ===

    void UpdateSeismicAudio();
    void ProcessThreatLevelChange();
    void PlayEmergencyAlert();
    void StopEmergencyAlert();
    void CalculateVolcanicAudioIntensity();
    void ApplyVolcanicCameraShake(float Intensity);

    // === AUDIO HELPERS ===

    void InitializeAudioComponents();
    void UpdateRumbleVolume(float VolumeMultiplier);
    void UpdateRumblePitch(float PitchMultiplier);
    void FadeInRumble(float Duration = 2.0f);
    void FadeOutRumble(float Duration = 2.0f);
};