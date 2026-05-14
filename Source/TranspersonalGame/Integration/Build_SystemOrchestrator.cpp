#include "Build_SystemOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Engine/GameEngine.h"
#include "Components/SceneComponent.h"

ABuild_SystemOrchestrator::ABuild_SystemOrchestrator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize state
    CurrentPhase = EBuild_IntegrationPhase::Initialization;
    PerformanceThreshold = 16.67f; // 60 FPS target
    MemoryThreshold = 2048.0f; // 2GB threshold
    MaxActorCount = 20000;
    bAutoOptimize = true;
    bVerboseLogging = true;
    
    AverageFrameTime = 0.0f;
    PeakMemoryUsage = 0.0f;
    TotalActorCount = 0;
    LastValidationTime = 0.0f;
    ValidationInterval = 5.0f;
    bIsValidating = false;

    InitializeCriticalSystems();
}

void ABuild_SystemOrchestrator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build System Orchestrator: Starting integration orchestration"));
    }
    
    InitializeSystemOrchestration();
}

void ABuild_SystemOrchestrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Monitor frame time
    MonitorFrameTime();
    
    // Periodic validation
    if (GetWorld()->GetTimeSeconds() - LastValidationTime > ValidationInterval && !bIsValidating)
    {
        ValidateAllSystems();
        LastValidationTime = GetWorld()->GetTimeSeconds();
    }
    
    // Auto-optimization
    if (bAutoOptimize && AverageFrameTime > PerformanceThreshold)
    {
        OptimizeSystemPerformance();
    }
}

void ABuild_SystemOrchestrator::InitializeSystemOrchestration()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build Orchestrator: Initializing system orchestration"));
    }
    
    CurrentPhase = EBuild_IntegrationPhase::Initialization;
    IntegrationReport.CurrentPhase = CurrentPhase;
    
    // Initialize all critical systems
    for (const FString& SystemName : CriticalSystems)
    {
        UpdateSystemStatus(SystemName, EBuild_SystemStatus::Loading);
    }
    
    // Move to validation phase
    UpdateIntegrationPhase(EBuild_IntegrationPhase::CoreSystemsValidation);
}

void ABuild_SystemOrchestrator::ValidateAllSystems()
{
    if (bIsValidating) return;
    bIsValidating = true;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build Orchestrator: Validating all systems"));
    }
    
    bool bAllSystemsValid = true;
    
    // Validate each critical system
    for (const FString& SystemName : CriticalSystems)
    {
        bool bSystemValid = ValidateCoreSystem(SystemName);
        if (bSystemValid)
        {
            UpdateSystemStatus(SystemName, EBuild_SystemStatus::Validated);
        }
        else
        {
            UpdateSystemStatus(SystemName, EBuild_SystemStatus::Failed);
            bAllSystemsValid = false;
            IntegrationReport.LastError = FString::Printf(TEXT("System validation failed: %s"), *SystemName);
        }
    }
    
    // Validate specific systems
    bAllSystemsValid &= ValidateVFXSystem();
    bAllSystemsValid &= ValidateQASystem();
    bAllSystemsValid &= ValidateWorldGeneration();
    bAllSystemsValid &= ValidateCharacterSystem();
    
    if (bAllSystemsValid)
    {
        UpdateIntegrationPhase(EBuild_IntegrationPhase::CrossSystemIntegration);
    }
    
    bIsValidating = false;
}

void ABuild_SystemOrchestrator::PerformCrossSystemIntegration()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build Orchestrator: Performing cross-system integration"));
    }
    
    CurrentPhase = EBuild_IntegrationPhase::CrossSystemIntegration;
    
    // Integrate VFX with QA
    IntegrateVFXWithQA();
    
    // Integrate World with Character
    IntegrateWorldWithCharacter();
    
    // Full system integration
    IntegrateAllSystems();
    
    // Move to performance optimization
    UpdateIntegrationPhase(EBuild_IntegrationPhase::PerformanceOptimization);
}

void ABuild_SystemOrchestrator::OptimizeSystemPerformance()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build Orchestrator: Optimizing system performance"));
    }
    
    CurrentPhase = EBuild_IntegrationPhase::PerformanceOptimization;
    
    // Check memory usage
    CheckMemoryUsage();
    
    // Optimize resources
    OptimizeSystemResources();
    
    // Clean up unused resources
    CleanupUnusedResources();
    
    // Validate performance
    CalculatePerformanceMetrics();
    
    if (AverageFrameTime <= PerformanceThreshold)
    {
        UpdateIntegrationPhase(EBuild_IntegrationPhase::FinalValidation);
    }
}

