#pragma once

#include "CoreMinimal.h"
#include "AudioTypes.generated.h"

/**
 * Central audio type definitions for the Transpersonal Game
 * Consolidates all audio-related enums and structs to avoid duplication
 */

// =============================================================================
// EMOTIONAL STATES
// =============================================================================

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    // Core Emotional States
    Calm = 0        UMETA(DisplayName = "Calm"),
    Tense          UMETA(DisplayName = "Tense"),
    Danger         UMETA(DisplayName = "Danger"),
    Wonder         UMETA(DisplayName = "Wonder"),
    Fear           UMETA(DisplayName = "Fear"),
    Melancholy     UMETA(DisplayName = "Melancholy"),
    Discovery      UMETA(DisplayName = "Discovery"),
    Survival       UMETA(DisplayName = "Survival"),
    
    // Extended Emotional States
    Isolation      UMETA(DisplayName = "Isolation"),
    Connection     UMETA(DisplayName = "Connection"),
    Mystical       UMETA(DisplayName = "Mystical"),
    Scientific     UMETA(DisplayName = "Scientific")
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe = 0       UMETA(DisplayName = "Safe"),
    Cautious       UMETA(DisplayName = "Cautious"),
    Alert          UMETA(DisplayName = "Alert"),
    Danger         UMETA(DisplayName = "Danger"),
    Critical       UMETA(DisplayName = "Critical"),
    Combat         UMETA(DisplayName = "Combat")
};

// =============================================================================
// ENVIRONMENTAL TYPES
// =============================================================================

UENUM(BlueprintType)
enum class EEnvironmentalZone : uint8
{
    Forest = 0     UMETA(DisplayName = "Forest"),
    Plains         UMETA(DisplayName = "Plains"),
    Swamp          UMETA(DisplayName = "Swamp"),
    Mountains      UMETA(DisplayName = "Mountains"),
    Cave           UMETA(DisplayName = "Cave"),
    River          UMETA(DisplayName = "River"),
    Clearing       UMETA(DisplayName = "Clearing"),
    DenseJungle    UMETA(DisplayName = "Dense Jungle"),
    RockyOutcrop   UMETA(DisplayName = "Rocky Outcrop")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn = 0       UMETA(DisplayName = "Dawn"),
    Morning        UMETA(DisplayName = "Morning"),
    Noon           UMETA(DisplayName = "Noon"),
    Afternoon      UMETA(DisplayName = "Afternoon"),
    Dusk           UMETA(DisplayName = "Dusk"),
    Night          UMETA(DisplayName = "Night"),
    DeepNight      UMETA(DisplayName = "Deep Night")
};

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    Clear = 0      UMETA(DisplayName = "Clear"),
    Cloudy         UMETA(DisplayName = "Cloudy"),
    LightRain      UMETA(DisplayName = "Light Rain"),
    HeavyRain      UMETA(DisplayName = "Heavy Rain"),
    Storm          UMETA(DisplayName = "Storm"),
    Fog            UMETA(DisplayName = "Fog"),
    Wind           UMETA(DisplayName = "Wind")
};

// =============================================================================
// MUSIC SYSTEM TYPES
// =============================================================================

UENUM(BlueprintType)
enum class EMusicLayer : uint8
{
    Foundation = 0 UMETA(DisplayName = "Foundation Layer"),
    Harmony        UMETA(DisplayName = "Harmony Layer"),
    Melody         UMETA(DisplayName = "Melody Layer"),
    Rhythm         UMETA(DisplayName = "Rhythm Layer"),
    Texture        UMETA(DisplayName = "Texture Layer"),
    Tension        UMETA(DisplayName = "Tension Layer"),
    Release        UMETA(DisplayName = "Release Layer"),
    Ethereal       UMETA(DisplayName = "Ethereal Layer")
};

UENUM(BlueprintType)
enum class EMusicState : uint8
{
    // Exploration States
    Exploration_Calm = 0    UMETA(DisplayName = "Exploration - Calm"),
    Exploration_Tense       UMETA(DisplayName = "Exploration - Tense"),
    
    // Danger States
    Danger_Approaching      UMETA(DisplayName = "Danger - Approaching"),
    Danger_Imminent        UMETA(DisplayName = "Danger - Imminent"),
    
