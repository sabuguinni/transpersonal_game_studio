#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Fleeing     UMETA(DisplayName = "Fleeing")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed = 400.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChargeSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 1;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat state machine
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage_Combat(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCharge(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertPackMembers();

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanSeeTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercent() const;

    // Species configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    bool bIsAggressive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float StalkDuration = 5.f;

private:
    UPROPERTY()
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY()
    AActor* CombatTarget = nullptr;

    float AttackCooldownTimer = 0.f;
    float StalkTimer = 0.f;
    bool bCharging = false;
    FVector ChargeTargetLocation = FVector::ZeroVector;

    void HandleIdleState(float DeltaTime);
    void HandleStalkingState(float DeltaTime);
    void HandleChargingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleRetreatingState(float DeltaTime);

    AActor* FindNearestPlayer() const;
    float GetDistanceToTarget() const;
};
