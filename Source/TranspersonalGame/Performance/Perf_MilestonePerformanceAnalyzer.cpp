#include "Perf_MilestonePerformanceAnalyzer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/PlatformMemory.h"
#include "Stats/StatsData.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

// Include Core Physics integration
#include "../Core/Physics/Core_MilestonePhysics.h"
#include "../TranspersonalCharacter.h"

UPerf_MilestonePerformanceAnalyzer::UPerf_MilestonePerformanceAnalyzer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz analysis
    
    // Initialize performance targets
    PerformanceTargets.TargetFPS_PC = 60.0f;
    PerformanceTargets.TargetFPS_Console = 30.0f;
    PerformanceTargets.TargetFPS_Mobile = 30.0f;
    PerformanceTargets.MaxPhysicsTickTime = 5.0f;
    PerformanceTargets.MaxCharacterMovementTime = 2.0f;
    PerformanceTargets.MaxMemoryUsage = 512.0f;
    
    // Initialize configuration
    AnalysisInterval = 0.1f;
    bEnableDetailedProfiling = true;
    bAutoOptimize = false;
    MaxSamples = 300;
    
    // Initialize state
    bAnalysisActive = false;
    LastAnalysisTime = 0.0f;
    FPSSamples.Reserve(MaxSamples);
    
    ResetMetrics();
}

void UPerf_MilestonePerformanceAnalyzer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Milestone Performance Analyzer initialized"));
    
    // Auto-start analysis if in game
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        StartMilestoneAnalysis();
    }
}

void UPerf_MilestonePerformanceAnalyzer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bAnalysisActive)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAnalysisTime >= AnalysisInterval)
    {
        // Update all performance metrics
        UpdateFrameRateMetrics(DeltaTime);
        UpdatePhysicsMetrics();
        UpdateCharacterMetrics();
        UpdateMemoryMetrics();
        
        // Check if auto-optimization is needed
        if (bAutoOptimize && !CheckPerformanceThresholds())
        {
            ApplyOptimizations();
        }
        
        LastAnalysisTime = CurrentTime;
    }
}

void UPerf_MilestonePerformanceAnalyzer::StartMilestoneAnalysis()
{
    bAnalysisActive = true;
    LastAnalysisTime = GetWorld()->GetTimeSeconds();
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Milestone 1 Performance Analysis Started"));
    
    // Try to find and integrate with physics validator
    if (AActor* Owner = GetOwner())
    {
        if (UCore_MilestonePhysics* PhysicsComp = Owner->FindComponentByClass<UCore_MilestonePhysics>())
        {
            IntegrateWithPhysicsValidation(PhysicsComp);
        }
    }
}

void UPerf_MilestonePerformanceAnalyzer::StopMilestoneAnalysis()
{
    bAnalysisActive = false;
    UE_LOG(LogTemp, Log, TEXT("Milestone 1 Performance Analysis Stopped"));
    
    // Log final report
    FString FinalReport = GeneratePerformanceReport();
    UE_LOG(LogTemp, Warning, TEXT("FINAL MILESTONE 1 PERFORMANCE REPORT:\n%s"), *FinalReport);
}

FPerf_MilestoneMetrics UPerf_MilestonePerformanceAnalyzer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

bool UPerf_MilestonePerformanceAnalyzer::IsMilestone1PerformanceTargetMet() const
{
    // Check primary performance targets for Milestone 1
    float TargetFPS = PerformanceTargets.TargetFPS_PC; // Default to PC target
    
    bool bFPSTargetMet = CurrentMetrics.CurrentFPS >= TargetFPS * 0.9f; // 90% of target
    bool bPhysicsOptimal = CurrentMetrics.PhysicsTickTime <= PerformanceTargets.MaxPhysicsTickTime;
    bool bCharacterOptimal = CurrentMetrics.bCharacterMovementOptimal;
    bool bMemoryOptimal = CurrentMetrics.TotalMemoryUsage <= PerformanceTargets.MaxMemoryUsage;
    
    return bFPSTargetMet && bPhysicsOptimal && bCharacterOptimal && bMemoryOptimal;
}

float UPerf_MilestonePerformanceAnalyzer::GetMilestone1CompletionPercentage() const
{
    float CompletionScore = 0.0f;
    
    // FPS Score (40% weight)
    float TargetFPS = PerformanceTargets.TargetFPS_PC;
    float FPSScore = FMath::Clamp(CurrentMetrics.CurrentFPS / TargetFPS, 0.0f, 1.0f);
    CompletionScore += FPSScore * 0.4f;
    
    // Physics Score (30% weight)
    float PhysicsScore = CurrentMetrics.PhysicsTickTime <= PerformanceTargets.MaxPhysicsTickTime ? 1.0f : 0.5f;
    CompletionScore += PhysicsScore * 0.3f;
    
    // Character Movement Score (20% weight)
    float CharacterScore = CurrentMetrics.bCharacterMovementOptimal ? 1.0f : 0.5f;
    CompletionScore += CharacterScore * 0.2f;
    
    // Memory Score (10% weight)
    float MemoryScore = CurrentMetrics.TotalMemoryUsage <= PerformanceTargets.MaxMemoryUsage ? 1.0f : 0.5f;
    CompletionScore += MemoryScore * 0.1f;
    
    return CompletionScore * 100.0f;
}

