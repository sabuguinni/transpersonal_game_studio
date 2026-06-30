#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
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
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Damage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float StunDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    FName AttackType = NAME_None;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DetectPlayer(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float TakeDamage_Combat(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAlive() const { return DinoStats.CurrentHealth > 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetPackLeader(AActor* Leader) { PackLeader = Leader; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void NotifyPackMembersOfTarget(AActor* Target);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_DinoStats DinoStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

private:
    float AttackCooldownTimer = 0.f;
    float StateTimer = 0.f;

    void UpdateHuntingBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateFleeingBehavior(float DeltaTime);
    bool IsTargetInAttackRange() const;
    float GetDistanceToTarget() const;
};

UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatAI : public APawn
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeDinosaur(ECombat_DinoSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    UDinosaurCombatComponent* CombatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* AttackSphere;

private:
    UFUNCTION()
    void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
