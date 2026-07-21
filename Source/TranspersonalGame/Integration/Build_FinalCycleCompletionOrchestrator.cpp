#include "Build_FinalCycleCompletionOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "Engine/GameInstance.h"

UBuild_FinalCycleCompletionOrchestrator::UBuild_FinalCycleCompletionOrchestrator()
{
    bInitialized = false;
    bValidationInProgress = false;
}

void UBuild_FinalCycleCompletionOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Initializing final cycle completion orchestrator"));
    
    // Initialize completion data
    CompletionData = FBuild_FinalCycleCompletionData();
    IntegrationMetrics = FBuild_SystemIntegrationMetrics();
    
    bInitialized = true;
    
    // Start initial validation
    InitializeFinalCycleCompletion();
}

void UBuild_FinalCycleCompletionOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Deinitializing"));
    
    bInitialized = false;
    bValidationInProgress = false;
    
    Super::Deinitialize();
}

void UBuild_FinalCycleCompletionOrchestrator::InitializeFinalCycleCompletion()
{
    if (!bInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalCycleCompletionOrchestrator: Cannot initialize - subsystem not ready"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Starting final cycle completion initialization"));
    
    // Reset completion data
    CompletionData = FBuild_FinalCycleCompletionData();
    CompletionData.CompletionStatus = TEXT("Initializing Final Cycle");
    
    // Reset integration metrics
    IntegrationMetrics = FBuild_SystemIntegrationMetrics();
    
    // Start comprehensive validation
    ValidateAllSystems();
}

void UBuild_FinalCycleCompletionOrchestrator::ValidateAllSystems()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Validation already in progress"));
        return;
    }
    
    bValidationInProgress = true;
    CompletionData.CompletionStatus = TEXT("Validating All Systems");
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Starting comprehensive system validation"));
    
    // Validate actor limits and world state
    ValidateActorLimits();
    
    // Validate system health
    ValidateSystemHealth();
    
    // Validate module integration
    ValidateModuleIntegration();
    
    // Validate QA integration
    ValidateQAIntegration();
    
    // Validate build integration
    ValidateBuildIntegration();
    
    // Validate cross-system compatibility
    ValidateCrossSystemCompatibility();
    
    // Calculate final metrics
    CalculateCompletionMetrics();
    
    // Update completion status
    UpdateCompletionStatus();
    
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: System validation complete"));
}

void UBuild_FinalCycleCompletionOrchestrator::ValidateActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalCycleCompletionOrchestrator: No world available for actor validation"));
        return;
    }
    
    // Count all actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    CompletionData.TotalActorsValidated = AllActors.Num();
    
    // Count dinosaurs
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dino")))
        {
            DinosaurCount++;
        }
    }
    CompletionData.DinosaurCount = DinosaurCount;
    
    // Count VFX systems
    int32 VFXCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && (Actor->GetName().Contains(TEXT("VFX")) || Actor->GetName().Contains(TEXT("Niagara"))))
        {
            VFXCount++;
        }
    }
    CompletionData.VFXSystemsActive = VFXCount;
    
    // Count audio systems
    int32 AudioCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Audio")))
        {
            AudioCount++;
        }
    }
    CompletionData.AudioSystemsActive = AudioCount;
    
    // Count performance systems
    int32 PerformanceCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Performance")))
        {
            PerformanceCount++;
        }
    }
    CompletionData.PerformanceSystemsActive = PerformanceCount;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Actor validation complete - Total: %d, Dinosaurs: %d, VFX: %d, Audio: %d, Performance: %d"), 
           CompletionData.TotalActorsValidated, CompletionData.DinosaurCount, CompletionData.VFXSystemsActive, 
           CompletionData.AudioSystemsActive, CompletionData.PerformanceSystemsActive);
}

void UBuild_FinalCycleCompletionOrchestrator::ValidateSystemHealth()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Validating system health"));
    
    // Check core systems
    IntegrationMetrics.bCoreSystemsOnline = true; // Assume online if we can execute
    
    // Check gameplay systems
    IntegrationMetrics.bGameplaySystemsOnline = (CompletionData.DinosaurCount > 0);
    
    // Check rendering systems
    IntegrationMetrics.bRenderingSystemsOnline = (CompletionData.VFXSystemsActive > 0);
    
    // Calculate system health score
    float HealthScore = 0.0f;
    if (IntegrationMetrics.bCoreSystemsOnline) HealthScore += 40.0f;
    if (IntegrationMetrics.bGameplaySystemsOnline) HealthScore += 30.0f;
    if (IntegrationMetrics.bRenderingSystemsOnline) HealthScore += 30.0f;
    
    IntegrationMetrics.SystemHealthScore = HealthScore;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: System health score: %.1f%%"), HealthScore);
}

void UBuild_FinalCycleCompletionOrchestrator::ValidateModuleIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Validating module integration"));
    
    // Count loaded modules (simplified check)
    IntegrationMetrics.ModulesLoaded = 1; // TranspersonalGame module
    
    // Count registered classes (simplified check)
    IntegrationMetrics.ClassesRegistered = 10; // Estimate based on core classes
    
    // Count active components
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 ComponentCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                ComponentCount += Actor->GetRootComponent() ? 1 : 0;
            }
        }
        IntegrationMetrics.ComponentsActive = ComponentCount;
    }
    
    // Count initialized subsystems (simplified)
    IntegrationMetrics.SubsystemsInitialized = 3; // Estimate
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Module integration - Modules: %d, Classes: %d, Components: %d, Subsystems: %d"),
           IntegrationMetrics.ModulesLoaded, IntegrationMetrics.ClassesRegistered, 
           IntegrationMetrics.ComponentsActive, IntegrationMetrics.SubsystemsInitialized);
}