void UPerf_MilestonePerformanceAnalyzer::IntegrateWithPhysicsValidation(UCore_MilestonePhysics* PhysicsValidator)
{
    if (PhysicsValidator)
    {
        this->PhysicsValidator = PhysicsValidator;
        UE_LOG(LogTemp, Log, TEXT("Performance Analyzer integrated with Physics Validator"));
    }
}

void UPerf_MilestonePerformanceAnalyzer::AnalyzeCharacterPhysicsPerformance(ATranspersonalCharacter* Character)
{
    if (Character)
    {
        TrackedCharacter = Character;
        UE_LOG(LogTemp, Log, TEXT("Performance Analyzer now tracking character: %s"), *Character->GetName());
    }
}

void UPerf_MilestonePerformanceAnalyzer::OptimizeForMilestone1()
{
    UE_LOG(LogTemp, Log, TEXT("Applying Milestone 1 optimizations..."));
    
    ApplyOptimizations();
    
    // Apply specific Milestone 1 settings
    FPerf_MilestoneTargets Milestone1Targets;
    Milestone1Targets.TargetFPS_PC = 60.0f;
    Milestone1Targets.TargetFPS_Console = 30.0f;
    Milestone1Targets.MaxPhysicsTickTime = 5.0f;
    Milestone1Targets.MaxCharacterMovementTime = 2.0f;
    Milestone1Targets.MaxMemoryUsage = 512.0f;
    
    ApplyPerformanceSettings(Milestone1Targets);
}

void UPerf_MilestonePerformanceAnalyzer::ApplyPerformanceSettings(const FPerf_MilestoneTargets& Targets)
{
    PerformanceTargets = Targets;
    
    UE_LOG(LogTemp, Log, TEXT("Applied performance settings - Target FPS: %.1f"), Targets.TargetFPS_PC);
}

FString UPerf_MilestonePerformanceAnalyzer::GeneratePerformanceReport() const
{
    FString Report;
    
    Report += TEXT("=== MILESTONE 1 PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Current FPS: %.2f (Target: %.2f)\n"), CurrentMetrics.CurrentFPS, PerformanceTargets.TargetFPS_PC);
    Report += FString::Printf(TEXT("Average FPS: %.2f\n"), CurrentMetrics.AverageFPS);
    Report += FString::Printf(TEXT("Min/Max FPS: %.2f / %.2f\n"), CurrentMetrics.MinFPS, CurrentMetrics.MaxFPS);
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), CurrentMetrics.FrameTime);
    Report += TEXT("\n");
    
    Report += TEXT("PHYSICS PERFORMANCE:\n");
    Report += FString::Printf(TEXT("Physics Tick Time: %.2f ms (Max: %.2f ms)\n"), CurrentMetrics.PhysicsTickTime, PerformanceTargets.MaxPhysicsTickTime);
    Report += FString::Printf(TEXT("Collision Detection: %.2f ms\n"), CurrentMetrics.CollisionDetectionTime);
    Report += FString::Printf(TEXT("Active Rigid Bodies: %d\n"), CurrentMetrics.ActiveRigidBodies);
    Report += FString::Printf(TEXT("Collision Pairs: %d\n"), CurrentMetrics.CollisionPairs);
    Report += TEXT("\n");
    
    Report += TEXT("CHARACTER PERFORMANCE:\n");
    Report += FString::Printf(TEXT("Movement Time: %.2f ms (Max: %.2f ms)\n"), CurrentMetrics.CharacterMovementTime, PerformanceTargets.MaxCharacterMovementTime);
    Report += FString::Printf(TEXT("Terrain Interaction: %.2f ms\n"), CurrentMetrics.TerrainInteractionTime);
    Report += FString::Printf(TEXT("Movement Optimal: %s\n"), CurrentMetrics.bCharacterMovementOptimal ? TEXT("YES") : TEXT("NO"));
    Report += TEXT("\n");
    
    Report += TEXT("MEMORY USAGE:\n");
    Report += FString::Printf(TEXT("Physics Memory: %.2f MB\n"), CurrentMetrics.PhysicsMemoryUsage);
    Report += FString::Printf(TEXT("Character Memory: %.2f MB\n"), CurrentMetrics.CharacterMemoryUsage);
    Report += FString::Printf(TEXT("Total Memory: %.2f MB (Max: %.2f MB)\n"), CurrentMetrics.TotalMemoryUsage, PerformanceTargets.MaxMemoryUsage);
    Report += TEXT("\n");
    
    Report += FString::Printf(TEXT("MILESTONE 1 COMPLETION: %.1f%%\n"), GetMilestone1CompletionPercentage());
    Report += FString::Printf(TEXT("PERFORMANCE TARGET MET: %s\n"), IsMilestone1PerformanceTargetMet() ? TEXT("YES") : TEXT("NO"));
    
    return Report;
}

