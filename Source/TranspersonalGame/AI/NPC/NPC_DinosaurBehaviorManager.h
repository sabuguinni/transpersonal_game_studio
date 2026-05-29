#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector LastPlayerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float TimeSincePlayerSeen;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsHostile;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex;

    FNPC_DinosaurMemory()
    {
        LastPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        bPlayerIsHostile = false;
        CurrentPatrolIndex = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Species Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ENPC_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MovementSpeed;

    // Behavior State
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    FNPC_DinosaurMemory Memory;

    // AI References
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class AAIController* AIController;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComp;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdatePlayerMemory(const FVector& PlayerLocation, bool bIsHostile);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void GeneratePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayerPresence();

private:
    void InitializeSpeciesTraits();
    void UpdateBehaviorLogic(float DeltaTime);
    void UpdateBlackboard();
    
    FVector HomeLocation;
    float StateTimer;
    APawn* CachedPlayer;
};