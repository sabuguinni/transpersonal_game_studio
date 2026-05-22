#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Physics/PhysicsFiltering.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "EngineArchitectCore.generated.h"

// Forward declarations
class UPhysicsSettings;
class URendererSettings;

/**
 * Core engine architecture rules and validation system
 * Defines technical standards that ALL agents must follow
 */
UCLASS(BlueprintType, Blueprintable, Category = "Engine Architecture")
class TRANSPERSONALGAME_API UEngineArchitectCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UEngineArchitectCore();

    // === CORE ARCHITECTURE VALIDATION ===
    
    /** Validate all core engine systems are properly initialized */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    bool ValidateCoreEngineSystems();
    
    /** Enforce physics settings for dinosaur survival gameplay */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void EnforcePhysicsArchitecture();
    
    /** Validate performance requirements (60fps PC / 30fps console) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    bool ValidatePerformanceTargets();
    
    /** Check world partition setup for large worlds (>4km²) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    bool ValidateWorldPartitionSetup();

    // === AGENT COMPLIANCE VALIDATION ===
    
    /** Validate that all spawned actors follow architecture rules */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    int32 ValidateSpawnedActorCompliance();
    
    /** Check for orphaned headers without .cpp implementations */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    TArray<FString> FindOrphanedHeaders();
    
    /** Validate module dependencies are correctly defined */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    bool ValidateModuleDependencies();

    // === DINOSAUR PHYSICS ARCHITECTURE ===
    
    /** Setup collision channels for dinosaur interactions */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void SetupDinosaurCollisionChannels();
    
    /** Validate dinosaur physics bodies are properly configured */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    bool ValidateDinosaurPhysics();
    
    /** Setup ragdoll physics for dinosaur death states */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Architecture")
    void SetupDinosaurRagdollPhysics();

protected:
    // === ARCHITECTURE RULES (ENFORCED) ===
    
    /** Maximum actor count per biome before performance degradation */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Limits")
    int32 MaxActorsPerBiome = 500;
    
    /** Required FPS for PC builds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFPS_PC = 60.0f;
    
    /** Required FPS for console builds */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFPS_Console = 30.0f;
    
    /** World size threshold requiring World Partition */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Architecture")
    float WorldPartitionThreshold = 4000000.0f; // 4km²
    
    /** Collision channels for dinosaur gameplay */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Architecture")
    TArray<FString> RequiredCollisionChannels;

private:
    // === INTERNAL VALIDATION ===
    
    /** Check if physics world is properly initialized */
    bool IsPhysicsWorldValid() const;
    
    /** Validate rendering pipeline settings */
    bool ValidateRenderingPipeline() const;
    
    /** Check memory usage and performance metrics */
    bool CheckPerformanceMetrics() const;
    
    /** Validate all required UE5 subsystems are active */
    bool ValidateSubsystems() const;
    
    // === ARCHITECTURE ENFORCEMENT ===
    
    /** Last validation timestamp */
    UPROPERTY()
    float LastValidationTime;
    
    /** Validation results cache */
    UPROPERTY()
    TMap<FString, bool> ValidationResultsCache;
    
    /** Performance metrics history */
    UPROPERTY()
    TArray<float> PerformanceHistory;
};