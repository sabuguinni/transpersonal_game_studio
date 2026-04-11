#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Sound/SoundClass.h"
#include "Components/AudioComponent.h"
#include "AudioDevice.h"
#include "AudioSystemManager.generated.h"

/**
 * Central audio system manager for the Transpersonal Game
 * Handles adaptive music, consciousness-based audio states, and spatial audio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Core audio state management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(float DeltaTime);

    // Consciousness-based audio
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void SetConsciousnessLevel(float Level);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void TriggerTranscendenceAudio();

    // Adaptive music system
    UFUNCTION(BlueprintCallable, Category = "Music")
    void TransitionToMusicState(const FString& StateName, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Music")
    void SetMusicIntensity(float Intensity);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalAudio(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetBiomeAudio(const FString& BiomeName);

    // Spatial audio management
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void RegisterSpatialAudioSource(AActor* SourceActor, USoundBase* Sound);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void UpdateSpatialAudioAttenuation();

protected:
    // Audio state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentConsciousnessLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentMusicIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FString CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FString CurrentMusicState;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceComponent;

    // Audio assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<FString, USoundBase*> MusicStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<FString, USoundBase*> BiomeAmbiences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TMap<FString, USoundBase*> ConsciousnessAudioStates;

private:
    // Internal audio processing
    void ProcessAudioFades(float DeltaTime);
    void UpdateConsciousnessAudioFilters();
    void CalculateSpatialAudioParameters();

    // Fade management
    struct FAudioFade
    {
        UAudioComponent* Component;
        float StartVolume;
        float TargetVolume;
        float FadeTime;
        float ElapsedTime;
    };

    TArray<FAudioFade> ActiveFades;
    UWorld* WorldContext;
};