#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../../Shared/SharedTypes.h"
#include "CombatAIManager.generated.h"

class ACombat_DinosaurPawn;
class UCombat_BehaviorComponent;

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4
};

UENUM(BlueprintType)
enum class ECombat_Formation : uint8
{
    None = 0,
    Circle = 1,
    Line = 2,
    Wedge = 3,
    Ambush = 4
};

USTRUCT(BlueprintType)
struct FCombat_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EncounterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FCombat_EncounterData()
    {
        Location = FVector::ZeroVector;
        ThreatLevel = ECombat_ThreatLevel::None;
        DinosaurCount = 0;
        EncounterRadius = 1000.0f;
        bIsActive = false;
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
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Combat AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TacticalUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 MaxSimultaneousCombats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnablePackCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnableTacticalPositioning;

    // Combat Encounters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounters")
    TArray<FCombat_EncounterData> CombatEncounters;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<ACombat_DinosaurPawn*> ActiveCombatants;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 CurrentActiveCombats;

    // Timers
    float TacticalUpdateTimer;
    float EncounterCheckTimer;

public:
    // Combat Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatSystem();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatant(ACombat_DinosaurPawn* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatant(ACombat_DinosaurPawn* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool StartCombatEncounter(const FVector& Location, ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombatEncounter(const FVector& Location);

    // Tactical AI
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void UpdateTacticalPositions();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector GetOptimalAttackPosition(ACombat_DinosaurPawn* Attacker, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    ECombat_Formation GetRecommendedFormation(const TArray<ACombat_DinosaurPawn*>& Pack);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack(const TArray<ACombat_DinosaurPawn*>& Pack, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void ExecuteFlankingManeuver(ACombat_DinosaurPawn* Leader, const TArray<ACombat_DinosaurPawn*>& Flankers);

    // Encounter System
    UFUNCTION(BlueprintCallable, Category = "Encounters")
    void CreateCombatEncounter(const FVector& Location, ECombat_ThreatLevel ThreatLevel, int32 DinosaurCount);

    UFUNCTION(BlueprintCallable, Category = "Encounters")
    void CheckEncounterTriggers();

    UFUNCTION(BlueprintCallable, Category = "Encounters")
    FCombat_EncounterData* GetNearestEncounter(const FVector& Location);

    // Utility
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsLocationInCombat(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    int32 GetCombatantsInRange(const FVector& Location, float Range) const;

private:
    void UpdateCombatSystem(float DeltaTime);
    void ProcessActiveCombats();
    void CleanupInactiveCombatants();
    bool CanStartNewCombat() const;
};