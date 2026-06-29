#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Enums — global scope (UHT rule 1)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl UMETA(DisplayName = "Pterodactyl"),
};

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Dead        UMETA(DisplayName = "Dead"),
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Charge      UMETA(DisplayName = "Charge"),
    Stomp       UMETA(DisplayName = "Stomp"),
    TailSwipe   UMETA(DisplayName = "TailSwipe"),
};

// ============================================================
// Structs — global scope (UHT rule 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float SightRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float HearingRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackDamage = 35.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float ChaseSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float PatrolSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float TerritoryRadius = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    bool bIsPack = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float PackAlertRadius = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_AttackType PrimaryAttack = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    float FleeHealthThreshold = 0.2f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float ThreatLevel = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    float LastSeenTime = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Threat")
    FVector LastKnownLocation = FVector::ZeroVector;
};

// ============================================================
// Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnStateChanged,
    ECombat_DinoState, OldState,
    ECombat_DinoState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnAttackLanded,
    AActor*, Target,
    float, DamageDealt);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnPackAlert,
    AActor*, SpottedActor);

// ============================================================
// ADinosaurCombatAIController
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    // ---- Lifecycle ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- State Machine ----
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|AI")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    // ---- Threat System ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel, const FVector& LastKnownLoc);

    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    float GetThreatLevel(AActor* ThreatActor) const;

    // ---- Attack ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool CanAttack() const;

    // ---- Pack Behaviour ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastPackAlert(AActor* SpottedActor);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackAlert(AActor* SpottedActor, const FVector& AlertLocation);

    // ---- Territory ----
    UFUNCTION(BlueprintPure, Category = "Combat|Territory")
    bool IsWithinTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Territory")
    void SetTerritoryCenter(const FVector& Center);

    // ---- Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoSpeciesConfig SpeciesConfig;

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnAttackLanded OnAttackLanded;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnPackAlert OnPackAlert;

    // ---- Runtime State (read-only) ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastAttack = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatEntry> ThreatMemory;

private:
    // ---- Perception ----
    UPROPERTY(VisibleAnywhere, Category = "Combat|Perception")
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, Category = "Combat|Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, Category = "Combat|Perception")
    UAISenseConfig_Hearing* HearingConfig;

    // ---- Internal ----
    void TickStateMachine(float DeltaTime);
    void TickChaseState(float DeltaTime);
    void TickAttackState(float DeltaTime);
    void TickPatrolState(float DeltaTime);
    void TickInvestigateState(float DeltaTime);
    void PurgeExpiredThreats();
    void ApplyMovementSpeed(float Speed);

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    float ThreatMemoryExpiry = 45.f;
    float PatrolWaitTimer = 0.f;
    FVector PatrolTarget = FVector::ZeroVector;
    bool bHasPatrolTarget = false;
};
