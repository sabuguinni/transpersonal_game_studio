#include "Perf_PhysicsArchitecturalMonitor.h"
#include "Core/Core_PhysicsArchitecturalIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Engine/GameInstance.h"

UPerf_PhysicsArchitecturalMonitor::UPerf_PhysicsArchitecturalMonitor()
    : PhysicsIntegrator(nullptr)
    , bMonitoringEnabled(true)
    , MonitoringUpdateFrequency(0.1f) // Update 10 times per second
    , TimeSinceLastUpdate(0.0f)
    , ViolationThresholdPercentage(110.0f) // 10% over budget triggers violation
{
}

void UPerf_PhysicsArchitecturalMonitor::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Initializing Physics Architectural Performance Monitor"));
    
    // Initialize default budgets
    InitializeDefaultBudgets();
    
    // Get reference to physics integrator
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        PhysicsIntegrator = GameInstance->GetSubsystem<UCore_PhysicsArchitecturalIntegrator>();
        if (PhysicsIntegrator)
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Connected to Physics Architectural Integrator"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠ Could not find Physics Architectural Integrator"));
        }
    }
    
    // Initialize monitoring
    InitializePhysicsMonitoring();
}

void UPerf_PhysicsArchitecturalMonitor::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Deinitializing Physics Architectural Performance Monitor"));
    
    // Clear references
    PhysicsIntegrator = nullptr;
    PhysicsSystemBudgets.Empty();
    ActiveViolations.Empty();
    
    Super::Deinitialize();
}

void UPerf_PhysicsArchitecturalMonitor::InitializePhysicsMonitoring()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing Physics Performance Monitoring"));
    
    // Reset monitoring state
    TimeSinceLastUpdate = 0.0f;
    ActiveViolations.Empty();
    
    // Initialize performance snapshot
    CurrentSnapshot = FPerf_PhysicsPerformanceSnapshot();
    
    UE_LOG(LogTemp, Log, TEXT("✓ Physics Performance Monitoring Initialized"));
}

void UPerf_PhysicsArchitecturalMonitor::UpdatePerformanceMonitoring(float DeltaTime)
{
    if (!bMonitoringEnabled)
    {
        return;
    }
    
    TimeSinceLastUpdate += DeltaTime;
    
    // Update at specified frequency
    if (TimeSinceLastUpdate >= MonitoringUpdateFrequency)
    {
        UpdatePhysicsMetrics();
        CheckBudgetViolations();
        CalculatePerformanceStats();
        
        TimeSinceLastUpdate = 0.0f;
    }
}

FPerf_PhysicsPerformanceSnapshot UPerf_PhysicsArchitecturalMonitor::GetCurrentPerformanceSnapshot() const
{
    return CurrentSnapshot;
}

bool UPerf_PhysicsArchitecturalMonitor::HasPerformanceViolations() const
{
    return ActiveViolations.Num() > 0;
}

TArray<FPerf_PerformanceViolation> UPerf_PhysicsArchitecturalMonitor::GetPerformanceViolations() const
{
    return ActiveViolations;
}

void UPerf_PhysicsArchitecturalMonitor::SetPhysicsSystemBudget(const FString& SystemName, float FrameTimeBudgetMs, float MemoryBudgetMB, int32 MaxCollisionObjects)
{
    FPerf_PhysicsSystemBudget Budget;
    Budget.SystemName = SystemName;
    Budget.FrameTimeBudgetMs = FrameTimeBudgetMs;
    Budget.MemoryBudgetMB = MemoryBudgetMB;
    Budget.MaxCollisionObjects = MaxCollisionObjects;
    Budget.bIsActive = true;
    
    PhysicsSystemBudgets.Add(SystemName, Budget);
    
    UE_LOG(LogTemp, Log, TEXT("Set budget for %s: %.2fms, %.2fMB, %d objects"), 
           *SystemName, FrameTimeBudgetMs, MemoryBudgetMB, MaxCollisionObjects);
}

FPerf_PhysicsSystemBudget UPerf_PhysicsArchitecturalMonitor::GetPhysicsSystemBudget(const FString& SystemName) const
{
    if (const FPerf_PhysicsSystemBudget* Budget = PhysicsSystemBudgets.Find(SystemName))
    {
        return *Budget;
    }
    
    return FPerf_PhysicsSystemBudget();
}

void UPerf_PhysicsArchitecturalMonitor::SetMonitoringEnabled(bool bEnabled)
{
    bMonitoringEnabled = bEnabled;
    
    if (bEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Performance Monitoring ENABLED"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Performance Monitoring DISABLED"));
        ActiveViolations.Empty();
    }
}

