#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    Canyon          UMETA(DisplayName = "Canyon"),
    Plains          UMETA(DisplayName = "Plains"),
    Cave            UMETA(DisplayName = "Cave"),
    River           UMETA(DisplayName = "River"),
    Mountain        UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Tense           UMETA(DisplayName = "Tense"),
    Danger          UMETA(DisplayName = "Danger"),
    Combat          UMETA(DisplayName = "Combat"),
    Escape          UMETA(DisplayName = "Escape")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EAudio_EnvironmentType EnvironmentType = EAudio_EnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TSoftObjectPtr<USoundCue> AmbientSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TSoftObjectPtr<UMetaSoundSource> MetaSoundSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float FadeOutTime = 1.5f;

    FAudio_EnvironmentConfig()
    {
        EnvironmentType = EAudio_EnvironmentType::Forest;
        BaseVolume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 1.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    TSoftObjectPtr<USoundCue> FootstepSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    float PitchVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    float MinTimeBetweenSteps = 0.3f;

    FAudio_FootstepConfig()
    {
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.2f;
        MinTimeBetweenSteps = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* EnvironmentAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* EffectsAudioComponent;

    // Environment Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TMap<EAudio_EnvironmentType, FAudio_EnvironmentConfig> EnvironmentConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EAudio_EnvironmentType CurrentEnvironment = EAudio_EnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EAudio_IntensityLevel CurrentIntensity = EAudio_IntensityLevel::Calm;

    // Footstep Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    TMap<ESurfaceType, FAudio_FootstepConfig> FootstepConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep Audio")
    float FootstepDetectionRadius = 100.0f;

    // Narrative Audio Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TSoftObjectPtr<USoundCue> DialogueSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float DialogueVolume = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bAutoPlayNarrativeAudio = true;

    // Dynamic Music System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    TMap<EAudio_IntensityLevel, TSoftObjectPtr<UMetaSoundSource>> IntensityMusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    float MusicTransitionTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Music")
    float MusicVolume = 0.6f;

public:
    // Environment Audio Methods
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EAudio_EnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetIntensityLevel(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentAudio(EAudio_EnvironmentType Environment, bool bFadeIn = true);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopEnvironmentAudio(bool bFadeOut = true);

    // Footstep Audio Methods
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepAudio(ESurfaceType SurfaceType, FVector Location, float VolumeScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterFootstepListener(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UnregisterFootstepListener(AActor* Actor);

    // Narrative Audio Methods
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayNarrativeAudio(USoundCue* NarrativeCue, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopNarrativeAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool IsNarrativeAudioPlaying() const;

    // Dynamic Music Methods
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToIntensityMusic(EAudio_IntensityLevel TargetIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEffectsVolume(float Volume);

    // Audio Event Methods
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurRoar(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayTRexFootsteps(FVector Location, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayTribalDrums(float Intensity = 0.5f);

protected:
    // Internal Audio Management
    void InitializeAudioComponents();
    void LoadEnvironmentConfigs();
    void LoadFootstepConfigs();
    void LoadMusicTracks();
    void UpdateAudioBasedOnGameState();

    // Audio Transition Helpers
    void FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime);
    void CrossfadeAudioComponents(UAudioComponent* FromComponent, UAudioComponent* ToComponent, float CrossfadeTime);

    // Footstep Detection
    TArray<TWeakObjectPtr<AActor>> FootstepListeners;
    float LastFootstepTime = 0.0f;

    // Audio State Tracking
    bool bEnvironmentAudioPlaying = false;
    bool bMusicPlaying = false;
    bool bNarrativeAudioPlaying = false;
    
    // Fade Timers
    FTimerHandle EnvironmentFadeTimer;
    FTimerHandle MusicFadeTimer;
    FTimerHandle EffectsFadeTimer;
};