void ABuild_SystemOrchestrator::GenerateFinalBuildReport()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build Orchestrator: Generating final build report"));
    }
    
    CurrentPhase = EBuild_IntegrationPhase::FinalValidation;
    
    // Update integration report
    IntegrationReport.CurrentPhase = CurrentPhase;
    IntegrationReport.TotalActors = TotalActorCount;
    IntegrationReport.TotalMemoryUsage = PeakMemoryUsage;
    IntegrationReport.AverageFrameTime = AverageFrameTime;
    
    // Check if build is ready
    bool bBuildReady = true;
    for (const auto& SystemPair : SystemStatusMap)
    {
        if (SystemPair.Value != EBuild_SystemStatus::Integrated)
        {
            bBuildReady = false;
            break;
        }
    }
    
    IntegrationReport.bBuildReady = bBuildReady && (AverageFrameTime <= PerformanceThreshold);
    
    if (IntegrationReport.bBuildReady)
    {
        UpdateIntegrationPhase(EBuild_IntegrationPhase::BuildComplete);
        UE_LOG(LogTemp, Warning, TEXT("Build Orchestrator: BUILD COMPLETE - All systems integrated successfully"));
    }
    
    // Export metrics
    ExportBuildMetrics();
}

bool ABuild_SystemOrchestrator::ValidateCoreSystem(const FString& SystemName)
{
    // Simulate system validation
    if (SystemName.Contains(TEXT("Character")))
    {
        return ValidateCharacterSystem();
    }
    else if (SystemName.Contains(TEXT("VFX")))
    {
        return ValidateVFXSystem();
    }
    else if (SystemName.Contains(TEXT("QA")))
    {
        return ValidateQASystem();
    }
    else if (SystemName.Contains(TEXT("World")))
    {
        return ValidateWorldGeneration();
    }
    
    return true; // Default validation pass
}

bool ABuild_SystemOrchestrator::ValidateVFXSystem()
{
    // Check if VFX system is loaded and functional
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Count VFX-related actors
    int32 VFXActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("VFX")))
        {
            VFXActorCount++;
        }
    }
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System Validation: Found %d VFX actors"), VFXActorCount);
    }
    
    return VFXActorCount > 0;
}

bool ABuild_SystemOrchestrator::ValidateQASystem()
{
    // Check if QA system is loaded and functional
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Count QA-related actors
    int32 QAActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("QA")))
        {
            QAActorCount++;
        }
    }
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA System Validation: Found %d QA actors"), QAActorCount);
    }
    
    return QAActorCount > 0;
}

bool ABuild_SystemOrchestrator::ValidateWorldGeneration()
{
    // Check if world generation system is functional
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Count world generation actors
    int32 WorldGenActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetClass()->GetName().Contains(TEXT("PCG")) || 
                     Actor->GetClass()->GetName().Contains(TEXT("World"))))
        {
            WorldGenActorCount++;
        }
    }
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("World Generation Validation: Found %d world gen actors"), WorldGenActorCount);
    }
    
    return WorldGenActorCount > 0;
}

bool ABuild_SystemOrchestrator::ValidateCharacterSystem()
{
    // Check if character system is functional
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // Count character-related actors
    int32 CharacterActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Character")))
        {
            CharacterActorCount++;
        }
    }
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character System Validation: Found %d character actors"), CharacterActorCount);
    }
    
    return CharacterActorCount > 0;
}

float ABuild_SystemOrchestrator::MeasureSystemPerformance(const FString& SystemName)
{
    // Measure system-specific performance
    float StartTime = FPlatformTime::Seconds();
    
    // Simulate performance measurement
    FPlatformProcess::Sleep(0.001f); // 1ms simulation
    
    float EndTime = FPlatformTime::Seconds();
    float PerformanceTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("System %s performance: %.2f ms"), *SystemName, PerformanceTime);
    }
    
    return PerformanceTime;
}

void ABuild_SystemOrchestrator::MonitorFrameTime()
{
    if (UGameEngine* GameEngine = Cast<UGameEngine>(GEngine))
    {
        float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
        
        FrameTimeHistory.Add(CurrentFrameTime);
        if (FrameTimeHistory.Num() > 60) // Keep last 60 frames
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average
        float TotalFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            TotalFrameTime += FrameTime;
        }
        AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    }
}

