#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsHostile = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float AggressionLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatStance CurrentStance = ECombatStance::Neutral;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float CombatReadiness = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> HostileTargets;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastCombatTime = 0.0f;
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core tactical functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnterCombatMode(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExitCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateTacticalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalPosition(AActor* Target, float PreferredDistance = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatStance(ECombatStance NewStance);

    // Pack coordination for group hunters
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinateWithPack(const TArray<AActor*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(AActor* Target, const TArray<AActor*>& PackMembers);

    // Threat assessment
    UFUNCTION(BlueprintPure, Category = "Combat")
    FCombat_ThreatAssessment GetThreatAssessment(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInCombat() const { return TacticalState.bInCombat; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetPrimaryTarget() const { return TacticalState.PrimaryTarget; }

protected:
    // Tactical state
    UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FCombat_TacticalState TacticalState;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float ThreatDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float CombatEngagementRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float PreferredCombatDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AggressionThreshold = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float CombatCooldownTime = 10.0f;

    // Pack behavior
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bUsePackTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float PackCoordinationRadius = 2000.0f;

private:
    // Internal threat tracking
    TMap<AActor*, FCombat_ThreatAssessment> ThreatMap;
    
    // Tactical calculations
    FVector CalculateOptimalAttackVector(AActor* Target) const;
    bool IsPositionSafe(const FVector& Position, AActor* Target) const;
    float CalculateThreatLevel(AActor* Target) const;
    void UpdateThreatAssessments(float DeltaTime);
    void CleanupInvalidThreats();
};