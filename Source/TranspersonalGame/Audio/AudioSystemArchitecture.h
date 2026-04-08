#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioSystemArchitecture.generated.h"

/**
 * Core Audio System Architecture for Jurassic Survival Game
 * Handles adaptive music, environmental audio, and creature behavior sounds
 * Based on Walter Murch principles: the sound that doesn't exist is often more powerful
 */

UENUM(BlueprintType)
enum class EAudioTensionLevel : uint8
{
    Safe = 0,           // Peaceful exploration, ambient nature sounds
    Cautious = 1,       // Subtle tension, distant sounds
    Alert = 2,          // Immediate danger nearby, heightened awareness
    Panic = 3,          // Direct threat, fight or flight
    Silent = 4          // Unnatural silence - something is very wrong
};

UENUM(BlueprintType)
enum class EEnvironmentalState : uint8
{
    Forest_Dense = 0,
    Forest_Clearing = 1,
    Riverside = 2,
    Caves = 3,
    Cliffs = 4,
    Swampland = 5,
    Plains = 6
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn = 0,
    Morning = 1,
    Midday = 2,
    Afternoon = 3,
    Dusk = 4,
    Night = 5,
    DeepNight = 6
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudioTensionLevel TensionLevel = EAudioTensionLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentalState Environment = EEnvironmentalState::Forest_Dense;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerStealthLevel = 0.0f; // 0.0 = making noise, 1.0 = completely silent

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NearbyCreatureDensity = 0.0f; // 0.0 = alone, 1.0 = surrounded

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRecentCombat = false;

    FAudioStateData()
    {
        TensionLevel = EAudioTensionLevel::Safe;
        Environment = EEnvironmentalState::Forest_Dense;
        TimeOfDay = ETimeOfDay::Morning;
        PlayerStealthLevel = 0.0f;
        NearbyCreatureDensity = 0.0f;
        bIsPlayerHidden = false;
        bRecentCombat = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Core system functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTensionLevel(EAudioTensionLevel NewTension);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Environmental Audio")
    void TransitionToEnvironment(EEnvironmentalState NewEnvironment, float TransitionTime = 2.0f);

    // Creature audio management
    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void RegisterCreaturePresence(class ADinosaurBase* Creature, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Creature Audio")
    void UnregisterCreaturePresence(class ADinosaurBase* Creature);

    // Adaptive music system
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void TriggerMusicTransition(EAudioTensionLevel TargetTension, float TransitionTime = 3.0f);

    // Silence management - the most important system
    UFUNCTION(BlueprintCallable, Category = "Silence System")
    void TriggerUnnatural Silence(float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Silence System")
    void BreakSilence(bool bWithJumpScare = false);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> EnvironmentalAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSounds")
    TObjectPtr<UMetaSoundSource> CreatureBehaviorMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TObjectPtr<UAudioComponent> CreatureAudioComponent;

private:
    // Internal state tracking
    bool bInSilentMode = false;
    float SilenceStartTime = 0.0f;
    float SilenceDuration = 0.0f;
    
    TArray<TWeakObjectPtr<class ADinosaurBase>> NearbyCreatures;
    
    void UpdateMusicParameters();
    void UpdateAmbienceParameters();
    void CalculateOptimalSilenceMoments();
};