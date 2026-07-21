// RaptorPackAI.h
// Agent #12 — Combat & Enemy AI Agent
// Raptor pack coordination system — flanking, distraction, coordinated attack
// CYCLE: PROD_CYCLE_AUTO_20260629_007

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "RaptorPackAI.generated.h"

// ============================================================
// ENUMS — Global scope (RULE 1: USTRUCT/UENUM at global scope)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_RaptorRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),       // Leads charge, primary threat
    Flanker     UMETA(DisplayName = "Flanker"),     // Circles to flank
    Distractor  UMETA(DisplayName = "Distractor"),  // Feints to draw attention
    Ambusher    UMETA(DisplayName = "Ambusher"),    // Waits in cover, strikes on signal
    Retreating  UMETA(DisplayName = "Retreating")   // Injured, pulling back
};

UENUM(BlueprintType)
enum class ECombat_PackState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Stalking    UMETA(DisplayName = "Stalking"),    // Pack detected prey, moving quietly
    Coordinating UMETA(DisplayName = "Coordinating"), // Assigning roles before attack
    Attacking   UMETA(DisplayName = "Attacking"),   // Full assault in progress
    Regrouping  UMETA(DisplayName = "Regrouping"),  // One member hurt, pack pulls back
    Fleeing     UMETA(DisplayName = "Fleeing")      // Multiple casualties, pack breaks
};

// ============================================================
// STRUCTS — Global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_RaptorMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    AActor* RaptorActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    ECombat_RaptorRole Role = ECombat_RaptorRole::Alpha;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    FVector AssignedFlankPosition = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_PackAttackPlan
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    FVector AlphaChargeDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    FVector LeftFlankPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    FVector RightFlankPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    float CoordinationDelay = 1.5f;   // Seconds before flankers move after alpha charges

    UPROPERTY(BlueprintReadWrite, Category = "Combat|RaptorPack")
    bool bPlanReady = false;
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_PackStateChanged, ECombat_PackState, OldState, ECombat_PackState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_RaptorKilled, AActor*, KilledRaptor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_PackFled, int32, SurvivingCount);

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Raptor Pack AI")
class TRANSPERSONALGAME_API URaptorPackAI : public UActorComponent
{
    GENERATED_BODY()

public:
    URaptorPackAI();

    // ---- Pack Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    int32 MaxPackSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float DetectionRange = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float AttackRange = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float FlankRadius = 400.0f;         // How far flankers spread from target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float CoordinationDelay = 1.5f;     // Alpha charges, flankers wait this long

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float RetreatHealthThreshold = 30.0f; // Individual raptor retreats below this HP

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float PackFleeThreshold = 0.5f;     // Pack flees when >50% members dead

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float AlphaAttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack|Config")
    float FlankAttackDamage = 18.0f;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|RaptorPack|State",
              meta = (AllowPrivateAccess = "true"))
    ECombat_PackState PackState = ECombat_PackState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|RaptorPack|State",
              meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_RaptorMember> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|RaptorPack|State",
              meta = (AllowPrivateAccess = "true"))
    AActor* TargetPawn = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|RaptorPack|State",
              meta = (AllowPrivateAccess = "true"))
    FCombat_PackAttackPlan CurrentAttackPlan;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|RaptorPack|State",
              meta = (AllowPrivateAccess = "true"))
    int32 AliveCount = 0;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Combat|RaptorPack|Events")
    FCombat_PackStateChanged OnPackStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|RaptorPack|Events")
    FCombat_RaptorKilled OnRaptorKilled;

    UPROPERTY(BlueprintAssignable, Category = "Combat|RaptorPack|Events")
    FCombat_PackFled OnPackFled;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void RegisterPackMember(AActor* RaptorActor, ECombat_RaptorRole Role);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void NotifyRaptorDamaged(AActor* RaptorActor, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void NotifyRaptorDead(AActor* RaptorActor);

    UFUNCTION(BlueprintCallable, Category = "Combat|RaptorPack")
    void ForcePackFlee();

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    ECombat_PackState GetPackState() const { return PackState; }

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    int32 GetAliveCount() const { return AliveCount; }

    UFUNCTION(BlueprintPure, Category = "Combat|RaptorPack")
    bool IsPackAttacking() const { return PackState == ECombat_PackState::Attacking; }

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void SetPackState(ECombat_PackState NewState);
    void BuildAttackPlan();
    void ExecuteAttackPlan();
    void ScanForTarget();
    void UpdateAliveCount();
    bool ShouldPackFlee() const;

    FTimerHandle CoordinationTimerHandle;
    FTimerHandle ScanTimerHandle;
    float TimeSinceLastAttack = 0.0f;
};
