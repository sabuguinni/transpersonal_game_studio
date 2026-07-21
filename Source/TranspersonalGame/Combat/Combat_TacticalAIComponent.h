#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAIComponent.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunt UMETA(DisplayName = "Hunt"),
    Engage UMETA(DisplayName = "Engage"),
    Flank UMETA(DisplayName = "Flank"),
    Retreat UMETA(DisplayName = "Retreat"),
    Ambush UMETA(DisplayName = "Ambush"),
    Coordinate UMETA(DisplayName = "Coordinate")
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None UMETA(DisplayName = "None"),
    Line UMETA(DisplayName = "Line"),
    Circle UMETA(DisplayName = "Circle"),
    Wedge UMETA(DisplayName = "Wedge"),
    Pincer UMETA(DisplayName = "Pincer"),
    Scatter UMETA(DisplayName = "Scatter")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    ECombat_Formation PreferredFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    float EngagementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    float RetreatThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    int32 PackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Data")
    float LastStateChangeTime;

    FCombat_TacticalData()
    {
        CurrentState = ECombat_TacticalState::Idle;
        PreferredFormation = ECombat_Formation::None;
        EngagementRange = 1000.0f;
        FlankingDistance = 500.0f;
        RetreatThreshold = 0.3f;
        PackSize = 1;
        bIsPackLeader = false;
        LastStateChangeTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAIComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical functions
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    ECombat_TacticalState GetTacticalState() const;

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalAssessment();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void ExecuteTacticalManeuver();

    // Formation management
    UFUNCTION(BlueprintCallable, Category = "Formation")
    void SetFormation(ECombat_Formation NewFormation);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    FVector GetFormationPosition(int32 MemberIndex);

    UFUNCTION(BlueprintCallable, Category = "Formation")
    void UpdateFormationPositions();

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void RegisterPackMember(UCombat_TacticalAIComponent* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void RemovePackMember(UCombat_TacticalAIComponent* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void BroadcastTacticalCommand(ECombat_TacticalState Command);

    // Threat assessment integration
    UFUNCTION(BlueprintCallable, Category = "Threat")
    void ProcessThreatData(const FVector& ThreatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Threat")
    bool ShouldEngageThreat(float ThreatLevel, float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Threat")
    FVector CalculateOptimalPosition(const FVector& ThreatLocation);

protected:
    // Tactical data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    class UBlackboardComponent* BlackboardComponent;

    // Pack members
    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<UCombat_TacticalAIComponent*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    UCombat_TacticalAIComponent* PackLeader;

    // Current target and threat
    UPROPERTY(BlueprintReadOnly, Category = "Target")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Target")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Target")
    float LastTargetUpdateTime;

    // Tactical parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
    float TacticalUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
    float FormationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
    float CoordinationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
    float StateChangeDelay;

private:
    // Internal state management
    float LastTacticalUpdate;
    float StateTimer;
    bool bInCombat;
    
    // Helper functions
    void UpdateBlackboardValues();
    void ProcessPackCoordination();
    void ExecuteStateLogic();
    bool CanChangeState() const;
    FVector GetFlankingPosition(const FVector& TargetLocation, bool bLeftFlank);
    void UpdateMovementTarget();
};