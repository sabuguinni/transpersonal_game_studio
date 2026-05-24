#include "Perf_PhysicsIntegrationProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Stats/Stats.h"

UPerf_PhysicsIntegrationProfiler::UPerf_PhysicsIntegrationProfiler()
    : bIsProfilingActive(false)
    , ProfilingStartTime(0.0f)
    , LastFrameTime(0.0f)
    , FrameCounter(0)
{
    // Initialize default optimization settings
    OptimizationSettings.MaxPhysicsBodies = 500;
    OptimizationSettings.PhysicsTickRate = 60.0f;
    OptimizationSettings.bEnableAdaptiveLOD = true;
    OptimizationSettings.LODDistanceThreshold = 2000.0f;
    OptimizationSettings.bEnablePhysicsCulling = true;
}

void UPerf_PhysicsIntegrationProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Integration Profiler initialized"));
    
    // Start automatic profiling
    StartPhysicsProfiler();
}

void UPerf_PhysicsIntegrationProfiler::Deinitialize()
{
    StopPhysicsProfiler();
    Super::Deinitialize();
}

void UPerf_PhysicsIntegrationProfiler::StartPhysicsProfiler()
{
    if (bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics profiler already active"));
        return;
    }

    bIsProfilingActive = true;
    ProfilingStartTime = FPlatformTime::Seconds();
    FrameTimeHistory.Empty();
    FrameCounter = 0;

    // Set up timer for periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProfilerUpdateTimer,
            this,
            &UPerf_PhysicsIntegrationProfiler::UpdatePhysicsMetrics,
            0.1f, // Update every 100ms
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Physics Integration Profiler started"));
}

void UPerf_PhysicsIntegrationProfiler::StopPhysicsProfiler()
{
    if (!bIsProfilingActive)
    {
        return;
    }

    bIsProfilingActive = false;

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProfilerUpdateTimer);
    }

    UE_LOG(LogTemp, Warning, TEXT("Physics Integration Profiler stopped"));
}

FPerf_PhysicsProfileData UPerf_PhysicsIntegrationProfiler::GetCurrentPhysicsProfile() const
{
    return CurrentProfileData;
}

void UPerf_PhysicsIntegrationProfiler::UpdatePhysicsOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    ApplyPhysicsOptimizations();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics optimization settings updated"));
}

int32 UPerf_PhysicsIntegrationProfiler::CountActivePhysicsBodies() const
{
    int32 BodyCount = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && IsValid(Actor))
            {
                if (UPrimitiveComponent* PrimComp = Actor->GetRootPrimitiveComponent())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        BodyCount++;
                    }
                }
            }
        }
    }
    
    return BodyCount;
}

float UPerf_PhysicsIntegrationProfiler::GetPhysicsMemoryUsage() const
{
    // Estimate physics memory usage based on active bodies
    int32 ActiveBodies = CountActivePhysicsBodies();
    
    // Rough estimate: each physics body uses approximately 2KB
    float EstimatedMemoryKB = ActiveBodies * 2.0f;
    return EstimatedMemoryKB / 1024.0f; // Convert to MB
}

void UPerf_PhysicsIntegrationProfiler::OptimizePhysicsPerformance()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot optimize - profiler not active"));
        return;
    }

    AnalyzePhysicsPerformance();
    ApplyPhysicsOptimizations();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics performance optimization applied"));
}

bool UPerf_PhysicsIntegrationProfiler::ValidatePhysicsIntegration() const
{
    // Check if Core_PhysicsGameModeIntegrator is working
    if (UWorld* World = GetWorld())
    {
        AGameModeBase* GameMode = World->GetAuthGameMode();
        if (!GameMode)
        {
            UE_LOG(LogTemp, Error, TEXT("No GameMode found for physics integration validation"));
            return false;
        }
        
        // Count physics actors
        int32 PhysicsActorCount = CountActivePhysicsBodies();
        if (PhysicsActorCount == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No physics actors found - integration may not be working"));
            return false;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Physics integration validation passed - %d physics actors found"), PhysicsActorCount);
        return true;
    }
    
    return false;
}

void UPerf_PhysicsIntegrationProfiler::RunPhysicsStressTest(int32 TestDuration)
{
    if (!bIsProfilingActive)
    {
        StartPhysicsProfiler();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Starting physics stress test for %d seconds"), TestDuration);
    
    // Create multiple physics objects for stress testing
    if (UWorld* World = GetWorld())
    {
        for (int32 i = 0; i < 50; i++)
        {
            FVector SpawnLocation = FVector(
                FMath::RandRange(-1000.0f, 1000.0f),
                FMath::RandRange(-1000.0f, 1000.0f),
                FMath::RandRange(500.0f, 1500.0f)
            );
            
            AStaticMeshActor* TestActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator);
            if (TestActor && TestActor->GetStaticMeshComponent())
            {
                TestActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
                TestActor->SetActorLabel(FString::Printf(TEXT("PhysicsStressTest_%d"), i));
            }
        }
        
        // Schedule cleanup after test duration
        FTimerHandle CleanupTimer;
        World->GetTimerManager().SetTimer(
            CleanupTimer,
            [this]()
            {
                GeneratePhysicsPerformanceReport();
                UE_LOG(LogTemp, Warning, TEXT("Physics stress test completed"));
            },
            TestDuration,
            false
        );
    }
}

void UPerf_PhysicsIntegrationProfiler::GeneratePhysicsPerformanceReport()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot generate report - profiler not active"));
        return;
    }

    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    if (FrameTimeHistory.Num() > 0)
    {
        for (float FrameTime : FrameTimeHistory)
        {
            AverageFrameTime += FrameTime;
        }
        AverageFrameTime /= FrameTimeHistory.Num();
    }

    // Generate report content
    FString ReportContent = FString::Printf(TEXT(
        "=== PHYSICS INTEGRATION PERFORMANCE REPORT ===\n"
        "Generated: %s\n"
        "Profiling Duration: %.2f seconds\n"
        "Total Frames Analyzed: %d\n"
        "Average Frame Time: %.3f ms\n"
        "Active Physics Bodies: %d\n"
        "Physics Memory Usage: %.2f MB\n"
        "Physics Tick Time: %.3f ms\n"
        "Optimization Settings:\n"
        "  - Max Physics Bodies: %d\n"
        "  - Physics Tick Rate: %.1f Hz\n"
        "  - Adaptive LOD: %s\n"
        "  - LOD Distance Threshold: %.1f units\n"
        "  - Physics Culling: %s\n"
        "=== END REPORT ===\n"
    ),
        *FDateTime::Now().ToString(),
        FPlatformTime::Seconds() - ProfilingStartTime,
        FrameCounter,
        AverageFrameTime * 1000.0f,
        CurrentProfileData.ActivePhysicsBodies,
        CurrentProfileData.MemoryUsageMB,
        CurrentProfileData.PhysicsTickTime * 1000.0f,
        OptimizationSettings.MaxPhysicsBodies,
        OptimizationSettings.PhysicsTickRate,
        OptimizationSettings.bEnableAdaptiveLOD ? TEXT("Enabled") : TEXT("Disabled"),
        OptimizationSettings.LODDistanceThreshold,
        OptimizationSettings.bEnablePhysicsCulling ? TEXT("Enabled") : TEXT("Disabled")
    );

    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("PhysicsPerformanceReport.txt");
    if (FFileHelper::SaveStringToFile(ReportContent, *ReportPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics performance report saved to: %s"), *ReportPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save physics performance report"));
    }
}

