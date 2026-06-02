#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "Combat_DinosaurAI.generated.h"

class ATranspersonalCharacter;

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle,
    Patrolling,
    Hunting,
    Attacking,
    Fleeing,
    Territorial,
    Feeding,
    Wounded
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Fear = 0.3f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    float TimeSinceTargetSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    ECombat_ThreatLevel CurrentThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    bool bCanSeeTarget = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_DinosaurAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    EDinosaurSpecies DinosaurType = EDinosaurSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    FCombat_DinosaurStats CombatStats;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical Data")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float StateChangeInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    bool bIsTerritorial = true;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinosaurState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetPatrolLocation();

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsTargetInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ProcessCombatBehavior(float DeltaTime);

private:
    float LastStateChangeTime = 0.0f;
    FVector InitialLocation = FVector::ZeroVector;
    FVector PatrolTarget = FVector::ZeroVector;

    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateAttackingBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    void UpdateTerritorialBehavior(float DeltaTime);
    void UpdateFeedingBehavior(float DeltaTime);
    void UpdateWoundedBehavior(float DeltaTime);

    ECombat_ThreatLevel CalculateThreatLevel(AActor* Target) const;
    bool ShouldEngageTarget(AActor* Target) const;
    bool ShouldFleeFromTarget(AActor* Target) const;
    void MoveTowardsLocation(const FVector& TargetLocation, float DeltaTime);
};