#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Patrol     UMETA(DisplayName = "Patrol"),
    Alert      UMETA(DisplayName = "Alert"),
    Stalk      UMETA(DisplayName = "Stalk"),
    Chase      UMETA(DisplayName = "Chase"),
    Flank      UMETA(DisplayName = "Flank"),
    Attack     UMETA(DisplayName = "Attack"),
    Retreat    UMETA(DisplayName = "Retreat"),
    Dead       UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex          UMETA(DisplayName = "T-Rex"),
    Velociraptor  UMETA(DisplayName = "Velociraptor"),
    Triceratops   UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl   UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float Health = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AggroRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChaseSpeed = 850.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DamagePerHit = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float RetreatHealthThreshold = 0.15f;
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float FlankingAngle = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCohesionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bAlphaLeadsCharge = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float TimeSinceLastAttack = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeCombatDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInAggroRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiatePackFlank(TArray<ADinosaurCombatAI*> PackMembers);

protected:
    void UpdatePatrolState(float DeltaTime);
    void UpdateAlertState(float DeltaTime);
    void UpdateChaseState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateRetreatState(float DeltaTime);

    int32 CurrentWaypointIndex = 0;
    float StateTimer = 0.0f;
};