void UPerf_PhysicsIntegrationProfiler::UpdatePhysicsMetrics()
{
    if (!bIsProfilingActive)
    {
        return;
    }

    // Update frame time tracking
    float CurrentTime = FPlatformTime::Seconds();
    if (LastFrameTime > 0.0f)
    {
        float FrameTime = CurrentTime - LastFrameTime;
        FrameTimeHistory.Add(FrameTime);
        
        // Keep only recent history (last 100 frames)
        if (FrameTimeHistory.Num() > 100)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average frame time
        float TotalFrameTime = 0.0f;
        for (float Time : FrameTimeHistory)
        {
            TotalFrameTime += Time;
        }
        CurrentProfileData.AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    }
    LastFrameTime = CurrentTime;
    FrameCounter++;

    // Update physics metrics
    CurrentProfileData.ActivePhysicsBodies = CountActivePhysicsBodies();
    CurrentProfileData.MemoryUsageMB = GetPhysicsMemoryUsage();
    CurrentProfileData.PhysicsTickTime = CurrentProfileData.AverageFrameTime * 0.3f; // Estimate physics portion
    CurrentProfileData.CollisionChecksPerFrame = CurrentProfileData.ActivePhysicsBodies * 2; // Rough estimate
}

void UPerf_PhysicsIntegrationProfiler::AnalyzePhysicsPerformance()
{
    // Analyze current performance and suggest optimizations
    if (CurrentProfileData.AverageFrameTime > 0.0333f) // > 30 FPS
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below 30 FPS - applying aggressive optimizations"));
        
        // Reduce physics body limit
        OptimizationSettings.MaxPhysicsBodies = FMath::Max(100, OptimizationSettings.MaxPhysicsBodies / 2);
        OptimizationSettings.bEnablePhysicsCulling = true;
        OptimizationSettings.LODDistanceThreshold = FMath::Max(1000.0f, OptimizationSettings.LODDistanceThreshold * 0.7f);
    }
    else if (CurrentProfileData.AverageFrameTime > 0.0167f) // > 60 FPS
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below 60 FPS - applying moderate optimizations"));
        
        OptimizationSettings.bEnableAdaptiveLOD = true;
        OptimizationSettings.bEnablePhysicsCulling = true;
    }
}

void UPerf_PhysicsIntegrationProfiler::ApplyPhysicsOptimizations()
{
    // Apply current optimization settings to the physics system
    if (UWorld* World = GetWorld())
    {
        // Count current physics bodies and disable excess ones if needed
        int32 CurrentBodies = 0;
        TArray<AActor*> PhysicsActors;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && IsValid(Actor))
            {
                if (UPrimitiveComponent* PrimComp = Actor->GetRootPrimitiveComponent())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PhysicsActors.Add(Actor);
                        CurrentBodies++;
                    }
                }
            }
        }
        
        // Disable physics on excess actors if over limit
        if (CurrentBodies > OptimizationSettings.MaxPhysicsBodies)
        {
            int32 ActorsToDisable = CurrentBodies - OptimizationSettings.MaxPhysicsBodies;
            for (int32 i = 0; i < ActorsToDisable && i < PhysicsActors.Num(); i++)
            {
                if (UPrimitiveComponent* PrimComp = PhysicsActors[i]->GetRootPrimitiveComponent())
                {
                    PrimComp->SetSimulatePhysics(false);
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Disabled physics on %d actors to meet performance target"), ActorsToDisable);
        }
    }
}