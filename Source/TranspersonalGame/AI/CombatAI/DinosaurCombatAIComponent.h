#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/NPCBehavior/NPCBehaviorComponent.h"
#include "DinosaurCombatAIComponent.generated.h"

// Combat phase for a dinosaur encounter
UENUM(BlueprintType)
enum class ECombat_DinoPhase : uint8
{
    Roaming        UMETA(DisplayName = "Roaming"),
    Stalking       UMETA(DisplayName = "Stalking"),
    Charging       UMETA(DisplayName = "Charging"),
    Flanking       UMETA(DisplayName = "Flanking"),
    Retreating     UMETA(DisplayName = "Retreating"),
    Feeding        UMETA(DisplayName = "Feeding")
};

// Species-level combat archetype
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex   UMETA(DisplayName = "T-Rex"),
    Velociraptor       UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus      UMETA(DisplayName = "Brachiosaurus"),
    Triceratops        UMETA(DisplayName = "Triceratops"),
    Pterodactyl        UMETA(DisplayName = "Pterodactyl")
};

// Tactical action the AI will execute this tick
UENUM(BlueprintType)
enum class ECombat_TacticalAction : uint8
{
    None           UMETA(DisplayName = "None"),
    MoveToFlank    UMETA(DisplayName = "MoveToFlank"),
    ChargeTarget   UMETA(DisplayName = "ChargeTarget"),
    CircleTarget   UMETA(DisplayName = "CircleTarget"),
    Retreat        UMETA(DisplayName = "Retreat"),
    CallPack       UMETA(DisplayName = "CallPack"),
    Bite           UMETA(DisplayName = "Bite"),
    Roar           UMETA(DisplayName = "Roar")
};

// Per-encounter combat state snapshot
USTRUCT(BlueprintType)
struct FCombat_EncounterState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    ECombat_DinoPhase CurrentPhase = ECombat_DinoPhase::Roaming;

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    ECombat_TacticalAction PendingAction = ECombat_TacticalAction::None;

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    bool bPackMembersNearby = false;

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    int32 PackMemberCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "CombatAI")
    float TimeSinceLastAttack = 0.0f;
};

// Species-specific combat parameters
USTRUCT(BlueprintType)
struct FCombat_SpeciesParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    float ChargeRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    bool bUsesPackTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    bool bCanFlanks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
    float FlankAngleDegrees = 90.0f;
};

/**
 * DinosaurCombatAIComponent
 * Drives tactical combat behavior for dinosaur pawns.
 * Integrates with UNPCBehaviorComponent to alert nearby NPCs when a dino charges.
 * Species-specific params drive T-Rex (solo ambush) vs Raptor (pack flanking) behavior.
 */
UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAIComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Species Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI|Species")
    FCombat_SpeciesParams SpeciesParams;

    // --- Runtime State ---
    UPROPERTY(BlueprintReadOnly, Category = "CombatAI|State")
    FCombat_EncounterState EncounterState;

    // --- Public API ---

    /** Called when this dino detects a target (player or NPC). */
    UFUNCTION(BlueprintCallable, Category = "CombatAI")
    void OnTargetDetected(AActor* Target, float InitialThreat);

    /** Called when target leaves detection radius. */
    UFUNCTION(BlueprintCallable, Category = "CombatAI")
    void OnTargetLost();

    /** Returns the recommended tactical action for this tick. */
    UFUNCTION(BlueprintCallable, Category = "CombatAI")
    ECombat_TacticalAction EvaluateTactics();

    /** Applies damage to target actor (calls UGameplayStatics::ApplyDamage). */
    UFUNCTION(BlueprintCallable, Category = "CombatAI")
    void ExecuteAttack(AActor* Target);

    /** Alerts all UNPCBehaviorComponents within AlertRadius of this dino's position. */
    UFUNCTION(BlueprintCallable, Category = "CombatAI")
    void AlertNearbyNPCs(float AlertRadius = 2000.0f);

    /** Computes the best flank position offset from the target. */
    UFUNCTION(BlueprintCallable, Category = "CombatAI")
    FVector ComputeFlankPosition(AActor* Target, bool bLeftSide) const;

    /** Initializes species params from the Species enum (call in BeginPlay or after setting Species). */
    UFUNCTION(BlueprintCallable, Category = "CombatAI")
    void InitSpeciesParams();

    /** Returns true if this dino is currently in an active combat phase. */
    UFUNCTION(BlueprintPure, Category = "CombatAI")
    bool IsInCombat() const;

private:
    UPROPERTY()
    AActor* CurrentTarget = nullptr;

    float AttackCooldownTimer = 0.0f;

    void TickCombatPhase(float DeltaTime);
    void TransitionToPhase(ECombat_DinoPhase NewPhase);
    ECombat_TacticalAction DecideTacticsForTRex() const;
    ECombat_TacticalAction DecideTacticsForRaptor() const;
    ECombat_TacticalAction DecideTacticsForHerbivore() const;
};
