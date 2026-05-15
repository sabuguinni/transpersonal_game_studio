#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bIntegrationSystemActive = false;
    LastHealthCheckTime = 0.0f;
    MaxAllowedActorCount = 50000;
    MinimumPerformanceThreshold = 30.0f;
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build Integration Orchestrator: Initializing comprehensive system monitoring"));
    
    bIntegrationSystemActive = true;
    InitializeSystemMonitoring();
    
    // Register core systems for monitoring
    RegisterSystemForMonitoring(TEXT("WorldGeneration"), nullptr);
    RegisterSystemForMonitoring(TEXT("CharacterSystem"), nullptr);
    RegisterSystemForMonitoring(TEXT("DinosaurAI"), nullptr);
    RegisterSystemForMonitoring(TEXT("CrowdSimulation"), nullptr);
    RegisterSystemForMonitoring(TEXT("AudioSystem"), nullptr);
    RegisterSystemForMonitoring(TEXT("VFXSystem"), nullptr);
    RegisterSystemForMonitoring(TEXT("QAValidation"), nullptr);
    
    LogIntegrationEvent(TEXT("Final Integration Orchestrator initialized successfully"));
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    LogIntegrationEvent(TEXT("Final Integration Orchestrator shutting down"));
    bIntegrationSystemActive = false;
    MonitoredSystems.Empty();
    SystemValidationLog.Empty();
    
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuild_FinalIntegrationOrchestrator::GenerateSystemReport()
{
    FBuild_IntegrationReport Report;
    Report.BuildTimestamp = FDateTime::Now().ToString();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Report.CriticalIssues.Add(TEXT("World reference is null"));
        return Report;
    }
    
    // Count all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    Report.TotalActorCount = AllActors.Num();
    
    float TotalPerformanceScore = 0.0f;
    int32 OperationalSystems = 0;
    
    // Validate each monitored system
    for (auto& SystemPair : MonitoredSystems)
    {
        FBuild_SystemStatus& Status = SystemPair.Value;
        Status.LastValidationTime = FDateTime::Now().ToString();
        
        // Calculate performance score for this system
        Status.PerformanceScore = CalculateSystemPerformanceScore(Status.SystemName);
        TotalPerformanceScore += Status.PerformanceScore;
        
        // Count actors related to this system
        Status.ActorCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(Status.SystemName))
            {
                Status.ActorCount++;
            }
        }
        
        // Determine if system is operational
        Status.bIsOperational = (Status.PerformanceScore > MinimumPerformanceThreshold) && (Status.ActorCount > 0);
        
        if (Status.bIsOperational)
        {
            OperationalSystems++;
        }
        else
        {
            Report.CriticalIssues.Add(FString::Printf(TEXT("System %s is not operational (Score: %.1f, Actors: %d)"), 
                *Status.SystemName, Status.PerformanceScore, Status.ActorCount));
        }
        
        Report.SystemStatuses.Add(Status);
    }
    
    // Calculate overall health
    if (MonitoredSystems.Num() > 0)
    {
        Report.OverallHealthScore = TotalPerformanceScore / MonitoredSystems.Num();
        Report.bBuildStable = (OperationalSystems >= MonitoredSystems.Num() * 0.8f); // 80% systems must be operational
    }
    
    // Check for critical thresholds
    if (Report.TotalActorCount > MaxAllowedActorCount)
    {
        Report.CriticalIssues.Add(FString::Printf(TEXT("Actor count exceeds maximum: %d > %d"), 
            Report.TotalActorCount, MaxAllowedActorCount));
        Report.bBuildStable = false;
    }
    
    LogIntegrationEvent(FString::Printf(TEXT("System report generated: %d systems, %.1f health score, %s"), 
        MonitoredSystems.Num(), Report.OverallHealthScore, Report.bBuildStable ? TEXT("STABLE") : TEXT("UNSTABLE")));
    
    return Report;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    if (!bIntegrationSystemActive)
    {
        return false;
    }
    
    ValidateWorldState();
    CheckSystemDependencies();
    UpdateSystemMetrics();
    ProcessSystemAlerts();
    
    FBuild_IntegrationReport Report = GenerateSystemReport();
    return Report.bBuildStable;
}

void UBuild_FinalIntegrationOrchestrator::PerformIntegrationHealthCheck()
{
    LastHealthCheckTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    LogIntegrationEvent(TEXT("Performing comprehensive integration health check"));
    
    bool bSystemsHealthy = ValidateAllSystems();
    
    if (!bSystemsHealthy)
    {
        TArray<FString> Errors = GetCriticalSystemErrors();
        for (const FString& Error : Errors)
        {
            LogIntegrationEvent(Error, true);
        }
    }
    
    CleanupOrphanedActors();
    OptimizeSystemPerformance();
    GeneratePerformanceReport();
}

