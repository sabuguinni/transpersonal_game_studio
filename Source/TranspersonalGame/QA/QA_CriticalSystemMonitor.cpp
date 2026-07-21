#include "QA_CriticalSystemMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectIterator.h"

UQA_CriticalSystemMonitor::UQA_CriticalSystemMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f; // Check every 2 seconds
    
    // Initialize monitoring parameters
    MaxMemoryUsageMB = 2048.0f;
    MinFrameRate = 30.0f;
    MaxActorCount = 5000;
    
    // Initialize monitoring state
    bMonitoringActive = false;
    MonitoringStartTime = 0.0f;
    LastFrameRate = 60.0f;
    CurrentMemoryUsage = 0.0f;
    CurrentActorCount = 0;
    
    // Initialize critical system status
    bGameModeValid = false;
    bGameStateValid = false;
    bPlayerControllerValid = false;
    bCharacterValid = false;
}

void UQA_CriticalSystemMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: BeginPlay - Starting critical system monitoring"));
    
    // Start monitoring
    StartMonitoring();
}

void UQA_CriticalSystemMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMonitoringActive)
    {
        UpdateSystemMetrics(DeltaTime);
        CheckCriticalSystems();
        CheckPerformanceThresholds();
        LogSystemStatus();
    }
}

void UQA_CriticalSystemMonitor::StartMonitoring()
{
    if (bMonitoringActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: Monitoring already active"));
        return;
    }
    
    bMonitoringActive = true;
    MonitoringStartTime = GetWorld()->GetTimeSeconds();
    
    // Clear previous results
    SystemAlerts.Empty();
    PerformanceHistory.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: Critical system monitoring started"));
    
    // Initial system validation
    ValidateInitialSystems();
}

void UQA_CriticalSystemMonitor::StopMonitoring()
{
    if (!bMonitoringActive)
    {
        return;
    }
    
    bMonitoringActive = false;
    float MonitoringDuration = GetWorld()->GetTimeSeconds() - MonitoringStartTime;
    
    // Generate final monitoring report
    GenerateMonitoringReport(MonitoringDuration);
    
    UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: Monitoring stopped after %.2f seconds"), MonitoringDuration);
}

void UQA_CriticalSystemMonitor::ValidateInitialSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: Validating initial critical systems"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddSystemAlert(EQA_SystemType::Engine, EQA_AlertSeverity::Critical, TEXT("World not available"));
        return;
    }
    
    // Validate Game Mode
    AGameModeBase* GameMode = World->GetAuthGameMode();
    if (GameMode)
    {
        bGameModeValid = true;
        UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: GameMode valid: %s"), *GameMode->GetClass()->GetName());
    }
    else
    {
        bGameModeValid = false;
        AddSystemAlert(EQA_SystemType::GameMode, EQA_AlertSeverity::Critical, TEXT("GameMode not found"));
    }
    
    // Validate Game State
    AGameStateBase* GameState = World->GetGameState();
    if (GameState)
    {
        bGameStateValid = true;
        UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: GameState valid: %s"), *GameState->GetClass()->GetName());
    }
    else
    {
        bGameStateValid = false;
        AddSystemAlert(EQA_SystemType::GameState, EQA_AlertSeverity::Critical, TEXT("GameState not found"));
    }
    
    // Validate Player Controller
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC)
    {
        bPlayerControllerValid = true;
        UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: PlayerController valid: %s"), *PC->GetClass()->GetName());
        
        // Check for possessed pawn
        APawn* Pawn = PC->GetPawn();
        if (Pawn)
        {
            bCharacterValid = true;
            UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: Character valid: %s"), *Pawn->GetClass()->GetName());
        }
        else
        {
            bCharacterValid = false;
            AddSystemAlert(EQA_SystemType::Character, EQA_AlertSeverity::High, TEXT("No pawn possessed by PlayerController"));
        }
    }
    else
    {
        bPlayerControllerValid = false;
        bCharacterValid = false;
        AddSystemAlert(EQA_SystemType::PlayerController, EQA_AlertSeverity::Critical, TEXT("PlayerController not found"));
    }
}

