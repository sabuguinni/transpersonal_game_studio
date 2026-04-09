#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "../WorldGeneration/ProceduralWorldGenerator.h"
#include "ArchitecturalStructure.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogArchitecture, Log, All);

/**
 * Architectural Periods for Prehistoric Structures
 */
UENUM(BlueprintType)
enum class EArchitecturalPeriod : uint8
{
    Paleolithic          UMETA(DisplayName = "Paleolithic (Old Stone Age)"),
    Mesolithic          UMETA(DisplayName = "Mesolithic (Middle Stone Age)"),
    Neolithic           UMETA(DisplayName = "Neolithic (New Stone Age)"),
    BronzeAge           UMETA(DisplayName = "Bronze Age"),
    IronAge             UMETA(DisplayName = "Iron Age"),
    Unknown             UMETA(DisplayName = "Unknown Period")
};

/**
 * Structure Types Based on Archaeological Evidence
 */
UENUM(BlueprintType)
enum class EStructureType : uint8
{
    // Dwellings
    Cave                UMETA(DisplayName = "Cave Dwelling"),
    RockShelter         UMETA(DisplayName = "Rock Shelter"),
    PitHouse            UMETA(DisplayName = "Pit House"),
    Hut                 UMETA(DisplayName = "Surface Hut"),
    Longhouse           UMETA(DisplayName = "Longhouse"),
    
    // Ceremonial
    StoneCircle         UMETA(DisplayName = "Stone Circle"),
    Dolmen              UMETA(DisplayName = "Dolmen"),
    Menhir              UMETA(DisplayName = "Standing Stone"),
    Cairn               UMETA(DisplayName = "Burial Cairn"),
    
    // Functional
    Granary             UMETA(DisplayName = "Food Storage"),
    Workshop            UMETA(DisplayName = "Tool Workshop"),
    Kiln                UMETA(DisplayName = "Pottery Kiln"),
    Hearth              UMETA(DisplayName = "Communal Hearth"),
    
    // Defensive
    Palisade            UMETA(DisplayName = "Wooden Palisade"),
    Earthwork           UMETA(DisplayName = "Earthwork"),
    Hillfort            UMETA(DisplayName = "Hill Fort"),
    
    // Infrastructure
    Bridge              UMETA(DisplayName = "Simple Bridge"),
    Path                UMETA(DisplayName = "Marked Path"),
    Well                UMETA(DisplayName = "Water Well"),
    
    // Ruins
    AbandonedStructure  UMETA(DisplayName = "Abandoned Ruins"),
    PartialRuin         UMETA(DisplayName = "Partial Ruins"),
    Foundation          UMETA(DisplayName = "Foundation Only")
};

/**
 * Construction Materials Available in Prehistoric Times
 */
UENUM(BlueprintType)
enum class EConstructionMaterial : uint8
{
    Wood                UMETA(DisplayName = "Wood"),
    Stone               UMETA(DisplayName = "Stone"),
    Bone                UMETA(DisplayName = "Bone"),
    Hide                UMETA(DisplayName = "Animal Hide"),
    Clay                UMETA(DisplayName = "Clay/Mud"),
    Thatch              UMETA(DisplayName = "Thatch/Grass"),
    Earth               UMETA(DisplayName = "Packed Earth"),
    Mixed               UMETA(DisplayName = "Mixed Materials")
};

/**
 * Condition/Age of Structure
 */
UENUM(BlueprintType)
enum class EStructureCondition : uint8
{
    New                 UMETA(DisplayName = "Recently Built"),
    WellMaintained      UMETA(DisplayName = "Well Maintained"),
    Weathered           UMETA(DisplayName = "Weathered"),
    Deteriorating       UMETA(DisplayName = "Deteriorating"),
    Ruined              UMETA(DisplayName = "Ruined"),
    AncientRuin         UMETA(DisplayName = "Ancient Ruin")
};

