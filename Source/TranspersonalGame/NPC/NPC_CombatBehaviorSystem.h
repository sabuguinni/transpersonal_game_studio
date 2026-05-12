#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_CombatBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_CombatState : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Wounded     UMETA(DisplayName = "Wounded"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Tail        UMETA(DisplayName = "Tail Whip"),
    Charge      UMETA(DisplayName = "Charge"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Roar        UMETA(DisplayName = "Intimidation Roar")
};

USTRUCT(BlueprintType)
struct FNPC_CombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DefenseRating = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearThreshold = 0.3f;

    FNPC_CombatStats()
    {
        MaxHealth = 100.0f;
        CurrentHealth = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 300.0f;
        AttackCooldown = 2.0f;
        DefenseRating = 0.1f;
        AggressionLevel = 0.5f;
        FearThreshold = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_CombatTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TWeakObjectPtr<APawn> TargetPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DistanceToTarget = 0.0f;

    FNPC_CombatTarget()
    {
        TargetPawn = nullptr;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        DistanceToTarget = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_CombatBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_CombatBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ENPC_CombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ENPC_CombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(ENPC_AttackType AttackType);

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPrimaryTarget(APawn* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat")
    APawn* GetPrimaryTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ScanForThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateTargetThreatLevels();

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, APawn* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DealDamageToTarget(APawn* Target, float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const { return CombatStats.CurrentHealth > 0.0f; }

    // Combat Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExitCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee() const;

    // Group Combat
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RespondToHelpCall(APawn* Caller);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinateGroupAttack();

protected:
    // Core Combat Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FNPC_CombatStats CombatStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    ENPC_CombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FNPC_CombatTarget PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_CombatTarget> PotentialTargets;

    // Combat Timing
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float LastAttackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float CombatStateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float ThreatScanInterval = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float LastThreatScanTime;

    // Combat Ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float FleeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float HelpCallRange = 1000.0f;

    // Group Combat
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<UNPC_CombatBehaviorSystem>> AlliedCombatants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bCanCallForHelp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bCanRespondToHelp = true;

private:
    // Internal Helper Functions
    void UpdateCombatState(float DeltaTime);
    void ProcessCombatBehavior(float DeltaTime);
    float CalculateThreatLevel(APawn* PotentialThreat) const;
    bool IsValidTarget(APawn* PotentialTarget) const;
    void CleanupInvalidTargets();
    FVector GetFleeDirection() const;
    void BroadcastHelpCall();
    void FindNearbyAllies();
};