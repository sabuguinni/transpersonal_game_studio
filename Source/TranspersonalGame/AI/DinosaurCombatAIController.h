#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/NPCBehaviorComponent.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Combat AI enums — prefixed Combat_ to avoid global conflicts
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Pterodactyl  UMETA(DisplayName = "Pterodactyl"),
    Generic      UMETA(DisplayName = "Generic Dinosaur")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None        UMETA(DisplayName = "No Attack"),
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw Swipe"),
    Charge      UMETA(DisplayName = "Charge"),
    Stomp       UMETA(DisplayName = "Stomp"),
    TailSwipe   UMETA(DisplayName = "Tail Swipe"),
    Pounce      UMETA(DisplayName = "Pounce")
};

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Lone        UMETA(DisplayName = "Lone Hunter"),
    PackLeader  UMETA(DisplayName = "Pack Leader"),
    PackMember  UMETA(DisplayName = "Pack Member"),
    Flanker     UMETA(DisplayName = "Flanker"),
    Distractor  UMETA(DisplayName = "Distractor")
};

// ============================================================
// Per-species combat profile — configurable in Blueprint
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float ChaseRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float AttackRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float AttackDamage = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float MoveSpeed_Patrol = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float MoveSpeed_Chase = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    float FleeHealthThreshold = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    bool bCanFlanK = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    ECombat_AttackType PrimaryAttack = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Profile")
    ECombat_AttackType SecondaryAttack = ECombat_AttackType::None;
};

// ============================================================
// Flanking data — used by pack hunters
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_FlankData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Flanking")
    FVector FlankTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Flanking")
    ECombat_TacticalRole Role = ECombat_TacticalRole::Lone;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Flanking")
    float FlankAngleDegrees = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Flanking")
    bool bFlankPositionReached = false;
};

// ============================================================
// Main Combat AI Controller
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- Species & Profile ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoProfile CombatProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_TacticalRole TacticalRole = ECombat_TacticalRole::Lone;

    // ---- Runtime State ----

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bCanAttackNow = true;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_FlankData FlankData;

    // ---- Pack coordination ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    TArray<ADinosaurCombatAIController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ADinosaurCombatAIController* PackLeaderRef = nullptr;

    // ---- Public API ----

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void NotifyPackOfTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssignFlankPosition(float AngleDegrees, float FlankRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplySpeciesDefaults(ECombat_DinoSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AttackType SelectBestAttack() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool ShouldFlee() const;

private:
    void UpdateCombatBehavior(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateFlanking(float DeltaTime);
    void ScanForTargets();
    void SetMovementSpeed(float Speed);

    UNPCBehaviorComponent* BehaviorComp = nullptr;
    float ScanInterval = 0.2f;
    float ScanTimer = 0.0f;
};
