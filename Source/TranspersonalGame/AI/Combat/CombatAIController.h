#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTags/GameplayTags.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UPawnSensingComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Fleeing,
    Territorial,
    Feeding,
    Investigating
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Extreme
};

USTRUCT(BlueprintType)
struct FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastSeen;

    UPROPERTY(BlueprintReadWrite)
    EThreatLevel PlayerThreatLevel;

    UPROPERTY(BlueprintReadWrite)
    bool bPlayerIsHiding;

    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> SearchLocations;

    UPROPERTY(BlueprintReadWrite)
    float LastDamageTime;

    UPROPERTY(BlueprintReadWrite)
    FVector LastDamageLocation;

    FCombatMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSinceLastSeen = 0.0f;
        PlayerThreatLevel = EThreatLevel::None;
        bPlayerIsHiding = false;
        LastDamageTime = 0.0f;
        LastDamageLocation = FVector::ZeroVector;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Perception Component
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState;

    // Combat Memory
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombatMemory CombatMemory;

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName PlayerActorKey;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName LastKnownLocationKey;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName CombatStateKey;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName ThreatLevelKey;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName PatrolLocationKey;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName IsPlayerVisibleKey;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdatePlayerMemory(AActor* Player, bool bIsVisible);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTakeDamage(float Damage, FVector DamageLocation);

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetBestAttackPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    EThreatLevel CalculateThreatLevel();

private:
    void InitializePerception();
    void UpdateCombatMemory(float DeltaTime);
    void ProcessTacticalDecisions();
};