void UQA_CriticalSystemMonitor::UpdateSystemMetrics(float DeltaTime)
{
    // Update frame rate
    if (DeltaTime > 0.0f)
    {
        LastFrameRate = 1.0f / DeltaTime;
    }
    
    // Update memory usage (simplified estimation)
    CurrentMemoryUsage = EstimateMemoryUsage();
    
    // Update actor count
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentActorCount = AllActors.Num();
    }
    
    // Store performance history
    FQA_PerformanceSnapshot Snapshot;
    Snapshot.Timestamp = GetWorld()->GetTimeSeconds();
    Snapshot.FrameRate = LastFrameRate;
    Snapshot.MemoryUsageMB = CurrentMemoryUsage;
    Snapshot.ActorCount = CurrentActorCount;
    
    PerformanceHistory.Add(Snapshot);
    
    // Keep only last 100 snapshots
    if (PerformanceHistory.Num() > 100)
    {
        PerformanceHistory.RemoveAt(0);
    }
}

void UQA_CriticalSystemMonitor::CheckCriticalSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Re-validate critical systems periodically
    bool bPreviousGameModeValid = bGameModeValid;
    bool bPreviousGameStateValid = bGameStateValid;
    bool bPreviousPlayerControllerValid = bPlayerControllerValid;
    bool bPreviousCharacterValid = bCharacterValid;
    
    // Check Game Mode
    AGameModeBase* GameMode = World->GetAuthGameMode();
    bGameModeValid = (GameMode != nullptr);
    
    if (bPreviousGameModeValid && !bGameModeValid)
    {
        AddSystemAlert(EQA_SystemType::GameMode, EQA_AlertSeverity::Critical, TEXT("GameMode became invalid"));
    }
    
    // Check Game State
    AGameStateBase* GameState = World->GetGameState();
    bGameStateValid = (GameState != nullptr);
    
    if (bPreviousGameStateValid && !bGameStateValid)
    {
        AddSystemAlert(EQA_SystemType::GameState, EQA_AlertSeverity::Critical, TEXT("GameState became invalid"));
    }
    
    // Check Player Controller
    APlayerController* PC = World->GetFirstPlayerController();
    bPlayerControllerValid = (PC != nullptr);
    
    if (bPreviousPlayerControllerValid && !bPlayerControllerValid)
    {
        AddSystemAlert(EQA_SystemType::PlayerController, EQA_AlertSeverity::Critical, TEXT("PlayerController became invalid"));
    }
    
    // Check Character
    if (PC)
    {
        APawn* Pawn = PC->GetPawn();
        bCharacterValid = (Pawn != nullptr);
        
        if (bPreviousCharacterValid && !bCharacterValid)
        {
            AddSystemAlert(EQA_SystemType::Character, EQA_AlertSeverity::High, TEXT("Character became invalid"));
        }
    }
    else
    {
        bCharacterValid = false;
    }
}

void UQA_CriticalSystemMonitor::CheckPerformanceThresholds()
{
    // Check frame rate threshold
    if (LastFrameRate < MinFrameRate)
    {
        AddSystemAlert(EQA_SystemType::Performance, EQA_AlertSeverity::Medium, 
            FString::Printf(TEXT("Frame rate below threshold: %.1f FPS"), LastFrameRate));
    }
    
    // Check memory usage threshold
    if (CurrentMemoryUsage > MaxMemoryUsageMB)
    {
        AddSystemAlert(EQA_SystemType::Performance, EQA_AlertSeverity::High, 
            FString::Printf(TEXT("Memory usage above threshold: %.1f MB"), CurrentMemoryUsage));
    }
    
    // Check actor count threshold
    if (CurrentActorCount > MaxActorCount)
    {
        AddSystemAlert(EQA_SystemType::Performance, EQA_AlertSeverity::Medium, 
            FString::Printf(TEXT("Actor count above threshold: %d"), CurrentActorCount));
    }
}

