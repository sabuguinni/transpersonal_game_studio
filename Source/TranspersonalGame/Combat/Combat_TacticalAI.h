#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_CombatState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Hunting UMETA(DisplayName = "Hunting"),
    Attacking UMETA(DisplayName = "Attacking"),
    Defending UMETA(DisplayName = "Defending"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Circling UMETA(DisplayName = "Circling")
};

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Alpha UMETA(DisplayName = "Alpha Leader"),
    Flanker UMETA(DisplayName = "Flanker"),
    Distractor UMETA(DisplayName = "Distractor"),
    Defender UMETA(DisplayName = "Defender"),
    Scout UMETA(DisplayName = "Scout")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CirclingRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 3;

    FCombat_TacticalData()
    {
        AttackRange = 300.0f;
        FlankingDistance = 500.0f;
        CirclingRadius = 400.0f;
        RetreatThreshold = 0.3f;
        PackSize = 3;
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

    // Combat State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_CombatState CurrentState = ECombat_CombatState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_TacticalRole TacticalRole = ECombat_TacticalRole::Scout;

    // Tactical Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    float CoordinationRadius = 1000.0f;

    // Target and Pack Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat Runtime")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Runtime")
    TArray<UCombat_TacticalAI*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Runtime")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void JoinPack(UCombat_TacticalAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteTacticalManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatState();

protected:
    // Internal AI Logic
    void ProcessIdleState();
    void ProcessHuntingState();
    void ProcessAttackingState();
    void ProcessDefendingState();
    void ProcessFleeingState();
    void ProcessCirclingState();

    // Pack Coordination
    void CoordinateWithPack();
    void AssignTacticalRoles();
    UCombat_TacticalAI* FindPackLeader();

    // Utility Functions
    float GetDistanceToTarget() const;
    bool CanSeeTarget() const;
    FVector GetCirclingPosition() const;
    bool ShouldRetreat() const;

private:
    float StateTimer = 0.0f;
    float LastCoordinationTime = 0.0f;
    FVector InitialPosition = FVector::ZeroVector;
    bool bIsPackLeader = false;
};