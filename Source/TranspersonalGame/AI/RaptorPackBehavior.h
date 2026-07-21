// RaptorPackBehavior.h
// Agent #12 — Combat & Enemy AI Agent
// Raptor pack coordination: flanking, distraction, synchronized attack

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "RaptorPackBehavior.generated.h"

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Distractor   UMETA(DisplayName = "Distractor"),
    FlankerLeft  UMETA(DisplayName = "FlankerLeft"),
    FlankerRight UMETA(DisplayName = "FlankerRight"),
    PackLeader   UMETA(DisplayName = "PackLeader")
};

UENUM(BlueprintType)
enum class ECombat_PackState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Stalking     UMETA(DisplayName = "Stalking"),
    Flanking     UMETA(DisplayName = "Flanking"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Retreating   UMETA(DisplayName = "Retreating")
};

USTRUCT(BlueprintType)
struct FCombat_RaptorPackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float DetectionRange = 2500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float FlankRadius = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackDamage = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float AttackCooldown = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float StalkSpeed = 350.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float ChaseSpeed = 700.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float FlankPositionTolerance = 80.f;

    // Time before synchronized attack signal is sent
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float SyncAttackDelay = 2.5f;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URaptorPackBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorPackBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FCombat_RaptorPackConfig Config;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_RaptorRole Role = ECombat_RaptorRole::Distractor;

    // --- State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    ECombat_PackState PackState = ECombat_PackState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    bool bFlankPositionReached = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Raptor")
    bool bSyncAttackSignalReceived = false;

    // --- Pack references (set by PackLeader) ---
    UPROPERTY(BlueprintReadWrite, Category = "Combat|Raptor")
    TArray<URaptorPackBehaviorComponent*> PackMembers;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void SetPackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void ReceiveSyncAttackSignal();

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void BroadcastSyncAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    FVector CalculateFlankPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    bool IsTargetInSight(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Raptor")
    ECombat_PackState GetPackState() const { return PackState; }

private:
    float AttackCooldownTimer = 0.f;
    float SyncAttackTimer = 0.f;
    bool bAttackCooldownActive = false;

    void TickIdle(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickRetreating(float DeltaTime);

    void TransitionToState(ECombat_PackState NewState);
    void ScanForPlayer();
    void MoveTowardsLocation(const FVector& TargetLocation, float Speed, float DeltaTime);
};
