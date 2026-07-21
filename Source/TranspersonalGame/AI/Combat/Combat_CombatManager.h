#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Combat_CombatManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatEncounter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> Enemies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float EncounterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsActive;

    FCombat_CombatEncounter()
    {
        Location = FVector::ZeroVector;
        ThreatLevel = 0.5f;
        EncounterRadius = 1000.0f;
        bIsActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PlayerThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownPlayerPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TimeSinceLastSighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FVector> FlankingPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bPlayerInCombat;

    FCombat_TacticalData()
    {
        PlayerThreatLevel = 0.0f;
        LastKnownPlayerPosition = FVector::ZeroVector;
        TimeSinceLastSighting = 0.0f;
        bPlayerInCombat = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_CombatManager : public AActor
{
    GENERATED_BODY()

public:
    ACombat_CombatManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat System")
    TArray<FCombat_CombatEncounter> ActiveEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat System")
    FCombat_TacticalData CurrentTacticalSituation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat System")
    float GlobalThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat System")
    float CombatUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat System")
    float MaxCombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat System")
    int32 MaxSimultaneousEncounters;

private:
    float LastUpdateTime;
    TArray<AActor*> TrackedEnemies;
    AActor* PlayerCharacter;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void InitializeCombatSystem();

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void RegisterEnemy(AActor* Enemy, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void UnregisterEnemy(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void StartCombatEncounter(const FVector& Location, const TArray<AActor*>& Enemies);

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void EndCombatEncounter(int32 EncounterIndex);

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void UpdateTacticalSituation();

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    float CalculatePlayerThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    TArray<FVector> GenerateFlankingPositions(const FVector& PlayerPos, int32 NumPositions) const;

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    bool IsPlayerInCombat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    AActor* GetNearestEnemy(const FVector& Position) const;

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void SetGlobalThreatLevel(float NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    float GetGlobalThreatLevel() const { return GlobalThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Combat System")
    void ProcessCombatAI(float DeltaTime);
};