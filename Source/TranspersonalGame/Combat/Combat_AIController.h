#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "Combat_AIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None UMETA(DisplayName = "No Threat"),
    Low UMETA(DisplayName = "Low Threat"),
    Medium UMETA(DisplayName = "Medium Threat"),
    High UMETA(DisplayName = "High Threat"),
    Critical UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite UMETA(DisplayName = "Bite Attack"),
    Claw UMETA(DisplayName = "Claw Attack"),
    Charge UMETA(DisplayName = "Charge Attack"),
    Tail UMETA(DisplayName = "Tail Swipe"),
    Stomp UMETA(DisplayName = "Stomp Attack")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    TArray<AActor*> SecondaryTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    ECombat_ThreatLevel CurrentThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanSeePlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float DistanceToPlayer = 0.0f;
};

UCLASS()
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Perception System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* CombatBehaviorTree;

    // Combat Data
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombat_TacticalData TacticalData;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ChaseRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float PatrolRadius = 1000.0f;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetThreatLevel(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateTacticalData(float DeltaTime);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Tactical AI Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void CoordinatePackAttack(const TArray<ACombat_AIController*>& PackMembers);

    // Blackboard Keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName TargetActorKey = "TargetActor";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName LastKnownLocationKey = "LastKnownLocation";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName ThreatLevelKey = "ThreatLevel";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName IsInCombatKey = "IsInCombat";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName PatrolLocationKey = "PatrolLocation";

private:
    // Internal state
    float LastAttackTime = 0.0f;
    float CombatStateTimer = 0.0f;
    bool bHasValidTarget = false;
    
    // Internal functions
    void InitializePerception();
    void UpdateBlackboardData();
    void ProcessPerceptionData();
    AActor* SelectBestTarget(const TArray<AActor*>& PotentialTargets);
    float CalculateThreatScore(AActor* Target);
};