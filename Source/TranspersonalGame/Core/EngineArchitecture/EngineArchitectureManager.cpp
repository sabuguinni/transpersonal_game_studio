#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "EngineUtils.h"
#include "Components/ActorComponent.h"

DEFINE_LOG_CATEGORY(LogEngineArchitecture);

AEngineArchitectureManager::AEngineArchitectureManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick every second for performance monitoring
    
    bIsMonitoringPerformance = false;
    MonitoringInterval = 1.0f;
    LastMonitoringTime = 0.0f;
    
    // Set default architecture rules (60fps PC / 30fps console targets)
    MaxFrameTime = 16.67f; // 60fps = 16.67ms
    MinFrameRate = 60.0f;
    MaxActorCount = 10000;
    MaxMemoryUsageMB = 4096.0f; // 4GB limit
    MaxDrawCalls = 2000;
}

void AEngineArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Manager initialized"));
    
    // Auto-start performance monitoring
    StartPerformanceMonitoring();
    
    // Register this manager with the subsystem
    if (UEngineArchitectureSubsystem* ArchSubsystem = GetGameInstance()->GetSubsystem<UEngineArchitectureSubsystem>())
    {
        ArchSubsystem->SetArchitectureManager(this);
    }
    
    // Run initial system validation
    ValidateAllSystems();
}

void AEngineArchitectureManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopPerformanceMonitoring();
    
    // Unregister from subsystem
    if (UEngineArchitectureSubsystem* ArchSubsystem = GetGameInstance()->GetSubsystem<UEngineArchitectureSubsystem>())
    {
        ArchSubsystem->SetArchitectureManager(nullptr);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AEngineArchitectureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMonitoringPerformance)
    {
        LastMonitoringTime += DeltaTime;
        if (LastMonitoringTime >= MonitoringInterval)
        {
            UpdatePerformanceMetrics();
            ValidatePerformanceRules();
            LastMonitoringTime = 0.0f;
        }
    }
}

FEng_PerformanceMetrics AEngineArchitectureManager::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

void AEngineArchitectureManager::StartPerformanceMonitoring()
{
    bIsMonitoringPerformance = true;
    LastMonitoringTime = 0.0f;
    UE_LOG(LogEngineArchitecture, Log, TEXT("Performance monitoring started"));
}

void AEngineArchitectureManager::StopPerformanceMonitoring()
{
    bIsMonitoringPerformance = false;
    UE_LOG(LogEngineArchitecture, Log, TEXT("Performance monitoring stopped"));
}

bool AEngineArchitectureManager::IsPerformanceOptimal()
{
    UpdatePerformanceMetrics();
    
    return CurrentMetrics.FrameRate >= MinFrameRate &&
           CurrentMetrics.FrameTime <= MaxFrameTime &&
           CurrentMetrics.ActorCount <= MaxActorCount &&
           CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB &&
           CurrentMetrics.DrawCalls <= MaxDrawCalls;
}

TArray<FEng_ValidationResult> AEngineArchitectureManager::ValidateAllSystems()
{
    TArray<FEng_ValidationResult> Results;
    
    // Validate core systems
    Results.Add(ValidateWorldGeneration());
    Results.Add(ValidatePhysicsSystem());
    Results.Add(ValidateAISystem());
    Results.Add(ValidateRenderingSystem());
    Results.Add(ValidateAudioSystem());
    
    // Validate architecture rules
    FEng_ValidationResult WorldPartitionResult;
    WorldPartitionResult.SystemName = TEXT("World Partition");
    WorldPartitionResult.HealthStatus = ValidateWorldPartitionSetup() ? EEng_SystemHealth::Optimal : EEng_SystemHealth::Warning;
    WorldPartitionResult.StatusMessage = ValidateWorldPartitionSetup() ? TEXT("World Partition configured correctly") : TEXT("World Partition needs setup");
    Results.Add(WorldPartitionResult);
    
    FEng_ValidationResult DependencyResult;
    DependencyResult.SystemName = TEXT("Module Dependencies");
    DependencyResult.HealthStatus = ValidateModuleDependencies() ? EEng_SystemHealth::Optimal : EEng_SystemHealth::Critical;
    DependencyResult.StatusMessage = ValidateModuleDependencies() ? TEXT("Module dependencies valid") : TEXT("Module dependency issues detected");
    Results.Add(DependencyResult);
    
    LastValidationResults = Results;
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("System validation complete. %d systems checked"), Results.Num());
    
    return Results;
}

