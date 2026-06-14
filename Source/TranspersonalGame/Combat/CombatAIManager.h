#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "CombatAIManager.generated.h"

class ACombat_DinosaurAIController;
class UBehaviorTreeComponent;
class UBlackboardComponent;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatAIManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat zone detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    USphereComponent* CombatDetectionSphere;

    // Combat state management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AlertRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackCoordinationRange = 1500.0f;

    // AI behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float HuntingEfficiency = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float TerritorialRadius = 2500.0f;

    // Combat tactics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bEnablePackHunting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bEnableFlankingBehavior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bEnableAmbushTactics = true;

    // Dynamic difficulty
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float PlayerSkillLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float DifficultyScaling = 1.0f;

public:
    // Combat management functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateCombatEncounter(AActor* Player, AActor* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombatEncounter(AActor* Player, AActor* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackHunting(TArray<AActor*> PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteFlankingManeuver(AActor* Dinosaur, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TriggerAmbushBehavior(AActor* Dinosaur, AActor* Target);

    // AI state queries
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInCombat(AActor* Dinosaur) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanEngageCombat(AActor* Dinosaur, AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetOptimalAttackDistance(AActor* Dinosaur) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    FVector GetFlankingPosition(AActor* Dinosaur, AActor* Target) const;

    // Difficulty adaptation
    UFUNCTION(BlueprintCallable, Category = "Difficulty")
    void AdjustDifficultyBasedOnPerformance(bool PlayerWon, float EncounterDuration);

    UFUNCTION(BlueprintCallable, Category = "Difficulty")
    void SetDynamicDifficulty(float NewDifficulty);

private:
    // Internal combat tracking
    TMap<AActor*, float> CombatTimers;
    TMap<AActor*, AActor*> CombatPairs;
    TArray<AActor*> ActiveCombatants;

    // Pack hunting coordination
    TMap<AActor*, TArray<AActor*>> PackGroups;
    TMap<AActor*, FVector> FlankingPositions;

    // Performance tracking for difficulty
    float AverageEncounterDuration = 30.0f;
    float PlayerWinRate = 0.5f;
    int32 TotalEncounters = 0;

    // Helper functions
    void UpdateCombatTimers(float DeltaTime);
    void ManagePackCoordination();
    void CalculateFlankingPositions();
    bool ValidateFlankingPosition(const FVector& Position, AActor* Target) const;
};