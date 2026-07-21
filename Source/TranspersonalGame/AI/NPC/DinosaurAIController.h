#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinosaurAIController.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FNPC_DinoPerceptionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 4500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float AttackRadius = 350.0f;
};

UCLASS(ClassGroup = "TranspersonalAI", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaTime) override;

    // --- Blackboard Keys ---
    static const FName BB_TargetActor;
    static const FName BB_PatrolLocation;
    static const FName BB_HomeLocation;
    static const FName BB_AIState;
    static const FName BB_bPlayerDetected;
    static const FName BB_bIsAggressive;
    static const FName BB_DistanceToTarget;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "AI|State", meta = (AllowPrivateAccess = "true"))
    ENPC_DinoAIState CurrentAIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    FNPC_DinoPerceptionConfig PerceptionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float PatrolWaitTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
    UBehaviorTree* BehaviorTree;

    // --- Public Interface ---
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetAIState(ENPC_DinoAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    ENPC_DinoAIState GetAIState() const { return CurrentAIState; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    void OnTargetDetected(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void OnTargetLost();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ForcePatrol();

protected:
    UPROPERTY()
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY()
    UAISenseConfig_Hearing* HearingConfig;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void SetupPerception();
    void UpdateBlackboard();
    void ChooseNextPatrolPoint();

    FVector HomeLocation;
    FVector CurrentPatrolTarget;
    float PatrolWaitTimer;
    bool bWaitingAtPatrolPoint;
};
