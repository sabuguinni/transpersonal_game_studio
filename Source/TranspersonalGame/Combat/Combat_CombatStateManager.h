#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Combat_CombatStateManager.generated.h"

class ACombat_CombatManager;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* Instigator;

    UPROPERTY(BlueprintReadOnly)
    AActor* Target;

    UPROPERTY(BlueprintReadOnly)
    float Damage;

    UPROPERTY(BlueprintReadOnly)
    FVector Location;

    UPROPERTY(BlueprintReadOnly)
    float Timestamp;

    FCombat_CombatEvent()
    {
        Instigator = nullptr;
        Target = nullptr;
        Damage = 0.0f;
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* ThreatActor;

    UPROPERTY(BlueprintReadOnly)
    float ThreatLevel;

    UPROPERTY(BlueprintReadOnly)
    float LastSeenTime;

    UPROPERTY(BlueprintReadOnly)
    FVector LastKnownPosition;

    UPROPERTY(BlueprintReadOnly)
    bool bIsActive;

    FCombat_ThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = 0.0f;
        LastSeenTime = 0.0f;
        LastKnownPosition = FVector::ZeroVector;
        bIsActive = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCombat_CombatStateManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCombat_CombatStateManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Combat state management
    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void RegisterCombatActor(AActor* Actor, float CombatRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void UnregisterCombatActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    bool IsActorInCombat(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void StartCombat(AActor* Instigator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void EndCombat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void ProcessCombatEvent(const FCombat_CombatEvent& CombatEvent);

    // Threat tracking
    UFUNCTION(BlueprintCallable, Category = "Threat System")
    void AddThreat(AActor* ThreatActor, AActor* TargetActor, float ThreatAmount);

    UFUNCTION(BlueprintCallable, Category = "Threat System")
    void RemoveThreat(AActor* ThreatActor, AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Threat System")
    float GetThreatLevel(AActor* ThreatActor, AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Threat System")
    TArray<FCombat_ThreatData> GetThreatsForActor(AActor* Actor) const;

    // Combat queries
    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    TArray<AActor*> GetNearbyEnemies(AActor* Actor, float Radius = 2000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    AActor* GetNearestEnemy(AActor* Actor, float MaxRange = 1500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    bool IsLocationSafe(const FVector& Location, float SafetyRadius = 500.0f) const;

    // Combat statistics
    UFUNCTION(BlueprintCallable, Category = "Combat Stats")
    int32 GetActiveCombatCount() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Stats")
    float GetAverageCombatDuration() const;

protected:
    // Internal tracking
    UPROPERTY()
    TMap<AActor*, float> CombatActors;

    UPROPERTY()
    TMap<AActor*, TArray<FCombat_ThreatData>> ThreatMap;

    UPROPERTY()
    TArray<FCombat_CombatEvent> RecentCombatEvents;

    UPROPERTY()
    TMap<AActor*, float> CombatStartTimes;

    // Combat parameters
    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    float CombatTimeoutDuration;

    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    float ThreatDecayRate;

    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    float MaxThreatDistance;

    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    int32 MaxCombatEventsHistory;

private:
    void UpdateCombatStates();
    void DecayThreats();
    void CleanupOldEvents();

    FTimerHandle CombatUpdateTimer;
};