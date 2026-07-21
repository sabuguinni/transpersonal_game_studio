#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinoCombatAIController.generated.h"

// Combat threat level for tactical AI decisions
UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

// Combat stance — drives animation and attack selection
UENUM(BlueprintType)
enum class ECombat_DinoStance : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Charging    UMETA(DisplayName = "Charging"),
    Biting      UMETA(DisplayName = "Biting"),
    TailSwiping UMETA(DisplayName = "TailSwiping"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Dead        UMETA(DisplayName = "Dead")
};

// Attack type selection
UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    TailSwipe   UMETA(DisplayName = "TailSwipe"),
    BodySlam    UMETA(DisplayName = "BodySlam"),
    Pounce      UMETA(DisplayName = "Pounce")
};

// Blackboard key names (string constants to avoid typos)
namespace CombatBBKeys
{
    static const FName TargetActor     = TEXT("TargetActor");
    static const FName LastKnownPos    = TEXT("LastKnownPos");
    static const FName ThreatLevel     = TEXT("ThreatLevel");
    static const FName DinoStance      = TEXT("DinoStance");
    static const FName AttackCooldown  = TEXT("AttackCooldown");
    static const FName PackLeader      = TEXT("PackLeader");
    static const FName FlankPosition   = TEXT("FlankPosition");
    static const FName bIsPackLeader   = TEXT("bIsPackLeader");
    static const FName bTargetVisible  = TEXT("bTargetVisible");
    static const FName HealthNorm      = TEXT("HealthNorm");
    static const FName bRetreatFlag    = TEXT("bRetreatFlag");
}

UCLASS(ClassGroup = "TranspersonalGame|Combat", meta = (DisplayName = "Dino Combat AI Controller"))
class TRANSPERSONALGAME_API ADinoCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinoCombatAIController();

    // --- Perception ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception")
    UAIPerceptionComponent* PerceptionComponent_Combat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Perception")
    UAISenseConfig_Damage* DamageConfig;

    // --- Behavior Tree ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|BehaviorTree")
    UBehaviorTree* CombatBehaviorTree;

    // --- Combat State ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoStance CurrentStance;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_AttackType LastAttackType;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float AttackCooldownRemaining;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    bool bIsPackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    AActor* CurrentTarget;

    // --- Pack Coordination ---
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Pack")
    TArray<ADinoCombatAIController*> PackMembers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Pack")
    float PackCoordinationRadius;

    // --- Tuning ---
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tuning")
    float SightRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tuning")
    float SightLoseSightRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tuning")
    float HearingRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tuning")
    float AttackRange_Bite;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tuning")
    float AttackRange_TailSwipe;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tuning")
    float AttackCooldown_Base;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Tuning")
    float RetreatHealthThreshold;

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearCombatTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_AttackType SelectBestAttack(float DistanceToTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void NotifyPackOfTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector ComputeFlankPosition(AActor* Target, int32 FlankIndex, int32 TotalFlankers) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthNormalized() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    void InitPerception();
    void UpdateBlackboard();
    void ScanForPackMembers();
    void CoordinatePackAttack();

    float TimeSinceLastPackScan;
    static constexpr float PackScanInterval = 2.0f;
};
