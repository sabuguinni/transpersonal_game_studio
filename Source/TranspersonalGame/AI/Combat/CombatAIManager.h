#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "AI/Navigation/NavigationTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "TranspersonalGame/SharedTypes.h"
#include "CombatAIManager.generated.h"

class APawn;
class AAIController;
class UBehaviorTreeComponent;
class UBlackboardComponent;

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee"),
    Hunt        UMETA(DisplayName = "Hunt")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackCoordination = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HelpRadius = 1000.0f;

    FCombat_TacticalData()
    {
        AttackRange = 300.0f;
        FleeThreshold = 0.3f;
        AggressionLevel = 0.5f;
        PackCoordination = 0.7f;
        bCanCallForHelp = true;
        HelpRadius = 1000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatAI(AAIController* AIController);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetThreatLevel(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalSituation();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalDecision();

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel AssessThreat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterThreat(AActor* ThreatActor, ECombat_ThreatLevel ThreatLevel);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForHelp(const FVector& Location, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RespondToHelpCall(const FVector& Location, AActor* Caller);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> FindNearbyAllies(float SearchRadius);

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlee(const FVector& DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlank(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAmbush(const FVector& AmbushLocation);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_AIState GetCurrentAIState() const { return CurrentAIState; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

protected:
    // Core Properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentAIState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    AAIController* OwnerAIController;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UBlackboardComponent* BlackboardComponent;

    // Threat Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> KnownThreats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<AActor*> PackMembers;

    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatScanInterval = 0.5f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float LastThreatScanTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TacticalUpdateInterval = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float LastTacticalUpdateTime;

private:
    // Internal Helper Functions
    void UpdateBlackboardValues();
    bool IsValidThreat(AActor* PotentialThreat) const;
    FVector CalculateFlankPosition(AActor* Target);
    FVector CalculateFleeDirection(const FVector& DangerLocation);
    void CleanupInvalidThreats();
};