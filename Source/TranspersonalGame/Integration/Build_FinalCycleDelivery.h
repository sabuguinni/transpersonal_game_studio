#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Build_FinalCycleDelivery.generated.h"

/**
 * Final Cycle Delivery System - Integration Agent #19
 * Orchestrates final delivery of all agent outputs into cohesive build
 * Validates all systems, performs final integration tests, and prepares release
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_FinalCycleDelivery : public AActor
{
    GENERATED_BODY()

public:
    ABuild_FinalCycleDelivery();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === FINAL DELIVERY ORCHESTRATION ===
    
    /** Execute complete final delivery process */
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    void ExecuteFinalDelivery();
    
    /** Validate all agent outputs and integration */
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateAllAgentOutputs();
    
    /** Perform final system integration */
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool PerformFinalIntegration();
    
    /** Generate final delivery report */
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    void GenerateFinalDeliveryReport();

    // === SYSTEM VALIDATION ===
    
    /** Validate core game systems */
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateCoreSystems();
    
    /** Validate VFX and particle systems */
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateVFXSystems();
    
    /** Validate audio and sound systems */
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateAudioSystems();
    
    /** Validate AI and behavior systems */
    UFUNCTION(BlueprintCallable, Category = "System Validation")
    bool ValidateAISystems();

    // === PERFORMANCE VALIDATION ===
    
    /** Check performance metrics and optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidatePerformanceMetrics();
    
    /** Validate memory usage and optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateMemoryUsage();
    
    /** Check frame rate and rendering performance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ValidateRenderingPerformance();

    // === BUILD PREPARATION ===
    
    /** Prepare final build for deployment */
    UFUNCTION(BlueprintCallable, Category = "Build Preparation")
    void PrepareFinalBuild();
    
    /** Package all assets and content */
    UFUNCTION(BlueprintCallable, Category = "Build Preparation")
    bool PackageAllAssets();
    
    /** Validate build integrity */
    UFUNCTION(BlueprintCallable, Category = "Build Preparation")
    bool ValidateBuildIntegrity();

protected:
    // === DELIVERY STATUS ===
    
    /** Current delivery phase */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Status")
    EBuild_DeliveryPhase CurrentDeliveryPhase;
    
    /** Overall delivery progress (0.0 to 1.0) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Status")
    float DeliveryProgress;
    
    /** Number of systems validated */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Status")
    int32 SystemsValidated;
    
    /** Number of critical issues found */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Status")
    int32 CriticalIssuesFound;

    // === VALIDATION RESULTS ===
    
    /** Core systems validation result */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    bool bCoreSystemsValid;
    
    /** VFX systems validation result */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    bool bVFXSystemsValid;
    
    /** Audio systems validation result */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    bool bAudioSystemsValid;
    
    /** AI systems validation result */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    bool bAISystemsValid;
    
    /** Performance validation result */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Validation Results")
    bool bPerformanceValid;

    // === DELIVERY METRICS ===
    
    /** Total number of actors in level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Metrics")
    int32 TotalActorCount;
    
    /** Number of functional systems */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Metrics")
    int32 FunctionalSystemCount;
    
    /** Build size in MB */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Metrics")
    float BuildSizeMB;
    
    /** Estimated performance score */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery Metrics")
    float PerformanceScore;

private:
    // === INTERNAL VALIDATION ===
    
    /** Validate individual system components */
    bool ValidateSystemComponents();
    
    /** Check cross-system dependencies */
    bool ValidateCrossSystemDependencies();
    
    /** Verify asset integrity */
    bool VerifyAssetIntegrity();
    
    /** Test gameplay functionality */
    bool TestGameplayFunctionality();
    
    /** Generate validation report */
    void GenerateValidationReport();
    
    /** Log delivery progress */
    void LogDeliveryProgress(const FString& Message);
    
    /** Update delivery metrics */
    void UpdateDeliveryMetrics();
};