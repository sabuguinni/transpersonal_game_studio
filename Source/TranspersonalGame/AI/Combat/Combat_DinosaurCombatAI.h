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
#include "Components/StaticMeshComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Combat_DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunting UMETA(DisplayName = "Hunting"),
    Attacking UMETA(DisplayName = "Attacking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackHunting UMETA(DisplayName = "Pack Hunting")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    int32 PackSize = 1;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurCombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_DinosaurStats DinosaurStats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    float LastTargetSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsAlpha = false;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromThreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void PatrolTerritory();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializePackBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetVisible() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetupSpeciesStats();

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void UpdateCombatBehavior(float DeltaTime);
    void HandleIdleState();
    void HandlePatrolState();
    void HandleHuntingState();
    void HandleAttackingState();
    void HandleFleeingState();
    void HandleTerritorialState();
    void HandlePackHuntingState();

    bool ShouldAttackTarget() const;
    bool ShouldFleeFromTarget() const;
    bool IsInTerritory() const;
    FVector GetRandomPatrolPoint() const;
    void BroadcastToPackMembers(const FString& Message);

private:
    float StateTimer;
    float AttackCooldown;
    float LastAttackTime;
    bool bHasValidTarget;
    FVector InitialSpawnLocation;
};