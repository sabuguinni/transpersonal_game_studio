#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Aware       UMETA(DisplayName = "Aware"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Carnotaurus     UMETA(DisplayName = "Carnotaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float DetectionRadius = 1500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackDamage = 40.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ChaseSpeed = 600.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float PatrolSpeed = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    int32 PackSize = 1;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── Perception ──────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetPrimaryTarget() const { return PrimaryTarget; }

    // ── State ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetThreatLevel(ECombat_ThreatLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_ThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitDinoStats(ECombat_DinoSpecies Species);

    // ── Combat Actions ───────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ChaseTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromThreat(AActor* Threat);

    // ── Pack Coordination ────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AlertPackMembers(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssignFlankRole(int32 RoleIndex);

    // ── Blackboard Keys ─────────────────────────────────────────
    static const FName BB_TargetActor;
    static const FName BB_ThreatLevel;
    static const FName BB_PatrolOrigin;
    static const FName BB_LastKnownPos;
    static const FName BB_CanAttack;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_DinoSpecies DinoSpecies;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    FCombat_DinoStats DinoStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    AActor* PrimaryTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float LastAttackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    int32 FlankRoleIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PackAlertRadius;

private:
    void UpdateThreatList(float DeltaTime);
    void SelectBestTarget();
    void UpdateBlackboard();
    FCombat_DinoStats GetStatsForSpecies(ECombat_DinoSpecies Species) const;
};
