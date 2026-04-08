// VFX Manager - Jurassic Survival Game
// Transpersonal Game Studio - VFX Agent #17
// CYCLE_ID: PROD_JURASSIC_001

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "VFXManager.generated.h"

/**
 * Central manager for all VFX systems in the game
 * Handles LOD, performance optimization, and effect pooling
 * Philosophy: "A good VFX is one the player doesn't know they saw, but made the moment unforgettable"
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFXManager : public AActor
{
    GENERATED_BODY()

public:
    AVFXManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core VFX Categories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    TMap<FString, UNiagaraSystem*> EnvironmentalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creatures")
    TMap<FString, UNiagaraSystem*> CreatureEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    TMap<FString, UNiagaraSystem*> SurvivalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temporal")
    TMap<FString, UNiagaraSystem*> TemporalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TMap<FString, UNiagaraSystem*> DestructionEffects;

    // Performance Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance_Tier1 = 2000.0f; // 20m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance_Tier2 = 5000.0f; // 50m

    // Active Effects Tracking
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY()
    TArray<UNiagaraComponent*> EffectPool;

public:
    // Primary Interface Functions
    
    /**
     * Spawn an environmental effect (rain, fog, dust)
     * These effects are Tier 1 priority - always visible
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEnvironmentalEffect(const FString& EffectName, 
                                              const FVector& Location, 
                                              const FRotator& Rotation = FRotator::ZeroRotator,
                                              const FVector& Scale = FVector::OneVector);

    /**
     * Spawn a creature interaction effect (footsteps, breathing, territorial marks)
     * These effects use distance-based LOD
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnCreatureEffect(const FString& EffectName, 
                                         const FVector& Location, 
                                         const FRotator& Rotation = FRotator::ZeroRotator,
                                         AActor* AttachToActor = nullptr);

    /**
     * Spawn a survival mechanics effect (fire, blood, tool impacts)
     * These effects are gameplay critical - Tier 1 priority
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnSurvivalEffect(const FString& EffectName, 
                                         const FVector& Location, 
                                         const FRotator& Rotation = FRotator::ZeroRotator,
                                         float Intensity = 1.0f);

    /**
     * Spawn a temporal/magical effect (gem energy, time distortion)
     * These effects are story moments only - Tier 3 priority
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnTemporalEffect(const FString& EffectName, 
                                         const FVector& Location, 
                                         const FRotator& Rotation = FRotator::ZeroRotator,
                                         float PowerLevel = 1.0f);

    /**
     * Spawn a destruction effect (debris, impacts, structural damage)
     * These effects are combat situations - Tier 2 priority
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnDestructionEffect(const FString& EffectName, 
                                            const FVector& Location, 
                                            const FVector& ImpactDirection,
                                            float Force = 1.0f);

    /**
     * Stop and pool an effect for reuse
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopEffect(UNiagaraComponent* Effect);

    /**
     * Emergency function to clear all effects if performance drops
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void ClearAllEffects();

    /**
     * Get current performance stats
     */
    UFUNCTION(BlueprintCallable, Category = "VFX")
    FString GetPerformanceStats() const;

private:
    // Internal Management
    void UpdateLODLevels();
    void ManageEffectPool();
    UNiagaraComponent* GetPooledEffect();
    void ReturnEffectToPool(UNiagaraComponent* Effect);
    int32 CalculateLODLevel(const FVector& EffectLocation) const;
    void ApplyLODToEffect(UNiagaraComponent* Effect, int32 LODLevel);
    
    // Performance Monitoring
    float LastPerformanceCheck = 0.0f;
    float PerformanceCheckInterval = 1.0f; // Check every second
    
    // Player reference for distance calculations
    UPROPERTY()
    APawn* PlayerPawn;
};