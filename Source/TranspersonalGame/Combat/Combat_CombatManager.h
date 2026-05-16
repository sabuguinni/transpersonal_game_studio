#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Combat_CombatManager.generated.h"

class APawn;
class ACharacter;
class UBehaviorTreeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, APawn*, Attacker, APawn*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealt, APawn*, Attacker, APawn*, Victim, float, Damage);

/**
 * Central combat management system for dinosaur AI and player combat
 * Handles combat state tracking, damage calculation, and AI coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_CombatManager : public UObject
{
    GENERATED_BODY()

public:
    UCombat_CombatManager();

    // Core combat management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeCombatManager(UWorld* InWorld);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterCombatant(APawn* Combatant, ECombat_CombatRole Role);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterCombatant(APawn* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool StartCombat(APawn* Attacker, APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat(APawn* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateDamage(APawn* Attacker, APawn* Target, ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(APawn* Attacker, APawn* Target, float Damage);

    // Combat state queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
    bool IsInCombat(APawn* Combatant) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
    APawn* GetCombatTarget(APawn* Combatant) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
    TArray<APawn*> GetNearbyEnemies(APawn* Combatant, float Radius) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
    float GetCombatThreatLevel(APawn* Combatant) const;

    // AI coordination
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CoordinatePackAttack(const TArray<APawn*>& PackMembers, APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatAI(float DeltaTime);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStarted;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatEnded;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnDamageDealt OnDamageDealt;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    UWorld* CombatWorld;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TMap<APawn*, ECombat_CombatRole> RegisteredCombatants;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TMap<APawn*, APawn*> CombatPairs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TMap<APawn*, float> CombatTimers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float BaseDamageMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float CombatTimeout = 30.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float PackCoordinationRadius = 1500.0f;

private:
    void CleanupExpiredCombat();
    float CalculateSpeciesDamageModifier(APawn* Attacker, APawn* Target);
    void NotifyAIOfCombatState(APawn* Combatant, bool bInCombat);
};