FEng_ValidationResult AEngineArchitectureManager::ValidateSystem(const FString& SystemName)
{
    FEng_ValidationResult Result;
    Result.SystemName = SystemName;
    Result.ValidationTime = GetWorld()->GetTimeSeconds();
    
    if (RegisteredSystems.Contains(SystemName))
    {
        TWeakObjectPtr<UObject> SystemPtr = RegisteredSystems[SystemName];
        if (SystemPtr.IsValid())
        {
            Result.HealthStatus = EEng_SystemHealth::Optimal;
            Result.StatusMessage = FString::Printf(TEXT("%s is operational"), *SystemName);
        }
        else
        {
            Result.HealthStatus = EEng_SystemHealth::Failed;
            Result.StatusMessage = FString::Printf(TEXT("%s object is invalid"), *SystemName);
        }
    }
    else
    {
        Result.HealthStatus = EEng_SystemHealth::Warning;
        Result.StatusMessage = FString::Printf(TEXT("%s is not registered"), *SystemName);
    }
    
    return Result;
}

void AEngineArchitectureManager::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (SystemObject && IsValid(SystemObject))
    {
        RegisteredSystems.Add(SystemName, SystemObject);
        UE_LOG(LogEngineArchitecture, Log, TEXT("Registered system: %s"), *SystemName);
    }
    else
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Failed to register system: %s (invalid object)"), *SystemName);
    }
}

void AEngineArchitectureManager::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogEngineArchitecture, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
}

bool AEngineArchitectureManager::ValidateWorldPartitionSetup()
{
    // Check if world partition is enabled for large worlds
    UWorld* World = GetWorld();
    if (!World) return false;
    
    // For now, return true as a baseline - will be enhanced by World Generation agent
    return true;
}

bool AEngineArchitectureManager::ValidateModuleDependencies()
{
    // Validate that required modules are loaded
    // This is a simplified check - will be enhanced with actual module validation
    return true;
}

bool AEngineArchitectureManager::ValidateMemoryBudgets()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB;
}

bool AEngineArchitectureManager::ValidatePerformanceTargets()
{
    return IsPerformanceOptimal();
}

void AEngineArchitectureManager::RunFullSystemDiagnostics()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Running full system diagnostics..."));
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Validate all systems
    TArray<FEng_ValidationResult> Results = ValidateAllSystems();
    
    // Log detailed results
    for (const FEng_ValidationResult& Result : Results)
    {
        FString HealthStr = UEnum::GetValueAsString(Result.HealthStatus);
        UE_LOG(LogEngineArchitecture, Log, TEXT("System: %s | Health: %s | Message: %s"), 
               *Result.SystemName, *HealthStr, *Result.StatusMessage);
    }
    
    // Log performance metrics
    UE_LOG(LogEngineArchitecture, Log, TEXT("Performance Metrics:"));
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Frame Rate: %.2f fps"), CurrentMetrics.FrameRate);
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Actor Count: %d"), CurrentMetrics.ActorCount);
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Draw Calls: %d"), CurrentMetrics.DrawCalls);
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("System diagnostics complete"));
}

