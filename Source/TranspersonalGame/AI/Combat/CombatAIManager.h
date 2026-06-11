#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"
#include "AI/Dinosaur/DinosaurBehaviorTree.h"
#include "CombatAIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* ThreatSource = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionModifier = 1.0f;

    FCombat_ThreatAssessment()
    {
        ThreatLevel = ECombat_ThreatLevel::None;
        ThreatDistance = 0.0f;
        ThreatSource = nullptr;
        AggressionModifier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Combat AI Core Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_ThreatAssessment AssessThreat(AActor* PotentialThreat, AActor* Assessor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> GetNearbyThreats(AActor* Assessor, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCombatEncounter(AActor* Aggressor, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombatEncounter(AActor* Combatant);

    // Tactical Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    FVector CalculateFlankingPosition(AActor* Attacker, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    bool ShouldRetreat(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    void CoordinatePackAttack(TArray<AActor*> PackMembers, AActor* Target);

protected:
    // Combat Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> ActiveCombatants;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TMap<AActor*, FCombat_ThreatAssessment> ThreatAssessments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ThreatUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float MaxCombatRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float FlankingAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float RetreatHealthThreshold = 0.3f;

    // Timers
    FTimerHandle ThreatAssessmentTimer;

private:
    // Internal Functions
    void UpdateThreatAssessments();
    void ProcessCombatLogic();
    float CalculateThreatScore(AActor* Threat, AActor* Assessor);
    bool IsValidCombatant(AActor* Actor);
};