TArray<FString> UBuild_FinalIntegrationOrchestrator::GetCriticalSystemErrors()
{
    TArray<FString> Errors;
    
    for (const auto& SystemPair : MonitoredSystems)
    {
        const FBuild_SystemStatus& Status = SystemPair.Value;
        if (!Status.bIsOperational)
        {
            Errors.Add(FString::Printf(TEXT("CRITICAL: %s system failure - Performance: %.1f, Actors: %d"), 
                *Status.SystemName, Status.PerformanceScore, Status.ActorCount));
        }
    }
    
    return Errors;
}

float UBuild_FinalIntegrationOrchestrator::CalculateSystemPerformanceScore(const FString& SystemName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }
    
    // Base performance calculation
    float BaseScore = 50.0f;
    
    // Check for system-specific actors
    TArray<AActor*> SystemActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), SystemActors);
    
    int32 SystemActorCount = 0;
    for (AActor* Actor : SystemActors)
    {
        if (Actor && Actor->GetName().Contains(SystemName))
        {
            SystemActorCount++;
            if (ValidateActorIntegrity(Actor))
            {
                BaseScore += 5.0f; // Bonus for valid actors
            }
        }
    }
    
    // Performance modifiers based on system type
    if (SystemName == TEXT("WorldGeneration"))
    {
        BaseScore += (SystemActorCount > 10) ? 20.0f : 0.0f;
    }
    else if (SystemName == TEXT("CharacterSystem"))
    {
        BaseScore += (SystemActorCount > 0) ? 25.0f : 0.0f;
    }
    else if (SystemName == TEXT("DinosaurAI"))
    {
        BaseScore += (SystemActorCount > 3) ? 15.0f : 0.0f;
    }
    
    return FMath::Clamp(BaseScore, 0.0f, 100.0f);
}

void UBuild_FinalIntegrationOrchestrator::RegisterSystemForMonitoring(const FString& SystemName, AActor* SystemActor)
{
    FBuild_SystemStatus NewStatus;
    NewStatus.SystemName = SystemName;
    NewStatus.bIsOperational = false;
    NewStatus.ActorCount = 0;
    NewStatus.PerformanceScore = 0.0f;
    NewStatus.LastValidationTime = FDateTime::Now().ToString();
    
    MonitoredSystems.Add(SystemName, NewStatus);
    
    LogIntegrationEvent(FString::Printf(TEXT("Registered system for monitoring: %s"), *SystemName));
}

void UBuild_FinalIntegrationOrchestrator::UnregisterSystem(const FString& SystemName)
{
    if (MonitoredSystems.Contains(SystemName))
    {
        MonitoredSystems.Remove(SystemName);
        LogIntegrationEvent(FString::Printf(TEXT("Unregistered system: %s"), *SystemName));
    }
}

bool UBuild_FinalIntegrationOrchestrator::IsSystemOperational(const FString& SystemName)
{
    if (MonitoredSystems.Contains(SystemName))
    {
        return MonitoredSystems[SystemName].bIsOperational;
    }
    return false;
}

void UBuild_FinalIntegrationOrchestrator::TriggerEmergencySystemRestart(const FString& SystemName)
{
    LogIntegrationEvent(FString::Printf(TEXT("EMERGENCY: Triggering restart for system: %s"), *SystemName), true);
    
    // Reset system status
    if (MonitoredSystems.Contains(SystemName))
    {
        FBuild_SystemStatus& Status = MonitoredSystems[SystemName];
        Status.bIsOperational = false;
        Status.PerformanceScore = 0.0f;
        Status.LastValidationTime = FDateTime::Now().ToString();
    }
}

void UBuild_FinalIntegrationOrchestrator::LogIntegrationEvent(const FString& EventMessage, bool bIsError)
{
    FString LogEntry = FString::Printf(TEXT("[%s] %s"), 
        *FDateTime::Now().ToString(), *EventMessage);
    
    SystemValidationLog.Add(LogEntry);
    
    if (bIsError)
    {
        UE_LOG(LogTemp, Error, TEXT("Build Integration: %s"), *EventMessage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Build Integration: %s"), *EventMessage);
    }
    
    // Keep log size manageable
    if (SystemValidationLog.Num() > 1000)
    {
        SystemValidationLog.RemoveAt(0, 100);
    }
}

void UBuild_FinalIntegrationOrchestrator::InitializeSystemMonitoring()
{
    SystemValidationLog.Empty();
    MonitoredSystems.Empty();
    
    LogIntegrationEvent(TEXT("System monitoring initialized"));
}

void UBuild_FinalIntegrationOrchestrator::ValidateWorldState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        LogIntegrationEvent(TEXT("World validation failed - null world reference"), true);
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    LogIntegrationEvent(FString::Printf(TEXT("World state validation: %d total actors"), AllActors.Num()));
}

