#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Circling    UMETA(DisplayName = "Circling"),
    Ambush      UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class ECombat_AggressionLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Berserk     UMETA(DisplayName = "Berserk")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float RetreatHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    float CirclingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    bool bCanAmbush = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactical")
    bool bCanCallReinforcements = false;

    FCombat_TacticalData()
    {
        AttackRange = 500.0f;
        DetectionRange = 1500.0f;
        RetreatHealthThreshold = 0.3f;
        CirclingDistance = 800.0f;
        PackSize = 1;
        bCanAmbush = false;
        bCanCallReinforcements = false;
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

    // Tactical state management
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat Tactical")
    ECombat_TacticalState GetTacticalState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void SetAggressionLevel(ECombat_AggressionLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Combat Tactical")
    ECombat_AggressionLevel GetAggressionLevel() const { return AggressionLevel; }

    // Target management
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat Tactical")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    bool IsTargetInRange(float Range) const;

    // Combat decision making
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    bool ShouldAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    bool ShouldCircle() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    FVector GetOptimalAttackPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    FVector GetRetreatPosition() const;

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void JoinPack(UCombat_TacticalAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void LeavePack();

    UFUNCTION(BlueprintPure, Category = "Combat Tactical")
    bool IsInPack() const { return PackMembers.Num() > 1 || PackLeader != nullptr; }

    // Combat events
    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void OnTakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void OnTargetLost();

    UFUNCTION(BlueprintCallable, Category = "Combat Tactical")
    void OnTargetAcquired(AActor* NewTarget);

protected:
    // Current tactical state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_TacticalState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_AggressionLevel AggressionLevel;

    // Target tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    float LastTargetSightTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FVector LastKnownTargetLocation;

    // Tactical configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Config")
    FCombat_TacticalData TacticalData;

    // Pack behavior
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    UCombat_TacticalAI* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    TArray<UCombat_TacticalAI*> PackMembers;

    // Internal timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timers")
    float StateChangeTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timers")
    float LastAttackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timers")
    float CirclingTimer;

private:
    // Internal helper functions
    void UpdateTacticalBehavior(float DeltaTime);
    void ProcessPatrolState(float DeltaTime);
    void ProcessHuntingState(float DeltaTime);
    void ProcessAttackingState(float DeltaTime);
    void ProcessRetreatingState(float DeltaTime);
    void ProcessCirclingState(float DeltaTime);
    void ProcessAmbushState(float DeltaTime);

    bool CanSeeTarget() const;
    float GetDistanceToTarget() const;
    float GetHealthPercentage() const;
    void UpdatePackCoordination();
    FVector CalculateFlankingPosition() const;
};