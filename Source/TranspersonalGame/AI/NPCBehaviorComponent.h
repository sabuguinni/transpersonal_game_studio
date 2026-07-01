#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feed        UMETA(DisplayName = "Feed"),
    Rest        UMETA(DisplayName = "Rest"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float FleeHealthThreshold = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float ChaseSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PackCoordinationRadius = 1500.0f;
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    bool bIsPlayerThreat = false;
};

/**
 * UNPCBehaviorComponent
 * Core behavior component for all NPC dinosaurs and primitive humans.
 * Drives state machine: Idle → Patrol → Alert → Chase → Attack / Flee
 * Supports pack hunting coordination for Raptors.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "NPC Behavior Component")
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State Machine ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_BehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatDetected(AActor* ThreatActor, float ThreatDistance);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnThreatLost();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnTakeDamage(float DamageAmount);

    // --- Memory System ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
    void RecordThreatMemory(FVector Location, float ThreatLevel, bool bIsPlayer);

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    bool HasRecentThreatMemory(float WithinSeconds = 30.0f) const;

    UFUNCTION(BlueprintPure, Category = "NPC|Memory")
    FVector GetLastKnownThreatLocation() const;

    // --- Pack Coordination ---
    UFUNCTION(BlueprintCallable, Category = "NPC|Pack")
    void AlertPackMembers(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC|Pack")
    void OnPackAlertReceived(FVector ThreatLocation, AActor* AlertSource);

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    FNPC_BehaviorConfig BehaviorConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Config")
    float MaxHealth = 1000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float CurrentHealth = 1000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    float CurrentHunger = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    bool bIsAlerted = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
    AActor* CurrentThreatTarget = nullptr;

private:
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;
    ENPC_BehaviorState PreviousState = ENPC_BehaviorState::Idle;

    TArray<FNPC_MemoryEntry> ThreatMemories;

    float StateTimer = 0.0f;
    float HungerDecayRate = 2.0f;

    FVector PatrolHomeLocation = FVector::ZeroVector;
    int32 CurrentPatrolIndex = 0;

    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickFlee(float DeltaTime);
    void TickFeed(float DeltaTime);
    void TickRest(float DeltaTime);

    void ScanForThreats();
    void CleanOldMemories(float MaxAge = 120.0f);
    bool ShouldFlee() const;
};
