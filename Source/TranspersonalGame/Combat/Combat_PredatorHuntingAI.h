#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "AI/NPC/NPC_DinosaurHerdBehavior.h"
#include "Combat_PredatorHuntingAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_HuntingState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Stalking UMETA(DisplayName = "Stalking"),
    Chasing UMETA(DisplayName = "Chasing"),
    Attacking UMETA(DisplayName = "Attacking"),
    Feeding UMETA(DisplayName = "Feeding"),
    Retreating UMETA(DisplayName = "Retreating")
};

USTRUCT(BlueprintType)
struct FCombat_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float DistanceToTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    bool bIsHerdMember;

    FCombat_HuntTarget()
    {
        TargetActor = nullptr;
        DistanceToTarget = 0.0f;
        ThreatLevel = 0.0f;
        bIsHerdMember = false;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_PredatorHuntingAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_PredatorHuntingAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Hunting State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting AI")
    ECombat_HuntingState CurrentHuntingState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting AI")
    float HuntingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting AI")
    float StalkingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting AI")
    float AttackRange;

    // Target Selection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Selection")
    TArray<FCombat_HuntTarget> PotentialTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Selection")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Selection")
    float PreferredPreySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Selection")
    bool bPreferIsolatedTargets;

    // Hunting Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Behavior")
    float StalkingSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Behavior")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Behavior")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Behavior")
    float LastAttackTime;

    // Pack Hunting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    TArray<UCombat_PredatorHuntingAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Hunting")
    UCombat_PredatorHuntingAI* PackLeader;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Hunting AI")
    void ScanForTargets();

    UFUNCTION(BlueprintCallable, Category = "Hunting AI")
    void SelectBestTarget();

    UFUNCTION(BlueprintCallable, Category = "Hunting AI")
    void StartStalking(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunting AI")
    void StartChasing(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunting AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Hunting AI")
    void TriggerHerdFleeResponse(AActor* HerdTarget);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void JoinPack(UCombat_PredatorHuntingAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Hunting")
    void LeavePack();

private:
    void UpdateHuntingState(float DeltaTime);
    void HandleIdleState();
    void HandleStalkingState();
    void HandleChasingState();
    void HandleAttackingState();
    void HandleFeedingState();
    void HandleRetreatingState();

    float CalculateThreatLevel(AActor* Target);
    bool IsTargetInHerd(AActor* Target);
    FVector CalculateStalkingPosition(AActor* Target);
};