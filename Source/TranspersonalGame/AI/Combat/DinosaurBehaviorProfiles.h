#pragma once

#include "CoreMinimal.h"
#include "CombatAIArchitecture.h"
#include "Engine/DataAsset.h"
#include "DinosaurBehaviorProfiles.generated.h"

/**
 * Dinosaur Species-Specific Combat Behavior Profiles
 * Each species has unique hunting, territorial, and social behaviors
 * Based on real paleontological research and game design requirements
 */

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Large Predators
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Allosaurus          UMETA(DisplayName = "Allosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Carnotaurus         UMETA(DisplayName = "Carnotaurus"),
    
    // Medium Predators
    Utahraptor          UMETA(DisplayName = "Utahraptor"),
    Deinonychus         UMETA(DisplayName = "Deinonychus"),
    Dilophosaurus       UMETA(DisplayName = "Dilophosaurus"),
    
    // Small Predators/Pack Hunters
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Compsognathus       UMETA(DisplayName = "Compsognathus"),
    Troodon             UMETA(DisplayName = "Troodon"),
    
    // Large Herbivores
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    
    // Medium Herbivores
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Iguanodon           UMETA(DisplayName = "Iguanodon"),
    
    // Small Herbivores (Domesticatable)
    Gallimimus          UMETA(DisplayName = "Gallimimus"),
    Pachycephalosaurus  UMETA(DisplayName = "Pachycephalosaurus"),
    
    // Flying
    Pteranodon          UMETA(DisplayName = "Pteranodon"),
    Quetzalcoatlus      UMETA(DisplayName = "Quetzalcoatlus"),
    
    // Aquatic
    Mosasaurus          UMETA(DisplayName = "Mosasaurus"),
    Plesiosaur          UMETA(DisplayName = "Plesiosaur")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FHuntingPattern
{
    GENERATED_BODY()

    // Preferred hunting time
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNocturnalHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCrepuscularHunter = true; // Dawn/Dusk

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDiurnalHunter = false;

    // Hunting strategy
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAmbushPredator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPursuitPredator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOpportunisticScavenger = false;

    // Preferred prey size relative to self (0.1 = much smaller, 1.0 = same size, 2.0 = twice as large)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.1, ClampMax = 3.0))
    float PreferredPreySize = 0.7f;

    // Hunting range from territory center
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HuntingRadius = 2000.0f;

    // Time spent hunting before returning to territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHuntingDuration = 1800.0f; // 30 minutes

    // Patience when stalking (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StalkingPatience = 120.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerritorialBehavior
{
    GENERATED_BODY()

    // Territory size in Unreal units
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 1500.0f;

    // How aggressively they defend territory (0.0 = passive, 1.0 = extremely aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float TerritorialAggression = 0.6f;

    // Distance at which they start warning intruders
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WarningDistance = 800.0f;

    // Distance at which they attack intruders
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDistance = 400.0f;

    // Whether they share territory with same species
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSocialSpecies = false;

    // Whether they tolerate smaller species in territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bToleratesSmallSpecies = true;

    // Preferred territory type
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PreferredBiomes;

    // Marking behavior (roaring, scent marking, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MarkingFrequency = 600.0f; // Every 10 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MarkingRange = 1200.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FSocialBehavior
{
    GENERATED_BODY()

    // Pack/herd size preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OptimalPackSize = 1;

    // Social hierarchy
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAlphaStructure = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMatriarchalSociety = false;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsesVocalCommunication = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsesBodyLanguage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsesChemicalSignals = false;

    // Cooperation levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float CooperationLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float GroupLoyalty = 0.5f;

    // Distance they maintain from pack members
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredPackDistance = 500.0f;

    // How quickly they respond to pack calls for help
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PackResponseTime = 5.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPlayerInteractionProfile
{
    GENERATED_BODY()

    // Base reaction to human presence
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNaturallyAfraidOfHumans = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCuriousAboutHumans = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSeesHumansAsPrey = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSeesHumansAsRivals = false;

    // Domestication potential (only for small herbivores)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DomesticationDifficulty = 1.0f; // 0.1 = easy, 1.0 = very hard

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustBuildingRate = 0.1f; // Per positive interaction

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustDecayRate = 0.05f; // Per day without interaction

    // Preferred interaction distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ComfortDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlightDistance = 500.0f;

    // Response to player actions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseToleranceLevel = 0.5f; // 0.0 = very sensitive, 1.0 = ignores noise

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementToleranceLevel = 0.3f; // Response to sudden movements

    // Memory of player interactions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryDuration = 3600.0f; // How long they remember the player (seconds)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRemembersNegativeInteractions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRemembersPositiveInteractions = true;
};

/**
 * Data Asset containing complete behavior profile for a dinosaur species
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDinosaurBehaviorProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    // Species identification
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    EDinosaurSpecies Species = EDinosaurSpecies::TyrannosaurusRex;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    FString SpeciesName = "Tyrannosaurus Rex";

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    FString ScientificName = "Tyrannosaurus rex";

    // Base combat role and capabilities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    EDinosaurCombatRole PrimaryCombatRole = EDinosaurCombatRole::Predator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FCombatCapabilities BaseCombatCapabilities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FCombatPersonality BasePersonality;

    // Behavior patterns
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    FHuntingPattern HuntingBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    FTerritorialBehavior TerritorialBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    FSocialBehavior SocialBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    FPlayerInteractionProfile PlayerInteraction;

    // Environmental preferences
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    TArray<FString> PreferredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float OptimalTemperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    float TemperatureTolerance = 10.0f; // +/- range

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    bool bRequiresWaterAccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    bool bCanSwim = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    bool bCanClimb = false;

    // Gameplay tags for this species
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer SpeciesTags;

    // Audio cues for this species
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<class USoundBase*> IdleSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<class USoundBase*> AlertSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<class USoundBase*> AttackSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<class USoundBase*> PainSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<class USoundBase*> DeathSounds;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<class USoundBase*> CommunicationSounds;
};