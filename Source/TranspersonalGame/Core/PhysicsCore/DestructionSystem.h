// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "NiagaraComponent.h"
#include "DestructionSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDestructionSystem, Log, All);

/**
 * @brief Advanced destruction system for environmental interaction
 * 
 * Handles realistic destruction of environmental objects when dinosaurs
 * interact with them. Features:
 * - Chaos Destruction with Geometry Collections
 * - Physics Fields for controlled destruction
 * - Performance-optimized fragment management
 * - Integration with Niagara for visual effects
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDestructionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Initialize destruction system for this object
     * 
     * @param DestructionMesh Geometry Collection to use for destruction
     * @param DestructionThreshold Force required to trigger destruction
     * @param MaxFragments Maximum number of fragments to create
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void InitializeDestruction(UGeometryCollection* DestructionMesh, float DestructionThreshold = 1000.0f, int32 MaxFragments = 50);

    /**
     * @brief Trigger destruction at specific location with force
     * 
     * @param ImpactLocation World location of impact
     * @param ImpactForce Force magnitude applied
     * @param ImpactRadius Radius of destruction effect
     * @param Instigator Actor causing the destruction
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(FVector ImpactLocation, float ImpactForce, float ImpactRadius = 200.0f, AActor* Instigator = nullptr);

    /**
     * @brief Apply radial destruction force using Physics Fields
     * 
     * @param FieldLocation Center of destruction field
     * @param FieldRadius Radius of effect
     * @param FieldStrength Strength of destruction force
     * @param Duration How long the field remains active
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDestructionField(FVector FieldLocation, float FieldRadius, float FieldStrength, float Duration = 1.0f);

    /**
     * @brief Check if object can be destroyed based on current state
     */
    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool CanBeDestroyed() const;

    /**
     * @brief Get current destruction health (0 = fully destroyed, 1 = intact)
     */
    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetDestructionHealth() const { return CurrentHealth; }

    /**
     * @brief Enable/disable destruction for this object
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionEnabled(bool bEnabled) { bDestructionEnabled = bEnabled; }

    /**
     * @brief Clean up destruction fragments for performance
     */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

protected:
    /** Destruction event delegate */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FDestructionEvent, AActor*, DestroyedActor, FVector, ImpactLocation, float, ImpactForce, AActor*, Instigator);

    /** Called when destruction is triggered */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FDestructionEvent OnDestruction;

    /** Geometry Collection component for destruction */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UGeometryCollectionComponent* GeometryCollectionComponent;

    /** Physics Field component for destruction effects */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFieldSystemComponent* FieldSystemComponent;

    /** Niagara component for destruction VFX */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* DestructionVFXComponent;

    /** Is destruction enabled for this object */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings")
    bool bDestructionEnabled = true;

    /** Force threshold required to trigger destruction */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings", meta = (ClampMin = "0.0", ClampMax = "50000.0"))
    float DestructionThreshold = 1000.0f;

    /** Maximum number of fragments to create */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings", meta = (ClampMin = "1", ClampMax = "500"))
    int32 MaxFragments = 50;

    /** Current health of the object (1.0 = intact, 0.0 = fully destroyed) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction State")
    float CurrentHealth = 1.0f;

    /** Has this object been destroyed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction State")
    bool bIsDestroyed = false;

    /** Destruction VFX to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    class UNiagaraSystem* DestructionEffect;

    /** Sound to play on destruction */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    class USoundBase* DestructionSound;

    /** Time to keep fragments before cleanup (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "5.0", ClampMax = "300.0"))
    float FragmentLifetime = 30.0f;

    /** Minimum fragment size to keep (smaller fragments are removed) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float MinFragmentSize = 0.5f;

private:
    /** Setup Geometry Collection for destruction */
    void SetupGeometryCollection();
    
    /** Create Physics Field for destruction */
    void CreateDestructionField(FVector Location, float Radius, float Strength);
    
    /** Apply damage to object health */
    void ApplyDamage(float Damage);
    
    /** Spawn destruction effects */
    void SpawnDestructionEffects(FVector Location);
    
    /** Schedule fragment cleanup */
    void ScheduleFragmentCleanup();

    /** Timer handle for fragment cleanup */
    FTimerHandle FragmentCleanupTimer;

    /** List of created fragments for tracking */
    UPROPERTY()
    TArray<AActor*> CreatedFragments;

    /** Cached world reference */
    UPROPERTY()
    UWorld* CachedWorld;
};

/**
 * @brief Destruction Manager Subsystem
 * 
 * Manages all destruction in the world for performance optimization
 */
UCLASS()
class TRANSPERSONALGAME_API UDestructionManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Register destruction system for management */
    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    void RegisterDestructionSystem(UDestructionSystem* DestructionSystem);

    /** Unregister destruction system */
    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    void UnregisterDestructionSystem(UDestructionSystem* DestructionSystem);

    /** Get destruction manager instance */
    UFUNCTION(BlueprintPure, Category = "Destruction Manager")
    static UDestructionManagerSubsystem* Get(const UObject* WorldContext);

    /** Clean up all destruction fragments for performance */
    UFUNCTION(BlueprintCallable, Category = "Destruction Manager")
    void CleanupAllFragments();

    /** Get current destruction performance metrics */
    UFUNCTION(BlueprintPure, Category = "Destruction Manager")
    void GetDestructionMetrics(int32& ActiveFragments, int32& ActiveDestructions) const;

protected:
    /** All registered destruction systems */
    UPROPERTY()
    TArray<UDestructionSystem*> RegisteredSystems;

    /** Performance counters */
    int32 TotalActiveFragments = 0;
    int32 TotalActiveDestructions = 0;

private:
    /** Update performance metrics */
    void UpdateMetrics();

    /** Timer for performance updates */
    FTimerHandle MetricsUpdateTimer;
};