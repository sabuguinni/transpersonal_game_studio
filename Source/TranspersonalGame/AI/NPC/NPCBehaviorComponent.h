#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunt UMETA(DisplayName = "Hunt"),
    Chase UMETA(DisplayName = "Chase"),
    Attack UMETA(DisplayName = "Attack"),
    Flee UMETA(DisplayName = "Flee"),
    Feed UMETA(DisplayName = "Feed"),
    Rest UMETA(DisplayName = "Rest"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Aggression = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Curiosity = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Territorial = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PackInstinct = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HuntingSkill = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearThreshold = 40.0f;
};

USTRUCT(BlueprintType)
struct FNPC_PatrolData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTimeAtPoint = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bRandomPatrol = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_DinosaurSpecies DinosaurSpecies = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_BehaviorStats BehaviorStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_PatrolData PatrolData;

    // Detection and Awareness
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float ChaseRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float LoseTargetTime = 10.0f;

    // Target Management
    UPROPERTY(BlueprintReadOnly, Category = "Target")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Target")
    float TimeSinceLastTargetSeen = 0.0f;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartChase(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartFlee();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool IsPlayerInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    AActor* FindNearestPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorBasedOnSpecies();

private:
    // Internal state management
    FVector LastKnownTargetLocation = FVector::ZeroVector;
    int32 CurrentPatrolIndex = 0;
    float StateChangeTimer = 0.0f;
    bool bIsInitialized = false;

    // Behavior update functions
    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateChaseBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void UpdateFleeBehavior(float DeltaTime);
    
    // Utility functions
    void InitializeSpeciesBehavior();
    FVector GetNextPatrolPoint();
    void MoveToLocation(const FVector& TargetLocation);
    bool HasReachedLocation(const FVector& TargetLocation, float Tolerance = 100.0f) const;
};