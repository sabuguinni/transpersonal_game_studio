#include "DirectorSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UDirectorSystemManager::UDirectorSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 times per second
    
    // Initialize default values
    CurrentActiveAgent = 1; // Start with Studio Director (agent #1)
    ChainProgress = 0.0f;
    TargetFPS = 60; // Default to PC target
    CurrentFrameTime = 0.0f;
    PerformanceWarningThreshold = 20.0f; // 20ms = 50fps warning
    bSystemsInitialized = false;
    LastPerformanceCheck = 0.0f;
    
    // Initialize agent chain
    InitializeAgentChain();
}

void UDirectorSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Beginning play - initializing system chain"));
    
    // Initialize systems after a short delay to ensure all components are ready
    FTimerHandle InitTimer;
    GetWorld()->GetTimerManager().SetTimer(InitTimer, this, &UDirectorSystemManager::InitializeSystemChain, 1.0f, false);
}

void UDirectorSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Check system health every 5 seconds
    if (GetWorld()->GetTimeSeconds() - LastPerformanceCheck > 5.0f)
    {
        LastPerformanceCheck = GetWorld()->GetTimeSeconds();
        LogSystemStatus();
    }
}

void UDirectorSystemManager::InitializeSystemChain()
{
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Initializing 18-agent system chain"));
    
    if (bSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Systems already initialized"));
        return;
    }
    
    // Validate dependencies first
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("DirectorSystemManager: System dependency validation failed"));
        return;
    }
    
    // Initialize in dependency order
    // Engine Architect → Core Systems → World Generator → Environment Artist → etc.
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Agent Chain initialized with %d agents"), AgentChainNames.Num());
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Current active agent: %s"), *GetCurrentAgentName());
    
    bSystemsInitialized = true;
    
    // Register this manager as a system component
    RegisterSystemComponent(this, TEXT("DirectorSystemManager"));
}

FDir_SystemHealthReport UDirectorSystemManager::GetSystemHealthReport()
{
    FDir_SystemHealthReport Report;
    Report.bSystemsHealthy = bSystemsInitialized;
    Report.CurrentFPS = 1000.0f / FMath::Max(CurrentFrameTime, 0.001f);
    Report.ActiveAgent = GetCurrentAgentName();
    Report.ChainProgress = ChainProgress;
    Report.RegisteredSystemsCount = RegisteredSystems.Num();
    
    // Check performance health
    Report.bPerformanceHealthy = (CurrentFrameTime <= PerformanceWarningThreshold);
    
    // Generate health message
    if (Report.bSystemsHealthy && Report.bPerformanceHealthy)
    {
        Report.HealthMessage = FString::Printf(TEXT("All systems operational. FPS: %.1f, Agent: %s"), 
            Report.CurrentFPS, *Report.ActiveAgent);
    }
    else
    {
        Report.HealthMessage = FString::Printf(TEXT("ISSUES DETECTED - FPS: %.1f, Systems: %s"), 
            Report.CurrentFPS, Report.bSystemsHealthy ? TEXT("OK") : TEXT("ERROR"));
    }
    
    return Report;
}

void UDirectorSystemManager::EmergencyShutdown(const FString& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("DirectorSystemManager: EMERGENCY SHUTDOWN - %s"), *Reason);
    
    // Disable all registered systems
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value && IsValid(SystemPair.Value))
        {
            SystemPair.Value->SetComponentTickEnabled(false);
        }
    }
    
    // Reset state
    bSystemsInitialized = false;
    CurrentActiveAgent = 1;
    ChainProgress = 0.0f;
    
    // Log emergency shutdown
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, 
            FString::Printf(TEXT("EMERGENCY SHUTDOWN: %s"), *Reason));
    }
}

bool UDirectorSystemManager::AdvanceToNextAgent()
{
    if (CurrentActiveAgent >= AgentChainNames.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Chain complete - resetting to agent #1"));
        CurrentActiveAgent = 1;
        ChainProgress = 0.0f;
        return false; // Chain completed
    }
    
    CurrentActiveAgent++;
    ChainProgress = static_cast<float>(CurrentActiveAgent - 1) / static_cast<float>(AgentChainNames.Num());
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Advanced to agent #%d: %s (Progress: %.1f%%)"), 
        CurrentActiveAgent, *GetCurrentAgentName(), ChainProgress * 100.0f);
    
    return true;
}

