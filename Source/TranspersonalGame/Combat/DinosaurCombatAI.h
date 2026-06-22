#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flank       UMETA(DisplayName = "Flank"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "TyrannosaurusRex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus")
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChaseSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FCombat_PatrolWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WaitTime = 3.0f;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // State machine
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    // Combat actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DetectPlayer(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Combat(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const { return DinoStats.CurrentHealth > 0.0f; }

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void NotifyPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssignFlankPosition(int32 PackIndex, int32 TotalPackSize);

    // Patrol
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddPatrolWaypoint(FVector Location, float WaitTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartPatrol();

    // Species setup
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeForSpecies(ECombat_DinoSpecies Species);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_DinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FCombat_PatrolWaypoint> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector FlankOffset = FVector::ZeroVector;

private:
    float AttackTimer = 0.0f;
    int32 CurrentWaypointIndex = 0;
    float WaypointWaitTimer = 0.0f;
    bool bWaitingAtWaypoint = false;

    void UpdateIdle(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateAlert(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateFlank(float DeltaTime);
    void UpdateRetreat(float DeltaTime);

    float GetDistanceToTarget() const;
    bool IsPlayerInRange(float Range) const;
    void MoveTowardsTarget(float Speed, float DeltaTime);
    void MoveTowardsFlankPosition(float DeltaTime);
};