/**
 * Interior Layout Configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FInteriorLayout
{
    GENERATED_BODY()

    /** Interior space dimensions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions")
    FVector InteriorSize = FVector(400.0f, 400.0f, 250.0f);

    /** Number of rooms/spaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    int32 RoomCount = 1;

    /** Has central hearth/fire pit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasCentralHearth = true;

    /** Has sleeping areas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasSleepingArea = true;

    /** Has storage areas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasStorageArea = false;

    /** Has work areas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    bool bHasWorkArea = false;

    /** Entrance count */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Access", meta = (ClampMin = "1"))
    int32 EntranceCount = 1;

    /** Has smoke hole/ventilation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ventilation")
    bool bHasSmokeHole = true;

    /** Interior props density (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PropsDensity = 0.3f;
};

/**
 * Structural Integrity and Damage
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStructuralIntegrity
{
    GENERATED_BODY()

    /** Overall structural health (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StructuralHealth = 1.0f;

    /** Weather damage level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherDamage = 0.0f;

    /** Age-related deterioration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AgeDamage = 0.0f;

    /** Animal damage (termites, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AnimalDamage = 0.0f;

    /** Has been abandoned */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    bool bIsAbandoned = false;

    /** Time since abandonment (days) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (ClampMin = "0"))
    int32 DaysAbandoned = 0;
};

/**
 * Archaeological Evidence and Story
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArchaeologicalEvidence
{
    GENERATED_BODY()

    /** Who lived here last */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FString LastInhabitants = TEXT("Unknown");

    /** Primary function/purpose */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Function")
    FString PrimaryFunction = TEXT("Dwelling");

    /** Signs of daily life */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evidence")
    TArray<FString> DailyLifeEvidence;

    /** Tools and artifacts present */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evidence")
    TArray<FString> ArtifactsPresent;

    /** Food remains/evidence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evidence")
    TArray<FString> FoodEvidence;

    /** Reason for abandonment */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FString AbandonmentReason = TEXT("Unknown");

    /** Cultural significance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culture")
    FString CulturalSignificance = TEXT("None");

    /** Estimated age (years) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dating", meta = (ClampMin = "0"))
    int32 EstimatedAge = 0;
};

/**
 * Architectural Structure Asset - Defines structure types and configurations
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchitecturalStructureAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Structure name */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FString StructureName = TEXT("Unknown Structure");

    /** Architectural period */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EArchitecturalPeriod Period = EArchitecturalPeriod::Paleolithic;

    /** Structure type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EStructureType StructureType = EStructureType::Hut;

    /** Primary construction material */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Construction")
    EConstructionMaterial PrimaryMaterial = EConstructionMaterial::Wood;

    /** Secondary materials */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Construction")
    TArray<EConstructionMaterial> SecondaryMaterials;

    /** Exterior mesh */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> ExteriorMesh;

    /** Interior mesh (if different) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> InteriorMesh;

    /** Damage variation meshes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    TArray<TSoftObjectPtr<UStaticMesh>> DamageVariations;

    /** Material instances for different conditions */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ConditionMaterials;

    /** Interior layout configuration */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interior")
    FInteriorLayout InteriorLayout;

    /** Structural integrity settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Integrity")
    FStructuralIntegrity DefaultIntegrity;

    /** Archaeological evidence */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Archaeology")
    FArchaeologicalEvidence ArchaeologicalData;

    /** Suitable biomes for this structure */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
    TArray<EBiomeType> SuitableBiomes;

    /** Preferred terrain slope (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D PreferredSlope = FVector2D(0.0f, 15.0f);

    /** Distance from water requirements */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D WaterDistanceRange = FVector2D(50.0f, 500.0f);

    /** Elevation range for placement */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
    FVector2D ElevationRange = FVector2D(0.0f, 1000.0f);

    /** Rarity (0.0 to 1.0, lower = rarer) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Rarity = 0.1f;

    /** Can be clustered with other structures */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
    bool bCanCluster = true;

    /** Minimum distance from other structures */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
    float MinDistanceFromOthers = 100.0f;

    /** Collision enabled */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    bool bEnableCollision = true;

    /** Can be entered by player */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bCanEnter = true;

    /** Has interactive elements */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool bHasInteractables = true;

    /** Cull distance */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float CullDistance = 15000.0f;

    /** Use Nanite rendering */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    bool bUseNanite = true;

    /** Cast shadows */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendering")
    bool bCastShadows = true;
};

/**
 * Architectural Structure Actor - Represents a built structure in the world
 * Each structure tells a story through its condition, contents, and archaeological evidence
 */
UCLASS()
class TRANSPERSONALGAME_API AArchitecturalStructure : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructure();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    /** Root scene component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    /** Exterior mesh component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ExteriorMeshComponent;

    /** Interior mesh component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> InteriorMeshComponent;

    /** Structure asset configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TSoftObjectPtr<UArchitecturalStructureAsset> StructureAsset;

    /** Current condition of the structure */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    EStructureCondition CurrentCondition = EStructureCondition::WellMaintained;

    /** Current structural integrity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    FStructuralIntegrity StructuralIntegrity;

    /** Archaeological evidence for this instance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archaeology")
    FArchaeologicalEvidence ArchaeologicalEvidence;

    /** Interior props spawned */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interior")
    TArray<TObjectPtr<AActor>> InteriorProps;

    /** Is currently occupied */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occupation")
    bool bIsOccupied = false;

    /** Current occupants (if any) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occupation")
    TArray<FString> CurrentOccupants;

    /** Initialize structure from asset */
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void InitializeFromAsset(UArchitecturalStructureAsset* Asset);

    /** Update structure condition based on time and environment */
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void UpdateStructuralCondition(float DeltaTime);

    /** Apply damage to structure */
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyDamage(float DamageAmount, const FString& DamageSource);

    /** Repair structure */
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void RepairStructure(float RepairAmount);

    /** Spawn interior props based on archaeological evidence */
    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SpawnInteriorProps();

    /** Clear interior props */
    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ClearInteriorProps();

    /** Get interior entrance points */
    UFUNCTION(BlueprintPure, Category = "Interior")
    TArray<FVector> GetEntrancePoints() const;

    /** Check if position is inside structure */
    UFUNCTION(BlueprintPure, Category = "Interior")
    bool IsPositionInside(const FVector& WorldPosition) const;

    /** Get archaeological story text */
    UFUNCTION(BlueprintPure, Category = "Archaeology")
    FString GetArchaeologicalStory() const;

    /** Set occupation status */
    UFUNCTION(BlueprintCallable, Category = "Occupation")
    void SetOccupationStatus(bool bOccupied, const TArray<FString>& Occupants);

    /** Get structure age in years */
    UFUNCTION(BlueprintPure, Category = "History")
    int32 GetStructureAge() const;

    /** Get cultural significance description */
    UFUNCTION(BlueprintPure, Category = "Culture")
    FString GetCulturalSignificance() const;

private:
    /** Update visual appearance based on condition */
    void UpdateVisualCondition();

    /** Generate random archaeological evidence */
    void GenerateArchaeologicalEvidence();

    /** Calculate natural deterioration */
    float CalculateNaturalDeterioration(float DeltaTime) const;

    /** Last update time for deterioration */
    float LastDeteriorationUpdate = 0.0f;

    /** Random seed for this structure instance */
    int32 StructureSeed = 0;
};