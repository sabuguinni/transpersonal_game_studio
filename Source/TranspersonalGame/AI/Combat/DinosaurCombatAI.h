// DinosaurCombatAI.h
// Agent #12 — Combat & Enemy AI — Cycle PROD_AUTO_20260628_011
// Dinosaur combat AI component: threat assessment, attack selection, pack coordination
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Carnotaurus UMETA(DisplayName = "Carnotaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus")
};

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    PackHunt    UMETA(DisplayName = "PackHunt")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Charge      UMETA(DisplayName = "Charge"),
    Claw        UMETA(DisplayName = "Claw"),
    Tail        UMETA(DisplayName = "TailSwipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce")
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WindupTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = false;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsVisible = false;
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggroRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FCombat_AttackData> AvailableAttacks;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats DinoStats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    TArray<FCombat_ThreatEntry> ThreatList;

    // Pack coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<UDinosaurCombatAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCoordinationRadius = 3000.0f;

    // UFUNCTION interface
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, float InitialScore);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatScore(AActor* ThreatActor, float Delta);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetThreatScore(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SelectPrimaryTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TransitionToState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void NotifyPackOfThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AttackType SelectBestAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatScore(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinateFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankingPosition(int32 FlankIndex, int32 TotalFlankers) const;

private:
    float AttackCooldownRemaining = 0.0f;
    float StateTimer = 0.0f;
    float LastPackNotifyTime = 0.0f;
    int32 FlankIndex = 0;

    void TickIdle(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickStalking(float DeltaTime);
    void TickCharging(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickRetreating(float DeltaTime);
    void TickPackHunt(float DeltaTime);

    void ScanForThreats();
    void PruneStaleThreats();
};