void UQA_CriticalSystemMonitor::LogSystemStatus()
{
    // Log system status periodically (every 30 seconds)
    static float LastStatusLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastStatusLogTime > 30.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("=== QA CRITICAL SYSTEM STATUS ==="));
        UE_LOG(LogTemp, Warning, TEXT("GameMode: %s"), bGameModeValid ? TEXT("VALID") : TEXT("INVALID"));
        UE_LOG(LogTemp, Warning, TEXT("GameState: %s"), bGameStateValid ? TEXT("VALID") : TEXT("INVALID"));
        UE_LOG(LogTemp, Warning, TEXT("PlayerController: %s"), bPlayerControllerValid ? TEXT("VALID") : TEXT("INVALID"));
        UE_LOG(LogTemp, Warning, TEXT("Character: %s"), bCharacterValid ? TEXT("VALID") : TEXT("INVALID"));
        UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f FPS"), LastFrameRate);
        UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentMemoryUsage);
        UE_LOG(LogTemp, Warning, TEXT("Actor Count: %d"), CurrentActorCount);
        UE_LOG(LogTemp, Warning, TEXT("Active Alerts: %d"), SystemAlerts.Num());
        UE_LOG(LogTemp, Warning, TEXT("=== END SYSTEM STATUS ==="));
        
        LastStatusLogTime = CurrentTime;
    }
}

float UQA_CriticalSystemMonitor::EstimateMemoryUsage()
{
    // Simplified memory usage estimation
    // In a real implementation, you would use platform-specific APIs
    
    float EstimatedUsage = 512.0f; // Base usage
    
    // Add estimation based on actor count
    EstimatedUsage += CurrentActorCount * 0.1f; // ~0.1MB per actor
    
    // Add estimation based on loaded objects
    int32 ObjectCount = 0;
    for (TObjectIterator<UObject> It; It; ++It)
    {
        ObjectCount++;
    }
    EstimatedUsage += ObjectCount * 0.001f; // ~0.001MB per object
    
    return EstimatedUsage;
}

void UQA_CriticalSystemMonitor::AddSystemAlert(EQA_SystemType SystemType, EQA_AlertSeverity Severity, const FString& Message)
{
    FQA_SystemAlert Alert;
    Alert.SystemType = SystemType;
    Alert.Severity = Severity;
    Alert.Message = Message;
    Alert.Timestamp = GetWorld()->GetTimeSeconds();
    
    SystemAlerts.Add(Alert);
    
    // Log the alert
    FString SystemName = GetSystemTypeName(SystemType);
    FString SeverityName = GetAlertSeverityName(Severity);
    
    UE_LOG(LogTemp, Error, TEXT("QA_CriticalSystemMonitor: [%s] %s - %s"), *SeverityName, *SystemName, *Message);
    
    // Keep only last 50 alerts
    if (SystemAlerts.Num() > 50)
    {
        SystemAlerts.RemoveAt(0);
    }
}

FString UQA_CriticalSystemMonitor::GetSystemTypeName(EQA_SystemType SystemType)
{
    switch (SystemType)
    {
        case EQA_SystemType::Engine: return TEXT("Engine");
        case EQA_SystemType::GameMode: return TEXT("GameMode");
        case EQA_SystemType::GameState: return TEXT("GameState");
        case EQA_SystemType::PlayerController: return TEXT("PlayerController");
        case EQA_SystemType::Character: return TEXT("Character");
        case EQA_SystemType::Performance: return TEXT("Performance");
        case EQA_SystemType::Memory: return TEXT("Memory");
        case EQA_SystemType::Network: return TEXT("Network");
        default: return TEXT("Unknown");
    }
}

FString UQA_CriticalSystemMonitor::GetAlertSeverityName(EQA_AlertSeverity Severity)
{
    switch (Severity)
    {
        case EQA_AlertSeverity::Low: return TEXT("LOW");
        case EQA_AlertSeverity::Medium: return TEXT("MEDIUM");
        case EQA_AlertSeverity::High: return TEXT("HIGH");
        case EQA_AlertSeverity::Critical: return TEXT("CRITICAL");
        default: return TEXT("UNKNOWN");
    }
}