void UPerf_PhysicsArchitecturalMonitor::ForcePerformanceOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Forcing Performance Optimization Pass"));
    
    // Update metrics immediately
    UpdatePhysicsMetrics();
    CheckBudgetViolations();
    
    // Apply optimizations based on violations
    if (HasPerformanceViolations())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance violations detected - applying optimizations"));
        
        for (const FPerf_PerformanceViolation& Violation : ActiveViolations)
        {
            UE_LOG(LogTemp, Warning, TEXT("Violation: %s - %s (%.2f%% over budget)"), 
                   *Violation.ViolatingSystem, *Violation.ViolationType, Violation.ViolationPercentage);
        }
        
        // TODO: Implement specific optimization strategies
        // - Reduce physics simulation quality
        // - Disable non-essential collision objects
        // - Reduce update frequencies
        // - Apply LOD to physics objects
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("✓ No performance violations detected"));
    }
}

FString UPerf_PhysicsArchitecturalMonitor::GetPerformanceStatsSummary() const
{
    FString Summary;
    
    Summary += FString::Printf(TEXT("=== Physics Performance Summary ===\n"));
    Summary += FString::Printf(TEXT("Current FPS: %.1f\n"), CurrentSnapshot.CurrentFPS);
    Summary += FString::Printf(TEXT("Frame Time: %.2fms\n"), CurrentSnapshot.FrameTimeMs);
    Summary += FString::Printf(TEXT("Physics Time: %.2fms\n"), CurrentSnapshot.PhysicsTimeMs);
    Summary += FString::Printf(TEXT("Collision Time: %.2fms\n"), CurrentSnapshot.CollisionTimeMs);
    Summary += FString::Printf(TEXT("Memory Usage: %.2fMB\n"), CurrentSnapshot.MemoryUsageMB);
    Summary += FString::Printf(TEXT("Active Physics Actors: %d\n"), CurrentSnapshot.ActivePhysicsActors);
    Summary += FString::Printf(TEXT("Active Collision Objects: %d\n"), CurrentSnapshot.ActiveCollisionObjects);
    Summary += FString::Printf(TEXT("Budget Violations: %d\n"), ActiveViolations.Num());
    
    if (HasPerformanceViolations())
    {
        Summary += TEXT("\n=== Active Violations ===\n");
        for (const FPerf_PerformanceViolation& Violation : ActiveViolations)
        {
            Summary += FString::Printf(TEXT("- %s: %s (%.1f%% over budget)\n"), 
                       *Violation.ViolatingSystem, *Violation.ViolationType, Violation.ViolationPercentage);
        }
    }
    
    return Summary;
}

void UPerf_PhysicsArchitecturalMonitor::InitializeDefaultBudgets()
{
    // Set default budgets for core physics systems
    SetPhysicsSystemBudget(TEXT("PhysicsManager"), 2.0f, 200.0f, 1000);
    SetPhysicsSystemBudget(TEXT("CollisionSystem"), 1.5f, 150.0f, 2000);
    SetPhysicsSystemBudget(TEXT("MaterialPhysics"), 1.0f, 100.0f, 500);
    SetPhysicsSystemBudget(TEXT("TerrainPhysics"), 3.0f, 300.0f, 1500);
    SetPhysicsSystemBudget(TEXT("RagdollSystem"), 2.5f, 250.0f, 200);
    SetPhysicsSystemBudget(TEXT("DestructionSystem"), 4.0f, 400.0f, 100);
    SetPhysicsSystemBudget(TEXT("FluidDynamics"), 3.5f, 350.0f, 300);
    
    UE_LOG(LogTemp, Log, TEXT("✓ Default physics system budgets initialized"));
}

void UPerf_PhysicsArchitecturalMonitor::UpdatePhysicsMetrics()
{
    // Update current frame time
    CurrentSnapshot.FrameTimeMs = GetCurrentFrameTimeMs();
    CurrentSnapshot.CurrentFPS = (CurrentSnapshot.FrameTimeMs > 0.0f) ? (1000.0f / CurrentSnapshot.FrameTimeMs) : 60.0f;
    
    // Update memory usage
    CurrentSnapshot.MemoryUsageMB = GetCurrentMemoryUsageMB();
    
    // Update physics-specific metrics
    CurrentSnapshot.ActivePhysicsActors = CountActivePhysicsActors();
    CurrentSnapshot.ActiveCollisionObjects = CountActiveCollisionObjects();
    
    // Estimate physics and collision times (simplified)
    CurrentSnapshot.PhysicsTimeMs = CurrentSnapshot.ActivePhysicsActors * 0.01f; // 0.01ms per physics actor
    CurrentSnapshot.CollisionTimeMs = CurrentSnapshot.ActiveCollisionObjects * 0.005f; // 0.005ms per collision object
}