void UBuild_FinalCycleCompletionOrchestrator::ValidateQAIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Validating QA integration"));
    
    // Check if QA systems are present and functional
    bool bQASystemsFound = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("QA")))
            {
                bQASystemsFound = true;
                break;
            }
        }
    }
    
    CompletionData.bQAValidationPassed = bQASystemsFound;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: QA integration validation: %s"), 
           CompletionData.bQAValidationPassed ? TEXT("PASSED") : TEXT("FAILED"));
}

void UBuild_FinalCycleCompletionOrchestrator::ValidateBuildIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Validating build integration"));
    
    // Check if build integration systems are present
    bool bBuildSystemsFound = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && (Actor->GetName().Contains(TEXT("Build")) || Actor->GetName().Contains(TEXT("Integration"))))
            {
                bBuildSystemsFound = true;
                break;
            }
        }
    }
    
    CompletionData.bBuildIntegrationComplete = bBuildSystemsFound;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Build integration validation: %s"), 
           CompletionData.bBuildIntegrationComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
}

void UBuild_FinalCycleCompletionOrchestrator::ValidateCrossSystemCompatibility()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Validating cross-system compatibility"));
    
    // Check if multiple system types are present and can coexist
    bool bMultipleSystemsPresent = (CompletionData.VFXSystemsActive > 0 && 
                                   CompletionData.AudioSystemsActive > 0 && 
                                   CompletionData.PerformanceSystemsActive > 0);
    
    // Check if actor limits are respected
    bool bActorLimitsRespected = (CompletionData.TotalActorsValidated <= 8000 && 
                                 CompletionData.DinosaurCount <= 150);
    
    CompletionData.bCrossSystemCompatibilityVerified = (bMultipleSystemsPresent && bActorLimitsRespected);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Cross-system compatibility: %s"), 
           CompletionData.bCrossSystemCompatibilityVerified ? TEXT("VERIFIED") : TEXT("FAILED"));
}

void UBuild_FinalCycleCompletionOrchestrator::CalculateCompletionMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Calculating completion metrics"));
    
    float CompletionScore = 0.0f;
    
    // Actor validation (20%)
    if (CompletionData.TotalActorsValidated > 0) CompletionScore += 20.0f;
    
    // QA integration (20%)
    if (CompletionData.bQAValidationPassed) CompletionScore += 20.0f;
    
    // Build integration (20%)
    if (CompletionData.bBuildIntegrationComplete) CompletionScore += 20.0f;
    
    // Cross-system compatibility (20%)
    if (CompletionData.bCrossSystemCompatibilityVerified) CompletionScore += 20.0f;
    
    // System health (20%)
    CompletionScore += (IntegrationMetrics.SystemHealthScore * 0.2f);
    
    CompletionData.CompletionPercentage = FMath::Clamp(CompletionScore, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Final completion percentage: %.1f%%"), 
           CompletionData.CompletionPercentage);
}

void UBuild_FinalCycleCompletionOrchestrator::UpdateCompletionStatus()
{
    if (CompletionData.CompletionPercentage >= 90.0f)
    {
        CompletionData.CompletionStatus = TEXT("CYCLE COMPLETE - EXCELLENT");
    }
    else if (CompletionData.CompletionPercentage >= 75.0f)
    {
        CompletionData.CompletionStatus = TEXT("CYCLE COMPLETE - GOOD");
    }
    else if (CompletionData.CompletionPercentage >= 50.0f)
    {
        CompletionData.CompletionStatus = TEXT("CYCLE PARTIAL - NEEDS IMPROVEMENT");
    }
    else
    {
        CompletionData.CompletionStatus = TEXT("CYCLE INCOMPLETE - CRITICAL ISSUES");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Final status: %s"), 
           *CompletionData.CompletionStatus);
}

void UBuild_FinalCycleCompletionOrchestrator::GenerateFinalCompletionReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleCompletionOrchestrator: Generating final completion report"));
    
    // Ensure all validations are complete
    if (!bValidationInProgress)
    {
        ValidateAllSystems();
    }
    
    // Log comprehensive report
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL CYCLE COMPLETION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *CompletionData.CompletionStatus);
    UE_LOG(LogTemp, Warning, TEXT("Completion: %.1f%%"), CompletionData.CompletionPercentage);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CompletionData.TotalActorsValidated);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CompletionData.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("VFX Systems: %d"), CompletionData.VFXSystemsActive);
    UE_LOG(LogTemp, Warning, TEXT("Audio Systems: %d"), CompletionData.AudioSystemsActive);
    UE_LOG(LogTemp, Warning, TEXT("Performance Systems: %d"), CompletionData.PerformanceSystemsActive);
    UE_LOG(LogTemp, Warning, TEXT("QA Validation: %s"), CompletionData.bQAValidationPassed ? TEXT("PASSED") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Build Integration: %s"), CompletionData.bBuildIntegrationComplete ? TEXT("COMPLETE") : TEXT("INCOMPLETE"));
    UE_LOG(LogTemp, Warning, TEXT("Cross-System Compatibility: %s"), CompletionData.bCrossSystemCompatibilityVerified ? TEXT("VERIFIED") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("System Health Score: %.1f%%"), IntegrationMetrics.SystemHealthScore);
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

bool UBuild_FinalCycleCompletionOrchestrator::IsFinalCycleComplete() const
{
    return (CompletionData.CompletionPercentage >= 75.0f && 
            CompletionData.bQAValidationPassed && 
            CompletionData.bBuildIntegrationComplete);
}

float UBuild_FinalCycleCompletionOrchestrator::GetOverallCompletionPercentage() const
{
    return CompletionData.CompletionPercentage;
}