void UQA_CriticalSystemMonitor::GenerateMonitoringReport(float MonitoringDuration)
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA CRITICAL SYSTEM MONITORING REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Monitoring Duration: %.2f seconds"), MonitoringDuration);
    
    // System status summary
    UE_LOG(LogTemp, Warning, TEXT("Final System Status:"));
    UE_LOG(LogTemp, Warning, TEXT("- GameMode: %s"), bGameModeValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("- GameState: %s"), bGameStateValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("- PlayerController: %s"), bPlayerControllerValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogTemp, Warning, TEXT("- Character: %s"), bCharacterValid ? TEXT("VALID") : TEXT("INVALID"));
    
    // Performance summary
    if (PerformanceHistory.Num() > 0)
    {
        float AvgFrameRate = 0.0f;
        float AvgMemoryUsage = 0.0f;
        int32 AvgActorCount = 0;
        
        for (const FQA_PerformanceSnapshot& Snapshot : PerformanceHistory)
        {
            AvgFrameRate += Snapshot.FrameRate;
            AvgMemoryUsage += Snapshot.MemoryUsageMB;
            AvgActorCount += Snapshot.ActorCount;
        }
        
        int32 SnapshotCount = PerformanceHistory.Num();
        AvgFrameRate /= SnapshotCount;
        AvgMemoryUsage /= SnapshotCount;
        AvgActorCount /= SnapshotCount;
        
        UE_LOG(LogTemp, Warning, TEXT("Performance Averages:"));
        UE_LOG(LogTemp, Warning, TEXT("- Frame Rate: %.1f FPS"), AvgFrameRate);
        UE_LOG(LogTemp, Warning, TEXT("- Memory Usage: %.1f MB"), AvgMemoryUsage);
        UE_LOG(LogTemp, Warning, TEXT("- Actor Count: %d"), AvgActorCount);
    }
    
    // Alert summary
    UE_LOG(LogTemp, Warning, TEXT("Alert Summary:"));
    UE_LOG(LogTemp, Warning, TEXT("- Total Alerts: %d"), SystemAlerts.Num());
    
    int32 CriticalAlerts = 0;
    int32 HighAlerts = 0;
    int32 MediumAlerts = 0;
    int32 LowAlerts = 0;
    
    for (const FQA_SystemAlert& Alert : SystemAlerts)
    {
        switch (Alert.Severity)
        {
            case EQA_AlertSeverity::Critical: CriticalAlerts++; break;
            case EQA_AlertSeverity::High: HighAlerts++; break;
            case EQA_AlertSeverity::Medium: MediumAlerts++; break;
            case EQA_AlertSeverity::Low: LowAlerts++; break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("- Critical: %d"), CriticalAlerts);
    UE_LOG(LogTemp, Warning, TEXT("- High: %d"), HighAlerts);
    UE_LOG(LogTemp, Warning, TEXT("- Medium: %d"), MediumAlerts);
    UE_LOG(LogTemp, Warning, TEXT("- Low: %d"), LowAlerts);
    
    UE_LOG(LogTemp, Warning, TEXT("=== END MONITORING REPORT ==="));
}

// Getter functions
TArray<FQA_SystemAlert> UQA_CriticalSystemMonitor::GetSystemAlerts() const
{
    return SystemAlerts;
}

TArray<FQA_PerformanceSnapshot> UQA_CriticalSystemMonitor::GetPerformanceHistory() const
{
    return PerformanceHistory;
}

bool UQA_CriticalSystemMonitor::IsMonitoringActive() const
{
    return bMonitoringActive;
}

bool UQA_CriticalSystemMonitor::AreAllCriticalSystemsValid() const
{
    return bGameModeValid && bGameStateValid && bPlayerControllerValid && bCharacterValid;
}

float UQA_CriticalSystemMonitor::GetLastFrameRate() const
{
    return LastFrameRate;
}

float UQA_CriticalSystemMonitor::GetCurrentMemoryUsage() const
{
    return CurrentMemoryUsage;
}

int32 UQA_CriticalSystemMonitor::GetCurrentActorCount() const
{
    return CurrentActorCount;
}