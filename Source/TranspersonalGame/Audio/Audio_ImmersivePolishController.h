#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TriggerSphere.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "Camera/CameraShakeBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "Audio_ImmersivePolishController.generated.h"

UENUM(BlueprintType)
enum class EAudio_FeedbackType : uint8
{
    ScreenShake     UMETA(DisplayName = "Screen Shake"),
    DamageFlash     UMETA(DisplayName = "Damage Flash"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    Environmental  UMETA(DisplayName = "Environmental")
};

USTRUCT(BlueprintType)
struct FAudio_FeedbackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    EAudio_FeedbackType FeedbackType = EAudio_FeedbackType::Environmental;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float AudioVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Feedback")
    bool bLoopAudio = false;

    FAudio_FeedbackConfig()
    {
        FeedbackType = EAudio_FeedbackType::Environmental;
        TriggerRadius = 500.0f;
        AudioVolume = 1.0f;
        ShakeIntensity = 1.0f;
        bLoopAudio = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ImmersivePolishController : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ImmersivePolishController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* SecondaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* FootstepParticleComponent;

    // Audio feedback configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Polish")
    FAudio_FeedbackConfig FeedbackConfig;

    // Sound assets for different feedback types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* TRexProximitySound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* DamageFlashSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* FootstepDustSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    USoundCue* EnvironmentalAmbientSound;

    // Screen shake assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    TSubclassOf<UCameraShakeBase> TRexProximityShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    TSubclassOf<UCameraShakeBase> DamageFlashShake;

    // Immersive feedback state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsPlayerInProximity = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentShakeIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float EnvironmentalAudioLevel = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    // Core immersive feedback functions
    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerScreenShakeFeedback(EAudio_FeedbackType FeedbackType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerDamageFlashFeedback(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerFootstepDustFeedback(FVector FootstepLocation, bool bIsHeavyCreature = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void UpdateEnvironmentalAudio(float TimeOfDay, float WeatherIntensity);

    // Proximity detection and feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void OnPlayerEnterProximity(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void OnPlayerExitProximity(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void OnTRexProximityDetected(float Distance);

    // Day/night cycle audio management
    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TransitionToDayAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TransitionToNightAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void UpdateDayNightCycle(float TimeOfDay);

    // Environmental audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerVolcanicAudioFeedback(float VolcanicIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerForestAudioFeedback(float ForestDensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void TriggerRiverAudioFeedback(float RiverFlow);

    // Audio system integration
    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void InitializeImmersiveAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Audio Polish")
    void UpdateAudioPolishSystems(float DeltaTime);

private:
    // Internal feedback management
    void ProcessScreenShake(float Intensity, TSubclassOf<UCameraShakeBase> ShakeClass);
    void ProcessAudioFeedback(USoundCue* SoundToPlay, float Volume);
    void ProcessParticleFeedback(FVector Location, bool bIsHeavy);
    
    // Audio state tracking
    float LastShakeTime = 0.0f;
    float LastDamageTime = 0.0f;
    float CurrentDayNightPhase = 0.5f;
    
    // Performance optimization
    float AudioUpdateInterval = 0.1f;
    float LastAudioUpdate = 0.0f;
};