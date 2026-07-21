#include "Perf_PhysicsRegistryIntegrator.h"
#include "Core/Core_PhysicsRegistryBridge.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UPerf_PhysicsRegistryIntegrator::UPerf_PhysicsRegistryIntegrator()
{
    CurrentProfile = EPerf_PerformanceProfile::Development;
    bPerformanceMonitoringEnabled = true;
    EmergencyOptimizationThreshold = 85.0f;
    MonitoringInterval = 0.1f; // 100ms monitoring interval
    bConnectedToRegistry = false;
    PhysicsRegistryBridge = nullptr;
}

void UPerf_PhysicsRegistryIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Physics Registry Integrator initializing"));
    
    // Initialize performance budgets
    InitializePerformanceBudgets();
    
    // Attempt to connect to Physics Registry Bridge
    ConnectToPhysicsRegistry();
    
    // Start performance monitoring if enabled
    if (bPerformanceMonitoringEnabled && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            PerformanceMonitoringTimer,
            this,
            &UPerf_PhysicsRegistryIntegrator::UpdatePerformanceMetrics,
            MonitoringInterval,
            true
        );
        
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Performance monitoring started (%.1fms interval)"), MonitoringInterval * 1000.0f);
    }
}

void UPerf_PhysicsRegistryIntegrator::Deinitialize()
{
    // Stop performance monitoring
    if (GetWorld() && PerformanceMonitoringTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
    }
    
    // Clear Physics Registry Bridge reference
    PhysicsRegistryBridge = nullptr;
    bConnectedToRegistry = false;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Physics Registry Integrator deinitialized"));
    
    Super::Deinitialize();
}

bool UPerf_PhysicsRegistryIntegrator::ConnectToPhysicsRegistry()
{
    // Try to get Core Physics Registry Bridge subsystem
    if (GetGameInstance())
    {
        PhysicsRegistryBridge = GetGameInstance()->GetSubsystem<UCore_PhysicsRegistryBridge>();
        
        if (PhysicsRegistryBridge)
        {
            bConnectedToRegistry = true;
            UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Successfully connected to Core Physics Registry Bridge"));
            
            // Get initial metrics
            UpdatePerformanceMetrics();
            
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Core Physics Registry Bridge not found - creating standalone mode"));
        }
    }
    
    bConnectedToRegistry = false;
    return false;
}

FPerf_PhysicsRegistryMetrics UPerf_PhysicsRegistryIntegrator::GetPhysicsRegistryMetrics()
{
    if (bConnectedToRegistry && PhysicsRegistryBridge)
    {
        // Get metrics from Physics Registry Bridge
        // Note: This would call actual bridge methods when available
        // For now, return cached metrics
        return LastMetrics;
    }
    
    // Return default metrics if not connected
    FPerf_PhysicsRegistryMetrics DefaultMetrics;
    DefaultMetrics.PerformanceBudget = GetCurrentPerformanceBudget();
    return DefaultMetrics;
}

void UPerf_PhysicsRegistryIntegrator::SetPerformanceProfile(EPerf_PerformanceProfile Profile)
{
    CurrentProfile = Profile;
    float NewBudget = GetCurrentPerformanceBudget();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Performance profile changed to %s (%.1fms budget)"), 
           *UEnum::GetValueAsString(Profile), NewBudget);
    
    // Send budget update to Physics Registry Bridge
    if (bConnectedToRegistry && PhysicsRegistryBridge)
    {
        // Note: This would call actual bridge method when available
        // PhysicsRegistryBridge->SetPerformanceBudget(NewBudget);
    }
    
    // Update last metrics with new budget
    LastMetrics.PerformanceBudget = NewBudget;
    if (LastMetrics.PhysicsFrameTime > 0.0f)
    {
        LastMetrics.BudgetUsagePercent = (LastMetrics.PhysicsFrameTime / NewBudget) * 100.0f;
        LastMetrics.bWithinBudget = LastMetrics.BudgetUsagePercent <= 100.0f;
    }
}

bool UPerf_PhysicsRegistryIntegrator::SendOptimizationDirective(const FString& SystemName, const FPerf_PhysicsOptimizationDirective& Directive)
{
    if (!bConnectedToRegistry || !PhysicsRegistryBridge)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Cannot send directive - not connected to Physics Registry Bridge"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Sending optimization directive to %s (Target: %.2fms, Current: %.2fms)"),
           *SystemName, Directive.TargetBudget, Directive.CurrentUsage);
    
    // Send directive through Physics Registry Bridge messaging system
    // Note: This would use actual bridge messaging when available
    // PhysicsRegistryBridge->BroadcastPhysicsSystemMessage(SystemName, DirectiveMessage);
    
    return true;
}

