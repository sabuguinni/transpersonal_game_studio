#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "AudioDevice.h"
#include "AudioMixerBlueprintLibrary.h"
#include "MetasoundSource.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudioLayer : uint8
{
    Ambience,
    Music,
    SFX,
    Voice,
    UI
};

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Awakening,
    Grounded,
    Elevated,
    Transcendent,
    Unity
};

USTRUCT(BlueprintType)
struct FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<USoundCue> CurrentSound = nullptr;
};

USTRUCT(BlueprintType)
struct FConsciousnessAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMetaSoundSource> AmbienceMetaSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UMetaSoundSource> MusicMetaSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReverbWetness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LowPassFilter = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpatialBlend = 0.5f;
};

/**
 * Audio System Manager - Handles adaptive audio, consciousness-based mixing, and MetaSounds integration
 * Implements Walter Murch's philosophy: "The sound that doesn't exist is often more powerful than the sound that does"
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetAudioLayerVolume(EAudioLayer Layer, float Volume, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySoundOnLayer(EAudioLayer Layer, USoundCue* Sound, bool bLoop = false);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSoundOnLayer(EAudioLayer Layer, float FadeTime = 2.0f);

    // Consciousness-Based Audio
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void SetConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void TransitionToConsciousnessState(EConsciousnessState NewState, float TransitionTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    EConsciousnessState GetCurrentConsciousnessState() const { return CurrentConsciousnessState; }

    // MetaSounds Integration
    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void SetMetaSoundParameter(EAudioLayer Layer, FName ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void TriggerMetaSoundEvent(EAudioLayer Layer, FName EventName);

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateListenerPosition(FVector Position, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void RegisterSpatialAudioSource(AActor* SourceActor, USoundCue* Sound, float MaxDistance = 1000.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void SetEnvironmentalReverb(float ReverbAmount, float DecayTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void UpdateWeatherAudio(float WindIntensity, float RainIntensity, float ThunderProbability);

protected:
    // Audio Layer Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layers")
    TMap<EAudioLayer, FAudioLayerConfig> AudioLayers;

    // Consciousness Audio Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    TMap<EConsciousnessState, FConsciousnessAudioProfile> ConsciousnessProfiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness Audio")
    EConsciousnessState CurrentConsciousnessState = EConsciousnessState::Awakening;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness Audio")
    EConsciousnessState TargetConsciousnessState = EConsciousnessState::Awakening;

    // Transition Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transitions")
    bool bIsTransitioning = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transitions")
    float TransitionProgress = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transitions")
    float TransitionDuration = 5.0f;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TMap<EAudioLayer, TObjectPtr<UAudioComponent>> AudioComponents;

    // MetaSounds References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    TMap<EAudioLayer, TObjectPtr<UMetaSoundSource>> ActiveMetaSounds;

private:
    void UpdateConsciousnessTransition(float DeltaTime);
    void ApplyConsciousnessAudioProfile(const FConsciousnessAudioProfile& Profile, float BlendWeight = 1.0f);
    void InitializeAudioComponents();
    void SetupDefaultConsciousnessProfiles();

    FAudioDevice* GetAudioDevice() const;
    void UpdateAudioMixerSettings();
};