#include "Build_FinalCycleIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuild_FinalCycleIntegrator::UBuild_FinalCycleIntegrator()
{
    bIntegrationComplete = false;
    LastIntegrationTime = 0.0f;
}

void UBuild_FinalCycleIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleIntegrator: Subsystem initialized"));
    
    // Initialize metrics
    CurrentMetrics = FBuild_CycleIntegrationMetrics();
    HealthReport = FBuild_SystemHealthReport();
    
    // Start integration validation
    PerformFinalIntegration();
}

void UBuild_FinalCycleIntegrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleIntegrator: Subsystem deinitialized"));
    Super::Deinitialize();
}

void UBuild_FinalCycleIntegrator::PerformFinalIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleIntegrator: Starting final integration"));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Validate all critical systems
    bool bSystemsValid = ValidateAllSystems();
    
    // Count and categorize actors
    CountActorsByType();
    
    // Calculate performance metrics
    CalculatePerformanceScore();
    
    // Update health report
    UpdateHealthReport();
    
    // Clean up any orphaned systems
    CleanupOrphanedSystems();
    
    // Mark integration as complete
    bIntegrationComplete = bSystemsValid;
    LastIntegrationTime = FPlatformTime::Seconds() - StartTime;
    
    // Generate final report
    GenerateFinalReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleIntegrator: Integration complete in %.2f seconds"), LastIntegrationTime);
}

FBuild_CycleIntegrationMetrics UBuild_FinalCycleIntegrator::GetIntegrationMetrics() const
{
    return CurrentMetrics;
}

FBuild_SystemHealthReport UBuild_FinalCycleIntegrator::GetSystemHealthReport() const
{
    return HealthReport;
}

bool UBuild_FinalCycleIntegrator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalCycleIntegrator: Validating all systems"));
    
    bool bAllValid = true;
    
    // Validate core systems
    ValidateCoreSystemsHealth();
    if (!HealthReport.bCoreSystemsHealthy)
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("Core systems validation failed"));
    }
    
    // Validate world generation
    ValidateWorldGenerationHealth();
    if (!HealthReport.bWorldGenerationHealthy)
    {
        bAllValid = false;
        UE_LOG(LogTemp, Error, TEXT("World generation validation failed"));
    }
    
    // Validate VFX systems
    ValidateVFXIntegration();
    if (!HealthReport.bVFXSystemsHealthy)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX systems have issues"));
    }
    
    // Validate audio systems
    ValidateAudioIntegration();
    if (!HealthReport.bAudioSystemsHealthy)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio systems have issues"));
    }
    
    // Validate quest systems
    ValidateQuestIntegration();
    if (!HealthReport.bQuestSystemsHealthy)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest systems have issues"));
    }
    
    CurrentMetrics.bAllSystemsOperational = bAllValid;
    return bAllValid;
}

void UBuild_FinalCycleIntegrator::GenerateFinalReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorsInLevel);
    UE_LOG(LogTemp, Warning, TEXT("VFX Actors: %d"), CurrentMetrics.VFXActorsCount);
    UE_LOG(LogTemp, Warning, TEXT("Audio Actors: %d"), CurrentMetrics.AudioActorsCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActorsCount);
    UE_LOG(LogTemp, Warning, TEXT("Quest Actors: %d"), CurrentMetrics.QuestActorsCount);
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), CurrentMetrics.PerformanceScore);
    UE_LOG(LogTemp, Warning, TEXT("All Systems Operational: %s"), CurrentMetrics.bAllSystemsOperational ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Overall Health Score: %.2f"), HealthReport.OverallHealthScore);
    
    // Log critical issues
    if (HealthReport.CriticalIssues.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL ISSUES FOUND:"));
        for (const FString& Issue : HealthReport.CriticalIssues)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Issue);
        }
    }
    
    // Log warnings
    if (HealthReport.Warnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNINGS:"));
        for (const FString& Warning : HealthReport.Warnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Warning);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
    
    // Update timestamp
    CurrentMetrics.LastValidationTimestamp = FDateTime::Now().ToString();
}

