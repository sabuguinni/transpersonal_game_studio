#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurBase.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex           UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor         UMETA(DisplayName = "Velociraptor"),
    Triceratops    UMETA(DisplayName = "Triceratops"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus    UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Patrol         UMETA(DisplayName = "Patrol"),
    Hunt           UMETA(DisplayName = "Hunt"),
    Chase          UMETA(DisplayName = "Chase"),
    Attack         UMETA(DisplayName = "Attack"),
    Flee           UMETA(DisplayName = "Flee"),
    Feed           UMETA(DisplayName = "Feed"),
    Sleep          UMETA(DisplayName = "Sleep"),
    Territorial    UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 0.5f; // 0.0 = Passive, 1.0 = Highly Aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    bool bIsCarnivore = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_DinosaurBase : public APawn
{
    GENERATED_BODY()

public:
    ANPC_DinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DinosaurMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPawnSensingComponent* PawnSensing;

    // Dinosaur Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // Territory and Navigation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory")
    int32 CurrentPatrolIndex;

    // Target and Threat Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    AActor* LastKnownThreat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    float LastThreatTime;

    // Pack Behavior (for pack hunters like Raptors)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    TArray<ANPC_DinosaurBase*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    ANPC_DinosaurBase* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack")
    bool bIsPackLeader;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsTargetInRange(AActor* Target, float Range) const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsInTerritory(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void GeneratePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetNextPatrolPoint();

    // Pack Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ANPC_DinosaurBase* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void BroadcastToPackMembers(const FString& Message);

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target) const;

    // Perception Callbacks
    UFUNCTION()
    void OnPawnSeen(APawn* SeenPawn);

    UFUNCTION()
    void OnPawnLost(APawn* LostPawn);

protected:
    // Internal AI Logic
    void UpdateAI(float DeltaTime);
    void ProcessIdleState(float DeltaTime);
    void ProcessPatrolState(float DeltaTime);
    void ProcessHuntState(float DeltaTime);
    void ProcessChaseState(float DeltaTime);
    void ProcessAttackState(float DeltaTime);
    void ProcessFleeState(float DeltaTime);

    // Utility Functions
    FVector GetRandomPointInTerritory() const;
    bool HasLineOfSightToTarget(AActor* Target) const;
    float GetDistanceToTarget(AActor* Target) const;

private:
    // Timers
    float StateTimer;
    float LastAttackTime;
    float PatrolWaitTime;
    
    // AI Constants
    static constexpr float ATTACK_COOLDOWN = 2.0f;
    static constexpr float PATROL_WAIT_TIME = 3.0f;
    static constexpr float THREAT_MEMORY_TIME = 10.0f;
};