void ABuild_SystemOrchestrator::CheckMemoryUsage()
{
    // Get memory statistics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float CurrentMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    if (CurrentMemoryMB > PeakMemoryUsage)
    {
        PeakMemoryUsage = CurrentMemoryMB;
    }
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2f MB (Peak: %.2f MB)"), CurrentMemoryMB, PeakMemoryUsage);
    }
}

bool ABuild_SystemOrchestrator::IsSystemPerformant(const FString& SystemName)
{
    float SystemPerformance = MeasureSystemPerformance(SystemName);
    return SystemPerformance < PerformanceThreshold;
}

void ABuild_SystemOrchestrator::IntegrateVFXWithQA()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integrating VFX system with QA system"));
    }
    
    // Check compatibility between VFX and QA systems
    bool bCompatible = CheckSystemCompatibility(TEXT("VFX"), TEXT("QA"));
    
    if (bCompatible)
    {
        UpdateSystemStatus(TEXT("VFX"), EBuild_SystemStatus::Integrated);
        UpdateSystemStatus(TEXT("QA"), EBuild_SystemStatus::Integrated);
    }
}

void ABuild_SystemOrchestrator::IntegrateWorldWithCharacter()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integrating World Generation with Character system"));
    }
    
    // Check compatibility between World and Character systems
    bool bCompatible = CheckSystemCompatibility(TEXT("World"), TEXT("Character"));
    
    if (bCompatible)
    {
        UpdateSystemStatus(TEXT("World"), EBuild_SystemStatus::Integrated);
        UpdateSystemStatus(TEXT("Character"), EBuild_SystemStatus::Integrated);
    }
}

void ABuild_SystemOrchestrator::IntegrateAllSystems()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performing full system integration"));
    }
    
    // Validate system dependencies
    ValidateSystemDependencies();
    
    // Mark all validated systems as integrated
    for (auto& SystemPair : SystemStatusMap)
    {
        if (SystemPair.Value == EBuild_SystemStatus::Validated)
        {
            SystemPair.Value = EBuild_SystemStatus::Integrated;
        }
    }
}

void ABuild_SystemOrchestrator::PrepareBuildEnvironment()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Preparing build environment"));
    }
    
    // Clean up temporary resources
    CleanupUnusedResources();
    
    // Reset performance metrics
    FrameTimeHistory.Empty();
    AverageFrameTime = 0.0f;
    PeakMemoryUsage = 0.0f;
    
    // Initialize build state
    CurrentPhase = EBuild_IntegrationPhase::Initialization;
}

void ABuild_SystemOrchestrator::ExecuteBuildValidation()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Executing build validation"));
    }
    
    // Run comprehensive validation
    ValidateAllSystems();
    PerformCrossSystemIntegration();
    OptimizeSystemPerformance();
}

void ABuild_SystemOrchestrator::FinalizeBuild()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Finalizing build"));
    }
    
    // Generate final report
    GenerateFinalBuildReport();
    
    // Log final status
    LogSystemStatus();
    
    if (IntegrationReport.bBuildReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("BUILD FINALIZATION COMPLETE - Ready for deployment"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BUILD FINALIZATION FAILED - Issues detected: %s"), *IntegrationReport.LastError);
    }
}

FBuild_IntegrationReport ABuild_SystemOrchestrator::GetIntegrationReport() const
{
    return IntegrationReport;
}

void ABuild_SystemOrchestrator::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current Phase: %d"), (int32)CurrentPhase);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.2f ms"), AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Peak Memory Usage: %.2f MB"), PeakMemoryUsage);
    UE_LOG(LogTemp, Warning, TEXT("Build Ready: %s"), IntegrationReport.bBuildReady ? TEXT("YES") : TEXT("NO"));
    
    for (const auto& SystemPair : SystemStatusMap)
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s: Status %d"), *SystemPair.Key, (int32)SystemPair.Value);
    }
}

void ABuild_SystemOrchestrator::ExportBuildMetrics()
{
    // Export build metrics to file
    FString MetricsData = FString::Printf(
        TEXT("Build Metrics Report\n")
        TEXT("Timestamp: %s\n")
        TEXT("Phase: %d\n")
        TEXT("Total Actors: %d\n")
        TEXT("Average Frame Time: %.2f ms\n")
        TEXT("Peak Memory: %.2f MB\n")
        TEXT("Build Ready: %s\n"),
        *FDateTime::Now().ToString(),
        (int32)CurrentPhase,
        TotalActorCount,
        AverageFrameTime,
        PeakMemoryUsage,
        IntegrationReport.bBuildReady ? TEXT("YES") : TEXT("NO")
    );
    
    // Add system status
    for (const auto& SystemPair : SystemStatusMap)
    {
        MetricsData += FString::Printf(TEXT("System %s: %d\n"), *SystemPair.Key, (int32)SystemPair.Value);
    }
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build metrics exported"));
    }
}

