#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"
#include "GameFramework/GameModeBase.h"
#include "AudioSystemCore.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm = 0,
    Tense,
    Danger,
    Wonder,
    Melancholy,
    Terror
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    Forest = 0,
    Plains,
    Swamp,
    Mountains,
    Cave,
    River
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn = 0,
    Morning,
    Midday,
    Afternoon,
    Dusk,
    Night,
    DeepNight
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType EnvironmentType = EEnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0.0 = safe, 1.0 = maximum danger

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerStress = 0.0f; // 0.0 = relaxed, 1.0 = maximum stress

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNearWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInShelter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemCore();

    // Core system functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(class ADinosaurBase* Dinosaur, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentalSound(const FVector& Location, const FString& SoundName, float Volume = 1.0f);

    // Adaptive music system
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StartAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void StopAdaptiveMusic(float FadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicIntensity(float Intensity); // 0.0 = calm, 1.0 = intense

protected:
    // Current audio state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // Audio components for different layers
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbienceComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* EnvironmentComponent;

    // Sound assets organized by category
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    TMap<EEmotionalState, class USoundCue*> MusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Assets")
    TMap<EEnvironmentType, class USoundCue*> AmbienceTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX Assets")
    TMap<FString, class USoundCue*> EnvironmentalSounds;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundAttenuation* DefaultAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundAttenuation* DinosaurAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbienceVolume = 0.8f;

private:
    // Internal state management
    bool bIsInitialized = false;
    float CurrentMusicIntensity = 0.0f;
    EEmotionalState TargetEmotionalState = EEmotionalState::Calm;
    
    // Transition management
    FTimerHandle MusicTransitionTimer;
    
    void UpdateMusicLayer();
    void UpdateAmbienceLayer();
    void CalculateAudioParameters();
    float CalculateThreatLevel() const;
    void OnMusicTransitionComplete();
};