    // Combat States
    Combat_Light           UMETA(DisplayName = "Combat - Light"),
    Combat_Intense         UMETA(DisplayName = "Combat - Intense"),
    
    // Discovery States
    Discovery_Wonder       UMETA(DisplayName = "Discovery - Wonder"),
    Discovery_Fear         UMETA(DisplayName = "Discovery - Fear"),
    Discovery_Scientific   UMETA(DisplayName = "Discovery - Scientific"),
    Discovery_Mystical     UMETA(DisplayName = "Discovery - Mystical"),
    
    // Survival States
    Survival_Crafting      UMETA(DisplayName = "Survival - Crafting"),
    Survival_Shelter       UMETA(DisplayName = "Survival - Shelter"),
    Survival_Desperate     UMETA(DisplayName = "Survival - Desperate"),
    
    // Time-based States
    Night_Peaceful         UMETA(DisplayName = "Night - Peaceful"),
    Night_Threatening      UMETA(DisplayName = "Night - Threatening"),
    
    // Connection States
    Connection_Nature      UMETA(DisplayName = "Connection - Nature"),
    Connection_Creatures   UMETA(DisplayName = "Connection - Creatures"),
    Connection_Time        UMETA(DisplayName = "Connection - Time"),
    
    // Isolation States
    Isolation_Peaceful     UMETA(DisplayName = "Isolation - Peaceful"),
    Isolation_Lonely       UMETA(DisplayName = "Isolation - Lonely"),
    Isolation_Existential  UMETA(DisplayName = "Isolation - Existential")
};

UENUM(BlueprintType)
enum class EAudioLayer : uint8
{
    Music = 0      UMETA(DisplayName = "Music"),
    Ambience       UMETA(DisplayName = "Ambience"),
    SFX            UMETA(DisplayName = "SFX"),
    UI             UMETA(DisplayName = "UI"),
    Voice          UMETA(DisplayName = "Voice"),
    Dynamic        UMETA(DisplayName = "Dynamic")
};

// =============================================================================
// AUDIO DATA STRUCTURES
// =============================================================================

USTRUCT(BlueprintType)
struct FEmotionalAudioState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    EEmotionalState CurrentState = EEmotionalState::Calm;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float TransitionSpeed = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct FAudioLayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Config")
    class UMetaSoundSource* MetaSound = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseVolume = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Config")
    float FadeTime = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Config")
    bool bLoop = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Config")
    int32 Priority = 1;
};

USTRUCT(BlueprintType)
struct FMusicLayerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    EMusicLayer Layer = EMusicLayer::Foundation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    float FilterCutoff = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer State")
    float FadeTime = 2.0f;
};

USTRUCT(BlueprintType)
struct FMusicEmotionalContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    EMusicState PrimaryState = EMusicState::Exploration_Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    EMusicState SecondaryState = EMusicState::Exploration_Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendRatio = 0.0f; // 0.0 = Pure Primary, 1.0 = Pure Secondary

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TensionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WonderLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IsolationLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConnectionLevel = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TemporalDisplacement = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    float TimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    int32 DinosaurProximityCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Context")
    bool bInShelter = false;
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EMusicState CurrentMusicState = EMusicState::Exploration_Calm;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EEnvironmentalZone CurrentEnvironmentalZone = EEnvironmentalZone::Forest;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TensionLevel = 0.0f; // 0.0 = calm, 1.0 = maximum tension

    UPROPERTY(BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DangerProximity = 0.0f; // 0.0 = safe, 1.0 = immediate danger

    UPROPERTY(BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    bool bIsInShelter = false;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    bool bIsNearWater = false;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherIntensity = 0.0f; // 0.0 = clear, 1.0 = storm

    UPROPERTY(BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeartbeatIntensity = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    bool bIsBeingStalked = false;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float BreathingRate = 1.0f; // Multiplicador da respiração normal

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    ETimeOfDay TimeOfDayEnum = ETimeOfDay::Morning;

    UPROPERTY(BlueprintReadWrite, Category = "Audio State")
    EWeatherState WeatherState = EWeatherState::Clear;
};

USTRUCT(BlueprintType)
struct FMusicTransitionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float CrossfadeTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float LayerFadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float ParameterInterpolationTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bUseSmartTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bSyncToMusicalBeat = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BPM = 80.0f;
};