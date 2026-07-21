#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinoAICombatComponent.generated.h"

// ============================================================
// Combat AI enums — prefixed ECombat_ to avoid name collisions
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
    Investigate UMETA(DisplayName = "Investigate"),
    Stalk       UMETA(DisplayName = "Stalk"),
    Charge      UMETA(DisplayName = "Charge"),
    Attack      UMETA(DisplayName = "Attack"),
    Roar        UMETA(DisplayName = "Roar"),
    Retreat     UMETA(DisplayName = "Retreat"),
    Feeding     UMETA(DisplayName = "Feeding"),
    PackHunt    UMETA(DisplayName = "PackHunt")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Charge      UMETA(DisplayName = "Charge"),
    TailSwipe   UMETA(DisplayName = "TailSwipe"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce")
};

UENUM(BlueprintType)
enum class ECombat_AggressionLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Territorial UMETA(DisplayName = "Territorial"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Frenzied    UMETA(DisplayName = "Frenzied")
};

// ============================================================
// Combat structs
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = true;
};

USTRUCT(BlueprintType)
struct FCombat_DinoPerception
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SmellRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseTargetRadius = 4500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    bool bIsNocturnal = false;
};

USTRUCT(BlueprintType)
struct FCombat_PackBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 MinPackSize = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    int32 MaxPackSize = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float PackCoordinationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    float FlankingAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bUsesDecoyTactic = false;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float LastSeenTimestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    bool bIsCurrentTarget = false;
};

// ============================================================
// Main combat component
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UDinoAICombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinoAICombatComponent();

    // ---- Species & Identity ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Identity")
    FName DinoID;

    // ---- Combat State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Combat")
    ECombat_DinoState CurrentCombatState = ECombat_DinoState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Combat")
    ECombat_AggressionLevel AggressionLevel = ECombat_AggressionLevel::Territorial;

    // ---- Attacks ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Combat")
    TArray<FCombat_AttackData> AvailableAttacks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Combat")
    float LastAttackTimestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Combat")
    float GlobalAttackCooldown = 1.5f;

    // ---- Perception ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Perception")
    FCombat_DinoPerception PerceptionConfig;

    // ---- Pack Behavior ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Pack")
    FCombat_PackBehavior PackConfig;

    // ---- Threat Tracking ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Threat")
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Threat")
    AActor* PrimaryTarget = nullptr;

    // ---- Stats ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dino|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MoveSpeed_Patrol = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MoveSpeed_Chase = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|Stats")
    float MoveSpeed_Charge = 900.0f;

    // ---- Daily Phase Aggression Modifiers ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|DailyPhase")
    float AggressionMultiplier_Day = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|DailyPhase")
    float AggressionMultiplier_Night = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|DailyPhase")
    float AggressionMultiplier_Dusk = 1.5f;

    // ---- Behavior Tree ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dino|AI")
    UBehaviorTree* BehaviorTree = nullptr;

    // ============================================================
    // Public API
    // ============================================================

    /** Register a new threat actor. Score 0-1. Tag e.g. "Player", "Fire", "Noise" */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterThreat(AActor* ThreatActor, float ThreatScore, FName ThreatTag);

    /** Remove a threat from the list */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearThreat(AActor* ThreatActor);

    /** Execute best available attack against current PrimaryTarget */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ExecuteAttack(ECombat_AttackType PreferredType);

    /** Apply damage to this dinosaur */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float TakeDamage_Dino(float DamageAmount, AActor* DamageInstigator);

    /** Force transition to a new combat state */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_DinoState NewState);

    /** Returns highest threat score in current threat list */
    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHighestThreatScore() const;

    /** Returns the primary target (highest threat actor) */
    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetPrimaryTarget() const;

    /** Check if target is within attack range for given attack type */
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsTargetInAttackRange(AActor* Target, ECombat_AttackType AttackType) const;

    /** Returns true if dino can attack (cooldown elapsed) */
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanAttack() const;

    /** Compute flanking position for pack hunt — offset from target by angle */
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    FVector ComputeFlankingPosition(AActor* Target, float AngleOffset, float Distance) const;

    /** Notify pack members of target location */
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastTargetToPackMembers(AActor* Target, float Radius);

    /** Apply daily phase aggression multiplier */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDailyPhaseModifier(uint8 DailyPhaseIndex);

    /** Roar — triggers fear in nearby player, alerts pack */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TriggerRoar(float FearRadius, float FearDuration);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float StateTimer = 0.0f;
    float CurrentAggressionMultiplier = 1.0f;
    float TickAccumulator = 0.0f;
    static constexpr float TickInterval = 0.1f; // 10 Hz AI tick

    void TickCombatAI(float DeltaTime);
    void UpdatePrimaryTarget();
    void DecayThreatScores(float DeltaTime);
    void EvaluateCombatTransition();
    FCombat_AttackData* FindBestAttack(ECombat_AttackType PreferredType);
    void ApplyAttackDamage(const FCombat_AttackData& Attack);
};