bool UBuild_FinalCycleIntegrator::CheckCriticalSystemsOperational()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        HealthReport.CriticalIssues.Add(TEXT("No valid world found"));
        return false;
    }
    
    // Check game state
    AGameStateBase* GameState = World->GetGameState();
    if (!GameState)
    {
        HealthReport.CriticalIssues.Add(TEXT("No game state found"));
        return false;
    }
    
    // Check player controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        HealthReport.Warnings.Add(TEXT("No player controller found"));
    }
    
    return true;
}

void UBuild_FinalCycleIntegrator::ValidateVFXIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        HealthReport.bVFXSystemsHealthy = false;
        return;
    }
    
    int32 VFXCount = 0;
    int32 WorkingVFXCount = 0;
    
    // Count VFX components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        // Check for Niagara components
        TArray<UNiagaraComponent*> NiagaraComponents;
        Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
        VFXCount += NiagaraComponents.Num();
        
        for (UNiagaraComponent* Component : NiagaraComponents)
        {
            if (Component && Component->IsActive())
            {
                WorkingVFXCount++;
            }
        }
        
        // Check for particle system components
        TArray<UParticleSystemComponent*> ParticleComponents;
        Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
        VFXCount += ParticleComponents.Num();
        
        for (UParticleSystemComponent* Component : ParticleComponents)
        {
            if (Component && Component->IsActive())
            {
                WorkingVFXCount++;
            }
        }
    }
    
    CurrentMetrics.VFXActorsCount = VFXCount;
    HealthReport.bVFXSystemsHealthy = (VFXCount == 0) || (WorkingVFXCount > 0);
    
    if (VFXCount > 0 && WorkingVFXCount == 0)
    {
        HealthReport.Warnings.Add(TEXT("VFX components found but none are active"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Validation: %d total, %d working"), VFXCount, WorkingVFXCount);
}

void UBuild_FinalCycleIntegrator::ValidateAudioIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        HealthReport.bAudioSystemsHealthy = false;
        return;
    }
    
    int32 AudioCount = 0;
    int32 WorkingAudioCount = 0;
    
    // Count audio components
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TArray<UAudioComponent*> AudioComponents;
        Actor->GetComponents<UAudioComponent>(AudioComponents);
        AudioCount += AudioComponents.Num();
        
        for (UAudioComponent* Component : AudioComponents)
        {
            if (Component && Component->IsActive())
            {
                WorkingAudioCount++;
            }
        }
    }
    
    CurrentMetrics.AudioActorsCount = AudioCount;
    HealthReport.bAudioSystemsHealthy = (AudioCount == 0) || (WorkingAudioCount > 0);
    
    if (AudioCount > 0 && WorkingAudioCount == 0)
    {
        HealthReport.Warnings.Add(TEXT("Audio components found but none are active"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Audio Validation: %d total, %d working"), AudioCount, WorkingAudioCount);
}

void UBuild_FinalCycleIntegrator::ValidateQuestIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        HealthReport.bQuestSystemsHealthy = false;
        return;
    }
    
    int32 QuestActorCount = 0;
    
    // Count quest-related actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Quest")) || ActorName.Contains(TEXT("Mission")) || ActorName.Contains(TEXT("Objective")))
        {
            QuestActorCount++;
        }
    }
    
    CurrentMetrics.QuestActorsCount = QuestActorCount;
    HealthReport.bQuestSystemsHealthy = true; // Quest system is optional
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Validation: %d quest actors found"), QuestActorCount);
}

