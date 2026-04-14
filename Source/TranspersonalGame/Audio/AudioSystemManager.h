#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient UMETA(DisplayName = "Ambient"),
    Emotional UMETA(DisplayName = "Emotional"),
    Action UMETA(DisplayName = "Action"),
    Spiritual UMETA(DisplayName = "Spiritual")
};

UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    Forest UMETA(DisplayName = "Forest"),
    Cave UMETA(DisplayName = "Cave"),
    Plains UMETA(DisplayName = "Plains"),
    Sacred UMETA(DisplayName = "Sacred")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundscapeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_EnvironmentType EnvironmentType = EAudio_EnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MusicVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bEnableAdaptiveMusic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ConsciousnessInfluence = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_MusicLayer CurrentLayer = EAudio_MusicLayer::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float EmotionalState = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SpiritualResonance = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetSoundscapeConfig(const FAudio_SoundscapeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateMusicState(const FAudio_MusicState& NewState);

    // Adaptive music system
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionToMusicLayer(EAudio_MusicLayer NewLayer, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetEmotionalState(float EmotionalValue);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetSpiritualResonance(float ResonanceValue);

    // Environment audio
    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void SetEnvironmentType(EAudio_EnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void PlayEnvironmentSound(const FString& SoundName, FVector Location, float Volume = 1.0f);

    // Consciousness-based audio
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void TriggerConsciousnessShift(float ShiftIntensity);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void PlaySacredSound(const FString& SoundName, float SpiritualPower = 1.0f);

    // Volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Controls")
    void SetSFXVolume(float Volume);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Audio System")
    FAudio_SoundscapeConfig GetCurrentSoundscapeConfig() const { return CurrentSoundscapeConfig; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    FAudio_MusicState GetCurrentMusicState() const { return CurrentMusicState; }

    UFUNCTION(BlueprintPure, Category = "Audio System")
    EAudio_EnvironmentType GetCurrentEnvironment() const { return CurrentEnvironment; }

protected:
    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> SFXAudioComponent;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_SoundscapeConfig CurrentSoundscapeConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudio_MusicState CurrentMusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_EnvironmentType CurrentEnvironment = EAudio_EnvironmentType::Forest;

    // Audio assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TMap<EAudio_EnvironmentType, TObjectPtr<USoundCue>> EnvironmentSounds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TMap<EAudio_MusicLayer, TObjectPtr<UMetaSoundSource>> MusicLayers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TMap<FString, TObjectPtr<USoundCue>> SacredSounds;

private:
    // Internal methods
    void UpdateAmbientAudio();
    void UpdateMusicAudio();
    void ProcessConsciousnessInfluence();
    
    // Timers
    FTimerHandle MusicTransitionTimer;
    FTimerHandle ConsciousnessUpdateTimer;
};