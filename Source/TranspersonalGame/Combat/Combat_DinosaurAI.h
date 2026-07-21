#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "Combat_DinosaurAI.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
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
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Fear = 0.3f;

    FCombat_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 200.0f;
        DetectionRange = 800.0f;
        MovementSpeed = 300.0f;
        Aggression = 0.5f;
        Fear = 0.3f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core AI State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    ECombat_DinosaurSpecies Species = ECombat_DinosaurSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur AI")
    FCombat_DinosaurStats Stats;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components", meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComp;

    // Target Management
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = 0.0f;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadWrite, Category = "Pack Behavior")
    TArray<UCombat_DinosaurAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCoordinationRadius = 1000.0f;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float Damage, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UCombat_DinosaurAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AlertPack(AActor* Threat);

    // Species-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    void InitializeSpeciesStats();

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    bool IsApexPredator() const;

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    bool IsHerbivore() const;

    UFUNCTION(BlueprintCallable, Category = "Species Behavior")
    bool IsCarnivore() const;

private:
    // Internal state management
    void UpdateAIState(float DeltaTime);
    void ProcessPerception();
    void UpdatePackBehavior();
    void HandleCombatLogic();
    void HandleMovement();

    // Timers
    float StateChangeTimer = 0.0f;
    float PerceptionUpdateTimer = 0.0f;
    float PackUpdateTimer = 0.0f;

    // Constants
    static constexpr float PERCEPTION_UPDATE_INTERVAL = 0.5f;
    static constexpr float PACK_UPDATE_INTERVAL = 1.0f;
    static constexpr float STATE_CHANGE_COOLDOWN = 2.0f;
};