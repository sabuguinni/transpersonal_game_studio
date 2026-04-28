// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioTypes.generated.h"

/**
 * Emotional states that drive adaptive music system
 * Based on the transpersonal game's core emotional journey
 */
UENUM(BlueprintType)
enum class EAudio_EmotionalState : uint8
{
    Wonder,         // First moments of discovery - awe and curiosity
    Tension,        // Predator awareness - heightened alertness
    Fear,           // Direct threat - fight or flight response
    Calm,           // Safe moments - peaceful exploration
    Melancholy,     // Isolation and longing for home
    Triumph,        // Successful survival moments
    Transcendence   // Moments of deep connection with nature
};

/**
 * Environmental audio zones for dynamic soundscape
 */
UENUM(BlueprintType)
enum class EEnvironmentalZone : uint8
{
    Forest,         // Dense vegetation with creature activity
    Plains,         // Open grasslands with distant sounds
    Wetlands,       // Swamps and water sources
    Mountains,      // Rocky terrain with echoes
    Caves,          // Underground spaces with reverb
    Coastline,      // Ocean waves and shore sounds
    Valley          // Protected areas with unique acoustics
};

/**
 * Creature behavior states affecting audio
 */
UENUM(BlueprintType)
enum class ECreatureBehaviorState : uint8
{
    Passive,        // Normal behavior - ambient creature sounds
    Alert,          // Aware of player presence - cautious sounds
    Hunting,        // Predator mode - threatening audio
    Feeding,        // Eating behavior - specific feeding sounds
    Territorial,    // Defending territory - warning calls
    Mating,         // Courtship behavior - unique vocalizations
    Fleeing         // Escape behavior - panic sounds
};

/**
 * Time of day affecting audio mix and creature behavior
 */
UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn,           // Early morning - awakening sounds
    Morning,        // Active period - full creature activity
    Midday,         // Peak activity - intense soundscape
    Afternoon,      // Calming period - reduced activity
    Dusk,           // Transition - changing creature patterns
    Night,          // Nocturnal activity - different creature sets
    DeepNight       // Minimal activity - mysterious ambience
};

/**
 * Audio layer configuration for procedural mixing
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bIsLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    EAudio_EmotionalState TargetEmotionalState = EAudio_EmotionalState::Calm;

    FAudioLayerConfig()
    {
        LayerName = TEXT("DefaultLayer");
        Volume = 1.0f;
        FadeTime = 2.0f;
        bIsLooping = true;
        TargetEmotionalState = EAudio_EmotionalState::Calm;
    }
};

/**
 * Creature audio profile for individual dinosaur sound design
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCreatureAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    FString CreatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    TArray<USoundBase*> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    TArray<USoundBase*> AlertSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    TArray<USoundBase*> AggressiveSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    TArray<USoundBase*> PainSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    TArray<USoundBase*> DeathSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    float VoiceVariationRange = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature Audio")
    float MaxAudibleDistance = 2000.0f;

    FCreatureAudioProfile()
    {
        CreatureName = TEXT("Unknown Creature");
        VoiceVariationRange = 0.2f;
        MaxAudibleDistance = 2000.0f;
    }
};

/**
 * Environmental soundscape configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentalSoundscape
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    EEnvironmentalZone ZoneType = EEnvironmentalZone::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TArray<USoundBase*> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TArray<USoundBase*> WeatherSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    TArray<USoundBase*> RandomElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float RandomElementChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    FVector2D RandomElementInterval = FVector2D(30.0f, 120.0f);

    FEnvironmentalSoundscape()
    {
        ZoneType = EEnvironmentalZone::Forest;
        RandomElementChance = 0.1f;
        RandomElementInterval = FVector2D(30.0f, 120.0f);
    }
};

/**
 * Adaptive music configuration based on emotional state
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAdaptiveMusicConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    EAudio_EmotionalState EmotionalState = EAudio_EmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    TArray<FAudioLayerConfig> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float TransitionTime = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float IntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    bool bAllowSilence = false;

    FAdaptiveMusicConfig()
    {
        EmotionalState = EAudio_EmotionalState::Calm;
        TransitionTime = 4.0f;
        IntensityMultiplier = 1.0f;
        bAllowSilence = false;
    }
};