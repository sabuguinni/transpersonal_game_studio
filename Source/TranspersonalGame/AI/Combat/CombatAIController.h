#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../NPC/NPCBehaviorComponent.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Melee       UMETA(DisplayName = "Melee Attack"),
    Charge      UMETA(DisplayName = "Charge Attack"),
    Bite        UMETA(DisplayName = "Bite Attack"),
    Tail        UMETA(DisplayName = "Tail Swipe"),
    Stomp       UMETA(DisplayName = "Stomp Attack"),
    Pack        UMETA(DisplayName = "Pack Coordination")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation;

    FCombat_ThreatAssessment()
    {
        ThreatActor = nullptr;
        ThreatLevel = ECombat_ThreatLevel::None;
        Distance = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FCombat_AttackPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ExecutionTime;

    FCombat_AttackPattern()
    {
        AttackType = ECombat_AttackType::Melee;
        Damage = 10.0f;
        Range = 200.0f;
        Cooldown = 2.0f;
        ExecutionTime = 1.0f;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    // NPC Behavior Integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UNPCBehaviorComponent* NPCBehavior;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float RetreatThreshold;

    // Threat Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<FCombat_ThreatAssessment> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_ThreatLevel CurrentThreatLevel;

    // Attack Patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FCombat_AttackPattern> AvailableAttacks;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombat_AttackPattern CurrentAttack;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackMember;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    ACombatAIController* PackLeader;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AssessThreat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_ThreatLevel CalculateThreatLevel(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatList();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* SelectBestTarget();

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(const FCombat_AttackPattern& Attack);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_AttackPattern SelectBestAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExitCombatMode();

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ACombatAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void SendPackSignal(const FString& Signal);

    // Integration with NPC Behavior
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateCombatEmotions();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void RecordCombatMemory(AActor* Opponent, bool bVictorious);

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName TargetActorKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName ThreatLevelKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName IsInCombatKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName LastKnownLocationKey;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName PackLeaderKey;

private:
    void InitializePerception();
    void InitializeBlackboard();
    void FindNearbyPackMembers();
    float CalculateDistance(AActor* Actor);
    bool IsValidTarget(AActor* Actor);
    void CleanupOldThreats();
};