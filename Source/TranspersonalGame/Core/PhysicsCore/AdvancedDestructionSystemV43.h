#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "AdvancedDestructionSystemV43.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDestructionV43, Log, All);

// Destruction event types for the Jurassic survival game
UENUM(BlueprintType)
enum class EDestructionEventType : uint8
{
    TreeFall = 0,           // Trees falling from dinosaur impact
    RockCrush = 1,          // Rocks being crushed
    StructureCollapse = 2,  // Player-built structures collapsing
    GroundCrack = 3,        // Ground cracking from heavy impacts
    VegetationTrampling = 4, // Vegetation being trampled
    WaterSplash = 5         // Water displacement from large creatures
};

// Destruction intensity levels
UENUM(BlueprintType)
enum class EDestructionIntensity : uint8
{
    Light = 0,      // Small debris, minor damage
    Medium = 1,     // Moderate destruction
    Heavy = 2,      // Major destruction
    Catastrophic = 3 // Complete destruction
};

// Delegate for destruction events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDestructionEvent, EDestructionEventType, EventType, 
    AActor*, DestroyedActor, FVector, Location, EDestructionIntensity, Intensity);

// Forward declarations
class UGeometryCollectionComponent;
class AFieldSystemActor;
class UNiagaraSystem;

/**
 * Advanced Destruction System V43
 * Handles realistic environmental destruction for the Jurassic survival game
 * Features: Tree falling, rock crushing, structure collapse, ground deformation
 * Optimized for large-scale destruction with performance LOD
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAdvancedDestructionSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UAdvancedDestructionSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Initialize destruction system for this actor */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void InitializeDestructionSystem();

    /** Setup geometry collection for destruction */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void SetupGeometryCollection(UStaticMeshComponent* MeshComponent);

    /** Apply destruction force at specific location */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void ApplyDestructionForce(FVector Location, float Radius, float Strength, 
        EDestructionEventType EventType = EDestructionEventType::RockCrush);

    /** Apply directional destruction (e.g., tree falling) */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void ApplyDirectionalDestruction(FVector Direction, float Force, 
        EDestructionEventType EventType = EDestructionEventType::TreeFall);

    /** Trigger catastrophic destruction */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void TriggerCatastrophicDestruction(EDestructionIntensity Intensity);

    /** Set destruction threshold for different damage types */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void SetDestructionThreshold(EDestructionEventType EventType, float Threshold);

    /** Enable/disable destruction for performance */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void SetDestructionEnabled(bool bEnabled);

    /** Get destruction health (0.0 = destroyed, 1.0 = intact) */
    UFUNCTION(BlueprintPure, Category = \"Destruction System\")
    float GetDestructionHealth() const { return DestructionHealth; }

    /** Check if actor can be destroyed */
    UFUNCTION(BlueprintPure, Category = \"Destruction System\")
    bool CanBeDestroyed() const;

    /** Get accumulated damage */
    UFUNCTION(BlueprintPure, Category = \"Destruction System\")
    float GetAccumulatedDamage() const { return AccumulatedDamage; }

    /** Reset destruction state */
    UFUNCTION(BlueprintCallable, Category = \"Destruction System\")
    void ResetDestructionState();

    /** Destruction event dispatcher */
    UPROPERTY(BlueprintAssignable, Category = \"Destruction Events\")
    FOnDestructionEvent OnDestructionEvent;

protected:
    /** Geometry collection component for destruction */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Destruction\")
    TObjectPtr<UGeometryCollectionComponent> GeometryCollectionComponent;

    /** Field system component for physics fields */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Destruction\")
    TObjectPtr<UFieldSystemComponent> FieldSystemComponent;

    /** Current destruction health (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\", meta = (ClampMin = \"0.0\", ClampMax = \"1.0\"))
    float DestructionHealth = 1.0f;

    /** Maximum health before destruction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\")
    float MaxDestructionHealth = 100.0f;

    /** Accumulated damage over time */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Destruction State\")
    float AccumulatedDamage = 0.0f;

    /** Whether destruction is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\")
    bool bDestructionEnabled = true;

    /** Whether this object has been destroyed */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Destruction State\")
    bool bIsDestroyed = false;

    /** Destruction thresholds for different event types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\")
    TMap<EDestructionEventType, float> DestructionThresholds;

    /** Destruction intensity multipliers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\")
    TMap<EDestructionIntensity, float> IntensityMultipliers;

    /** Visual effects for destruction events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction VFX\")
    TMap<EDestructionEventType, TObjectPtr<UNiagaraSystem>> DestructionEffects;

    /** Audio effects for destruction events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Audio\")
    TMap<EDestructionEventType, TObjectPtr<USoundBase>> DestructionSounds;

    /** Debris spawn settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\")
    bool bSpawnDebris = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\")
    int32 MaxDebrisCount = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Destruction Settings\")
    float DebrisLifetime = 30.0f;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    float MaxDestructionDistance = 5000.0f; // 50m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    bool bUseDistanceLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Performance\")
    int32 MaxSimultaneousDestructions = 5;

private:
    /** Initialize default destruction thresholds */
    void InitializeDestructionThresholds();

    /** Initialize intensity multipliers */
    void InitializeIntensityMultipliers();

    /** Process destruction damage */
    void ProcessDestructionDamage(float Damage, EDestructionEventType EventType, FVector Location);

    /** Execute destruction sequence */
    void ExecuteDestruction(EDestructionEventType EventType, EDestructionIntensity Intensity, FVector Location);

    /** Spawn destruction effects */
    void SpawnDestructionEffects(EDestructionEventType EventType, FVector Location, EDestructionIntensity Intensity);

    /** Spawn debris pieces */
    void SpawnDebris(FVector Location, EDestructionIntensity Intensity);

    /** Check distance LOD for destruction */
    bool ShouldProcessDestruction() const;

    /** Get player distance for LOD calculations */
    float GetPlayerDistance() const;

    /** Cleanup destroyed pieces */
    void CleanupDestroyedPieces();

    // Internal state
    TArray<TObjectPtr<AActor>> SpawnedDebris;
    FTimerHandle CleanupTimer;
    int32 CurrentDestructionCount = 0;
    
    // Performance tracking
    float LastDestructionTime = 0.0f;
    int32 DestructionEventCount = 0;
};