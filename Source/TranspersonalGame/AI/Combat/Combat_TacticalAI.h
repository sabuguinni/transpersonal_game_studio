#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "../../Shared/SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunt UMETA(DisplayName = "Hunt"),
    Engage UMETA(DisplayName = "Engage"),
    Flank UMETA(DisplayName = "Flank"),
    Retreat UMETA(DisplayName = "Retreat"),
    Regroup UMETA(DisplayName = "Regroup")
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None UMETA(DisplayName = "None"),
    Line UMETA(DisplayName = "Line"),
    Circle UMETA(DisplayName = "Circle"),
    Pincer UMETA(DisplayName = "Pincer"),
    Ambush UMETA(DisplayName = "Ambush")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CoordinationSkill = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MaxPackSize = 6;

    FCombat_TacticalData()
    {
        AggressionLevel = 0.5f;
        CoordinationSkill = 0.7f;
        RetreatThreshold = 0.3f;
        FlankingRange = 1500.0f;
        MaxPackSize = 6;
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
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState GetTacticalState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(APawn* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    APawn* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(UCombat_TacticalAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInPack() const { return PackMembers.Num() > 1 || PackLeader != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFlankingManeuver(FVector TargetPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateAttack();

protected:
    // Tactical state management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_Formation CurrentFormation;

    // Target and pack management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    APawn* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombat_TacticalAI* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<UCombat_TacticalAI*> PackMembers;

    // Tactical data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalData;

    // Timing and coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float StateTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float LastCoordinationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CoordinationInterval = 2.0f;

private:
    // Internal tactical logic
    void UpdateTacticalBehavior(float DeltaTime);
    void ProcessHuntingBehavior();
    void ProcessEngagementBehavior();
    void ProcessFlankingBehavior();
    void ProcessRetreatBehavior();
    
    // Pack coordination
    void UpdatePackCoordination();
    void BroadcastTacticalInfo();
    void ReceiveTacticalInfo(const FVector& TargetPos, ECombat_TacticalState SuggestedState);
    
    // Utility functions
    bool CanSeeTarget() const;
    float GetDistanceToTarget() const;
    FVector CalculateFlankingPosition() const;
    bool ShouldRetreat() const;
};