FString UDirectorSystemManager::GetCurrentAgentName()
{
    if (AgentChainNames.IsValidIndex(CurrentActiveAgent - 1))
    {
        return AgentChainNames[CurrentActiveAgent - 1];
    }
    return TEXT("Unknown Agent");
}

void UDirectorSystemManager::RegisterSystemComponent(UActorComponent* Component, const FString& SystemName)
{
    if (!Component || SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("DirectorSystemManager: Invalid component or system name for registration"));
        return;
    }
    
    RegisteredSystems.Add(SystemName, Component);
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Registered system '%s' (%d total systems)"), 
        *SystemName, RegisteredSystems.Num());
}

void UDirectorSystemManager::UnregisterSystemComponent(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager: Unregistered system '%s'"), *SystemName);
    }
}

UActorComponent* UDirectorSystemManager::GetSystemComponent(const FString& SystemName)
{
    if (UActorComponent** FoundComponent = RegisteredSystems.Find(SystemName))
    {
        return *FoundComponent;
    }
    return nullptr;
}

void UDirectorSystemManager::InitializeAgentChain()
{
    AgentChainNames.Empty();
    AgentChainNames.Add(TEXT("Studio Director"));
    AgentChainNames.Add(TEXT("Engine Architect"));
    AgentChainNames.Add(TEXT("Core Systems Programmer"));
    AgentChainNames.Add(TEXT("Performance Optimizer"));
    AgentChainNames.Add(TEXT("Procedural World Generator"));
    AgentChainNames.Add(TEXT("Environment Artist"));
    AgentChainNames.Add(TEXT("Architecture & Interior Agent"));
    AgentChainNames.Add(TEXT("Lighting & Atmosphere Agent"));
    AgentChainNames.Add(TEXT("Character Artist Agent"));
    AgentChainNames.Add(TEXT("Animation Agent"));
    AgentChainNames.Add(TEXT("NPC Behavior Agent"));
    AgentChainNames.Add(TEXT("Combat & Enemy AI Agent"));
    AgentChainNames.Add(TEXT("Crowd & Traffic Simulation"));
    AgentChainNames.Add(TEXT("Quest & Mission Designer"));
    AgentChainNames.Add(TEXT("Narrative & Dialogue Agent"));
    AgentChainNames.Add(TEXT("Audio Agent"));
    AgentChainNames.Add(TEXT("VFX Agent"));
    AgentChainNames.Add(TEXT("QA & Testing Agent"));
    AgentChainNames.Add(TEXT("Integration & Build Agent"));
}

void UDirectorSystemManager::UpdatePerformanceMetrics()
{
    // Get current frame time
    CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Adjust target FPS based on platform
    #if PLATFORM_DESKTOP
        TargetFPS = 60;
        PerformanceWarningThreshold = 20.0f; // 50fps warning
    #else
        TargetFPS = 30;
        PerformanceWarningThreshold = 40.0f; // 25fps warning
    #endif
}

bool UDirectorSystemManager::ValidateSystemDependencies()
{
    // Check if we have a valid world
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("DirectorSystemManager: No valid world found"));
        return false;
    }
    
    // Check if we have a valid owner
    if (!GetOwner())
    {
        UE_LOG(LogTemp, Error, TEXT("DirectorSystemManager: No valid owner actor"));
        return false;
    }
    
    // Additional dependency checks can be added here
    return true;
}

void UDirectorSystemManager::LogSystemStatus()
{
    FDir_SystemHealthReport Report = GetSystemHealthReport();
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemManager Status: %s"), *Report.HealthMessage);
    
    if (GEngine && !Report.bPerformanceHealthy)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Performance Warning: %.1fms frame time"), CurrentFrameTime));
    }
}

// === ADirectorSystemActor Implementation ===

ADirectorSystemActor::ADirectorSystemActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create the director system manager component
    DirectorSystemManager = CreateDefaultSubobject<UDirectorSystemManager>(TEXT("DirectorSystemManager"));
}

void ADirectorSystemActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("DirectorSystemActor: Actor spawned and ready"));
    
    // The DirectorSystemManager will initialize itself in its BeginPlay
}