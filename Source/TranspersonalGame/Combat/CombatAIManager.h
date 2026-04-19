#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "../SharedTypes.h"
#include "CombatAIManager.generated.h"

class APawn;
class ACharacter;
class UCombat_TacticalAIComponent;

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TimeSinceLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsHostile;

    FCombat_ThreatAssessment()
    {
        ThreatLevel = 0.0f;
        Distance = 0.0f;
        LastKnownPosition = FVector::ZeroVector;
        TimeSinceLastSeen = 0.0f;
        bIsHostile = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalMode CurrentMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> AlliedUnits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> EnemyTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    AActor* PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float GroupCohesion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector GroupCenter;

    FCombat_TacticalState()
    {
        CurrentMode = ECombat_TacticalMode::Patrol;
        PrimaryTarget = nullptr;
        GroupCohesion = 1.0f;
        GroupCenter = FVector::ZeroVector;
    }
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatAIManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical AI functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatAI();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssessThreat(AActor* PotentialThreat, FCombat_ThreatAssessment& OutAssessment);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SelectTacticalMode();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateGroupTactics();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterPackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdatePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target, int32 FlankIndex);

    // Threat management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevels(float DeltaTime);

    // Combat state queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    bool IsInCombat() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    ECombat_TacticalMode GetCurrentTacticalMode() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    float GetGroupCohesion() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat AI")
    FVector GetOptimalAttackPosition(AActor* Target) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    FCombat_TacticalState TacticalState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TMap<AActor*, FCombat_ThreatAssessment> ThreatMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TacticalUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxEngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MinFlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float GroupFormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 MaxPackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnablePackTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnableFlankingManeuvers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnableAmbushTactics;

private:
    float LastTacticalUpdate;
    float ThreatDecayRate;
    
    void CalculateGroupCenter();
    void UpdateGroupCohesion();
    ECombat_TacticalManeuver SelectOptimalManeuver(AActor* Target) const;
    bool CanExecuteManeuver(ECombat_TacticalManeuver Maneuver) const;
};