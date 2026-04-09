// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "CreatureAudioComponent.generated.h"

/**
 * Creature Audio Component
 * 
 * Gives voice to every creature in our prehistoric world. Each dinosaur has its own
 * unique vocal signature, behavioral audio patterns, and emotional expressions.
 * 
 * Philosophy: Every creature is an individual with their own voice. The player should
 * be able to recognize specific dinosaurs by their calls, creating emotional bonds
 * even with potentially dangerous creatures.
 * 
 * Inspired by the individual creature personalities in RDR2 and the procedural
 * creature audio systems of Subnautica.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCreatureAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCreatureAudioComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CREATURE IDENTITY ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Identity")
    FCreatureAudioProfile AudioProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Identity")
    FString CreatureID; // Unique identifier for this individual creature

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Identity")
    float VoicePitchVariation = 0.0f; // Individual voice characteristic (-1.0 to 1.0)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Identity")
    float VoiceTimbreVariation = 0.0f; // Individual voice timbre (-1.0 to 1.0)

    // === BEHAVIORAL STATE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECreatureBehaviorState CurrentBehaviorState = ECreatureBehaviorState::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float EmotionalIntensity = 0.5f; // How intense the current emotional state is

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsAlpha = false; // Alpha creatures have different vocal patterns

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsInjured = false; // Injured creatures sound different

    // === VOCALIZATION SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalization")
    float VocalizationFrequency = 0.1f; // Base chance per second to vocalize

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalization")
    FVector2D VocalizationInterval = FVector2D(5.0f, 30.0f); // Min/Max seconds between vocalizations

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalization")
    bool bCanVocalize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vocalization")
    float SilenceChance = 0.3f; // Chance to remain silent even when triggered

    // === PROXIMITY AWARENESS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float PlayerProximityThreshold = 1000.0f; // Distance at which creature becomes aware of player

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float ThreatProximityThreshold = 500.0f; // Distance at which creature feels threatened

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    bool bIsPlayerVisible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float DistanceToPlayer = 10000.0f;

    // === AUDIO COMPONENTS ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* PrimaryVoiceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* SecondaryVoiceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* BreathingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* FootstepsComponent;

public:
    // === BEHAVIOR STATE MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void SetBehaviorState(ECreatureBehaviorState NewState, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void TriggerVocalization(ECreatureBehaviorState StateOverride = ECreatureBehaviorState::Passive, bool bForceVocalization = false);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void TriggerPainSound(float PainIntensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void TriggerDeathSound();

    // === PROXIMITY UPDATES ===

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void UpdatePlayerProximity(float Distance, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void OnPlayerSpotted();

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void OnPlayerLost();

    // === CREATURE INTERACTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void RespondToOtherCreature(UCreatureAudioComponent* OtherCreature, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void TriggerTerritorialCall();

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void TriggerMatingCall();

    // === AUDIO CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetVocalizationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetVoiceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetCreatureInjured(bool bInjured);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetCreatureAsAlpha(bool bAlpha);

protected:
    // === INTERNAL VOCALIZATION LOGIC ===

    UFUNCTION()
    void ProcessVocalization();

    UFUNCTION()
    void PlayVocalizationSound(USoundBase* Sound, float VolumeMultiplier = 1.0f, bool bUseSecondaryComponent = false);

    UFUNCTION()
    void UpdateBreathingAudio();

    UFUNCTION()
    void UpdateFootstepAudio();

    UFUNCTION()
    USoundBase* GetSoundForCurrentState() const;

    UFUNCTION()
    float CalculateVocalizationChance() const;

    UFUNCTION()
    void ApplyVoiceVariation(UAudioComponent* AudioComponent) const;

    // === TIMER MANAGEMENT ===

    FTimerHandle VocalizationTimer;
    FTimerHandle BreathingUpdateTimer;
    FTimerHandle ProximityCheckTimer;

    // === INTERNAL STATE ===

    float LastVocalizationTime = 0.0f;
    bool bRecentlyVocalized = false;
    float CurrentVoiceVolume = 1.0f;

    // === CONSTANTS ===

    static const float MinVocalizationInterval;
    static const float MaxVocalizationInterval;

private:
    // === HELPER FUNCTIONS ===

    void InitializeAudioComponents();
    void ScheduleNextVocalization();
    float GetBehaviorStateVolumeMultiplier() const;
    float GetDistanceVolumeMultiplier() const;
};