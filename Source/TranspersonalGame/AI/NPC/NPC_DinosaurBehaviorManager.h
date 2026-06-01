#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "NPC_DinosaurBehaviorManager.generated.h"

class ANPC_DinosaurAI;
class UBehaviorTreeComponent;
class UBlackboardComponent;

USTRUCT(BlueprintType)
struct FNPC_DinosaurBehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurBehaviorType CurrentBehavior = ENPC_DinosaurBehaviorType::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector PatrolCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LastBehaviorChangeTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> SeenActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> InterestingLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerSightingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatLevel = 0.0f;
};

/**
 * NPC Behavior Manager - Controls dinosaur AI behavior states, memory, and decision making
 * Manages patrol routes, hunting behavior, combat states, and social interactions
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_DinosaurBehaviorType GetCurrentBehavior() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorLogic(float DeltaTime);

    // Patrol System
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetPatrolArea(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool IsInPatrolArea(FVector Location) const;

    // Target and Threat Detection
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    AActor* GetCurrentTarget() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanSeeActor(AActor* Actor) const;

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RememberActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RememberLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ForgetActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool HasSeenActor(AActor* Actor) const;

    // Combat Behavior
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void EnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ExitCombatMode();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool IsInCombat() const;

    // Species-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeSpeciesBehavior(ENPC_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateSpeciesLogic(float DeltaTime);

protected:
    // Core behavior state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    FNPC_DinosaurBehaviorState BehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    FNPC_DinosaurMemory Memory;

    // Species configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ENPC_DinosaurSpecies DinosaurSpecies = ENPC_DinosaurSpecies::TRex;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float ChaseRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float BehaviorChangeInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float MemoryDuration = 30.0f;

    // AI References
    UPROPERTY()
    class ANPC_DinosaurAI* OwnerAI;

    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY()
    UBlackboardComponent* BlackboardComponent;

private:
    // Internal behavior logic
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateChaseBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void UpdateFleeBehavior(float DeltaTime);

    // Species-specific logic
    void UpdateTRexBehavior(float DeltaTime);
    void UpdateVelociraptorBehavior(float DeltaTime);
    void UpdateTriceratopsBehavior(float DeltaTime);
    void UpdateBrachiosaurusBehavior(float DeltaTime);

    // Utility functions
    float GetDistanceToTarget() const;
    FVector GetRandomPatrolPoint() const;
    bool ShouldChangeTarget(AActor* NewTarget) const;
    void CleanupMemory();
};