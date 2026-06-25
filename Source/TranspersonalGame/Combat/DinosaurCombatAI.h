#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus")
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentHealth = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritoryRadius = 5000.0f;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components")
    USphereComponent* AttackSphere;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeCombatDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

protected:
    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    float AttackCooldownTimer;

    UPROPERTY()
    FVector PatrolOrigin;

    void UpdateIdleState(float DeltaTime);
    void UpdateChaseState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateAlertState(float DeltaTime);

    UFUNCTION()
    void OnDetectionSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
