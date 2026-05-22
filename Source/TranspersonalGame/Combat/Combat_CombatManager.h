#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Combat_CombatManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatEvent, AActor*, Attacker, AActor*, Target, float, Damage);

/**
 * Combat Manager - Central system for managing all combat interactions
 * Handles damage dealing, combat state tracking, and combat events
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_CombatManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCombat_CombatManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Combat management functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RegisterCombatant(AActor* Actor, ECombat_CombatantType CombatantType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UnregisterCombatant(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool DealDamage(AActor* Attacker, AActor* Target, float Damage, const FVector& HitLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartCombat(AActor* Initiator, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombat(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetCombatantsInRange(const FVector& Location, float Radius, ECombat_CombatantType CombatantType = ECombat_CombatantType::Any) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* FindNearestEnemy(AActor* Searcher, float MaxRange = 2000.0f) const;

    // Combat state queries
    UFUNCTION(BlueprintCallable, Category = "Combat")
    int32 GetActiveCombatCount() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    TArray<AActor*> GetAllCombatants() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_CombatantType GetCombatantType(AActor* Actor) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatEvent OnCombatStarted;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatEvent OnDamageDealt;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatEvent OnCombatEnded;

protected:
    // Combat tracking
    UPROPERTY()
    TMap<AActor*, ECombat_CombatantType> RegisteredCombatants;

    UPROPERTY()
    TSet<AActor*> ActorsInCombat;

    UPROPERTY()
    TMap<AActor*, float> LastCombatTime;

    // Configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float CombatTimeoutDuration;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    float DefaultDamageMultiplier;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Settings")
    bool bEnableCombatLogging;

private:
    // Internal functions
    void CleanupExpiredCombat();
    bool CanDealDamage(AActor* Attacker, AActor* Target) const;
    void BroadcastCombatEvent(const FOnCombatEvent& Event, AActor* Attacker, AActor* Target, float Damage);

    // Timer handle for cleanup
    FTimerHandle CleanupTimerHandle;
};