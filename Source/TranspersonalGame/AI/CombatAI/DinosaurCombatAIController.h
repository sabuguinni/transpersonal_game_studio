#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "DinosaurCombatAIController.generated.h"

// Combat state for dinosaur AI
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

// Dinosaur species type — affects combat parameters
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl"),
    Generic     UMETA(DisplayName = "Generic")
};

// Pack coordination data
USTRUCT(BlueprintType)
struct FCombat_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* MemberActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoState MemberState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector AssignedFlankPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float LastUpdateTime = 0.0f;
};

// Threat event broadcast to NPCs
USTRUCT(BlueprintType)
struct FCombat_ThreatEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatRadius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float EventTime = 0.0f;
};

UCLASS(ClassGroup = "Combat AI", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // --- Species Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Species")
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::Generic;

    // --- Perception Radii ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float SightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float HearingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Perception")
    float ChaseRadius = 5000.0f;

    // --- Combat Parameters ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Combat")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Combat")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Combat")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Combat")
    float AlertSpeed = 350.0f;

    // --- Pack Behavior ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Pack")
    float FlankingRadius = 400.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    TArray<FCombat_PackMember> PackMembers;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|State")
    float AlertLevel = 0.0f;

    // --- Blackboard Keys ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Blackboard")
    FName BB_TargetActor = FName("TargetActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Blackboard")
    FName BB_TargetLocation = FName("TargetLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Blackboard")
    FName BB_DinoState = FName("DinoState");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Blackboard")
    FName BB_AlertLevel = FName("AlertLevel");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Blackboard")
    FName BB_FlankPosition = FName("FlankPosition");

    // --- Public Methods ---
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinoSpecies(ECombat_DinoSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BroadcastThreatToNPCs(AActor* ThreatTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ForceState(ECombat_DinoState NewState);

protected:
    UPROPERTY()
    UAIPerceptionComponent* PerceptionComp = nullptr;

    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComp = nullptr;

    UPROPERTY()
    UBlackboardComponent* BlackboardComp = nullptr;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void ApplySpeciesParameters();
    void UpdateCombatState(float DeltaTime);
    void ExecuteAttack();
    void AssignFlankPositions();
    void UpdateBlackboard();
    bool IsPlayerCharacter(AActor* Actor) const;
};
