#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAI.generated.h"

// ── Combat State Enum ──
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
};

// ── Dinosaur Species Enum ──
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
};

// ── Combat Threat Data ──
USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;
};

// ── Species Trait Data ──
USTRUCT(BlueprintType)
struct FCombat_SpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FleeHealthThreshold = 0.2f;
};

// ── Main Combat AI Component ──
UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    // ── Threat System ──
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetThreatLevel(AActor* Actor) const;

    // ── Combat Actions ──
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BeginStalking(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttemptFlee();

    // ── Pack Coordination ──
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPackMember(UCombat_DinosaurCombatAI* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SignalPackAttack(AActor* Target);

    // ── Species Config ──
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetSpecies(ECombat_DinoSpecies Species);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FCombat_SpeciesTraits GetSpeciesTraits() const { return SpeciesTraits; }

    // ── Properties ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_SpeciesTraits SpeciesTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsAlpha = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<UCombat_DinosaurCombatAI*> PackMembers;

private:
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;
    TArray<FCombat_ThreatEntry> ThreatList;

    void UpdateStateMachine(float DeltaTime);
    void UpdateThreatDecay(float DeltaTime);
    FCombat_SpeciesTraits BuildTraitsForSpecies(ECombat_DinoSpecies Species) const;

    float StateTimer = 0.0f;
    float AttackCooldown = 0.0f;
    AActor* CurrentTarget = nullptr;
};
