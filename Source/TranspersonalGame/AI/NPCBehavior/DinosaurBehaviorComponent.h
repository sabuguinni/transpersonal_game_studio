#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "DinosaurBehaviorComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — NPC_ prefix to avoid collision with other agents
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_DinoAIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ENPC_ThreatResponse : uint8
{
    Ignore      UMETA(DisplayName = "Ignore"),
    Investigate UMETA(DisplayName = "Investigate"),
    Flee        UMETA(DisplayName = "Flee"),
    Attack      UMETA(DisplayName = "Attack"),
    CallPack    UMETA(DisplayName = "Call Pack")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_DinoMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float TimeStamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FNPC_DinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float AttackRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float ChaseRadius = 4000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float RunSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float SprintSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    bool bIsPredator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    int32 MaxPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    ENPC_ThreatResponse DefaultThreatResponse = ENPC_ThreatResponse::Attack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float MemoryDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    float RoarCooldown = 15.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── State Machine ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetAIState(ENPC_DinoAIState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    ENPC_DinoAIState GetAIState() const { return CurrentState; }

    // ── Threat Detection ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool HasActiveThreat() const;

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    AActor* GetPrimaryThreat() const;

    // ── Pack Behavior ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void BroadcastPackAlert(AActor* ThreatActor, float Radius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void OnPackAlertReceived(AActor* ThreatActor, FVector ThreatLocation);

    // ── Patrol ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetPatrolOrigin(FVector Origin) { PatrolOrigin = Origin; }

    // ── Roar ───────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool TryRoar();

    // ── Species Config ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Species")
    FNPC_DinoSpeciesTraits SpeciesTraits;

    // ── State Accessors for AnimInstance ───────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsAttacking() const { return CurrentState == ENPC_DinoAIState::Attack; }

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsChasing() const { return CurrentState == ENPC_DinoAIState::Chase; }

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsDead() const { return CurrentState == ENPC_DinoAIState::Dead; }

    UFUNCTION(BlueprintPure, Category = "NPC|Behavior")
    bool IsRoaring() const { return bIsRoaring; }

private:
    UPROPERTY(VisibleAnywhere, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_DinoAIState CurrentState = ENPC_DinoAIState::Idle;

    UPROPERTY(VisibleAnywhere, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    ENPC_DinoAIState PreviousState = ENPC_DinoAIState::Idle;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_DinoMemoryEntry> ThreatMemory;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Patrol", meta = (AllowPrivateAccess = "true"))
    FVector PatrolOrigin = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, Category = "NPC|Patrol", meta = (AllowPrivateAccess = "true"))
    int32 PatrolPointIndex = 0;

    UPROPERTY(VisibleAnywhere, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    bool bIsRoaring = false;

    UPROPERTY(VisibleAnywhere, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    float LastRoarTime = -999.0f;

    UPROPERTY(VisibleAnywhere, Category = "NPC|State", meta = (AllowPrivateAccess = "true"))
    float StateEnteredTime = 0.0f;

    // Internal helpers
    void TickThreatMemory(float DeltaTime);
    void EvaluateThreatResponse();
    ENPC_ThreatResponse ClassifyThreat(float ThreatLevel) const;
    void OnStateEntered(ENPC_DinoAIState NewState);
    void OnStateExited(ENPC_DinoAIState OldState);
};
