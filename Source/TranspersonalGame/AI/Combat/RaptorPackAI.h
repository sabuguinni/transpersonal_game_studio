#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RaptorPackAI.generated.h"

// ============================================================
// Raptor Pack AI — Agent #12 Combat & Enemy AI
// Implements coordinated flanking attack logic for Velociraptor packs
// ============================================================

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha (Distractor)"),
    LeftFlanker UMETA(DisplayName = "Left Flanker"),
    RightFlanker UMETA(DisplayName = "Right Flanker"),
    Ambusher    UMETA(DisplayName = "Ambusher"),
    Scout       UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class ECombat_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Scouting    UMETA(DisplayName = "Scouting"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Feeding     UMETA(DisplayName = "Feeding")
};

USTRUCT(BlueprintType)
struct FCombat_RaptorMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    TWeakObjectPtr<AActor> RaptorActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    ECombat_RaptorRole Role = ECombat_RaptorRole::Alpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    bool bIsAlive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Raptor")
    FVector AssignedFlankPosition = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_PackConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float FlankRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float AttackDamage = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float AttackCooldown = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float RetreatHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float StalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float ChargeSpeed = 700.0f;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Pack AI")
class TRANSPERSONALGAME_API URaptorPackAI : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorPackAI();

    // ---- Pack State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    ECombat_PackState PackState = ECombat_PackState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FCombat_PackConfig PackConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    TArray<FCombat_RaptorMember> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    TWeakObjectPtr<AActor> CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    bool bFlankPositionsAssigned = false;

    // ---- Pack Coordination ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void RegisterPackMember(AActor* RaptorActor, ECombat_RaptorRole Role);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SetPackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AssignFlankPositions();

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    FVector GetFlankPosition(ECombat_RaptorRole Role, const FVector& TargetLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ExecutePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void OnPackMemberDied(AActor* DeadMember);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    int32 GetAliveCount() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    ECombat_PackState GetPackState() const { return PackState; }

    // ---- State Transitions ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void TransitionTo(ECombat_PackState NewState);

    // ---- Tick ----
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
    void UpdatePackAI(float DeltaTime);
    void UpdateStalking(float DeltaTime);
    void UpdateFlanking(float DeltaTime);
    void UpdateAttacking(float DeltaTime);
    bool IsTargetInRange(float Range) const;
    float GetDistanceToTarget() const;
};