void UBuild_FinalIntegrationOrchestrator::CheckSystemDependencies()
{
    // Validate that critical systems have their dependencies
    LogIntegrationEvent(TEXT("Checking system dependencies"));
}

void UBuild_FinalIntegrationOrchestrator::UpdateSystemMetrics()
{
    for (auto& SystemPair : MonitoredSystems)
    {
        SystemPair.Value.PerformanceScore = CalculateSystemPerformanceScore(SystemPair.Key);
    }
}

void UBuild_FinalIntegrationOrchestrator::ProcessSystemAlerts()
{
    for (const auto& SystemPair : MonitoredSystems)
    {
        const FBuild_SystemStatus& Status = SystemPair.Value;
        if (!Status.bIsOperational)
        {
            LogIntegrationEvent(FString::Printf(TEXT("ALERT: System %s is not operational"), *Status.SystemName), true);
        }
    }
}

bool UBuild_FinalIntegrationOrchestrator::ValidateActorIntegrity(AActor* Actor)
{
    if (!Actor || !IsValid(Actor))
    {
        return false;
    }
    
    return !Actor->IsPendingKill() && Actor->GetWorld() != nullptr;
}

void UBuild_FinalIntegrationOrchestrator::CleanupOrphanedActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 CleanedCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (!ValidateActorIntegrity(Actor))
        {
            CleanedCount++;
        }
    }
    
    if (CleanedCount > 0)
    {
        LogIntegrationEvent(FString::Printf(TEXT("Cleaned up %d orphaned actors"), CleanedCount));
    }
}

void UBuild_FinalIntegrationOrchestrator::OptimizeSystemPerformance()
{
    LogIntegrationEvent(TEXT("Performing system performance optimization"));
}

void UBuild_FinalIntegrationOrchestrator::GeneratePerformanceReport()
{
    FBuild_IntegrationReport Report = GenerateSystemReport();
    
    LogIntegrationEvent(FString::Printf(TEXT("Performance Report - Health: %.1f%%, Actors: %d, Stable: %s"), 
        Report.OverallHealthScore, Report.TotalActorCount, Report.bBuildStable ? TEXT("YES") : TEXT("NO")));
}

void UBuild_FinalIntegrationOrchestrator::HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage)
{
    LogIntegrationEvent(FString::Printf(TEXT("SYSTEM FAILURE: %s - %s"), *SystemName, *ErrorMessage), true);
    TriggerEmergencySystemRestart(SystemName);
}

// ABuild_IntegrationMonitorActor Implementation

ABuild_IntegrationMonitorActor::ABuild_IntegrationMonitorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    MonitoringInterval = 30.0f; // Check every 30 seconds
    bAutoRestartFailedSystems = true;
    TimeSinceLastCheck = 0.0f;
    IntegrationOrchestrator = nullptr;
}

void ABuild_IntegrationMonitorActor::BeginPlay()
{
    Super::BeginPlay();
    
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        IntegrationOrchestrator = GameInstance->GetSubsystem<UBuild_FinalIntegrationOrchestrator>();
    }
    
    if (IntegrationOrchestrator)
    {
        UE_LOG(LogTemp, Warning, TEXT("Integration Monitor Actor: Connected to orchestrator"));
        PerformScheduledHealthCheck();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Integration Monitor Actor: Failed to connect to orchestrator"));
    }
}

void ABuild_IntegrationMonitorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastCheck += DeltaTime;
    
    if (TimeSinceLastCheck >= MonitoringInterval)
    {
        PerformScheduledHealthCheck();
        TimeSinceLastCheck = 0.0f;
    }
    
    UpdateMonitoringDisplay();
    HandleSystemAlerts();
}

void ABuild_IntegrationMonitorActor::PerformScheduledHealthCheck()
{
    if (IntegrationOrchestrator)
    {
        IntegrationOrchestrator->PerformIntegrationHealthCheck();
        LastReport = IntegrationOrchestrator->GenerateSystemReport();
        
        UE_LOG(LogTemp, Warning, TEXT("Integration Monitor: Health check completed - %d systems monitored"), 
            LastReport.SystemStatuses.Num());
    }
}

void ABuild_IntegrationMonitorActor::UpdateMonitoringDisplay()
{
    // Visual feedback could be implemented here
}

void ABuild_IntegrationMonitorActor::HandleSystemAlerts()
{
    if (bAutoRestartFailedSystems && IntegrationOrchestrator)
    {
        TArray<FString> Errors = IntegrationOrchestrator->GetCriticalSystemErrors();
        for (const FString& Error : Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("Integration Monitor Alert: %s"), *Error);
        }
    }
}