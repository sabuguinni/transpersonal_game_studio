#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "SharedTypes.generated.h"

// SURVIVAL GAME ENUMS AND STRUCTS - DINOSAUR SURVIVAL THEME ONLY
// NO SPIRITUAL/MYSTICAL CONTENT ALLOWED

// Biome System Types
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Swamp           UMETA(DisplayName = "Swamp"),
    Forest          UMETA(DisplayName = "Forest"),
    Savanna         UMETA(DisplayName = "Savanna"),
    Desert          UMETA(DisplayName = "Desert"),
    SnowyMountain   UMETA(DisplayName = "Snowy Mountain")
};

UENUM(BlueprintType)
enum class EEng_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear"),
    Cloudy          UMETA(DisplayName = "Cloudy"),
    Rain            UMETA(DisplayName = "Rain"),
    Storm           UMETA(DisplayName = "Storm"),
    Fog             UMETA(DisplayName = "Fog"),
    Snow            UMETA(DisplayName = "Snow")
};

UENUM(BlueprintType)
enum class EEng_TimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Noon            UMETA(DisplayName = "Noon"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

// Survival System Types
UENUM(BlueprintType)
enum class EEng_SurvivalStat : uint8
{
    Health          UMETA(DisplayName = "Health"),
    Hunger          UMETA(DisplayName = "Hunger"),
    Thirst          UMETA(DisplayName = "Thirst"),
    Stamina         UMETA(DisplayName = "Stamina"),
    Temperature     UMETA(DisplayName = "Temperature"),
    Fear            UMETA(DisplayName = "Fear")
};

UENUM(BlueprintType)
enum class EEng_ThreatLevel : uint8
{
    Safe            UMETA(DisplayName = "Safe"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Dangerous       UMETA(DisplayName = "Dangerous"),
    Deadly          UMETA(DisplayName = "Deadly"),
    Extreme         UMETA(DisplayName = "Extreme")
};

// Dinosaur AI Types
UENUM(BlueprintType)
enum class EEng_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor          UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Pteranodon      UMETA(DisplayName = "Pteranodon"),
    Dilophosaurus   UMETA(DisplayName = "Dilophosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class EEng_DinosaurBehavior : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Grazing         UMETA(DisplayName = "Grazing"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Nesting         UMETA(DisplayName = "Nesting")
};

UENUM(BlueprintType)
enum class EEng_DinosaurDiet : uint8
{
    Carnivore       UMETA(DisplayName = "Carnivore"),
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Omnivore        UMETA(DisplayName = "Omnivore"),
    Piscivore       UMETA(DisplayName = "Piscivore")
};

// Crafting System Types
UENUM(BlueprintType)
enum class EEng_CraftingCategory : uint8
{
    Tools           UMETA(DisplayName = "Tools"),
    Weapons         UMETA(DisplayName = "Weapons"),
    Shelter         UMETA(DisplayName = "Shelter"),
    Clothing        UMETA(DisplayName = "Clothing"),
    Food            UMETA(DisplayName = "Food"),
    Medicine        UMETA(DisplayName = "Medicine")
};

UENUM(BlueprintType)
enum class EEng_ResourceType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Bone            UMETA(DisplayName = "Bone"),
    Hide            UMETA(DisplayName = "Hide"),
    Meat            UMETA(DisplayName = "Meat"),
    Fiber           UMETA(DisplayName = "Fiber"),
    Water           UMETA(DisplayName = "Water"),
    Fire            UMETA(DisplayName = "Fire")
};

// Quest System Types (SURVIVAL FOCUSED ONLY)
UENUM(BlueprintType)
enum class EEng_QuestType : uint8
{
    Survival        UMETA(DisplayName = "Survival"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Shelter         UMETA(DisplayName = "Shelter")
};

UENUM(BlueprintType)
enum class EEng_QuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

// Struct Definitions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SurvivalStats
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
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear;

    FEng_SurvivalStats()
    {
        Health = 100.0f;
        Hunger = 100.0f;
        Thirst = 100.0f;
        Stamina = 100.0f;
        Temperature = 37.0f; // Normal body temperature
        Fear = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EEng_DinosaurSpecies> NativeDinosaurs;

    FEng_BiomeData()
    {
        BiomeType = EEng_BiomeType::Forest;
        BiomeName = "Unknown Biome";
        Temperature = 25.0f;
        Humidity = 50.0f;
        ThreatLevel = EEng_ThreatLevel::Safe;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EEng_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EEng_DinosaurDiet Diet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EEng_DinosaurBehavior CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FVector HomeLocation;

    FEng_DinosaurData()
    {
        Species = EEng_DinosaurSpecies::Raptor;
        Name = "Unknown Dinosaur";
        Diet = EEng_DinosaurDiet::Carnivore;
        CurrentBehavior = EEng_DinosaurBehavior::Idle;
        Health = 100.0f;
        AggressionLevel = 50.0f;
        TerritoryRadius = 1000.0f;
        HomeLocation = FVector::ZeroVector;
    }
};