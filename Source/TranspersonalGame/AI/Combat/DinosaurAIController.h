#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinosaurAIController.generated.h"

// -----------------------------------------------------------------------
// ECombat_DinoSpecies — species enum for AI behaviour branching
// -----------------------------------------------------------------------
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Generic     UMETA(DisplayName = "Generic")
};

// -----------------------------------------------------------------------
// ECombat_DinoState — high-level FSM state
// -----------------------------------------------------------------------
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

// -----------------------------------------------------------------------
// FCombat_PerceptionConfig — tunable perception parameters per species
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FCombat_PerceptionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float LoseSightRadius = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float PeripheralVisionAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float AttackRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float ChaseSpeed = 1440.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float PatrolSpeed = 400.0f;
};

// -----------------------------------------------------------------------
// ADinosaurAIController — main combat AI controller
// -----------------------------------------------------------------------
UCLASS(ClassGroup = "TranspersonalGame|Combat", meta = (DisplayName = "Dinosaur AI Controller"))
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    // --- AAIController overrides ---
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    // --- Perception callback ---
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // --- Public combat interface ---
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    AActor* GetTargetActor() const { return TargetActor; }

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void TriggerRaptorFlank();

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    float GetDistanceToTarget() const;

    // --- Blackboard key names (static for BT tasks to reference) ---
    static const FName BB_PlayerActor;
    static const FName BB_PatrolTarget;
    static const FName BB_bIsChasing;
    static const FName BB_bCanAttack;
    static const FName BB_AttackCooldown;
    static const FName BB_FlankIndex;

protected:
    // --- Perception components ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception",
              meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent_Combat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception",
              meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    // --- Species & config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies DinoSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    FCombat_PerceptionConfig PerceptionConfig;

    // --- Behavior Tree ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|BehaviorTree")
    UBehaviorTree* BehaviorTreeAsset;

    // --- State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    AActor* TargetActor;

    // --- Raptor pack flanking ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    int32 FlankIndex;  // 0=frontal, 1=left, 2=right

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|RaptorPack")
    TArray<ADinosaurAIController*> PackMembers;

private:
    void InitPerceptionForSpecies();
    void UpdateBlackboard();
    void HandleTRexCombat(float DeltaTime);
    void HandleRaptorCombat(float DeltaTime);

    float AttackCooldownRemaining;
    float StateEntryTime;
};
