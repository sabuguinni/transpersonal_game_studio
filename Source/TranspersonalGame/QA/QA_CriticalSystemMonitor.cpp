#include "QA_CriticalSystemMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_CriticalSystemMonitor::UQA_CriticalSystemMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Monitor every second
    
    // Initialize monitoring thresholds
    MaxAllowedMemoryMB = 2048.0f;
    MinRequiredFPS = 30.0f;
    MaxCPUUsagePercent = 80.0f;
    
    // Initialize monitoring state
    bIsMonitoring = false;
    MonitoringStartTime = 0.0f;
    LastCriticalIncidentTime = 0.0f;
    
    // Initialize performance metrics
    CurrentFPS = 0.0f;
    CurrentMemoryUsageMB = 0.0f;
    CurrentCPUUsagePercent = 0.0f;
    
    CriticalIncidents.Empty();
    PerformanceHistory.Empty();
}

void UQA_CriticalSystemMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    StartMonitoring();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: Monitoring started"));
}

void UQA_CriticalSystemMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMonitoring();
    
    // Generate final monitoring report
    GenerateMonitoringReport();
    
    Super::EndPlay(EndPlayReason);
}

void UQA_CriticalSystemMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        MonitorCriticalSystems();
        UpdatePerformanceMetrics();
        CheckForCriticalIncidents();
    }
}

void UQA_CriticalSystemMonitor::StartMonitoring()
{
    if (!bIsMonitoring)
    {
        bIsMonitoring = true;
        MonitoringStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        // Clear previous data
        CriticalIncidents.Empty();
        PerformanceHistory.Empty();
        
        UE_LOG(LogTemp, Log, TEXT("Critical system monitoring started"));
    }
}

void UQA_CriticalSystemMonitor::StopMonitoring()
{
    if (bIsMonitoring)
    {
        bIsMonitoring = false;
        
        UE_LOG(LogTemp, Log, TEXT("Critical system monitoring stopped"));
    }
}

void UQA_CriticalSystemMonitor::MonitorCriticalSystems()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Monitor Game Mode
    AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
    if (!GameMode)
    {
        ReportCriticalIncident(TEXT("GameMode"), TEXT("No GameMode found in world"));
    }
    
    // Monitor Game State
    AGameStateBase* GameState = GetWorld()->GetGameState();
    if (!GameState)
    {
        ReportCriticalIncident(TEXT("GameState"), TEXT("No GameState found in world"));
    }
    
    // Monitor Player Controller
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        ReportCriticalIncident(TEXT("PlayerController"), TEXT("No PlayerController found"));
    }
    else
    {
        // Monitor Player Pawn
        APawn* PlayerPawn = PC->GetPawn();
        if (!PlayerPawn)
        {
            ReportCriticalIncident(TEXT("PlayerPawn"), TEXT("PlayerController has no Pawn"));
        }
    }
    
    // Monitor critical actors count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() == 0)
    {
        ReportCriticalIncident(TEXT("WorldActors"), TEXT("No actors found in world"));
    }
    else if (AllActors.Num() > 10000) // Arbitrary high threshold
    {
        ReportCriticalIncident(TEXT("WorldActors"), 
            FString::Printf(TEXT("Too many actors in world: %d"), AllActors.Num()));
    }
}

void UQA_CriticalSystemMonitor::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentFPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
    
    // Update memory usage (simplified - would need platform-specific code for real implementation)
    CurrentMemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Update CPU usage (placeholder - would need platform-specific implementation)
    CurrentCPUUsagePercent = FMath::RandRange(20.0f, 60.0f); // Simulated for now
    
    // Store performance history
    FQA_PerformanceSnapshot Snapshot;
    Snapshot.Timestamp = GetWorld()->GetTimeSeconds();
    Snapshot.FPS = CurrentFPS;
    Snapshot.MemoryUsageMB = CurrentMemoryUsageMB;
    Snapshot.CPUUsagePercent = CurrentCPUUsagePercent;
    Snapshot.ActorCount = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass()).Num();
    
    PerformanceHistory.Add(Snapshot);
    
    // Keep only last 300 snapshots (5 minutes at 1Hz)
    if (PerformanceHistory.Num() > 300)
    {
        PerformanceHistory.RemoveAt(0);
    }
}

