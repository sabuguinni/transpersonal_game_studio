#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerTypes.h"
#include "Engine/DataTable.h"
#include "MassDinosaurSubsystem.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

/**
 * Species behavior patterns for dinosaur herds
 */
UENUM(BlueprintType)
enum class EDinosaurBehaviorPattern : uint8
{
    Grazing,        // Herbivores feeding in groups
    Hunting,        // Carnivores stalking prey
    Migrating,      // Moving between territories
    Nesting,        // Protecting eggs/young
    Territorial,    // Defending territory
    Scavenging,     // Feeding on carcasses
    Sleeping,       // Rest periods
    Drinking,       // At water sources
    Socializing     // Herd bonding behaviors
};

/**
 * Time of day activity patterns
 */
UENUM(BlueprintType)
enum class EDinosaurActivityTime : uint8
{
    Diurnal,        // Active during day
    Nocturnal,      // Active during night
    Crepuscular,    // Active at dawn/dusk
    Cathemeral      // Active throughout day/night
};

/**
 * Herd composition and size data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurHerdData
{
    GENERATED_BODY()

    // Species identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SpeciesName;

    // Minimum and maximum herd sizes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "500"))
    int32 MinHerdSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "500"))
    int32 MaxHerdSize = 15;

    // Territory radius in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1000", ClampMax = "50000"))
    float TerritoryRadius = 10000.0f;

    // Primary behavior pattern
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorPattern PrimaryBehavior = EDinosaurBehaviorPattern::Grazing;

    // Activity time preference
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurActivityTime ActivityTime = EDinosaurActivityTime::Diurnal;

    // How often they change behavior (in seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "30", ClampMax = "3600"))
    float BehaviorChangeInterval = 300.0f;

    // Movement speed multiplier for this species
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float MovementSpeedMultiplier = 1.0f;

    // Aggression level (0 = peaceful, 1 = highly aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel = 0.2f;

    // Fear response level (0 = fearless, 1 = very skittish)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevel = 0.5f;
};

/**
 * Data table row for dinosaur species configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurSpeciesTableRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDinosaurHerdData HerdData;

    // Mesh variations for visual diversity
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UStaticMesh>> MeshVariations;

    // Material variations for color diversity
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialVariations;

    // Scale variation range (min, max)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ScaleVariation = FVector2D(0.8f, 1.2f);

    // Animation blueprint for this species
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftClassPtr<UAnimInstance> AnimationBlueprint;
};

/**
 * Active herd instance data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FActiveHerdInstance
{
    GENERATED_BODY()

    // Unique herd identifier
    UPROPERTY(BlueprintReadOnly)
    FGuid HerdID;

    // Species this herd belongs to
    UPROPERTY(BlueprintReadOnly)
    FName SpeciesName;

    // Current herd center location
    UPROPERTY(BlueprintReadWrite)
    FVector HerdCenter;

    // Current behavior state
    UPROPERTY(BlueprintReadWrite)
    EDinosaurBehaviorPattern CurrentBehavior;

    // Time until next behavior change
    UPROPERTY(BlueprintReadWrite)
    float TimeToNextBehaviorChange;

    // Mass entity archetype for this herd
    UPROPERTY(BlueprintReadOnly)
    FMassArchetypeHandle ArchetypeHandle;

    // Entities belonging to this herd
    UPROPERTY(BlueprintReadOnly)
    TArray<FMassEntityHandle> HerdMembers;

    // Target location for movement
    UPROPERTY(BlueprintReadWrite)
    FVector TargetLocation;

    // Current herd state flags
    UPROPERTY(BlueprintReadWrite)
    uint32 bIsMoving : 1;

    UPROPERTY(BlueprintReadWrite)
    uint32 bIsAlarmed : 1;

    UPROPERTY(BlueprintReadWrite)
    uint32 bIsFeeding : 1;

    FActiveHerdInstance()
    {
        HerdID = FGuid::NewGuid();
        CurrentBehavior = EDinosaurBehaviorPattern::Grazing;
        TimeToNextBehaviorChange = 300.0f;
        bIsMoving = false;
        bIsAlarmed = false;
        bIsFeeding = false;
    }
};

/**
 * Subsystem managing all dinosaur crowd simulation
 * Handles spawning, behavior management, and lifecycle of dinosaur herds
 */
UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Herd management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    FGuid SpawnHerd(FName SpeciesName, FVector Location, int32 HerdSize = -1);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    bool DespawnHerd(FGuid HerdID);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void DespawnAllHerds();

    // Herd behavior control
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    bool SetHerdBehavior(FGuid HerdID, EDinosaurBehaviorPattern NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    bool MoveHerdToLocation(FGuid HerdID, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    bool AlarmHerd(FGuid HerdID, FVector ThreatLocation);

    // Query functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    TArray<FActiveHerdInstance> GetActiveHerds() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    FActiveHerdInstance GetHerdByID(FGuid HerdID) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    TArray<FActiveHerdInstance> GetHerdsInRadius(FVector Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    int32 GetTotalDinosaurCount() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SetSpeciesDataTable(UDataTable* NewDataTable);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SetMaxSimulationDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Simulation")
    void SetMaxActiveHerds(int32 MaxHerds);

protected:
    // Species configuration data table
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TSoftObjectPtr<UDataTable> SpeciesDataTable;

    // Maximum distance from player to simulate herds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1000", ClampMax = "100000"))
    float MaxSimulationDistance = 50000.0f;

    // Maximum number of active herds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxActiveHerds = 50;

    // Update frequency for herd behaviors (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float BehaviorUpdateFrequency = 1.0f;

private:
    // Active herd instances
    UPROPERTY()
    TMap<FGuid, FActiveHerdInstance> ActiveHerds;

    // Cached species data for quick access
    UPROPERTY()
    TMap<FName, FDinosaurSpeciesTableRow> CachedSpeciesData;

    // Mass entity subsystem reference
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    // Mass spawner subsystem reference
    UPROPERTY()
    TObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

    // Behavior update timer
    float BehaviorUpdateTimer = 0.0f;

    // Internal helper functions
    void UpdateHerdBehaviors(float DeltaTime);
    void UpdateHerdPositions(float DeltaTime);
    void CullDistantHerds();
    void LoadSpeciesDataFromTable();
    FMassArchetypeHandle CreateHerdArchetype(const FDinosaurSpeciesTableRow& SpeciesData);
    void SpawnHerdMembers(FActiveHerdInstance& Herd, const FDinosaurSpeciesTableRow& SpeciesData, int32 Count);
    FVector GetRandomLocationAroundPoint(FVector Center, float Radius) const;
    EDinosaurBehaviorPattern SelectNextBehavior(const FActiveHerdInstance& Herd, const FDinosaurSpeciesTableRow& SpeciesData) const;
    bool IsLocationSafeForHerd(FVector Location, const FDinosaurSpeciesTableRow& SpeciesData) const;
    void HandleHerdInteractions();
    void ProcessHerdAlarms(float DeltaTime);
};