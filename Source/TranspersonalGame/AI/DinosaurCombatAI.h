#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPack = false;

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
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoStats Stats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    int32 CurrentPatrolIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetCurrentHealth() const { return Stats.CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsHostile() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeSpeciesStats();

private:
    float LastAttackTime = 0.0f;
    float StateEntryTime = 0.0f;

    void UpdateCombatState(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void ScanForThreats();
    void DecayThreatList(float DeltaTime);
    bool IsInAttackRange(AActor* Target) const;
};
