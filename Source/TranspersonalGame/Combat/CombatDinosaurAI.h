#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "CombatDinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Raptor_Pack      UMETA(DisplayName = "Raptor Pack Hunter"),
    TRex_Apex        UMETA(DisplayName = "T-Rex Apex Predator"),
    Triceratops_Tank UMETA(DisplayName = "Triceratops Tank"),
    Ankylosaurus_Def UMETA(DisplayName = "Ankylosaurus Defensive")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Patrolling      UMETA(DisplayName = "Patrolling Territory"),
    Hunting         UMETA(DisplayName = "Active Hunting"),
    Attacking       UMETA(DisplayName = "Direct Attack"),
    Flanking        UMETA(DisplayName = "Flanking Maneuver"),
    Defending       UMETA(DisplayName = "Defensive Stance"),
    Retreating      UMETA(DisplayName = "Strategic Retreat"),
    PackCoordinate  UMETA(DisplayName = "Pack Coordination"),
    Territorial     UMETA(DisplayName = "Territory Defense")
};

USTRUCT(BlueprintType)
struct FCombat_TerritoryZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsPackTerritory;

    FCombat_TerritoryZone()
    {
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        AggressionLevel = 0.5f;
        bIsPackTerritory = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float CoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsCoordinating;

    FCombat_PackBehavior()
    {
        PackLeader = nullptr;
        CoordinationRadius = 800.0f;
        FlankingDistance = 300.0f;
        bIsCoordinating = false;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombatDinosaurAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombatDinosaurAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception", meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception", meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComponent;

public:
    // Dinosaur Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ECombat_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ECombat_AIState CurrentAIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FCombat_TerritoryZone TerritoryZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FCombat_PackBehavior PackBehavior;

    // Combat Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritorialRadius;

    // Current Target
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float DistanceToTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bCanSeeTarget;

public:
    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ACombatDinosaurAI* PackLeaderAI);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    FVector GetFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory();

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI")
    void OnTargetDetected(AActor* DetectedTarget);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI")
    void OnTargetLost();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI")
    void OnTerritoryInvaded(AActor* Invader);

private:
    void UpdateCombatState();
    void UpdatePackCoordination();
    void UpdateTerritorialBehavior();
    void CalculateFlankingPositions();
    
    float LastStateUpdateTime;
    float StateUpdateInterval;
};