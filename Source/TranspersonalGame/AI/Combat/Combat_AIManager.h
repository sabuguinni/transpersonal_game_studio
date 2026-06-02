#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Combat_AIManager.generated.h"

class UCombat_TacticalFormationComponent;
class UCombat_ThreatAssessmentComponent;
class UCombat_DamageSystemComponent;

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle,
    Patrol,
    Alert,
    Hunting,
    Combat,
    Retreating,
    Flanking,
    Coordinating
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
struct FCombat_AITarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float Distance = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_FormationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat Formation")
    TArray<AActor*> FormationMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Formation")
    AActor* FormationLeader = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Formation")
    FVector FormationCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Formation")
    float FormationRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Formation")
    bool bIsActiveFormation = false;
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AIManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AIManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core AI State Management
    UPROPERTY(BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    ECombat_AIState CurrentAIState = ECombat_AIState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    ECombat_AIState PreviousAIState = ECombat_AIState::Idle;

    // Target Management
    UPROPERTY(BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    FCombat_AITarget PrimaryTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_AITarget> SecondaryTargets;

    // Formation and Coordination
    UPROPERTY(BlueprintReadWrite, Category = "Combat Formation", meta = (AllowPrivateAccess = "true"))
    FCombat_FormationData CurrentFormation;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Formation", meta = (AllowPrivateAccess = "true"))
    bool bUseFormationTactics = true;

    // Combat Parameters
    UPROPERTY(BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float DetectionRange = 1500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float AttackRange = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float FlankingDistance = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Parameters", meta = (AllowPrivateAccess = "true"))
    float RetreatThreshold = 0.3f;

    // AI Behavior Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Components", meta = (AllowPrivateAccess = "true"))
    UCombat_TacticalFormationComponent* FormationComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Components", meta = (AllowPrivateAccess = "true"))
    UCombat_ThreatAssessmentComponent* ThreatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Components", meta = (AllowPrivateAccess = "true"))
    UCombat_DamageSystemComponent* DamageComponent;

    // Core AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTargetAssessment();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void InitializeFormation(const TArray<AActor*>& Members, AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat Formation")
    void UpdateFormationPositions();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactics")
    void ExecuteCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void HandleDamageReceived(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_ThreatLevel GetCurrentThreatLevel() const;

private:
    // Internal state tracking
    float StateChangeTime = 0.0f;
    float LastTargetUpdateTime = 0.0f;
    float LastFormationUpdateTime = 0.0f;

    // Internal helper functions
    void UpdateAIStateMachine(float DeltaTime);
    void ScanForTargets();
    void EvaluateThreats();
    void UpdateCombatTactics();
    bool ShouldRetreat() const;
    FVector CalculateFlankingPosition(AActor* Target) const;
    void BroadcastFormationCommand(const FString& Command);
};