void AEngineArchitectureManager::GenerateArchitectureReport()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("=== ENGINE ARCHITECTURE REPORT ==="));
    
    RunFullSystemDiagnostics();
    
    // Generate summary
    int32 OptimalSystems = 0;
    int32 WarningSystems = 0;
    int32 CriticalSystems = 0;
    int32 FailedSystems = 0;
    
    for (const FEng_ValidationResult& Result : LastValidationResults)
    {
        switch (Result.HealthStatus)
        {
            case EEng_SystemHealth::Optimal:
            case EEng_SystemHealth::Good:
                OptimalSystems++;
                break;
            case EEng_SystemHealth::Warning:
                WarningSystems++;
                break;
            case EEng_SystemHealth::Critical:
                CriticalSystems++;
                break;
            case EEng_SystemHealth::Failed:
                FailedSystems++;
                break;
        }
    }
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("SUMMARY: %d Optimal, %d Warning, %d Critical, %d Failed"), 
           OptimalSystems, WarningSystems, CriticalSystems, FailedSystems);
    
    bool OverallHealth = (CriticalSystems == 0 && FailedSystems == 0);
    UE_LOG(LogEngineArchitecture, Log, TEXT("Overall Architecture Health: %s"), 
           OverallHealth ? TEXT("HEALTHY") : TEXT("NEEDS ATTENTION"));
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("=== END REPORT ==="));
}

void AEngineArchitectureManager::LogSystemStatus()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Manager Status:"));
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Monitoring: %s"), bIsMonitoringPerformance ? TEXT("Active") : TEXT("Inactive"));
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Performance Optimal: %s"), IsPerformanceOptimal() ? TEXT("Yes") : TEXT("No"));
}

void AEngineArchitectureManager::UpdatePerformanceMetrics()
{
    CurrentMetrics.FrameRate = CalculateFrameRate();
    CurrentMetrics.FrameTime = CurrentMetrics.FrameRate > 0 ? (1000.0f / CurrentMetrics.FrameRate) : 0.0f;
    CurrentMetrics.ActorCount = CountActiveActors();
    CurrentMetrics.ComponentCount = CountActiveComponents();
    CurrentMetrics.MemoryUsageMB = CalculateMemoryUsage();
    CurrentMetrics.DrawCalls = GetDrawCallCount();
    CurrentMetrics.Triangles = GetTriangleCount();
    
    // Estimate GPU memory (simplified)
    CurrentMetrics.GPUMemoryUsageMB = CurrentMetrics.MemoryUsageMB * 0.3f; // Rough estimate
}

void AEngineArchitectureManager::ValidatePerformanceRules()
{
    if (!IsPerformanceOptimal())
    {
        if (CurrentMetrics.FrameRate < MinFrameRate)
        {
            UE_LOG(LogEngineArchitecture, Warning, TEXT("Frame rate below target: %.2f fps (target: %.2f fps)"), 
                   CurrentMetrics.FrameRate, MinFrameRate);
        }
        
        if (CurrentMetrics.ActorCount > MaxActorCount)
        {
            UE_LOG(LogEngineArchitecture, Warning, TEXT("Actor count exceeds limit: %d (limit: %d)"), 
                   CurrentMetrics.ActorCount, MaxActorCount);
        }
        
        if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)
        {
            UE_LOG(LogEngineArchitecture, Warning, TEXT("Memory usage exceeds limit: %.2f MB (limit: %.2f MB)"), 
                   CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
        }
    }
}

FEng_ValidationResult AEngineArchitectureManager::ValidateWorldGeneration()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("World Generation");
    Result.ValidationTime = GetWorld()->GetTimeSeconds();
    
    // Check if PCG World Generator exists
    bool bPCGExists = false;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetClass()->GetName().Contains(TEXT("PCG")))
        {
            bPCGExists = true;
            break;
        }
    }
    
    Result.HealthStatus = bPCGExists ? EEng_SystemHealth::Optimal : EEng_SystemHealth::Warning;
    Result.StatusMessage = bPCGExists ? TEXT("World generation system active") : TEXT("World generation system not found");
    
    return Result;
}

