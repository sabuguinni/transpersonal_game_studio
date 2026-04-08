#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Apex        UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class EDinosaurCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Investigate UMETA(DisplayName = "Investigate"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Territorial UMETA(DisplayName = "Territorial"),
    Pack        UMETA(DisplayName = "Pack Behavior")
};

USTRUCT(BlueprintType)
struct FDinosaurCombatProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Cautious;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float TerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float HuntingRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float PackCoordinationRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    float DomesticationDifficulty = 1.0f;
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> DangerousLocations;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    AActor* LastAttacker = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastThreatTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector SafeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float PlayerTrustLevel = 0.0f; // For domestication
};

/**
 * Advanced Combat AI Controller for Dinosaurs
 * Implements tactical combat behavior, pack coordination, and individual personality traits
 */
UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    FDinosaurCombatProfile CombatProfile;

    UPROPERTY(BlueprintReadWrite, Category = "AI State")
    EDinosaurCombatState CurrentCombatState = EDinosaurCombatState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "AI Memory")
    FDinosaurMemory DinosaurMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    class UBlackboardAsset* CombatBlackboard;

    // Pack behavior
    UPROPERTY(BlueprintReadWrite, Category = "Pack Behavior")
    TArray<ADinosaurCombatAI*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadWrite, Category = "Pack Behavior")
    ADinosaurCombatAI* PackLeader = nullptr;

    // Combat timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float AttackCooldown = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat Timing")
    float LastAttackTime = 0.0f;

    // Dynamic difficulty adjustment
    UPROPERTY(BlueprintReadWrite, Category = "Dynamic Difficulty")
    float PlayerSkillAssessment = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Dynamic Difficulty")
    int32 PlayerDeathCount = 0;

public:
    // Combat behavior functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(EDinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector FindBestAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatAssessment(AActor* PotentialThreat);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ADinosaurCombatAI* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SendPackAlert(AActor* Threat, FVector ThreatLocation);

    // Domestication system
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessPlayerInteraction(float InteractionQuality);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanBeDomesticated() const;

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    float GetDomesticationProgress() const;

    // Dynamic difficulty
    UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
    void AdjustDifficultyBasedOnPlayerPerformance();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
    void OnPlayerDeath();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
    void OnPlayerSuccessfulEscape();

protected:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus);

    // Internal combat logic
    void UpdateCombatBehavior(float DeltaTime);
    void ProcessMemoryDecay(float DeltaTime);
    void EvaluateTacticalOptions();
    
    // Utility functions
    float CalculateThreatLevel(AActor* PotentialThreat) const;
    bool IsInPackFormation() const;
    FVector GetFlankingPosition(AActor* Target) const;
    void UpdateBlackboardValues();
};