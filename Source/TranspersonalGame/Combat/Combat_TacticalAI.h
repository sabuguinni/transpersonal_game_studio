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
    Idle,
    Patrol,
    Hunt,
    Engage,
    Flank,
    Retreat,
    Regroup,
    Ambush
};

UENUM(BlueprintType)
enum class ECombat_FormationRole : uint8
{
    Alpha,
    Hunter,
    Scout,
    Support,
    Flanker
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    FVector ThreatLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    float LastSeenTime;

    UPROPERTY(BlueprintReadWrite, Category = "Threat")
    bool bIsHostile;

    FCombat_ThreatAssessment()
    {
        ThreatLevel = 0.0f;
        ThreatLocation = FVector::ZeroVector;
        ThreatActor = nullptr;
        LastSeenTime = 0.0f;
        bIsHostile = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_FormationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    ECombat_FormationRole Role;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    FVector RelativePosition;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    float PreferredDistance;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    bool bMaintainFormation;

    FCombat_FormationData()
    {
        Role = ECombat_FormationRole::Hunter;
        RelativePosition = FVector::ZeroVector;
        PreferredDistance = 300.0f;
        bMaintainFormation = true;
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

    // Core tactical functions
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateOptimalPosition();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteFormationMovement();

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(UCombat_TacticalAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void BroadcastThreat(const FCombat_ThreatAssessment& Threat);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void ReceiveThreatAlert(const FCombat_ThreatAssessment& Threat);

    // Combat decision making
    UFUNCTION(BlueprintCallable, Category = "Combat Decision")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Decision")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat Decision")
    ECombat_TacticalState DetermineBestTactic();

    // Formation management
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormationRole(ECombat_FormationRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void UpdateFormationPosition();

    UFUNCTION(BlueprintCallable, Category = "Formation")
    bool IsInFormation() const;

protected:
    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ECombat_TacticalState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float StateTimer;

    // Threat tracking
    UPROPERTY(BlueprintReadOnly, Category = "Threat")
    TArray<FCombat_ThreatAssessment> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Threat")
    FCombat_ThreatAssessment PrimaryThreat;

    // Pack coordination
    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    UCombat_TacticalAI* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<UCombat_TacticalAI*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsPackLeader;

    // Formation data
    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    FCombat_FormationData FormationData;

    UPROPERTY(BlueprintReadWrite, Category = "Formation")
    FVector FormationCenter;

    // AI parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float ThreatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float CourageThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float FormationTolerance;

private:
    // Internal state management
    void TransitionToState(ECombat_TacticalState NewState);
    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateHuntState(float DeltaTime);
    void UpdateEngageState(float DeltaTime);
    void UpdateFlankState(float DeltaTime);
    void UpdateRetreatState(float DeltaTime);
    void UpdateRegroupState(float DeltaTime);
    void UpdateAmbushState(float DeltaTime);

    // Threat management
    void CleanupOldThreats(float DeltaTime);
    float CalculateThreatLevel(AActor* Target);
    bool IsInLineOfSight(AActor* Target);

    // Pack communication
    void SendPackMessage(const FString& Message);
    void ProcessPackMessages();

    // Utility functions
    FVector GetRandomPatrolPoint();
    bool HasClearPath(const FVector& Destination);
    float GetDistanceToPackCenter();
};