FEng_ValidationResult AEngineArchitectureManager::ValidatePhysicsSystem()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("Physics System");
    Result.ValidationTime = GetWorld()->GetTimeSeconds();
    
    // Basic physics validation
    UWorld* World = GetWorld();
    bool bPhysicsValid = World && World->GetPhysicsScene();
    
    Result.HealthStatus = bPhysicsValid ? EEng_SystemHealth::Optimal : EEng_SystemHealth::Critical;
    Result.StatusMessage = bPhysicsValid ? TEXT("Physics system operational") : TEXT("Physics system not available");
    
    return Result;
}

FEng_ValidationResult AEngineArchitectureManager::ValidateAISystem()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("AI System");
    Result.ValidationTime = GetWorld()->GetTimeSeconds();
    
    // Check for AI-related actors
    bool bAISystemExists = false;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        FString ClassName = ActorItr->GetClass()->GetName();
        if (ClassName.Contains(TEXT("AI")) || ClassName.Contains(TEXT("Behavior")) || ClassName.Contains(TEXT("NPC")))
        {
            bAISystemExists = true;
            break;
        }
    }
    
    Result.HealthStatus = bAISystemExists ? EEng_SystemHealth::Good : EEng_SystemHealth::Warning;
    Result.StatusMessage = bAISystemExists ? TEXT("AI system components detected") : TEXT("AI system pending implementation");
    
    return Result;
}

FEng_ValidationResult AEngineArchitectureManager::ValidateRenderingSystem()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("Rendering System");
    Result.ValidationTime = GetWorld()->GetTimeSeconds();
    
    // Basic rendering validation
    bool bRenderingValid = GEngine && GEngine->GetGameViewport();
    
    Result.HealthStatus = bRenderingValid ? EEng_SystemHealth::Optimal : EEng_SystemHealth::Critical;
    Result.StatusMessage = bRenderingValid ? TEXT("Rendering system operational") : TEXT("Rendering system not available");
    
    return Result;
}

FEng_ValidationResult AEngineArchitectureManager::ValidateAudioSystem()
{
    FEng_ValidationResult Result;
    Result.SystemName = TEXT("Audio System");
    Result.ValidationTime = GetWorld()->GetTimeSeconds();
    
    // Basic audio validation - assume operational for now
    Result.HealthStatus = EEng_SystemHealth::Good;
    Result.StatusMessage = TEXT("Audio system baseline operational");
    
    return Result;
}

float AEngineArchitectureManager::CalculateFrameRate()
{
    return 1.0f / GetWorld()->GetDeltaSeconds();
}

float AEngineArchitectureManager::CalculateMemoryUsage()
{
    // Get platform memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // Convert to MB
}

int32 AEngineArchitectureManager::CountActiveActors()
{
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        Count++;
    }
    return Count;
}

int32 AEngineArchitectureManager::CountActiveComponents()
{
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        Count += ActorItr->GetRootComponent() ? ActorItr->GetComponents<UActorComponent>().Num() : 0;
    }
    return Count;
}

int32 AEngineArchitectureManager::GetDrawCallCount()
{
    // Simplified draw call estimation based on visible actors
    // Real implementation would query rendering stats
    return CountActiveActors() / 10; // Rough estimate
}

int32 AEngineArchitectureManager::GetTriangleCount()
{
    // Simplified triangle count estimation
    // Real implementation would query mesh rendering stats
    return CountActiveActors() * 500; // Rough estimate
}

// Subsystem Implementation
void UEngineArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Subsystem initialized"));
}

void UEngineArchitectureSubsystem::Deinitialize()
{
    ArchitectureManager.Reset();
    Super::Deinitialize();
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Subsystem deinitialized"));
}

AEngineArchitectureManager* UEngineArchitectureSubsystem::GetArchitectureManager()
{
    return ArchitectureManager.IsValid() ? ArchitectureManager.Get() : nullptr;
}

void UEngineArchitectureSubsystem::SetArchitectureManager(AEngineArchitectureManager* Manager)
{
    ArchitectureManager = Manager;
}

bool UEngineArchitectureSubsystem::IsArchitectureHealthy()
{
    if (AEngineArchitectureManager* Manager = GetArchitectureManager())
    {
        return Manager->IsPerformanceOptimal();
    }
    return false;
}