#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Combat_DinosaurCombatBehavior.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Defending   UMETA(DisplayName = "Defending"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Circling    UMETA(DisplayName = "Circling")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Compsognathus UMETA(DisplayName = "Compsognathus")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Fear = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    bool bIsHerbivore = false;
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurCombatBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_DinosaurCombatState CurrentCombatState = ECombat_DinosaurCombatState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_DinosaurSpecies DinosaurSpecies = ECombat_DinosaurSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    FCombat_DinosaurStats CombatStats;

    // Target Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    float LastAttackTime = 0.0f;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeSpeciesStats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition() const;

    // Pack Behavior
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<UCombat_DinosaurCombatBehavior*> PackMembers;

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UCombat_DinosaurCombatBehavior* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsInPack() const;

    // AI Integration
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class AAIController* DinosaurAIController = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent = nullptr;

private:
    float CombatUpdateTimer = 0.0f;
    float AttackCooldown = 2.0f;
    float StateChangeTimer = 0.0f;
    
    void UpdateTargetDistance();
    void HandleIdleState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandleDefendingState(float DeltaTime);
    void HandleStalkingState(float DeltaTime);
    void HandleCirclingState(float DeltaTime);
};