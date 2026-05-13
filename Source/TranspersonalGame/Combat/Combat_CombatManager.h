#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Combat_CombatManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCombat_OnCombatEvent, AActor*, Attacker, AActor*, Target, float, Damage);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_CombatManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCombat_CombatManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Combat System Core
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool InitiateCombat(AActor* Attacker, AActor* Target, float Damage = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetCombatants();

    // Damage System
    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateDamage(AActor* Attacker, AActor* Target, float BaseDamage);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(AActor* Target, float Damage, AActor* DamageSource = nullptr);

    // Threat System
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddThreat(AActor* Target, AActor* ThreatSource, float ThreatAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetHighestThreatTarget(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetThreatLevel(AActor* Target, AActor* ThreatSource);

    // Combat Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FCombat_OnCombatEvent OnCombatStarted;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FCombat_OnCombatEvent OnDamageDealt;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FCombat_OnCombatEvent OnCombatEnded;

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterCombatant(AActor* Combatant);

    // Proximity Combat Detection
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CheckProximityCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetNearbyEnemies(AActor* Actor, float Range = 1000.0f);

protected:
    // Active combatants
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<AActor*> ActiveCombatants;

    // Threat table - maps Target -> ThreatSource -> ThreatAmount
    UPROPERTY()
    TMap<AActor*, TMap<AActor*, float>> ThreatTable;

    // Combat pairs tracking
    UPROPERTY()
    TMap<AActor*, AActor*> CombatPairs;

    // Combat timers
    UPROPERTY()
    TMap<AActor*, float> CombatTimers;

    // Combat settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatTimeout = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ProximityCheckInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float ThreatDecayRate = 1.0f;

private:
    // Internal helpers
    void UpdateCombatTimers(float DeltaTime);
    void DecayThreat(float DeltaTime);
    bool AreEnemies(AActor* Actor1, AActor* Actor2);
    
    FTimerHandle ProximityCheckTimer;
    FTimerHandle CombatUpdateTimer;
};