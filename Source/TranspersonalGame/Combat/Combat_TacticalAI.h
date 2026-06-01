#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Alert,
    Hunting,
    Attacking,
    Retreating,
    Coordinating
};

UENUM(BlueprintType)
enum class ECombat_AttackPattern : uint8
{
    Direct,
    Flanking,
    Ambush,
    PackHunt,
    Territorial
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackPattern PreferredPattern = ECombat_AttackPattern::Direct;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanCoordinate = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 1;

    FCombat_TacticalData()
    {
        AggressionLevel = 0.5f;
        FearThreshold = 0.3f;
        AttackRange = 500.0f;
        FleeRange = 200.0f;
        PreferredPattern = ECombat_AttackPattern::Direct;
        bCanCoordinate = false;
        PackSize = 1;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Combat))
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
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SelectAttackPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    // Target management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetPrimaryTarget() const { return PrimaryTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackMember(UCombat_TacticalAI* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemovePackMember(UCombat_TacticalAI* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<UCombat_TacticalAI*> GetPackMembers() const { return PackMembers; }

protected:
    // Core state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_TacticalState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_AttackPattern CurrentPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    FCombat_TacticalData TacticalData;

    // Target tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Target")
    AActor* PrimaryTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Target")
    float LastTargetDistance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Target")
    float TimeSinceLastSighting;

    // Pack coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    TArray<UCombat_TacticalAI*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    bool bIsPackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    UCombat_TacticalAI* PackLeader;

    // Timing
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
    float StateChangeTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    float AttackCooldown;

private:
    // Internal helper functions
    void UpdatePatrolBehavior();
    void UpdateAlertBehavior();
    void UpdateHuntingBehavior();
    void UpdateAttackingBehavior();
    void UpdateRetreatingBehavior();
    void UpdateCoordinatingBehavior();

    bool CanSeeTarget() const;
    bool IsTargetInRange() const;
    bool IsAttackOnCooldown() const;
    
    FVector CalculateFlankingPosition() const;
    FVector CalculateAmbushPosition() const;
    
    void BroadcastStateToPackMembers();
    void ReceivePackCommand(ECombat_TacticalState CommandState);
};