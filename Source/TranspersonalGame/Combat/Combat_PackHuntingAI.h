#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_PackHuntingAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Beta        UMETA(DisplayName = "Beta Flanker"),
    Gamma       UMETA(DisplayName = "Gamma Flanker"),
    Scout       UMETA(DisplayName = "Scout"),
    Ambusher    UMETA(DisplayName = "Ambusher")
};

UENUM(BlueprintType)
enum class ECombat_HuntPhase : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Positioning UMETA(DisplayName = "Positioning"),
    Coordinated UMETA(DisplayName = "Coordinated Attack"),
    Retreat     UMETA(DisplayName = "Retreat")
};

USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    TWeakObjectPtr<APawn> MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    ECombat_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    FVector AssignedPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float DistanceFromTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    bool bInPosition;

    FCombat_PackMember()
    {
        MemberPawn = nullptr;
        Role = ECombat_PackRole::Beta;
        AssignedPosition = FVector::ZeroVector;
        DistanceFromTarget = 0.0f;
        bInPosition = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    TWeakObjectPtr<APawn> TargetPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float TimeSinceLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    bool bIsValidTarget;

    FCombat_HuntTarget()
    {
        TargetPawn = nullptr;
        LastKnownPosition = FVector::ZeroVector;
        ThreatLevel = 1.0f;
        TimeSinceLastSeen = 0.0f;
        bIsValidTarget = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PackHuntingAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PackHuntingAI();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void InitializeAsPackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void JoinPack(UCombat_PackHuntingAI* PackLeader, ECombat_PackRole AssignedRole);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AddPackMember(APawn* NewMember, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RemovePackMember(APawn* Member);

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void SetHuntTarget(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ClearHuntTarget();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    APawn* GetCurrentTarget() const;

    // Hunt Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void StartCoordinatedHunt();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ExecutePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void OrderRetreat();

    // Position Calculation
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    FVector CalculateFlankingPosition(ECombat_PackRole Role, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    bool IsPackInPosition() const;

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void BroadcastToPackMembers(const FString& Command);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ReceivePackCommand(const FString& Command, UCombat_PackHuntingAI* Sender);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    bool IsPackLeader() const { return bIsPackLeader; }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    ECombat_PackRole GetPackRole() const { return MyPackRole; }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    ECombat_HuntPhase GetCurrentHuntPhase() const { return CurrentHuntPhase; }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    int32 GetPackSize() const { return PackMembers.Num(); }

protected:
    // Pack Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    ECombat_PackRole MyPackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    TWeakObjectPtr<UCombat_PackHuntingAI> PackLeaderRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    TArray<FCombat_PackMember> PackMembers;

    // Hunt State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    ECombat_HuntPhase CurrentHuntPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    FCombat_HuntTarget CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    FVector AssignedHuntPosition;

    // Hunt Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float CoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float AttackSignalDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float MaxHuntDuration;

    // Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float HuntStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float LastCoordinationUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    float PositionUpdateInterval;

private:
    // Internal Methods
    void UpdateHuntPhase(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    void UpdateTargetTracking(float DeltaTime);
    void CalculatePackPositions();
    bool ValidateTarget();
    void HandlePhaseTransition(ECombat_HuntPhase NewPhase);
    void ExecuteRoleBasedBehavior(float DeltaTime);
    FVector GetOptimalAttackVector() const;
    bool CanExecuteCoordinatedAttack() const;
    void CleanupInvalidMembers();
};

#include "Combat_PackHuntingAI.generated.h"