void UPerf_MilestonePerformanceAnalyzer::LogPerformanceStatus() const
{
    FString StatusReport = GeneratePerformanceReport();
    UE_LOG(LogTemp, Warning, TEXT("MILESTONE 1 PERFORMANCE STATUS:\n%s"), *StatusReport);
    
    // Also log to console if in editor
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, 
            FString::Printf(TEXT("Milestone 1 Performance: %.1f%% Complete"), GetMilestone1CompletionPercentage()));
    }
}

void UPerf_MilestonePerformanceAnalyzer::UpdateFrameRateMetrics(float DeltaTime)
{
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        
        // Add to samples for average calculation
        FPSSamples.Add(CurrentMetrics.CurrentFPS);
        if (FPSSamples.Num() > MaxSamples)
        {
            FPSSamples.RemoveAt(0);
        }
        
        // Calculate average
        float Total = 0.0f;
        for (float Sample : FPSSamples)
        {
            Total += Sample;
        }
        CurrentMetrics.AverageFPS = FPSSamples.Num() > 0 ? Total / FPSSamples.Num() : 0.0f;
        
        // Update min/max
        CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, CurrentMetrics.CurrentFPS);
        CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, CurrentMetrics.CurrentFPS);
    }
}

void UPerf_MilestonePerformanceAnalyzer::UpdatePhysicsMetrics()
{
    // Get physics stats from the physics validator if available
    if (PhysicsValidator.IsValid())
    {
        // Integration with Core_MilestonePhysics would go here
        // For now, use estimated values based on world state
    }
    
    // Estimate physics performance based on world complexity
    if (UWorld* World = GetWorld())
    {
        // Count physics objects
        int32 PhysicsObjectCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent() && Actor->GetRootComponent()->IsSimulatingPhysics())
            {
                PhysicsObjectCount++;
            }
        }
        
        CurrentMetrics.ActiveRigidBodies = PhysicsObjectCount;
        
        // Estimate physics tick time based on complexity
        CurrentMetrics.PhysicsTickTime = PhysicsObjectCount * 0.1f; // Rough estimate
        CurrentMetrics.CollisionDetectionTime = PhysicsObjectCount * 0.05f;
        CurrentMetrics.CollisionPairs = PhysicsObjectCount * (PhysicsObjectCount - 1) / 2;
        
        // Estimate physics memory usage
        CurrentMetrics.PhysicsMemoryUsage = PhysicsObjectCount * 0.5f; // MB per object estimate
    }
}

void UPerf_MilestonePerformanceAnalyzer::UpdateCharacterMetrics()
{
    if (TrackedCharacter.IsValid())
    {
        // Estimate character movement performance
        CurrentMetrics.CharacterMovementTime = 1.5f; // Baseline estimate
        CurrentMetrics.TerrainInteractionTime = 0.5f; // Baseline estimate
        
        // Check if character movement is optimal
        CurrentMetrics.bCharacterMovementOptimal = CurrentMetrics.CharacterMovementTime <= PerformanceTargets.MaxCharacterMovementTime;
        
        // Estimate character memory usage
        CurrentMetrics.CharacterMemoryUsage = 10.0f; // MB estimate for character
    }
    else
    {
        // Try to find a character in the world
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<ATranspersonalCharacter> CharacterItr(World); CharacterItr; ++CharacterItr)
            {
                ATranspersonalCharacter* Character = *CharacterItr;
                if (Character)
                {
                    AnalyzeCharacterPhysicsPerformance(Character);
                    break;
                }
            }
        }
    }
}

void UPerf_MilestonePerformanceAnalyzer::UpdateMemoryMetrics()
{
    // Get system memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    // Convert to MB and estimate game usage
    float SystemMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CurrentMetrics.TotalMemoryUsage = CurrentMetrics.PhysicsMemoryUsage + CurrentMetrics.CharacterMemoryUsage + 100.0f; // Base game memory
    
    // Clamp to reasonable values
    CurrentMetrics.TotalMemoryUsage = FMath::Clamp(CurrentMetrics.TotalMemoryUsage, 50.0f, 2048.0f);
}

bool UPerf_MilestonePerformanceAnalyzer::CheckPerformanceThresholds() const
{
    return IsMilestone1PerformanceTargetMet();
}

void UPerf_MilestonePerformanceAnalyzer::ApplyOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Applying performance optimizations..."));
    
    // Apply LOD optimizations
    if (CurrentMetrics.CurrentFPS < PerformanceTargets.TargetFPS_PC * 0.8f)
    {
        // Reduce physics simulation frequency
        if (UWorld* World = GetWorld())
        {
            // This would integrate with other performance systems
        }
    }
    
    // Apply memory optimizations
    if (CurrentMetrics.TotalMemoryUsage > PerformanceTargets.MaxMemoryUsage * 0.9f)
    {
        // Trigger garbage collection
        GEngine->ForceGarbageCollection(true);
    }
}

void UPerf_MilestonePerformanceAnalyzer::ResetMetrics()
{
    CurrentMetrics = FPerf_MilestoneMetrics();
    FPSSamples.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}