#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "../SharedTypes.h"
#include "DinosaurBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public APawn
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DinosaurMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPawnSensingComponent* PawnSensing;

    // Dinosaur Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Dinosaur Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Stats")
    float AttackRange;

    // AI Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class AAIController* DinosaurAI;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    APawn* TargetPawn;

    // Dinosaur Type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Type")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Type")
    EDinosaurBehavior BehaviorType;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Territory")
    FVector PatrolTarget;

    // Pack Behavior (for raptors)
    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<ADinosaurBase*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsPackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ADinosaurBase* PackLeader;

public:
    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsInAttackRange();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsInDetectionRange(APawn* TestPawn);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "AI")
    FVector GetRandomPatrolPoint();

    // Pack Functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ADinosaurBase* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void AlertPack(APawn* Threat);

    // Health Functions
    UFUNCTION(BlueprintCallable, Category = "Health")
    void TakeDamage(float Damage);

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsAlive();

protected:
    // AI Event Functions
    UFUNCTION()
    void OnPawnSeen(APawn* SeenPawn);

    UFUNCTION()
    void OnPawnLost(APawn* LostPawn);

    // Internal Functions
    void InitializeAI();
    void UpdatePatrol();
    void UpdatePackBehavior();
    bool IsPlayerPawn(APawn* TestPawn);
};