TArray<FPerf_PhysicsOptimizationDirective> UPerf_PhysicsRegistryIntegrator::AnalyzePhysicsPerformance()
{
    TArray<FPerf_PhysicsOptimizationDirective> Directives;
    
    if (!bConnectedToRegistry || !PhysicsRegistryBridge)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Cannot analyze performance - not connected to Physics Registry Bridge"));
        return Directives;
    }
    
    // Get current performance metrics
    FPerf_PhysicsRegistryMetrics CurrentMetrics = GetPhysicsRegistryMetrics();
    float TotalBudget = CurrentMetrics.PerformanceBudget;
    
    // Define system budget allocations (based on Core Physics Registry Bridge priorities)
    TMap<FString, float> SystemBudgets;
    SystemBudgets.Add(TEXT("PhysicsManager"), TotalBudget * 0.25f);        // 25% - Critical
    SystemBudgets.Add(TEXT("MaterialPhysics"), TotalBudget * 0.20f);       // 20% - Critical
    SystemBudgets.Add(TEXT("CollisionSystem"), TotalBudget * 0.20f);       // 20% - High
    SystemBudgets.Add(TEXT("TerrainPhysics"), TotalBudget * 0.15f);        // 15% - High
    SystemBudgets.Add(TEXT("RagdollSystem"), TotalBudget * 0.10f);         // 10% - Medium
    SystemBudgets.Add(TEXT("DestructionSystem"), TotalBudget * 0.05f);     // 5% - Medium
    SystemBudgets.Add(TEXT("VehiclePhysics"), TotalBudget * 0.03f);        // 3% - Low
    SystemBudgets.Add(TEXT("FluidDynamics"), TotalBudget * 0.02f);         // 2% - Low
    
    // Analyze each system
    for (const auto& SystemBudget : SystemBudgets)
    {
        FPerf_PhysicsOptimizationDirective Directive;
        Directive.SystemName = SystemBudget.Key;
        Directive.TargetBudget = SystemBudget.Value;
        
        // Estimate current usage (would come from actual metrics in real implementation)
        float EstimatedUsage = CurrentMetrics.PhysicsFrameTime * (SystemBudget.Value / TotalBudget);
        Directive.CurrentUsage = EstimatedUsage;
        
        // Check if optimization is required
        float UsagePercent = (EstimatedUsage / SystemBudget.Value) * 100.0f;
        Directive.bRequiresOptimization = UsagePercent > 80.0f;
        
        // Generate optimization suggestions
        if (Directive.bRequiresOptimization)
        {
            Directive.OptimizationSuggestions = GenerateOptimizationSuggestions(
                SystemBudget.Key, EstimatedUsage, SystemBudget.Value);
        }
        
        Directives.Add(Directive);
        
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: System %s - Target: %.2fms, Current: %.2fms, Optimization Required: %s"),
               *SystemBudget.Key, SystemBudget.Value, EstimatedUsage, 
               Directive.bRequiresOptimization ? TEXT("YES") : TEXT("NO"));
    }
    
    return Directives;
}

void UPerf_PhysicsRegistryIntegrator::TriggerEmergencyOptimization()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: EMERGENCY OPTIMIZATION TRIGGERED"));
    
    if (!bConnectedToRegistry || !PhysicsRegistryBridge)
    {
        UE_LOG(LogTemp, Error, TEXT("Performance Optimizer: Cannot trigger emergency optimization - not connected to Physics Registry Bridge"));
        return;
    }
    
    // Emergency optimization strategies
    TArray<FString> EmergencyActions;
    EmergencyActions.Add(TEXT("Reduce physics simulation frequency to 30Hz"));
    EmergencyActions.Add(TEXT("Disable non-critical physics systems temporarily"));
    EmergencyActions.Add(TEXT("Reduce collision detection precision"));
    EmergencyActions.Add(TEXT("Pause destruction system updates"));
    EmergencyActions.Add(TEXT("Limit active ragdoll count to 5"));
    
    // Send emergency directives to all physics systems
    for (const FString& Action : EmergencyActions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Emergency Action - %s"), *Action);
        
        // Send emergency directive through Physics Registry Bridge
        // PhysicsRegistryBridge->BroadcastEmergencyOptimization(Action);
    }
    
    // Temporarily switch to Console performance profile for reduced budget
    if (CurrentProfile != EPerf_PerformanceProfile::Console)
    {
        EPerf_PerformanceProfile PreviousProfile = CurrentProfile;
        SetPerformanceProfile(EPerf_PerformanceProfile::Console);
        
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Switched from %s to Console profile for emergency optimization"),
               *UEnum::GetValueAsString(PreviousProfile));
    }
}

bool UPerf_PhysicsRegistryIntegrator::ArePhysicsSystemsWithinBudget()
{
    FPerf_PhysicsRegistryMetrics CurrentMetrics = GetPhysicsRegistryMetrics();
    return CurrentMetrics.bWithinBudget;
}

float UPerf_PhysicsRegistryIntegrator::GetCurrentPerformanceBudget() const
{
    if (const float* Budget = PerformanceBudgets.Find(CurrentProfile))
    {
        return *Budget;
    }
    
    // Default to Development profile budget
    return 8.0f;
}

