#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackHunting UMETA(DisplayName = "Pack Hunting"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritorialRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HuntingRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 MaxPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsNocturnal = false;

    FNPC_DinosaurBehaviorProfile()
    {
        Species = ENPC_DinosaurSpecies::TRex;
        AggressionLevel = 0.8f;
        TerritorialRadius = 5000.0f;
        HuntingRange = 3000.0f;
        FleeThreshold = 0.3f;
        bIsPackAnimal = false;
        MaxPackSize = 1;
        MovementSpeed = 600.0f;
        DetectionRadius = 2000.0f;
        bIsNocturnal = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_PackBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    FVector PackCenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesion = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* HuntTarget = nullptr;

    FNPC_PackBehaviorData()
    {
        PackLeader = nullptr;
        PackCenterLocation = FVector::ZeroVector;
        PackCohesion = 1.0f;
        bIsHunting = false;
        HuntTarget = nullptr;
    }
};

/**
 * Advanced dinosaur behavior tree system with species-specific AI patterns
 * Handles territorial behavior, pack dynamics, hunting strategies, and environmental adaptation
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_DinosaurBehaviorProfile BehaviorProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    FNPC_PackBehaviorData PackData;

    // Behavior tree references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardComponent* BlackboardComponent = nullptr;

    // Territory and environment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    // Hunting and feeding
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float LastFeedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float HungerLevel = 0.5f;

    // Memory and awareness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownPrey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float AlertLevel = 0.0f;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeBehaviorProfile(ENPC_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void AddToPackBehavior(AActor* NewPackMember);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void RemoveFromPackBehavior(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* FindNearestPrey(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* FindNearestThreat(float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateHungerLevel(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool ShouldHunt() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ExecutePackHuntingStrategy();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdatePackCohesion();

private:
    void UpdateBehaviorState(float DeltaTime);
    void ProcessEnvironmentalAwareness();
    void HandleTerritorialBehavior();
    void HandlePackBehavior();
    void UpdateMemorySystem(float DeltaTime);
    
    // Species-specific behavior initializers
    void InitializeTRexBehavior();
    void InitializeRaptorBehavior();
    void InitializeHerbivoreBehavior();
    
    float LastBehaviorUpdate = 0.0f;
    float BehaviorUpdateInterval = 0.5f;
};