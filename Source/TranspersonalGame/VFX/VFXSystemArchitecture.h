#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "VFXSystemArchitecture.generated.h"

/**
 * VFX System Architecture for Transpersonal Game Studio
 * Jurassic Survival Game - Production Cycle PROD_JURASSIC_001
 * 
 * Core VFX philosophy:
 * - Tension through subtlety
 * - Organic naturalism
 * - Combat legibility first
 * - Jurassic scale communication
 */

UENUM(BlueprintType)
enum class EVFXCategory : uint8
{
    // Environmental Effects
    Weather_Rain        UMETA(DisplayName = "Weather - Rain"),
    Weather_Fog         UMETA(DisplayName = "Weather - Fog"),
    Weather_Wind        UMETA(DisplayName = "Weather - Wind"),
    
    // Creature Effects
    Creature_Breath     UMETA(DisplayName = "Creature - Breath"),
    Creature_Footstep   UMETA(DisplayName = "Creature - Footstep"),
    Creature_Impact     UMETA(DisplayName = "Creature - Impact"),
    Creature_Death      UMETA(DisplayName = "Creature - Death"),
    
    // Combat Effects
    Combat_Hit          UMETA(DisplayName = "Combat - Hit"),
    Combat_Block        UMETA(DisplayName = "Combat - Block"),
    Combat_Dodge        UMETA(DisplayName = "Combat - Dodge"),
    Combat_Critical     UMETA(DisplayName = "Combat - Critical"),
    
    // Environmental Interaction
    Env_Destruction     UMETA(DisplayName = "Environment - Destruction"),
    Env_Fire            UMETA(DisplayName = "Environment - Fire"),
    Env_Water           UMETA(DisplayName = "Environment - Water"),
    Env_Dust            UMETA(DisplayName = "Environment - Dust"),
    
    // Player Feedback
    Player_Damage       UMETA(DisplayName = "Player - Damage"),
    Player_Heal         UMETA(DisplayName = "Player - Heal"),
    Player_Stealth      UMETA(DisplayName = "Player - Stealth"),
    Player_Discovery    UMETA(DisplayName = "Player - Discovery")
};

UENUM(BlueprintType)
enum class EVFXPriority : uint8
{
    Critical    = 0,    // Combat-critical effects (damage, death)
    High        = 1,    // Important gameplay feedback
    Medium      = 2,    // Environmental enhancement
    Low         = 3,    // Atmospheric details
    Background  = 4     // Subtle ambient effects
};

UENUM(BlueprintType)
enum class EVFXLODLevel : uint8
{
    LOD0_Ultra  = 0,    // Full quality - close range
    LOD1_High   = 1,    // High quality - medium range
    LOD2_Medium = 2,    // Medium quality - far range
    LOD3_Low    = 3     // Low quality - very far range
};

USTRUCT(BlueprintType)
struct FVFXPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxParticlesPerSystem = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistanceThreshold_High = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistanceThreshold_Medium = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LODDistanceThreshold_Low = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableGPUSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableAsyncCompute = true;
};

/**
 * Core VFX System Manager
 * Manages all visual effects in the game with performance optimization
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFXSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFXSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXAtLocation(
        class UNiagaraSystem* VFXSystem,
        FVector Location,
        FRotator Rotation = FRotator::ZeroRotator,
        FVector Scale = FVector::OneVector,
        EVFXPriority Priority = EVFXPriority::Medium
    );

    UFUNCTION(BlueprintCallable, Category = "VFX")
    class UNiagaraComponent* SpawnVFXAttached(
        class UNiagaraSystem* VFXSystem,
        class USceneComponent* AttachToComponent,
        FName AttachPointName = NAME_None,
        FVector Location = FVector::ZeroVector,
        FRotator Rotation = FRotator::ZeroRotator,
        FVector Scale = FVector::OneVector,
        EVFXPriority Priority = EVFXPriority::Medium
    );

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetVFXQualityLevel(EVFXLODLevel QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void OptimizeVFXForDistance(float ViewerDistance);

    // Creature-specific VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void PlayCreatureBreathEffect(class AActor* Creature, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void PlayCreatureFootstepEffect(class AActor* Creature, FVector ImpactLocation, float CreatureWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Creature")
    void PlayCreatureImpactEffect(class AActor* Creature, FVector ImpactLocation, FVector ImpactNormal, float ImpactForce = 1.0f);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void PlayHitEffect(FVector HitLocation, FVector HitNormal, class AActor* HitActor, float Damage = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
    void PlayCriticalHitEffect(FVector HitLocation, FVector HitNormal, class AActor* HitActor);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    void PlayDestructionEffect(FVector Location, class UStaticMesh* DestroyedMesh, float DestructionScale = 1.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFXPerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFXLODLevel CurrentLODLevel = EVFXLODLevel::LOD1_High;

    // VFX System References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems|Creatures")
    class UNiagaraSystem* CreatureBreathSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems|Creatures")
    class UNiagaraSystem* CreatureFootstepSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems|Combat")
    class UNiagaraSystem* HitEffectSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems|Combat")
    class UNiagaraSystem* CriticalHitSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems|Environment")
    class UNiagaraSystem* DestructionSystem;

private:
    // Active VFX tracking for performance
    UPROPERTY()
    TArray<class UNiagaraComponent*> ActiveVFXComponents;

    // Performance monitoring
    float LastPerformanceCheck = 0.0f;
    const float PerformanceCheckInterval = 1.0f; // Check every second

    void PerformanceOptimizationTick();
    void CullDistantVFX();
    void AdjustVFXQualityBasedOnPerformance();
};