void UQA_CriticalSystemMonitor::CheckForCriticalIncidents()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Check FPS threshold
    if (CurrentFPS < MinRequiredFPS)
    {
        ReportCriticalIncident(TEXT("Performance"), 
            FString::Printf(TEXT("FPS below threshold: %.1f < %.1f"), CurrentFPS, MinRequiredFPS));
    }
    
    // Check memory threshold
    if (CurrentMemoryUsageMB > MaxAllowedMemoryMB)
    {
        ReportCriticalIncident(TEXT("Memory"), 
            FString::Printf(TEXT("Memory usage above threshold: %.1fMB > %.1fMB"), 
                CurrentMemoryUsageMB, MaxAllowedMemoryMB));
    }
    
    // Check CPU threshold
    if (CurrentCPUUsagePercent > MaxCPUUsagePercent)
    {
        ReportCriticalIncident(TEXT("CPU"), 
            FString::Printf(TEXT("CPU usage above threshold: %.1f%% > %.1f%%"), 
                CurrentCPUUsagePercent, MaxCPUUsagePercent));
    }
}

void UQA_CriticalSystemMonitor::ReportCriticalIncident(const FString& SystemName, const FString& Description)
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Avoid spam - only report same system once per 10 seconds
    if (CurrentTime - LastCriticalIncidentTime < 10.0f)
    {
        return;
    }
    
    FQA_CriticalIncident Incident;
    Incident.SystemName = SystemName;
    Incident.Description = Description;
    Incident.Timestamp = CurrentTime;
    Incident.Severity = EQA_IncidentSeverity::High;
    
    CriticalIncidents.Add(Incident);
    LastCriticalIncidentTime = CurrentTime;
    
    // Log the incident
    UE_LOG(LogTemp, Error, TEXT("CRITICAL INCIDENT [%s]: %s"), *SystemName, *Description);
    
    // Keep only last 100 incidents
    if (CriticalIncidents.Num() > 100)
    {
        CriticalIncidents.RemoveAt(0);
    }
}

void UQA_CriticalSystemMonitor::GenerateMonitoringReport()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MonitoringDuration = CurrentTime - MonitoringStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("=== CRITICAL SYSTEM MONITORING REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Monitoring Duration: %.1f seconds"), MonitoringDuration);
    UE_LOG(LogTemp, Warning, TEXT("Critical Incidents: %d"), CriticalIncidents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Snapshots: %d"), PerformanceHistory.Num());
    
    // Calculate average performance
    if (PerformanceHistory.Num() > 0)
    {
        float AvgFPS = 0.0f;
        float AvgMemory = 0.0f;
        float AvgCPU = 0.0f;
        
        for (const FQA_PerformanceSnapshot& Snapshot : PerformanceHistory)
        {
            AvgFPS += Snapshot.FPS;
            AvgMemory += Snapshot.MemoryUsageMB;
            AvgCPU += Snapshot.CPUUsagePercent;
        }
        
        int32 Count = PerformanceHistory.Num();
        AvgFPS /= Count;
        AvgMemory /= Count;
        AvgCPU /= Count;
        
        UE_LOG(LogTemp, Warning, TEXT("Average Performance - FPS: %.1f, Memory: %.1fMB, CPU: %.1f%%"), 
               AvgFPS, AvgMemory, AvgCPU);
    }
    
    // Report recent critical incidents
    UE_LOG(LogTemp, Warning, TEXT("Recent Critical Incidents:"));
    for (int32 i = FMath::Max(0, CriticalIncidents.Num() - 5); i < CriticalIncidents.Num(); i++)
    {
        const FQA_CriticalIncident& Incident = CriticalIncidents[i];
        UE_LOG(LogTemp, Warning, TEXT("  [%.1fs] %s: %s"), 
               Incident.Timestamp, *Incident.SystemName, *Incident.Description);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END MONITORING REPORT ==="));
}

TArray<FQA_CriticalIncident> UQA_CriticalSystemMonitor::GetCriticalIncidents() const
{
    return CriticalIncidents;
}

TArray<FQA_PerformanceSnapshot> UQA_CriticalSystemMonitor::GetPerformanceHistory() const
{
    return PerformanceHistory;
}

bool UQA_CriticalSystemMonitor::IsSystemHealthy() const
{
    // System is healthy if no critical incidents in last 30 seconds
    if (CriticalIncidents.Num() == 0)
    {
        return true;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const FQA_CriticalIncident& LastIncident = CriticalIncidents.Last();
    
    return (CurrentTime - LastIncident.Timestamp) > 30.0f;
}

FQA_SystemHealthStatus UQA_CriticalSystemMonitor::GetSystemHealthStatus() const
{
    FQA_SystemHealthStatus Status;
    Status.bIsHealthy = IsSystemHealthy();
    Status.CurrentFPS = CurrentFPS;
    Status.CurrentMemoryUsageMB = CurrentMemoryUsageMB;
    Status.CurrentCPUUsagePercent = CurrentCPUUsagePercent;
    Status.RecentIncidentCount = 0;
    
    // Count incidents in last 60 seconds
    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        for (const FQA_CriticalIncident& Incident : CriticalIncidents)
        {
            if (CurrentTime - Incident.Timestamp < 60.0f)
            {
                Status.RecentIncidentCount++;
            }
        }
    }
    
    return Status;
}