void UPerf_PhysicsRegistryIntegrator::InitializePerformanceBudgets()
{
    PerformanceBudgets.Empty();
    
    // Set performance budgets for each profile (in milliseconds)
    PerformanceBudgets.Add(EPerf_PerformanceProfile::Development, 8.0f);   // Generous budget for development
    PerformanceBudgets.Add(EPerf_PerformanceProfile::Console, 4.0f);       // Strict budget for console 30fps
    PerformanceBudgets.Add(EPerf_PerformanceProfile::PC_High, 6.0f);       // Balanced budget for high-end PC
    PerformanceBudgets.Add(EPerf_PerformanceProfile::PC_Ultra, 10.0f);     // High budget for ultra-high-end PC
    PerformanceBudgets.Add(EPerf_PerformanceProfile::Mobile, 2.0f);        // Very strict budget for mobile
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Performance budgets initialized"));
    for (const auto& Budget : PerformanceBudgets)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %.1fms"), 
               *UEnum::GetValueAsString(Budget.Key), Budget.Value);
    }
}

void UPerf_PhysicsRegistryIntegrator::UpdatePerformanceMetrics()
{
    if (!bConnectedToRegistry || !PhysicsRegistryBridge)
    {
        return;
    }
    
    // Get fresh metrics from Physics Registry Bridge
    // Note: This would call actual bridge methods when available
    // For now, simulate realistic metrics
    LastMetrics.PhysicsFrameTime = FMath::RandRange(1.5f, 3.5f);
    LastMetrics.ActivePhysicsBodies = FMath::RandRange(100, 200);
    LastMetrics.CollisionChecksPerFrame = FMath::RandRange(500, 1000);
    LastMetrics.PerformanceBudget = GetCurrentPerformanceBudget();
    LastMetrics.BudgetUsagePercent = (LastMetrics.PhysicsFrameTime / LastMetrics.PerformanceBudget) * 100.0f;
    LastMetrics.bWithinBudget = LastMetrics.BudgetUsagePercent <= 100.0f;
    LastMetrics.RegisteredSystemsCount = 8; // All Core physics systems
    
    // Check for emergency optimization threshold
    if (LastMetrics.BudgetUsagePercent >= EmergencyOptimizationThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Performance budget usage at %.1f%% - approaching emergency threshold (%.1f%%)"),
               LastMetrics.BudgetUsagePercent, EmergencyOptimizationThreshold);
        
        if (LastMetrics.BudgetUsagePercent >= 100.0f)
        {
            TriggerEmergencyOptimization();
        }
    }
}

TArray<FString> UPerf_PhysicsRegistryIntegrator::GenerateOptimizationSuggestions(const FString& SystemName, float CurrentUsage, float TargetBudget)
{
    TArray<FString> Suggestions;
    
    float OveragePercent = ((CurrentUsage - TargetBudget) / TargetBudget) * 100.0f;
    
    if (SystemName == TEXT("PhysicsManager"))
    {
        Suggestions.Add(TEXT("Reduce physics simulation frequency from 60Hz to 45Hz"));
        Suggestions.Add(TEXT("Enable physics sub-stepping for better performance"));
        Suggestions.Add(TEXT("Optimize physics world bounds"));
    }
    else if (SystemName == TEXT("CollisionSystem"))
    {
        Suggestions.Add(TEXT("Reduce collision detection precision"));
        Suggestions.Add(TEXT("Enable spatial partitioning optimization"));
        Suggestions.Add(TEXT("Limit collision queries per frame"));
    }
    else if (SystemName == TEXT("TerrainPhysics"))
    {
        Suggestions.Add(TEXT("Reduce terrain collision mesh resolution"));
        Suggestions.Add(TEXT("Enable terrain LOD for distant areas"));
        Suggestions.Add(TEXT("Cache terrain collision data"));
    }
    else if (SystemName == TEXT("RagdollSystem"))
    {
        Suggestions.Add(TEXT("Limit active ragdoll count"));
        Suggestions.Add(TEXT("Reduce ragdoll simulation quality"));
        Suggestions.Add(TEXT("Enable ragdoll LOD system"));
    }
    else if (SystemName == TEXT("DestructionSystem"))
    {
        Suggestions.Add(TEXT("Reduce destruction fragment count"));
        Suggestions.Add(TEXT("Limit concurrent destruction events"));
        Suggestions.Add(TEXT("Enable destruction LOD"));
    }
    else
    {
        Suggestions.Add(FString::Printf(TEXT("Reduce %s update frequency"), *SystemName));
        Suggestions.Add(FString::Printf(TEXT("Enable %s LOD system"), *SystemName));
        Suggestions.Add(FString::Printf(TEXT("Optimize %s algorithms"), *SystemName));
    }
    
    // Add severity-based suggestions
    if (OveragePercent > 50.0f)
    {
        Suggestions.Add(TEXT("CRITICAL: Consider temporarily disabling system"));
    }
    else if (OveragePercent > 25.0f)
    {
        Suggestions.Add(TEXT("HIGH: Implement aggressive optimization"));
    }
    
    return Suggestions;
}