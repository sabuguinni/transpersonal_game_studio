#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/NPCBehavior/NPCBehaviorComponent.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Pteranodon          UMETA(DisplayName = "Pteranodon"),
    Generic             UMETA(DisplayName = "Generic Dinosaur")
};

UENUM(BlueprintType)
enum class ECombat_ThreatResponse : uint8
{
    Ignore      UMETA(DisplayName = "Ignore"),
    Investigate UMETA(DisplayName = "Investigate"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    PackCall    UMETA(DisplayName = "Call Pack")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw Swipe"),
    Charge      UMETA(DisplayName = "Charge"),
    Tail        UMETA(DisplayName = "Tail Swipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce")
};

// ============================================================
// Structs — must be at global scope (UE5 UHT requirement)
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_SpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    // Detection ranges (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float HearingRange = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float SmellRange = 800.0f;

    // Attack parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType PrimaryAttack = ECombat_AttackType::Bite;

    // Movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Movement")
    float FleeSpeed = 700.0f;

    // Pack behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackCallRadius = 1500.0f;

    // Flee threshold (health ratio 0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Survival")
    float FleeHealthThreshold = 0.2f;
};

USTRUCT(BlueprintType)
struct FCombat_FlankPosition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsOccupied = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Pack")
    TWeakObjectPtr<AActor> OccupyingDino;
};

// ============================================================
// ADinosaurCombatAIController
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- Species Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_SpeciesTraits SpeciesTraits;

    // ---- Current Combat State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    ECombat_ThreatResponse CurrentResponse = ECombat_ThreatResponse::Ignore;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float HealthRatio = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    bool bIsInCombat = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
    float TimeSinceLastAttack = 0.0f;

    // ---- Target ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Target")
    TWeakObjectPtr<AActor> PrimaryTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Target")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    // ---- Pack Coordination ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    TArray<TWeakObjectPtr<ADinosaurCombatAIController>> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack")
    FCombat_FlankPosition AssignedFlankPosition;

    // ---- Blueprint Events ----
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackBegin(AActor* Target, ECombat_AttackType AttackType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnThreatDetected(AActor* ThreatActor, float Distance);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnFleeBegin(FVector FleeDestination);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnPackCallIssued(float Radius);

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Control")
    void SetHealthRatio(float NewRatio);

    UFUNCTION(BlueprintCallable, Category = "Combat|Control")
    void RegisterPackMember(ADinosaurCombatAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat|Control")
    void NotifyPackOfThreat(AActor* ThreatActor, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Query")
    ECombat_ThreatResponse GetCurrentResponse() const { return CurrentResponse; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Query")
    bool IsInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Query")
    bool IsInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Query")
    float GetDistanceToTarget() const;

    // ---- Apply species preset ----
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Combat|Species")
    void ApplySpeciesPreset(ECombat_DinoSpecies Species);

protected:
    void EvaluateThreatResponse(float DeltaTime);
    void ExecuteAttack();
    void ExecuteChase();
    void ExecuteFlee();
    void ExecutePackFlank();
    void ScanForThreats();
    void UpdatePackCoordination();
    FVector CalculateFlankPosition(bool bLeftFlank) const;

    // NPC Behavior component reference (from possessed pawn)
    UPROPERTY()
    UNPCBehaviorComponent* BehaviorComp = nullptr;

    float AITickAccumulator = 0.0f;
    static constexpr float AITickInterval = 0.1f; // 10Hz
};
