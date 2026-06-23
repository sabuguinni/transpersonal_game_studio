#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// ENUMS — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    Pterodactyl         UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
};

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Charging        UMETA(DisplayName = "Charging"),
    Biting          UMETA(DisplayName = "Biting"),
    Retreating      UMETA(DisplayName = "Retreating"),
    PackCoordinate  UMETA(DisplayName = "Pack Coordinate"),
    Ambushing       UMETA(DisplayName = "Ambushing"),
    Circling        UMETA(DisplayName = "Circling"),
};

UENUM(BlueprintType)
enum class ECombat_ThreatResponse : uint8
{
    Ignore          UMETA(DisplayName = "Ignore"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Stalk           UMETA(DisplayName = "Stalk"),
    Attack          UMETA(DisplayName = "Attack"),
    FleeFromThreat  UMETA(DisplayName = "Flee From Threat"),
    CallPack        UMETA(DisplayName = "Call Pack"),
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_SpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float DetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bUsesAmbushTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    bool bFlankingBehavior = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float RoarRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    int32 MaxPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Traits")
    float TerritoryRadius = 3000.0f;
};

USTRUCT(BlueprintType)
struct FCombat_TacticalMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    int32 FailedAttackCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    bool bTargetEnteredCover = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    FVector FlankApproachVector = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    float AggressionLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FCombat_PackSignal
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    ECombat_TacticalState RequestedState = ECombat_TacticalState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    float SignalStrength = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    bool bIsAlphaSignal = false;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
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
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Species")
    FCombat_SpeciesTraits SpeciesTraits;

    // ---- Tactical State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_TacticalState CurrentTacticalState = ECombat_TacticalState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_ThreatResponse ThreatResponse = ECombat_ThreatResponse::Ignore;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    FCombat_TacticalMemory TacticalMemory;

    // ---- Pack Coordination ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    bool bIsAlpha = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<ADinosaurCombatAIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    FCombat_PackSignal LastPackSignal;

    // ---- Target ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    float TimeSinceLastAttack = 0.0f;

    // ---- Perception ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception")
    UAIPerceptionComponent* AIPerception;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception")
    UAISenseConfig_Hearing* HearingConfig;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetSpecies(ECombat_DinoSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void RegisterPackMember(ADinosaurCombatAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void BroadcastPackSignal(FVector TargetLoc, ECombat_TacticalState RequestedState);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ReceivePackSignal(const FCombat_PackSignal& Signal);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    float CalculateFlankAngle() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    bool IsTargetInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void Roar();

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    FVector GetFlankPosition(int32 FlankIndex, int32 TotalFlankers) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void ApplySpeciesDefaults(ECombat_DinoSpecies InSpecies);

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    ECombat_ThreatResponse EvaluateThreat(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat|AI")
    void NotifyNearbyNPCs(FVector ThreatLocation, float DangerRadius);

protected:
    void UpdateTacticalBehavior(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    void ScanForTargets();
    void ExecuteFlankingManeuver();
    void ExecuteAmbushTactics();
    void UpdateTacticalMemory(float DeltaTime);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    float FlankAngleOffset = 0.0f;
    float RoarCooldown = 0.0f;
    float PackCoordinationTimer = 0.0f;
    int32 FlankIndexInPack = 0;
};
