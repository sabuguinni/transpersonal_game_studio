#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "TranspersonalGame/Core/ConsciousnessComponent.h"
#include "TranspersonalAudioManager.generated.h"

USTRUCT(BlueprintType)
struct FAudioConsciousnessProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EConsciousnessState ConsciousnessState = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<UMetaSoundSource> BinauraBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseFrequency = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ReverbWetness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float LowPassFilterFreq = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float SpatializationIntensity = 0.5f;
};

UCLASS()
class TRANSPERSONALGAME_API UTranspersonalAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Audio")
    void UpdateAudioForConsciousnessState(EConsciousnessState State, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Audio")
    void PlayBinauralBeats(float LeftFreq, float RightFreq, float Duration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Audio")
    void SetAudioSpatializationMode(bool bEnable3DAudio, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Audio")
    void TriggerAudioTransition(EConsciousnessState FromState, EConsciousnessState ToState, float TransitionTime = 2.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profiles")
    TMap<EConsciousnessState, FAudioConsciousnessProfile> ConsciousnessAudioProfiles;

    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> BinauralAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> TransitionAudioComponent;

    // Current audio state
    EConsciousnessState CurrentAudioState = EConsciousnessState::Ordinary;
    float CurrentIntensity = 0.0f;

    void InitializeAudioProfiles();
    void ApplyAudioProfile(const FAudioConsciousnessProfile& Profile, float IntensityModifier);
    void UpdateBinauralFrequencies(EConsciousnessState State, float Intensity);

    // Frequency mappings for different consciousness states
    float GetBinauralFrequencyForState(EConsciousnessState State, float Intensity);
};