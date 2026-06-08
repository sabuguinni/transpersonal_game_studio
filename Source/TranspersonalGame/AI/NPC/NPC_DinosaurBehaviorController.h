#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "NPC_DinosaurBehaviorController.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing UMETA(DisplayName = "Chasing"),
    Attacking UMETA(DisplayName = "Attacking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Grazing UMETA(DisplayName = "Grazing"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    PackHunting UMETA(DisplayName = "Pack Hunting"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Raptor UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerSightTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 PackSize;

    FNPC_DinosaurMemory()
    {
        LastPlayerLocation = FVector::ZeroVector;
        LastPlayerSightTime = 0.0f;
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
        AggressionLevel = 0.5f;
        FearLevel = 0.0f;
        PackSize = 1;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ANPC_DinosaurBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* PerceptionComponent;

    // Dinosaur Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float ChaseRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float AttackRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    float ChaseSpeed;

    // Behavior State
    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    FNPC_DinosaurMemory DinosaurMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    class ACharacter* TargetPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    float StateTimer;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<ANPC_DinosaurBehaviorController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ANPC_DinosaurBehaviorController* PackLeader;

public:
    // State Management
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    ENPC_DinosaurState GetDinosaurState() const { return CurrentState; }

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnPlayerSighted(class ACharacter* Player);

    UFUNCTION()
    void OnPlayerLost();

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetRandomPatrolPoint() const;

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ANPC_DinosaurBehaviorController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(FVector TargetLocation);

    // Species-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void InitializeTRexBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void InitializeRaptorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void InitializeHerbivoreBehavior();

protected:
    // Internal State Logic
    void UpdateBehaviorState(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleChasingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleGrazingState(float DeltaTime);

    // Utility Functions
    float GetDistanceToPlayer() const;
    bool CanSeePlayer() const;
    void UpdateMemory();
    void ConfigurePerceptionForSpecies();
};