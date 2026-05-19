#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackHunt    UMETA(DisplayName = "Pack Hunt"),
    Ambush      UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanCallForHelp;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Patrol;
        ThreatLevel = ECombat_ThreatLevel::None;
        AggressionLevel = 0.5f;
        TerritoryRadius = 2000.0f;
        DetectionRange = 1500.0f;
        AttackRange = 300.0f;
        bIsPackLeader = false;
        bCanCallForHelp = true;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState GetTacticalState() const { return TacticalData.CurrentState; }

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel CalculateThreatLevel(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevel(ECombat_ThreatLevel NewLevel);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPackLeader(bool bIsLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForPackSupport();

    // Tactical maneuvers
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteRetreat();

    // Territory management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInTerritory(FVector Position);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void DefendTerritory();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void PatrolTerritory();

    // Combat execution
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteCombatAction();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget(AActor* Target);

    // Getters/Setters
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalData(const FCombat_TacticalData& NewData) { TacticalData = NewData; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_TacticalData GetTacticalData() const { return TacticalData; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float StateChangeTimer;

private:
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateStalkingBehavior(float DeltaTime);
    void UpdateAttackingBehavior(float DeltaTime);
    void UpdateRetreatingBehavior(float DeltaTime);
    void UpdateTerritorialBehavior(float DeltaTime);
    void UpdatePackHuntBehavior(float DeltaTime);
    void UpdateAmbushBehavior(float DeltaTime);

    AActor* FindNearestThreat();
    bool IsTargetVisible(AActor* Target);
    float GetDistanceToTarget(AActor* Target);
};

#include "Combat_TacticalAI.generated.h"