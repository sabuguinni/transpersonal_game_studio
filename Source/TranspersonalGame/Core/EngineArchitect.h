#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EngineArchitect.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - ENGINE ARCHITECT
 * Agent #02 - Core Architecture Manager
 * 
 * This subsystem enforces architectural rules and manages system coordination.
 * ALL other agents must respect the constraints defined here.
 * 
 * RESPONSIBILITIES:
 * - Enforce performance constraints (60fps PC, 30fps console)
 * - Manage system priorities and resource allocation
 * - Coordinate between Core Systems, World Generation, and Performance
 * - Validate architectural compliance across all modules
 * - Provide centralized configuration for the entire game
 * 
 * ARCHITECTURAL PRINCIPLES:
 * 1. Performance is NON-NEGOTIABLE - framerate targets are HARD limits
 * 2. Scalability FIRST - support from 1km² to 64km² worlds
 * 3. Modularity - each system is independent but coordinated
 * 4. Future-proofing - architecture supports years of feature additions
 * 5. Debugging - comprehensive profiling and monitoring built-in
 */

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngineArchitect : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEngineArchitect();

    // ═══════════════════════════════════════════════════════════════
    // SUBSYSTEM LIFECYCLE
    // ═══════════════════════════════════════════════════════════════

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ═══════════════════════════════════════════════════════════════
    // MASTER ARCHITECTURE CONFIGURATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture", 
        meta = (AllowPrivateAccess = "true"))
    FEng_MasterArchitecture MasterConfig;

    // Get the singleton instance
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    static UEngineArchitect* Get(const UObject* WorldContext);

    // ═══════════════════════════════════════════════════════════════
    // PERFORMANCE MONITORING & ENFORCEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforcePerformanceLimits();

    // Emergency performance reduction
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerPerformanceEmergency();

    // ═══════════════════════════════════════════════════════════════
    // SYSTEM REGISTRATION & COORDINATION
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Systems")
    bool RegisterSystem(const FEng_SystemConfiguration& SystemConfig);

    UFUNCTION(BlueprintCallable, Category = "Systems")
    bool UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Systems")
    FEng_SystemConfiguration GetSystemConfig(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Systems")
    TArray<FString> GetAllRegisteredSystems() const;

    // ═══════════════════════════════════════════════════════════════
    // WORLD SCALE MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "World")
    void ConfigureWorldScale(EEng_WorldScale NewScale);

    UFUNCTION(BlueprintCallable, Category = "World")
    bool ShouldUseWorldPartition() const;

    UFUNCTION(BlueprintCallable, Category = "World")
    bool ShouldUseLevelStreaming() const;

    UFUNCTION(BlueprintCallable, Category = "World")
    float GetRecommendedStreamingDistance() const;

    // ═══════════════════════════════════════════════════════════════
    // PHYSICS ARCHITECTURE
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ConfigurePhysicsComplexity(EEng_SimulationComplexity Complexity);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool ShouldUseRagdoll() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool ShouldUseDestruction() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    int32 GetMaxPhysicsObjects() const;

    // ═══════════════════════════════════════════════════════════════
    // RENDERING ARCHITECTURE
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Rendering")
    bool ShouldUseLumen() const;

    UFUNCTION(BlueprintCallable, Category = "Rendering")
    bool ShouldUseNanite() const;

    UFUNCTION(BlueprintCallable, Category = "Rendering")
    int32 GetMaxDrawCalls() const;

    UFUNCTION(BlueprintCallable, Category = "Rendering")
    float GetMaxViewDistance() const;

    // ═══════════════════════════════════════════════════════════════
    // ARCHITECTURAL VALIDATION
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystemArchitecture() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetArchitecturalWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetArchitecturalErrors() const;

    // ═══════════════════════════════════════════════════════════════
    // DEBUG & PROFILING
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnableDebugMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void EnableProfiling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DumpArchitectureReport() const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidateArchitectureInEditor();

private:
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL STATE
    // ═══════════════════════════════════════════════════════════════

    // Registered systems
    UPROPERTY()
    TMap<FString, FEng_SystemConfiguration> RegisteredSystems;

    // Performance monitoring
    mutable float LastFPSCheck;
    mutable float CachedFPS;
    mutable int32 CachedMemoryUsage;

    // Validation cache
    mutable bool bValidationCacheValid;
    mutable TArray<FString> CachedWarnings;
    mutable TArray<FString> CachedErrors;

    // ═══════════════════════════════════════════════════════════════
    // INTERNAL METHODS
    // ═══════════════════════════════════════════════════════════════

    void InitializeDefaultConfiguration();
    void ValidateConfiguration();
    void UpdatePerformanceCache() const;
    void InvalidateValidationCache();
    void RunArchitecturalValidation() const;

    // Platform-specific optimizations
    void ApplyPlatformOptimizations();
    void ConfigureForPC();
    void ConfigureForConsole();
    void ConfigureForMobile();
};