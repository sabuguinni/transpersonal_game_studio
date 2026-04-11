#include "TranspersonalGame.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY(LogTranspersonalGame);

IMPLEMENT_PRIMARY_GAME_MODULE(FTranspersonalGameModule, TranspersonalGame, "TranspersonalGame");

void FTranspersonalGameModule::StartupModule()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGame Module Started"));
    
    // Initialize core systems
    InitializeCoreSystemsRegistry();
    InitializePerformanceMonitoring();
    InitializeIntegrationSystems();
    
    UE_LOG(LogTranspersonalGame, Warning, TEXT("All TranspersonalGame systems initialized successfully"));
}

void FTranspersonalGameModule::ShutdownModule()
{
    UE_LOG(LogTranspersonalGame, Warning, TEXT("TranspersonalGame Module Shutdown"));
    
    // Cleanup integration systems
    ShutdownIntegrationSystems();
    ShutdownPerformanceMonitoring();
    ShutdownCoreSystemsRegistry();
}

void FTranspersonalGameModule::InitializeCoreSystemsRegistry()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Initializing Core Systems Registry..."));
    
    // Register all core systems for integration
    CoreSystemsRegistry.Empty();
    
    // Physics and destruction systems
    CoreSystemsRegistry.Add(TEXT("PhysicsCore"), true);
    CoreSystemsRegistry.Add(TEXT("DestructionSystem"), true);
    CoreSystemsRegistry.Add(TEXT("RagdollSystem"), true);
    
    // World generation systems
    CoreSystemsRegistry.Add(TEXT("ProceduralWorldGen"), true);
    CoreSystemsRegistry.Add(TEXT("BiomeSystem"), true);
    CoreSystemsRegistry.Add(TEXT("TerrainSystem"), true);
    
    // Character and animation systems
    CoreSystemsRegistry.Add(TEXT("CharacterSystem"), true);
    CoreSystemsRegistry.Add(TEXT("AnimationSystem"), true);
    CoreSystemsRegistry.Add(TEXT("MotionMatching"), true);
    
    // AI and behavior systems
    CoreSystemsRegistry.Add(TEXT("NPCBehaviorSystem"), true);
    CoreSystemsRegistry.Add(TEXT("CombatAI"), true);
    CoreSystemsRegistry.Add(TEXT("CrowdSimulation"), true);
    
    // Audio and VFX systems
    CoreSystemsRegistry.Add(TEXT("AudioSystem"), true);
    CoreSystemsRegistry.Add(TEXT("VFXSystem"), true);
    CoreSystemsRegistry.Add(TEXT("LightingSystem"), true);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Core Systems Registry initialized with %d systems"), CoreSystemsRegistry.Num());
}

void FTranspersonalGameModule::InitializePerformanceMonitoring()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Initializing Performance Monitoring..."));
    
    // Initialize performance tracking
    PerformanceMetrics.Reset();
    PerformanceMetrics.Add(TEXT("FrameTime"), 0.0f);
    PerformanceMetrics.Add(TEXT("DrawCalls"), 0.0f);
    PerformanceMetrics.Add(TEXT("MemoryUsage"), 0.0f);
    PerformanceMetrics.Add(TEXT("CPUUsage"), 0.0f);
    PerformanceMetrics.Add(TEXT("GPUUsage"), 0.0f);
    
    // Set performance targets
    PerformanceTargets.Add(TEXT("TargetFPS_PC"), 60.0f);
    PerformanceTargets.Add(TEXT("TargetFPS_Console"), 30.0f);
    PerformanceTargets.Add(TEXT("MaxMemoryMB"), 8192.0f);
    PerformanceTargets.Add(TEXT("MaxDrawCalls"), 2000.0f);
    
    bPerformanceMonitoringEnabled = true;
    UE_LOG(LogTranspersonalGame, Log, TEXT("Performance Monitoring initialized"));
}

void FTranspersonalGameModule::InitializeIntegrationSystems()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Initializing Integration Systems..."));
    
    // Initialize build integration
    BuildIntegrationStatus.Empty();
    BuildIntegrationStatus.Add(TEXT("LastBuildTime"), FDateTime::Now().ToString());
    BuildIntegrationStatus.Add(TEXT("BuildStatus"), TEXT("Initializing"));
    BuildIntegrationStatus.Add(TEXT("LastSuccessfulBuild"), TEXT("None"));
    
    // Initialize agent coordination
    AgentCoordinationStatus.Empty();
    for (int32 i = 1; i <= 18; ++i)
    {
        FString AgentKey = FString::Printf(TEXT("Agent_%02d"), i);
        AgentCoordinationStatus.Add(AgentKey, TEXT("Pending"));
    }
    
    bIntegrationSystemsActive = true;
    UE_LOG(LogTranspersonalGame, Log, TEXT("Integration Systems initialized"));
}

void FTranspersonalGameModule::ShutdownCoreSystemsRegistry()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Shutting down Core Systems Registry..."));
    CoreSystemsRegistry.Empty();
}

void FTranspersonalGameModule::ShutdownPerformanceMonitoring()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Shutting down Performance Monitoring..."));
    bPerformanceMonitoringEnabled = false;
    PerformanceMetrics.Empty();
    PerformanceTargets.Empty();
}

void FTranspersonalGameModule::ShutdownIntegrationSystems()
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Shutting down Integration Systems..."));
    bIntegrationSystemsActive = false;
    BuildIntegrationStatus.Empty();
    AgentCoordinationStatus.Empty();
}

bool FTranspersonalGameModule::IsSystemRegistered(const FString& SystemName) const
{
    return CoreSystemsRegistry.Contains(SystemName) && CoreSystemsRegistry[SystemName];
}

void FTranspersonalGameModule::RegisterSystem(const FString& SystemName, bool bIsActive)
{
    CoreSystemsRegistry.Add(SystemName, bIsActive);
    UE_LOG(LogTranspersonalGame, Log, TEXT("System registered: %s (Active: %s)"), *SystemName, bIsActive ? TEXT("Yes") : TEXT("No"));
}

void FTranspersonalGameModule::UpdatePerformanceMetric(const FString& MetricName, float Value)
{
    if (bPerformanceMonitoringEnabled)
    {
        PerformanceMetrics.Add(MetricName, Value);
    }
}

float FTranspersonalGameModule::GetPerformanceMetric(const FString& MetricName) const
{
    if (PerformanceMetrics.Contains(MetricName))
    {
        return PerformanceMetrics[MetricName];
    }
    return 0.0f;
}

void FTranspersonalGameModule::UpdateBuildStatus(const FString& Status)
{
    BuildIntegrationStatus.Add(TEXT("BuildStatus"), Status);
    BuildIntegrationStatus.Add(TEXT("LastUpdateTime"), FDateTime::Now().ToString());
    UE_LOG(LogTranspersonalGame, Warning, TEXT("Build Status Updated: %s"), *Status);
}

void FTranspersonalGameModule::UpdateAgentStatus(int32 AgentNumber, const FString& Status)
{
    FString AgentKey = FString::Printf(TEXT("Agent_%02d"), AgentNumber);
    AgentCoordinationStatus.Add(AgentKey, Status);
    UE_LOG(LogTranspersonalGame, Log, TEXT("Agent %d Status: %s"), AgentNumber, *Status);
}