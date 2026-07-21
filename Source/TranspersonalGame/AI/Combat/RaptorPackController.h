#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "RaptorPackController.generated.h"

UENUM(BlueprintType)
enum class ECombat_RaptorState : uint8
{
    Roaming        UMETA(DisplayName = "Roaming"),
    Stalking       UMETA(DisplayName = "Stalking"),
    Flanking       UMETA(DisplayName = "Flanking"),
    Attacking      UMETA(DisplayName = "Attacking"),
    Retreating     UMETA(DisplayName = "Retreating"),
    PackRegrouping UMETA(DisplayName = "PackRegrouping")
};

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    AlphaLeader    UMETA(DisplayName = "AlphaLeader"),
    LeftFlanker    UMETA(DisplayName = "LeftFlanker"),
    RightFlanker   UMETA(DisplayName = "RightFlanker"),
    Distractor     UMETA(DisplayName = "Distractor")
};

USTRUCT(BlueprintType)
struct FCombat_RaptorPackMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    AActor* RaptorActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    ECombat_RaptorRole Role = ECombat_RaptorRole::AlphaLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    bool bIsAlive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    float LastAttackTime = 0.0f;
};

/**
 * ARaptorPackController — Combat AI controller for Velociraptor pack coordination.
 * Implements flanking logic: Alpha distracts, flankers circle and attack from sides.
 * Builds on ATRexBehaviorController pattern from Agent #11.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorPackController : public AAIController
{
    GENERATED_BODY()

public:
    ARaptorPackController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Pack State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|RaptorPack")
    ECombat_RaptorState PackState = ECombat_RaptorState::Roaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    TArray<FCombat_RaptorPackMember> PackMembers;

    // --- Detection ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float FlankingOffset = 600.0f;

    // --- Combat ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float AttackDamagePerRaptor = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
    float AttackCooldown = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float RetreatHealthThreshold = 0.25f;

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void RegisterPackMember(AActor* RaptorActor, ECombat_RaptorRole Role);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void UpdatePackBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    AActor* ScanForPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ExecutePackAttack(AActor* Target, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ReportMemberDeath(AActor* DeadRaptor);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    int32 GetAlivePackCount() const;

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    ECombat_RaptorState GetPackState() const { return PackState; }

private:
    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    float TimeSinceLastStateChange = 0.0f;
    float StateChangeCooldown = 3.0f;
    float PackAttackTimer = 0.0f;

    FVector GetFlankingPosition(AActor* Target, ECombat_RaptorRole Role) const;
    void TransitionToState(ECombat_RaptorState NewState);
};
