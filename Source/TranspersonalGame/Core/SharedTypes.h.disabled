#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "SharedTypes.generated.h"

// === NARRATIVE & DIALOGUE ENUMS ===

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Information     UMETA(DisplayName = "Information"),
    Warning         UMETA(DisplayName = "Warning"),
    Combat          UMETA(DisplayName = "Combat"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Trading         UMETA(DisplayName = "Trading"),
    Storytelling    UMETA(DisplayName = "Storytelling")
};

UENUM(BlueprintType)
enum class ENarr_CharacterRole : uint8
{
    Elder           UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Crafter         UMETA(DisplayName = "Crafter")
};

UENUM(BlueprintType)
enum class ENarr_StoryArc : uint8
{
    Introduction    UMETA(DisplayName = "Introduction"),
    FirstContact    UMETA(DisplayName = "First Contact"),
    Learning        UMETA(DisplayName = "Learning Phase"),
    Challenge       UMETA(DisplayName = "Major Challenge"),
    Growth          UMETA(DisplayName = "Character Growth"),
    Mastery         UMETA(DisplayName = "Mastery Phase"),
    Leadership      UMETA(DisplayName = "Leadership Role")
};

// === EXISTING ENUMS (from other agents) ===

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Swamp           UMETA(DisplayName = "Swamp"),
    Forest          UMETA(DisplayName = "Forest"), 
    Savanna         UMETA(DisplayName = "Savanna"),
    Desert          UMETA(DisplayName = "Desert"),
    Mountain        UMETA(DisplayName = "Mountain"),
    River           UMETA(DisplayName = "River"),
    Lake            UMETA(DisplayName = "Lake"),
    Canyon          UMETA(DisplayName = "Canyon")
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Compsognathus   UMETA(DisplayName = "Compsognathus")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Flee            UMETA(DisplayName = "Flee"),
    Feed            UMETA(DisplayName = "Feed"),
    Sleep           UMETA(DisplayName = "Sleep"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Territorial     UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ESurvivalNeed : uint8
{
    Health          UMETA(DisplayName = "Health"),
    Hunger          UMETA(DisplayName = "Hunger"),
    Thirst          UMETA(DisplayName = "Thirst"),
    Stamina         UMETA(DisplayName = "Stamina"),
    Warmth          UMETA(DisplayName = "Warmth"),
    Safety          UMETA(DisplayName = "Safety"),
    Rest            UMETA(DisplayName = "Rest")
};

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Cloudy          UMETA(DisplayName = "Cloudy"),
    Rain            UMETA(DisplayName = "Rain"),
    Storm           UMETA(DisplayName = "Storm"),
    Fog             UMETA(DisplayName = "Fog"),
    Drought         UMETA(DisplayName = "Drought")
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night"),
    Midnight        UMETA(DisplayName = "Midnight")
};

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Survival        UMETA(DisplayName = "Survival"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Social          UMETA(DisplayName = "Social"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Territory       UMETA(DisplayName = "Territory")
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

// === NARRATIVE STRUCTS ===

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ENarr_CharacterRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 ExperienceLevel;

    FNarr_CharacterProfile()
    {
        CharacterName = TEXT("");
        Role = ENarr_CharacterRole::Hunter;
        BackgroundStory = TEXT("");
        ExperienceLevel = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryArc StoryArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsTriggered;

    FNarr_StoryEvent()
    {
        EventName = TEXT("");
        StoryArc = ENarr_StoryArc::Introduction;
        EventDescription = TEXT("");
        bIsTriggered = false;
    }
};

// === EXISTING STRUCTS (from other agents) ===

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear;

    FSurvivalStats()
    {
        Health = 100.0f;
        Hunger = 100.0f;
        Thirst = 100.0f;
        Stamina = 100.0f;
        Fear = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    FBiomeData()
    {
        BiomeType = EBiomeType::Forest;
        BiomeCenter = FVector2D::ZeroVector;
        BiomeRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    bool bIsPackHunter;

    FDinosaurData()
    {
        Species = EDinosaurSpecies::Compsognathus;
        BehaviorState = EDinosaurBehaviorState::Idle;
        AggressionLevel = 50.0f;
        DetectionRange = 500.0f;
        bIsPackHunter = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWeatherData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWeatherType CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindSpeed;

    FWeatherData()
    {
        CurrentWeather = EWeatherType::Clear;
        Temperature = 25.0f;
        Humidity = 60.0f;
        WindSpeed = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredProgress;

    FQuestObjective()
    {
        ObjectiveText = TEXT("");
        bIsCompleted = false;
        CurrentProgress = 0;
        RequiredProgress = 1;
    }
};