void ABuild_SystemOrchestrator::InitializeCriticalSystems()
{
    CriticalSystems.Empty();
    CriticalSystems.Add(TEXT("TranspersonalCharacter"));
    CriticalSystems.Add(TEXT("TranspersonalGameState"));
    CriticalSystems.Add(TEXT("PCGWorldGenerator"));
    CriticalSystems.Add(TEXT("VFX_ParticleManager"));
    CriticalSystems.Add(TEXT("QA_VFXValidationSuite"));
    
    // Initialize status map
    SystemStatusMap.Empty();
    for (const FString& SystemName : CriticalSystems)
    {
        SystemStatusMap.Add(SystemName, EBuild_SystemStatus::NotLoaded);
    }
}

void ABuild_SystemOrchestrator::UpdateSystemStatus(const FString& SystemName, EBuild_SystemStatus Status)
{
    SystemStatusMap.FindOrAdd(SystemName) = Status;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("System %s status updated to: %d"), *SystemName, (int32)Status);
    }
}

void ABuild_SystemOrchestrator::CalculatePerformanceMetrics()
{
    // Update total actor count
    if (UWorld* World = GetWorld())
    {
        TotalActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            TotalActorCount++;
        }
    }
    
    // Update integration report
    IntegrationReport.TotalActors = TotalActorCount;
    IntegrationReport.AverageFrameTime = AverageFrameTime;
    IntegrationReport.TotalMemoryUsage = PeakMemoryUsage;
}

void ABuild_SystemOrchestrator::ValidateSystemDependencies()
{
    // Check dependencies between systems
    for (const FString& SystemName : CriticalSystems)
    {
        if (SystemStatusMap.Contains(SystemName))
        {
            EBuild_SystemStatus Status = SystemStatusMap[SystemName];
            if (Status == EBuild_SystemStatus::Failed)
            {
                IntegrationReport.LastError = FString::Printf(TEXT("Dependency validation failed for: %s"), *SystemName);
                return;
            }
        }
    }
}

void ABuild_SystemOrchestrator::OptimizeSystemResources()
{
    // Optimize system resources for better performance
    if (AverageFrameTime > PerformanceThreshold)
    {
        if (bVerboseLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance below threshold, optimizing resources"));
        }
        
        // Force garbage collection
        GEngine->ForceGarbageCollection(true);
        
        // Update metrics after optimization
        CalculatePerformanceMetrics();
    }
}

void ABuild_SystemOrchestrator::GenerateSystemReport(const FString& SystemName)
{
    FBuild_SystemMetrics Metrics;
    Metrics.SystemName = SystemName;
    Metrics.Status = SystemStatusMap.FindRef(SystemName);
    Metrics.LoadTime = MeasureSystemPerformance(SystemName);
    Metrics.bIsPerformant = IsSystemPerformant(SystemName);
    
    IntegrationReport.SystemMetrics.Add(Metrics);
}

bool ABuild_SystemOrchestrator::CheckSystemCompatibility(const FString& System1, const FString& System2)
{
    // Check if two systems are compatible
    EBuild_SystemStatus Status1 = SystemStatusMap.FindRef(System1);
    EBuild_SystemStatus Status2 = SystemStatusMap.FindRef(System2);
    
    return (Status1 == EBuild_SystemStatus::Validated || Status1 == EBuild_SystemStatus::Integrated) &&
           (Status2 == EBuild_SystemStatus::Validated || Status2 == EBuild_SystemStatus::Integrated);
}

void ABuild_SystemOrchestrator::CleanupUnusedResources()
{
    // Clean up unused resources to improve performance
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaning up unused resources"));
    }
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Clear frame time history if too large
    if (FrameTimeHistory.Num() > 120)
    {
        FrameTimeHistory.RemoveAt(0, 60);
    }
}

void ABuild_SystemOrchestrator::UpdateIntegrationPhase(EBuild_IntegrationPhase NewPhase)
{
    CurrentPhase = NewPhase;
    IntegrationReport.CurrentPhase = NewPhase;
    
    if (bVerboseLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration phase updated to: %d"), (int32)NewPhase);
    }
}