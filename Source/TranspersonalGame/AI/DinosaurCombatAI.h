// DinosaurCombatAI.h
// Agent #12 — Combat & Enemy AI
// Dinosaur combat intelligence: threat detection, pack tactics, territorial behavior

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Pteranodon          UMETA(DisplayName = "Pteranodon"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus")
};

UENUM(BlueprintType)
enum class ECombat_ThreatState : uint8
{
    Passive             UMETA(DisplayName = "Passive — unaware of player"),
    Alerted             UMETA(DisplayName = "Alerted — detected player"),
    Stalking            UMETA(DisplayName = "Stalking — tracking player"),
    Charging            UMETA(DisplayName = "Charging — direct attack"),
    Flanking            UMETA(DisplayName = "Flanking — pack maneuver"),
    Retreating          UMETA(DisplayName = "Retreating — wounded/outmatched"),
    Territorial         UMETA(DisplayName = "Territorial — defending zone"),
    Feeding             UMETA(DisplayName = "Feeding — occupied with kill")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite                UMETA(DisplayName = "Bite — primary jaw attack"),
    Claw                UMETA(DisplayName = "Claw — slashing strike"),
    Charge              UMETA(DisplayName = "Charge — momentum knockback"),
    TailSwipe           UMETA(DisplayName = "Tail Swipe — area knockback"),
    Stomp               UMETA(DisplayName = "Stomp — ground pound"),
    Pounce              UMETA(DisplayName = "Pounce — leap and pin")
};

// ============================================================
// STRUCTS — must be at global scope
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
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float RetreatHealthThreshold = 0.25f;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    float TimeLastSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    bool bIsVisible = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    bool bIsWounded = false;
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    AActor* AlphaLeader = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    FVector FlankPositionLeft = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    FVector FlankPositionRight = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    bool bFlankingActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    int32 DecoyIndex = 0;
};

// ============================================================
// COMPONENT CLASS
// ============================================================

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Combat AI")
class TRANSPERSONALGAME_API UDinosaurCombatAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Species Configuration ──────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_DinoStats Stats;

    // ── Current State ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_ThreatState CurrentThreatState = ECombat_ThreatState::Passive;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_ThreatMemory PrimaryThreat;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_PackCoordination PackData;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float TimeSinceLastAttack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsAlpha = false;

    // ── Combat Functions ───────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiatePackFlank(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void BroadcastThreatToPackMembers(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatLevel(AActor* PotentialThreat) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetFlankPosition(bool bLeftFlank, AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplySpeciesDefaults();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AttackType SelectBestAttack() const;

    // ── Events (Blueprint-implementable) ───────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnThreatDetected(AActor* Threat);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnAttackExecuted(ECombat_AttackType AttackType, float DamageDealt);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnPackFlankInitiated(FVector LeftPos, FVector RightPos);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnRetreatDecision();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Events")
    void OnThreatLost();

private:
    void UpdatePassiveState(float DeltaTime);
    void UpdateAlertedState(float DeltaTime);
    void UpdateStalkingState(float DeltaTime);
    void UpdateChargingState(float DeltaTime);
    void UpdateFlankingState(float DeltaTime);
    void UpdateRetreatState(float DeltaTime);

    FVector HomeLocation;
    float StateTimer = 0.0f;
    bool bHomeLocationSet = false;
};
