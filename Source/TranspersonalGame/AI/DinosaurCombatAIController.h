#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DinosaurCombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
};

USTRUCT(BlueprintType)
struct FCombat_DinoAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    bool bCanBreakGuard = false;
};

USTRUCT(BlueprintType)
struct FCombat_DinoPerceptionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float SmellRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    bool bMotionSensitive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Perception")
    float AlertDuration = 10.0f;
};

/**
 * UCombat_DinosaurCombatAIController
 * AI Controller for dinosaur combat behavior.
 * Handles T-Rex solo aggression and Raptor pack flanking tactics.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // --- State Machine ---

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetCombatState(ECombat_DinoAIState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|AI")
    ECombat_DinoAIState GetCombatState() const { return CurrentState; }

    // --- Threat Detection ---

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool DetectPlayer(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool IsPlayerInSight() const;

    // --- Combat Actions ---

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ChasePlayer(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ExecuteFlankingManeuver(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void Retreat(float DeltaTime);

    // --- Pack Coordination ---

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void RegisterPackMember(ADinosaurCombatAIController* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void AssignFlankRole(int32 RoleIndex);

    UFUNCTION(BlueprintPure, Category = "Combat|Pack")
    int32 GetFlankRole() const { return FlankRoleIndex; }

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoAttackData AttackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoPerceptionData PerceptionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float RetreatHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float FlankingAngleDegrees = 90.0f;

private:
    UPROPERTY()
    ECombat_DinoAIState CurrentState = ECombat_DinoAIState::Idle;

    UPROPERTY()
    APawn* PlayerPawn = nullptr;

    UPROPERTY()
    TArray<ADinosaurCombatAIController*> PackMembers;

    float AttackCooldownTimer = 0.0f;
    float AlertTimer = 0.0f;
    int32 FlankRoleIndex = 0;

    void TickIdle(float DeltaTime);
    void TickChasing(float DeltaTime);
    void TickAttacking(float DeltaTime);
    void TickFlanking(float DeltaTime);
    void TickRetreating(float DeltaTime);

    FVector GetFlankingTargetPosition() const;
    bool HasLineOfSightToPlayer() const;
};