void UPerf_PhysicsArchitecturalMonitor::CheckBudgetViolations()
{
    // Clear previous violations
    ActiveViolations.Empty();
    CurrentSnapshot.bBudgetViolation = false;
    
    // Calculate total physics budget
    float TotalPhysicsBudget = 0.0f;
    for (const auto& BudgetPair : PhysicsSystemBudgets)
    {
        if (BudgetPair.Value.bIsActive)
        {
            TotalPhysicsBudget += BudgetPair.Value.FrameTimeBudgetMs;
        }
    }
    
    // Check frame time violations
    float ActualPhysicsTime = CurrentSnapshot.PhysicsTimeMs + CurrentSnapshot.CollisionTimeMs;
    if (ActualPhysicsTime > TotalPhysicsBudget * (ViolationThresholdPercentage / 100.0f))
    {
        LogPerformanceViolation(TEXT("TotalPhysics"), TEXT("FrameTime"), ActualPhysicsTime, TotalPhysicsBudget);
        CurrentSnapshot.bBudgetViolation = true;
    }
    
    // Check memory violations
    float TotalMemoryBudget = 0.0f;
    for (const auto& BudgetPair : PhysicsSystemBudgets)
    {
        if (BudgetPair.Value.bIsActive)
        {
            TotalMemoryBudget += BudgetPair.Value.MemoryBudgetMB;
        }
    }
    
    if (CurrentSnapshot.MemoryUsageMB > TotalMemoryBudget * (ViolationThresholdPercentage / 100.0f))
    {
        LogPerformanceViolation(TEXT("TotalPhysics"), TEXT("Memory"), CurrentSnapshot.MemoryUsageMB, TotalMemoryBudget);
        CurrentSnapshot.bBudgetViolation = true;
    }
    
    // Check collision object violations
    int32 TotalCollisionBudget = 0;
    for (const auto& BudgetPair : PhysicsSystemBudgets)
    {
        if (BudgetPair.Value.bIsActive)
        {
            TotalCollisionBudget += BudgetPair.Value.MaxCollisionObjects;
        }
    }
    
    if (CurrentSnapshot.ActiveCollisionObjects > TotalCollisionBudget)
    {
        LogPerformanceViolation(TEXT("TotalPhysics"), TEXT("CollisionObjects"), 
                               static_cast<float>(CurrentSnapshot.ActiveCollisionObjects), 
                               static_cast<float>(TotalCollisionBudget));
        CurrentSnapshot.bBudgetViolation = true;
    }
}

void UPerf_PhysicsArchitecturalMonitor::LogPerformanceViolation(const FString& SystemName, const FString& ViolationType, float ActualValue, float BudgetValue)
{
    FPerf_PerformanceViolation Violation;
    Violation.ViolatingSystem = SystemName;
    Violation.ViolationType = ViolationType;
    Violation.ActualValue = ActualValue;
    Violation.BudgetValue = BudgetValue;
    Violation.ViolationPercentage = ((ActualValue - BudgetValue) / BudgetValue) * 100.0f;
    Violation.Timestamp = FDateTime::Now();
    
    ActiveViolations.Add(Violation);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Violation: %s %s - Actual: %.2f, Budget: %.2f (%.1f%% over)"), 
           *SystemName, *ViolationType, ActualValue, BudgetValue, Violation.ViolationPercentage);
}

void UPerf_PhysicsArchitecturalMonitor::CalculatePerformanceStats()
{
    // Additional performance calculations can be added here
    // For now, basic metrics are calculated in UpdatePhysicsMetrics()
}

float UPerf_PhysicsArchitecturalMonitor::GetCurrentFrameTimeMs() const
{
    // Get frame time from engine stats
    if (GEngine && GEngine->GetAverageFrameRate() > 0.0f)
    {
        return 1000.0f / GEngine->GetAverageFrameRate();
    }
    
    return 16.67f; // Default to 60 FPS
}

float UPerf_PhysicsArchitecturalMonitor::GetCurrentMemoryUsageMB() const
{
    // Get memory usage from platform
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

int32 UPerf_PhysicsArchitecturalMonitor::CountActivePhysicsActors() const
{
    int32 Count = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        Count++;
                    }
                }
            }
        }
    }
    
    return Count;
}

int32 UPerf_PhysicsArchitecturalMonitor::CountActiveCollisionObjects() const
{
    int32 Count = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor)
            {
                TArray<UPrimitiveComponent*> PrimComponents;
                Actor->GetComponents<UPrimitiveComponent>(PrimComponents);
                
                for (UPrimitiveComponent* PrimComp : PrimComponents)
                {
                    if (PrimComp && PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
                    {
                        Count++;
                    }
                }
            }
        }
    }
    
    return Count;
}