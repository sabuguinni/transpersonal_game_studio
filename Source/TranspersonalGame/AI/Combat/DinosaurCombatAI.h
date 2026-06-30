#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// DinosaurCombatAI.h — Combat & Enemy AI Agent #12
// Tactical combat state machine for dinosaur enemies.
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    PackHunt    UMETA(DisplayName = "PackHunt")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRadius = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    float ThreatScore = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    float LastSeenTime = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    // ---- Species & Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    // ---- State Machine ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState PreviousState = ECombat_DinoState::Idle;

    // ---- Threat System ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Threat")
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Threat")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Threat")
    float AggressionLevel = 0.f;

    // ---- Pack Hunt ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    TArray<UDinosaurCombatAI*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, float Score);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TransitionToState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeCombatDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void JoinPack(UDinosaurCombatAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitSpeciesPreset(ECombat_DinoSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const { return Stats.CurrentHealth > 0.f; }

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateStateMachine(float DeltaTime);
    void UpdateThreatList(float DeltaTime);
    void SelectPrimaryTarget();
    void ExecuteStateIdle(float DeltaTime);
    void ExecuteStateStalking(float DeltaTime);
    void ExecuteStateCharging(float DeltaTime);
    void ExecuteStateAttacking(float DeltaTime);
    void ExecuteStateRetreating(float DeltaTime);
    void ExecuteStateFleeing(float DeltaTime);
    void ExecuteStatePackHunt(float DeltaTime);
    void NotifyPackOfTarget(AActor* Target);
    float ComputeThreatScore(AActor* Actor) const;

    float StateTimer = 0.f;
    float AttackCooldown = 0.f;
    static constexpr float ThreatDecayRate = 5.f;
    static constexpr float ThreatExpireTime = 30.f;
};
