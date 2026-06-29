#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "AIController.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// ENUMS — DinosaurCombatAI (prefix: Combat_)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl"),
    Spinosaurus UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Charge      UMETA(DisplayName = "Charge"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Charge      UMETA(DisplayName = "Charge"),
    Tail        UMETA(DisplayName = "Tail Swipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float HearingRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold = 0.2f;
};

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    FName AnimationMontage = NAME_None;
};

USTRUCT(BlueprintType)
struct FCombat_EncounterRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    float ThreatTimestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;
};

// ============================================================
// DINOSAUR COMBAT AI COMPONENT
// ============================================================

UCLASS(ClassGroup = "Combat", meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Combat AI")
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Species & Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attacks")
    TArray<FCombat_AttackData> AvailableAttacks;

    // ---- State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_ThreatLevel CurrentThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget = nullptr;

    // ---- Memory ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    TArray<FCombat_EncounterRecord> EncounterMemory;

    // ---- Pack Behavior ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<UDinosaurCombatAIComponent*> PackMembers;

    // ---- Public Methods ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTakeDamage(float DamageAmount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(const FCombat_AttackData& Attack);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TransitionToState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_ThreatLevel EvaluateThreatLevel(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RecordEncounter(AActor* ThreatActor, ECombat_ThreatLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HasEncounteredBefore(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeSpeciesDefaults();

private:
    float LastAttackTime = 0.0f;
    float StateEntryTime = 0.0f;
    bool bIsInitialized = false;

    void UpdateCombatState(float DeltaTime);
    void UpdatePatrol(float DeltaTime);
    void UpdateAlert(float DeltaTime);
    void UpdateCharge(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateFlee(float DeltaTime);
    FCombat_AttackData SelectBestAttack() const;
    void ApplyTRexDefaults();
    void ApplyRaptorDefaults();
    void ApplyTriceratopsDefaults();
};

// ============================================================
// DINOSAUR COMBAT AI CONTROLLER
// ============================================================

UCLASS(DisplayName = "Dinosaur AI Controller")
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* DinosaurBehaviorTree = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* DinoBlackboard = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* PerceptionComponent = nullptr;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardState(ECombat_DinoState State);

    UFUNCTION(BlueprintCallable, Category = "AI")
    AActor* GetBlackboardTarget() const;

private:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void SetupPerceptionConfig();
};
