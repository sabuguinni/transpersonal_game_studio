#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
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
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPredator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_DinoStats Stats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AlertDecayTime = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FleeHealthThreshold = 0.2f;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnPlayerDetected(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage_Dino(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsHostile() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeFromSpecies();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float LastAttackTime = 0.0f;
    float AlertTimer = 0.0f;
    TWeakObjectPtr<AActor> CurrentTarget;

    void UpdateIdleState(float DeltaTime);
    void UpdateAlertedState(float DeltaTime);
    void UpdateChasingState(float DeltaTime);
    void UpdateAttackingState(float DeltaTime);
    void UpdateFleeingState(float DeltaTime);
};

UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatActor : public AActor
{
    GENERATED_BODY()

public:
    ADinosaurCombatActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDinosaurCombatAIComponent* CombatAI;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertNearbyPackMembers(float AlertRadius);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
