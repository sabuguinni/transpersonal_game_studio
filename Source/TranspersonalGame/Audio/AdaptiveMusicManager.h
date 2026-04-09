// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AdaptiveMusicManager.generated.h"

/**
 * Adaptive Music Manager
 * 
 * Core system that responds to emotional states and creates seamless musical transitions.
 * Inspired by the emotional depth of RDR2's adaptive music but designed for the unique
 * transpersonal journey of our prehistoric survival experience.
 * 
 * Philosophy: Music that the player never consciously notices but always emotionally feels.
 * Each transition is a statement about the player's internal state and relationship with the world.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAdaptiveMusicManager : public AActor
{
    GENERATED_BODY()

public:
    AAdaptiveMusicManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CORE EMOTIONAL STATE SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    EEmotionalState CurrentEmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    EEmotionalState TargetEmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float EmotionalTransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float EmotionalIntensity = 0.5f;

    // === ADAPTIVE MUSIC CONFIGURATIONS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TMap<EEmotionalState, FAdaptiveMusicConfig> MusicConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TArray<UAudioComponent*> ActiveAudioLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float MasterVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    bool bMusicEnabled = true;

    // === ENVIRONMENTAL AWARENESS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Awareness")
    EEnvironmentalZone CurrentEnvironmentalZone = EEnvironmentalZone::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Awareness")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Awareness")
    float WeatherIntensity = 0.0f;

    // === CREATURE PROXIMITY SYSTEM ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Proximity")
    float NearestPredatorDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Proximity")
    float NearestCreatureDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Proximity")
    int32 VisibleCreatureCount = 0;

public:
    // === PUBLIC INTERFACE FOR EMOTIONAL STATE CHANGES ===

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 4.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void ModifyEmotionalIntensity(float IntensityDelta, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TriggerEmotionalMoment(EEmotionalState MomentaryState, float Duration = 10.0f);

    // === ENVIRONMENTAL STATE UPDATES ===

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void UpdateEnvironmentalZone(EEnvironmentalZone NewZone);

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void UpdateTimeOfDay(ETimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void UpdateWeatherIntensity(float NewWeatherIntensity);

    // === CREATURE PROXIMITY UPDATES ===

    UFUNCTION(BlueprintCallable, Category = "Creature Proximity")
    void UpdateCreatureProximity(float PredatorDistance, float CreatureDistance, int32 CreatureCount);

    UFUNCTION(BlueprintCallable, Category = "Creature Proximity")
    void OnPredatorSpotted(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Creature Proximity")
    void OnPredatorLost();

    // === MUSIC CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMusicEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void FadeToSilence(float FadeTime = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Music Control")
    void FadeFromSilence(float FadeTime = 5.0f);

protected:
    // === INTERNAL MUSIC MANAGEMENT ===

    UFUNCTION()
    void UpdateMusicLayers();

    UFUNCTION()
    void TransitionToTargetState();

    UFUNCTION()
    void CalculateEmotionalStateFromContext();

    UFUNCTION()
    void CreateAudioLayer(const FAudioLayerConfig& LayerConfig, USoundBase* SoundAsset);

    UFUNCTION()
    void RemoveAudioLayer(int32 LayerIndex);

    UFUNCTION()
    void UpdateLayerVolumes();

    // === TIMER HANDLES ===

    FTimerHandle EmotionalMomentTimer;
    FTimerHandle MusicUpdateTimer;
    FTimerHandle ContextAnalysisTimer;

    // === INTERNAL STATE TRACKING ===

    float CurrentTransitionProgress = 0.0f;
    bool bIsInEmotionalMoment = false;
    EEmotionalState PreMomentState = EEmotionalState::Calm;

    // === AUDIO COMPONENT POOL ===

    UPROPERTY()
    TArray<UAudioComponent*> AudioComponentPool;

    static const int32 MaxAudioComponents = 16;

private:
    // === EMOTIONAL STATE CALCULATION ===

    float CalculateTensionFromProximity() const;
    float CalculateWonderFromEnvironment() const;
    float CalculateFearFromThreats() const;
    EEmotionalState DetermineOptimalEmotionalState() const;
};