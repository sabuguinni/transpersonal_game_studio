#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Landscape/Landscape.h"
#include "Core_PhysicsOptimizer.generated.h"

/**
 * Physics optimization system for terrain interaction and performance
 * Manages collision settings, physics simulation, and performance metrics
 * Optimizes character-terrain interaction and ragdoll physics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsOptimizer : public UObject
{
    GENERATED_BODY()

public:
    UCore_PhysicsOptimizer();

    /** Optimize character physics for terrain interaction */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeCharacterPhysics(ACharacter* Character);

    /** Optimize landscape collision settings for performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeLandscapePhysics(ALandscape* Landscape);

    /** Prepare skeletal mesh for ragdoll physics */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void PrepareRagdollPhysics(USkeletalMeshComponent* SkeletalMesh);

    /** Optimize static mesh physics for destruction system */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    void OptimizeDestructiblePhysics(UStaticMeshComponent* StaticMesh);

    /** Get current physics performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization")
    FString GetPhysicsPerformanceReport();

    /** Batch optimize all physics objects in world */
    UFUNCTION(BlueprintCallable, Category = "Physics Optimization", CallInEditor = true)
    void BatchOptimizeWorldPhysics();

protected:
    /** Current optimization level (0-3) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 OptimizationLevel;

    /** Enable advanced collision detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableAdvancedCollision;

    /** Maximum physics objects to simulate simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjects;

    /** Physics simulation time step */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsTimeStep;

private:
    /** Internal optimization state tracking */
    UPROPERTY()
    TMap<FString, float> PerformanceMetrics;

    /** Helper function to validate collision settings */
    bool ValidateCollisionSettings(UPrimitiveComponent* Component);

    /** Helper function to calculate physics load */
    float CalculatePhysicsLoad(UWorld* World);
};