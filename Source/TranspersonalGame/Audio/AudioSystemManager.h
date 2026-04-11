#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioSystemManager.generated.h"

/**
 * Audio System Manager - Core audio orchestration for Transpersonal Game
 * Handles adaptive music, spatial audio, consciousness-based audio transitions
 * Based on Walter Murch principles: "The sound that doesn't exist is often more powerful than the sound that does"
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // === CORE AUDIO MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    // === ADAPTIVE MUSIC SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TransitionToMusicState(const FString& NewState, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetConsciousnessLevel(float Level); // 0.0 = unconscious, 1.0 = fully awakened

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TriggerSpiritualMoment(const FString& MomentType);

    // === SPATIAL AUDIO ===
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlaySoundAtLocation(USoundBase* Sound, FVector Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void SetEnvironmentAudioProfile(const FString& ProfileName);

    // === CONSCIOUSNESS AUDIO EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void ApplyConsciousnessFilter(float Intensity); // Applies ethereal/spiritual audio processing

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void PlayNarrationVoice(const FString& NarrationKey, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void TriggerAncientWisdomWhisper(FVector Location);

protected:
    // === AUDIO STATE TRACKING ===
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FString CurrentMusicState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentConsciousnessLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float MasterVolumeLevel;

    // === AUDIO COMPONENTS ===
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> NarrationAudioComponent;

    // === AUDIO ASSETS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<FString, TObjectPtr<USoundBase>> MusicStates;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<FString, TObjectPtr<USoundBase>> EnvironmentProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TMap<FString, TObjectPtr<USoundBase>> NarrationVoices;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<USoundBase> SpiritualWhisperSound;

private:
    // === INTERNAL AUDIO PROCESSING ===
    void UpdateAdaptiveMusic(float DeltaTime);
    void ProcessConsciousnessAudio();
    void HandleAudioTransitions();

    // === AUDIO STATE VARIABLES ===
    float MusicTransitionTimer;
    float ConsciousnessFilterIntensity;
    bool bIsTransitioning;
    FString PendingMusicState;
    float PendingFadeTime;
};