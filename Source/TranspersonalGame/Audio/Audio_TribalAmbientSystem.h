#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_TribalAmbientSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_TribalActivity : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Storytelling    UMETA(DisplayName = "Storytelling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Celebration     UMETA(DisplayName = "Celebration"),
    Mourning        UMETA(DisplayName = "Mourning"),
    Ritual          UMETA(DisplayName = "Ritual")
};

UENUM(BlueprintType)
enum class EAudio_TribalMood : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Tense           UMETA(DisplayName = "Tense"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Excited         UMETA(DisplayName = "Excited"),
    Solemn          UMETA(DisplayName = "Solemn"),
    Joyful          UMETA(DisplayName = "Joyful")
};

USTRUCT(BlueprintType)
struct FAudio_TribalSoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    USoundCue* SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Audio")
    bool bLooping;

    FAudio_TribalSoundLayer()
    {
        SoundCue = nullptr;
        BaseVolume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bLooping = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_TribalAmbientSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_TribalAmbientSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core tribal audio management
    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void SetTribalActivity(EAudio_TribalActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void SetTribalMood(EAudio_TribalMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void SetTribalPopulation(int32 PopulationCount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void SetTimeOfDay(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    // Activity-specific audio triggers
    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void TriggerCraftingSound(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void TriggerStorytellingSession();

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void TriggerHuntingPreparation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void TriggerCelebrationEvent();

    // Environmental integration
    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void UpdateWeatherConditions(bool bIsRaining, bool bIsStormy, float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Tribal Audio")
    void UpdateDangerLevel(float DangerLevel); // 0.0 = safe, 1.0 = extreme danger

protected:
    // Current tribal state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal State")
    EAudio_TribalActivity CurrentActivity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal State")
    EAudio_TribalMood CurrentMood;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal State")
    int32 TribalPopulation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal State")
    float CurrentTimeOfDay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal State")
    float CurrentDangerLevel;

    // Audio components for layered soundscape
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* BaseAmbienceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* ActivityComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* CampfireComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* ConversationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* WeatherComponent;

    // Sound libraries for different activities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Libraries")
    TMap<EAudio_TribalActivity, FAudio_TribalSoundLayer> ActivitySounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Libraries")
    TMap<EAudio_TribalMood, FAudio_TribalSoundLayer> MoodSounds;

    // Campfire and environmental sounds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    USoundCue* CampfireCracklingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    USoundCue* DistantConversationSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    USoundCue* TribalMusicSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    USoundCue* RainOnShelterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    USoundCue* WindThroughCampSound;

    // Audio parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float ActivityTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float PopulationVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudibleDistance;

private:
    // Internal audio management
    void UpdateAudioLayers();
    void TransitionToActivity(EAudio_TribalActivity NewActivity);
    void TransitionToMood(EAudio_TribalMood NewMood);
    void UpdateVolumeBasedOnPopulation();
    void UpdateTimeOfDayEffects();
    void UpdateWeatherEffects();

    // Transition tracking
    float ActivityTransitionTimer;
    EAudio_TribalActivity TargetActivity;
    bool bIsTransitioning;
};