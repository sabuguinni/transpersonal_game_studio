#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle,
    Patrolling,
    Investigating,
    Stalking,
    Flanking,
    Attacking,
    Retreating,
    Regrouping
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombat_TacticalTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVisible = false;

    FCombat_TacticalTarget()
    {
        TargetActor = nullptr;
        LastKnownPosition = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        bIsVisible = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_FlankingPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Score = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOccupied = false;

    FCombat_FlankingPosition()
    {
        Position = FVector::ZeroVector;
        Score = 0.0f;
        bIsOccupied = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tactical State Management
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat Tactical AI")
    ECombat_TacticalState GetTacticalState() const { return CurrentTacticalState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    void AddTarget(AActor* Target, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    void RemoveTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    AActor* GetPrimaryTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    FVector GetTargetLastKnownPosition(AActor* Target) const;

    // Flanking System
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    FVector FindBestFlankingPosition(AActor* Target, float MinDistance = 500.0f, float MaxDistance = 1500.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    bool CanFlankTarget(AActor* Target) const;

    // Group Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    void RegisterWithGroup(UCombat_TacticalAI* GroupLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    void AddGroupMember(UCombat_TacticalAI* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    void CoordinateGroupAttack(AActor* Target);

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    float CalculateThreatLevel(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    ECombat_ThreatLevel GetOverallThreatLevel() const;

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    bool ShouldEngageTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical AI")
    FVector GetRetreatPosition() const;

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Tactical AI")
    ECombat_TacticalState CurrentTacticalState = ECombat_TacticalState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Tactical AI")
    TArray<FCombat_TacticalTarget> KnownTargets;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Tactical AI")
    TArray<FCombat_FlankingPosition> FlankingPositions;

    // Group Management
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Tactical AI")
    UCombat_TacticalAI* GroupLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Tactical AI")
    TArray<UCombat_TacticalAI*> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Tactical AI")
    bool bIsGroupLeader = false;

    // AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float HearingRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float FlankingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    bool bCanFlank = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    bool bCanRetreat = true;

    // Internal State
    UPROPERTY()
    float LastTargetUpdateTime = 0.0f;

    UPROPERTY()
    float StateChangeTime = 0.0f;

    UPROPERTY()
    FVector LastPlayerPosition = FVector::ZeroVector;

private:
    // Internal Methods
    void UpdateTargets(float DeltaTime);
    void UpdateTacticalState(float DeltaTime);
    void UpdateFlankingPositions();
    FCombat_TacticalTarget* FindTarget(AActor* Target);
    void CleanupInvalidTargets();
    bool IsValidFlankingPosition(const FVector& Position, AActor* Target) const;
    float CalculateFlankingScore(const FVector& Position, AActor* Target) const;
};