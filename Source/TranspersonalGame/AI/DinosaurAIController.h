#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinosaurAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackAlertRadius = 800.0f;
};

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // State machine
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinoState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_DinoState GetDinoState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AlertNearbyPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    // Species configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    FCombat_DinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComp;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    float LastAttackTime = 0.0f;
    float StateEnteredTime = 0.0f;

    // Patrol
    TArray<FVector> PatrolPoints;
    int32 CurrentPatrolIndex = 0;

    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateAlertedState(float DeltaTime);
    void UpdateChaseState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateFleeState(float DeltaTime);

    void GeneratePatrolPoints();
    void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void HandlePerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void ApplySpeciesDefaults();
};