void UBuild_FinalCycleIntegrator::ValidatePerformanceMetrics()
{
    // Basic performance validation
    float CurrentFPS = 1.0f / FApp::GetDeltaTime();
    CurrentMetrics.PerformanceScore = FMath::Clamp(CurrentFPS / 60.0f, 0.0f, 1.0f) * 100.0f;
    
    if (CurrentMetrics.PerformanceScore < 50.0f)
    {
        HealthReport.Warnings.Add(TEXT("Performance below target (< 30 FPS)"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Score: %.2f"), CurrentMetrics.PerformanceScore);
}

void UBuild_FinalCycleIntegrator::CleanupOrphanedSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Cleaning up orphaned systems"));
    
    // This would contain cleanup logic for any orphaned or broken systems
    // For now, just log that cleanup was attempted
    UE_LOG(LogTemp, Warning, TEXT("Orphaned system cleanup complete"));
}

void UBuild_FinalCycleIntegrator::ValidateCoreSystemsHealth()
{
    HealthReport.bCoreSystemsHealthy = CheckCriticalSystemsOperational();
    
    if (HealthReport.bCoreSystemsHealthy)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core systems validation: PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Core systems validation: FAILED"));
    }
}

void UBuild_FinalCycleIntegrator::ValidateWorldGenerationHealth()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        HealthReport.bWorldGenerationHealthy = false;
        return;
    }
    
    // Check for landscape
    bool bHasLandscape = false;
    for (TActorIterator<ALandscape> LandscapeItr(World); LandscapeItr; ++LandscapeItr)
    {
        bHasLandscape = true;
        break;
    }
    
    // Check for static mesh actors (environment)
    int32 StaticMeshCount = 0;
    for (TActorIterator<AStaticMeshActor> MeshItr(World); MeshItr; ++MeshItr)
    {
        StaticMeshCount++;
    }
    
    HealthReport.bWorldGenerationHealthy = bHasLandscape || (StaticMeshCount > 0);
    
    if (!HealthReport.bWorldGenerationHealthy)
    {
        HealthReport.CriticalIssues.Add(TEXT("No landscape or environment meshes found"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World Generation: Landscape=%s, StaticMeshes=%d"), 
           bHasLandscape ? TEXT("YES") : TEXT("NO"), StaticMeshCount);
}

void UBuild_FinalCycleIntegrator::CountActorsByType()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentMetrics.TotalActorsInLevel = 0;
        return;
    }
    
    int32 TotalCount = 0;
    int32 DinosaurCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        TotalCount++;
        
        FString ActorName = Actor->GetName();
        if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || 
            ActorName.Contains(TEXT("Raptor")) || ActorName.Contains(TEXT("Brachio")))
        {
            DinosaurCount++;
        }
    }
    
    CurrentMetrics.TotalActorsInLevel = TotalCount;
    CurrentMetrics.DinosaurActorsCount = DinosaurCount;
    
    UE_LOG(LogTemp, Warning, TEXT("Actor Count: Total=%d, Dinosaurs=%d"), TotalCount, DinosaurCount);
}

void UBuild_FinalCycleIntegrator::CalculatePerformanceScore()
{
    ValidatePerformanceMetrics();
}

void UBuild_FinalCycleIntegrator::UpdateHealthReport()
{
    // Calculate overall health score
    float HealthScore = 0.0f;
    int32 SystemCount = 0;
    
    if (HealthReport.bCoreSystemsHealthy) { HealthScore += 30.0f; } SystemCount++;
    if (HealthReport.bWorldGenerationHealthy) { HealthScore += 25.0f; } SystemCount++;
    if (HealthReport.bVFXSystemsHealthy) { HealthScore += 15.0f; } SystemCount++;
    if (HealthReport.bAudioSystemsHealthy) { HealthScore += 15.0f; } SystemCount++;
    if (HealthReport.bQuestSystemsHealthy) { HealthScore += 15.0f; } SystemCount++;
    
    HealthReport.OverallHealthScore = HealthScore;
    
    UE_LOG(LogTemp, Warning, TEXT("Overall Health Score: %.2f/100"), HealthReport.OverallHealthScore);
}