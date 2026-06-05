#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Patrolling   UMETA(DisplayName = "Patrolling"),
    Hunting      UMETA(DisplayName = "Hunting"),
    Fleeing      UMETA(DisplayName = "Fleeing"),
    Feeding      UMETA(DisplayName = "Feeding"),
    Sleeping     UMETA(DisplayName = "Sleeping"),
    Territorial  UMETA(DisplayName = "Territorial"),
    PackHunting  UMETA(DisplayName = "Pack Hunting")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex         UMETA(DisplayName = "T-Rex"),
    Raptor       UMETA(DisplayName = "Raptor"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus  UMETA(DisplayName = "Stegosaurus"),
    Pteranodon   UMETA(DisplayName = "Pteranodon")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurBehaviorState CurrentState = ENPC_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritorialRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector PatrolTarget = FVector::ZeroVector;

    FNPC_DinosaurBehaviorData()
    {
        CurrentState = ENPC_DinosaurBehaviorState::Idle;
        Species = ENPC_DinosaurSpecies::TRex;
        AggressionLevel = 0.5f;
        HungerLevel = 0.3f;
        FearLevel = 0.1f;
        TerritorialRadius = 5000.0f;
        DetectionRadius = 3000.0f;
        AttackRadius = 300.0f;
        bIsPackAnimal = false;
        PackLeader = nullptr;
        CurrentTarget = nullptr;
        HomeLocation = FVector::ZeroVector;
        PatrolTarget = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void ConfigureForSpecies(ENPC_DinosaurSpecies InSpecies);

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void JoinPack(AActor* Leader, const TArray<AActor*>& Members);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdatePackBehavior();

    // Target management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* FindNearestThreat(float SearchRadius = 3000.0f);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    AActor* FindNearestPrey(float SearchRadius = 3000.0f);

    // Patrol behavior
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetHomeLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GeneratePatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsWithinTerritory(const FVector& Location);

    // State queries
    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    ENPC_DinosaurBehaviorState GetCurrentState() const { return BehaviorData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsHungry() const { return BehaviorData.HungerLevel > 0.7f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsAggressive() const { return BehaviorData.AggressionLevel > 0.6f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsFearful() const { return BehaviorData.FearLevel > 0.5f; }

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool HasTarget() const { return BehaviorData.CurrentTarget != nullptr; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Data")
    FNPC_DinosaurBehaviorData BehaviorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species Config")
    TMap<ENPC_DinosaurSpecies, class UBehaviorTree*> SpeciesBehaviorTrees;

private:
    void UpdateBehaviorStats(float DeltaTime);
    void UpdateBlackboardValues();
    void HandleStateTransitions();
    
    float LastUpdateTime = 0.0f;
    float StateChangeTimer = 0.0f;
};