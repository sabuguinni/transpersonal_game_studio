#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "Combat_PackHuntingAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha UMETA(DisplayName = "Alpha Leader"),
    Beta UMETA(DisplayName = "Beta Second"),
    Hunter UMETA(DisplayName = "Hunter"),
    Flanker UMETA(DisplayName = "Flanker"),
    Scout UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class ECombat_HuntPhase : uint8
{
    Patrol UMETA(DisplayName = "Patrolling Territory"),
    Tracking UMETA(DisplayName = "Tracking Prey"),
    Positioning UMETA(DisplayName = "Positioning for Attack"),
    Coordinated_Attack UMETA(DisplayName = "Coordinated Attack"),
    Feeding UMETA(DisplayName = "Feeding"),
    Retreat UMETA(DisplayName = "Retreat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    TWeakObjectPtr<APawn> MemberPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    ECombat_PackRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    FVector AssignedPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    bool bIsInPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Member")
    float DistanceToTarget;

    FCombat_PackMember()
    {
        MemberPawn = nullptr;
        Role = ECombat_PackRole::Hunter;
        AssignedPosition = FVector::ZeroVector;
        bIsInPosition = false;
        DistanceToTarget = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    TWeakObjectPtr<APawn> TargetPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FVector LastKnownPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float EstimatedHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    bool bIsValidTarget;

    FCombat_HuntTarget()
    {
        TargetPawn = nullptr;
        LastKnownPosition = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        EstimatedHealth = 100.0f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void InitializePack(APawn* AlphaPawn);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AddPackMember(APawn* MemberPawn, ECombat_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void RemovePackMember(APawn* MemberPawn);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void SetHuntTarget(APawn* TargetPawn);

    // Hunt Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void StartHunt();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void UpdateHuntPhase(ECombat_HuntPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void AssignPositions();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ExecuteCoordinatedAttack();

    // Pack Communication
    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void BroadcastPackSignal(const FString& Signal);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void ReportMemberStatus(APawn* MemberPawn, bool bInPosition);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    ECombat_HuntPhase GetCurrentHuntPhase() const { return CurrentHuntPhase; }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Hunting")
    bool IsPackReady() const;

protected:
    // Pack Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Configuration")
    TArray<FCombat_PackMember> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Configuration")
    TWeakObjectPtr<APawn> AlphaPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FCombat_HuntTarget CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt State")
    ECombat_HuntPhase CurrentHuntPhase;

    // Hunt Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Parameters")
    float HuntRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Parameters")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Parameters")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Parameters")
    float CoordinationRadius;

    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Timing")
    float PositioningTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Timing")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Timing")
    float LastAttackTime;

private:
    // Internal Methods
    void UpdatePackPositions();
    void CheckPackReadiness();
    void CalculateFlankingPositions();
    FVector GetOptimalAttackPosition(ECombat_PackRole Role);
    bool IsTargetInRange() const;
    void CleanupInvalidMembers();
};