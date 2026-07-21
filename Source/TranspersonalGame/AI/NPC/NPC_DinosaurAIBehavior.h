#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "NPC_DinosaurAIBehavior.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"), 
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    Beta        UMETA(DisplayName = "Beta"),
    Scout       UMETA(DisplayName = "Scout"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Lone        UMETA(DisplayName = "Lone")
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bIsRestPoint;

    FNPC_PatrolPoint()
    {
        Location = FVector::ZeroVector;
        WaitTime = 5.0f;
        bIsRestPoint = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerSightTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> ThreatLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastFeedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeTerritory;

    FNPC_DinosaurMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        LastPlayerSightTime = 0.0f;
        LastFeedTime = 0.0f;
        HomeTerritory = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurAIBehavior : public AAIController
{
    GENERATED_BODY()

public:
    UNPC_DinosaurAIBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Possess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ESpeciesType DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_PackRole PackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurMemory DinosaurMemory;

    // Patrol System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector TerritoryCenter;

    // Detection Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float ChaseRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float AttackRadius;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<UNPC_DinosaurAIBehavior*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    UNPC_DinosaurAIBehavior* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCohesionRadius;

public:
    // State Management
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    ENPC_DinosaurState GetDinosaurState() const { return CurrentState; }

    // Patrol Functions
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetupPatrolRoute(const TArray<FVector>& Points, float Radius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void GenerateRandomPatrolPoints(int32 NumPoints = 5);

    // Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    void UpdatePlayerMemory(const FVector& PlayerLocation);

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(UNPC_DinosaurAIBehavior* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void FormPack(const TArray<UNPC_DinosaurAIBehavior*>& Members);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    FVector GetPackCenterLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(const FVector& TargetLocation);

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFleeing(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReturnToTerritory();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsInTerritory() const;

protected:
    // AI Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Functions
    void InitializeAIPerception();
    void SetupBlackboardValues();
    void UpdateBehaviorState(float DeltaTime);
    void ProcessPackBehavior(float DeltaTime);
    
    // Timers
    float StateChangeTimer;
    float MemoryUpdateTimer;
    float PackCoordinationTimer;
};