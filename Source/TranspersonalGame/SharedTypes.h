#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "SharedTypes.generated.h"

// Biome and Environment Types
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Swampland       UMETA(DisplayName = "Swampland"),
    Forest          UMETA(DisplayName = "Dense Forest"),
    Savanna         UMETA(DisplayName = "Open Savanna"),
    Desert          UMETA(DisplayName = "Arid Desert"),
    Mountains       UMETA(DisplayName = "Rocky Mountains"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    Coastline       UMETA(DisplayName = "Coastal Area")
};

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Skies"),
    Overcast        UMETA(DisplayName = "Overcast"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Storm           UMETA(DisplayName = "Thunderstorm"),
    Fog             UMETA(DisplayName = "Dense Fog"),
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

// Dinosaur and Wildlife Types
UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Allosaurus          UMETA(DisplayName = "Allosaurus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Pteranodon          UMETA(DisplayName = "Pteranodon"),
    Compsognathus       UMETA(DisplayName = "Compsognathus")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Grazing         UMETA(DisplayName = "Grazing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Territorial     UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class EDinosaurDiet : uint8
{
    Carnivore       UMETA(DisplayName = "Carnivore"),
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Omnivore        UMETA(DisplayName = "Omnivore"),
    Piscivore       UMETA(DisplayName = "Piscivore")
};

// Character and Survival Types
UENUM(BlueprintType)
enum class ESurvivalStat : uint8
{
    Health          UMETA(DisplayName = "Health"),
    Hunger          UMETA(DisplayName = "Hunger"),
    Thirst          UMETA(DisplayName = "Thirst"),
    Stamina         UMETA(DisplayName = "Stamina"),
    Fear            UMETA(DisplayName = "Fear"),
    Temperature     UMETA(DisplayName = "Temperature"),
    Infection       UMETA(DisplayName = "Infection")
};

UENUM(BlueprintType)
enum class EMovementType : uint8
{
    Walking         UMETA(DisplayName = "Walking"),
    Running         UMETA(DisplayName = "Running"),
    Crouching       UMETA(DisplayName = "Crouching"),
    Crawling        UMETA(DisplayName = "Crawling"),
    Swimming        UMETA(DisplayName = "Swimming"),
    Climbing        UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EToolType : uint8
{
    StoneKnife      UMETA(DisplayName = "Stone Knife"),
    WoodenSpear     UMETA(DisplayName = "Wooden Spear"),
    StoneAxe        UMETA(DisplayName = "Stone Axe"),
    BoneClub        UMETA(DisplayName = "Bone Club"),
    FireTorch       UMETA(DisplayName = "Fire Torch"),
    WaterContainer  UMETA(DisplayName = "Water Container"),
    TrapSnare       UMETA(DisplayName = "Trap Snare")
};

// Combat and Damage Types
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Blunt           UMETA(DisplayName = "Blunt Force"),
    Piercing        UMETA(DisplayName = "Piercing"),
    Slashing        UMETA(DisplayName = "Slashing"),
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Crush           UMETA(DisplayName = "Crush"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Poison          UMETA(DisplayName = "Poison")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Alert           UMETA(DisplayName = "Alert"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Stunned         UMETA(DisplayName = "Stunned"),
    Dead            UMETA(DisplayName = "Dead")
};

// Quest and Mission Types
UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Survival        UMETA(DisplayName = "Survival"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Story           UMETA(DisplayName = "Story"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Territory       UMETA(DisplayName = "Territory"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Locked          UMETA(DisplayName = "Locked"),
    Expired         UMETA(DisplayName = "Expired")
};

// Narrative and Character Types
UENUM(BlueprintType)
enum class ENarr_TribalRole : uint8
{
    Elder           UMETA(DisplayName = "Tribal Elder"),
    Scout           UMETA(DisplayName = "Scout"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Healer          UMETA(DisplayName = "Healer"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Survivor        UMETA(DisplayName = "Survivor"),
    Child           UMETA(DisplayName = "Child"),
    Outcast         UMETA(DisplayName = "Outcast")
};

UENUM(BlueprintType)
enum class ENarr_StoryArc : uint8
{
    Awakening       UMETA(DisplayName = "The Awakening"),
    FirstContact    UMETA(DisplayName = "First Contact"),
    TribalBonds     UMETA(DisplayName = "Tribal Bonds"),
    TheHunt         UMETA(DisplayName = "The Great Hunt"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    Migration       UMETA(DisplayName = "The Migration"),
    Survival        UMETA(DisplayName = "Ultimate Survival"),
    Legacy          UMETA(DisplayName = "The Legacy")
};

UENUM(BlueprintType)
enum class ENarr_EmotionalState : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Angry           UMETA(DisplayName = "Angry"),
    Hopeful         UMETA(DisplayName = "Hopeful"),
    Desperate       UMETA(DisplayName = "Desperate"),
    Determined      UMETA(DisplayName = "Determined"),
    Confused        UMETA(DisplayName = "Confused"),
    Grateful        UMETA(DisplayName = "Grateful"),
    Suspicious      UMETA(DisplayName = "Suspicious")
};

// Audio and VFX Types
UENUM(BlueprintType)
enum class EAudioType : uint8
{
    Ambient         UMETA(DisplayName = "Ambient"),
    Music           UMETA(DisplayName = "Music"),
    SFX             UMETA(DisplayName = "Sound Effects"),
    Voice           UMETA(DisplayName = "Voice"),
    UI              UMETA(DisplayName = "User Interface"),
    Footsteps       UMETA(DisplayName = "Footsteps"),
    Environmental   UMETA(DisplayName = "Environmental"),
    Combat          UMETA(DisplayName = "Combat")
};

UENUM(BlueprintType)
enum class EVFXType : uint8
{
    Fire            UMETA(DisplayName = "Fire"),
    Smoke           UMETA(DisplayName = "Smoke"),
    Water           UMETA(DisplayName = "Water"),
    Blood           UMETA(DisplayName = "Blood"),
    Dust            UMETA(DisplayName = "Dust"),
    Sparks          UMETA(DisplayName = "Sparks"),
    Magic           UMETA(DisplayName = "Magic"),
    Weather         UMETA(DisplayName = "Weather")
};

// Shared Structs
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.0f;

    FSurvivalStats()
    {
        Health = 100.0f;
        Hunger = 100.0f;
        Thirst = 100.0f;
        Stamina = 100.0f;
        Fear = 0.0f;
        Temperature = 37.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurSpecies Species = EDinosaurSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    EDinosaurDiet Diet = EDinosaurDiet::Carnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float Aggression = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    float AttackDamage = 25.0f;

    FDinosaurStats()
    {
        Species = EDinosaurSpecies::Velociraptor;
        Diet = EDinosaurDiet::Carnivore;
        Health = 100.0f;
        Aggression = 50.0f;
        DetectionRange = 1000.0f;
        AttackDamage = 25.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    FBiomeData()
    {
        BiomeType = EBiomeType::Forest;
        BiomeName = TEXT("Unknown Biome");
        Temperature = 25.0f;
        Humidity = 50.0f;
    }
};