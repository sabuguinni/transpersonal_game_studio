#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "MassSpawnerTypes.h"
#include "DinosaurHerdManager.generated.h"

class UMassEntitySubsystem;
class UMassSpawnerSubsystem;

UENUM(BlueprintType)
enum class EDinosaurHerdType : uint8
{
    Herbivore_Small,    // Compsognathus, Dryosaurus
    Herbivore_Medium,   // Parasaurolophus, Triceratops
    Herbivore_Large,    // Brontosaurus, Diplodocus
    Carnivore_Pack,     // Velociraptor, Deinonychus
    Carnivore_Solo,     // T-Rex, Allosaurus
    Flying_Small,       // Pteranodon groups
    Flying_Large        // Quetzalcoatlus
};

USTRUCT(BlueprintType)
struct FDinosaurHerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurHerdType HerdType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinHerdSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHerdSize = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DangerResponseRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFlockingBehavior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAvoidsPredators = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurHerdType> PredatorTypes;
};

USTRUCT(BlueprintType)
struct FDinosaurHerdInstance
{
    GENERATED_BODY()

    UPROPERTY()
    FMassEntityHandle LeaderEntity;

    UPROPERTY()
    TArray<FMassEntityHandle> MemberEntities;

    UPROPERTY()
    FVector CurrentDestination;

    UPROPERTY()
    FVector HomeTerritory;

    UPROPERTY()
    float TerritoryRadius;

    UPROPERTY()
    EDinosaurHerdType HerdType;

    UPROPERTY()
    float LastActivityTime;

    UPROPERTY()
    bool bInDanger;

    UPROPERTY()
    FVector DangerSource;
};

/**
 * Manages dinosaur herds using Mass Entity framework
 * Creates living ecosystems where dinosaurs have natural behaviors independent of player
 */
UCLASS()
class TRANSPERSONALGAME_API ADinosaurHerdManager : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurHerdManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd Configuration")
    TArray<FDinosaurHerdConfig> HerdConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxActiveHerds = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float DespawnRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxEntitiesPerFrame = 1000;

private:
    UPROPERTY()
    TArray<FDinosaurHerdInstance> ActiveHerds;

    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    float LastUpdateTime;
    int32 CurrentUpdateIndex;

    // Core herd management
    void UpdateHerdBehaviors(float DeltaTime);
    void SpawnNewHerds();
    void DespawnDistantHerds();
    void UpdateHerdMovement(FDinosaurHerdInstance& Herd, float DeltaTime);
    void UpdateHerdDanger(FDinosaurHerdInstance& Herd);

    // Flocking algorithms
    FVector CalculateCohesion(const FDinosaurHerdInstance& Herd);
    FVector CalculateSeparation(const FDinosaurHerdInstance& Herd);
    FVector CalculateAlignment(const FDinosaurHerdInstance& Herd);
    FVector CalculateAvoidance(const FDinosaurHerdInstance& Herd);

    // Territory and pathfinding
    FVector FindNewDestination(const FDinosaurHerdInstance& Herd);
    bool IsLocationSafe(const FVector& Location, EDinosaurHerdType HerdType);
    FVector GetRandomLocationInTerritory(const FDinosaurHerdInstance& Herd);

    // Interaction systems
    void HandlePredatorPreyInteraction();
    void HandlePlayerPresence();
    void TriggerHerdPanicResponse(FDinosaurHerdInstance& Herd, const FVector& DangerSource);

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void SpawnHerdAtLocation(EDinosaurHerdType HerdType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void DespawnHerd(int32 HerdIndex);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    TArray<FDinosaurHerdInstance> GetNearbyHerds(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Herd Management")
    void SetPlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawHerds();
};