#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "CombatAIComponent.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float BaseDamage = 25.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float KnockbackForce = 500.0f;
};

UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State machine
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentState; }

    // Threat system
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetHighestThreat() const;

    // Attack
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    // Flanking (raptor pack behavior)
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetFlankingTarget(AActor* Target, int32 FlankIndex);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankPosition(AActor* Target, int32 FlankIndex) const;

    // Perception
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeActor(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanHearActor(AActor* Target) const;

    // Species config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float HearingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Attack")
    FCombat_AttackData AttackData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float AggressionLevel = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    bool bIsPackMember = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    int32 PackFlankIndex = 0;

private:
    UPROPERTY()
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY()
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    float LastAttackTime = 0.0f;
    float StateTimer = 0.0f;

    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateAlertState(float DeltaTime);
    void UpdateChaseState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateFlankingState(float DeltaTime);
    void UpdateRetreatState(float DeltaTime);

    void ScanForThreats();
    void ApplySpeciesDefaults();
};
