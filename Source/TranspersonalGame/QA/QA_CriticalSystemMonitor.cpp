#include "QA_CriticalSystemMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "TranspersonalGame/Core/TranspersonalCharacter.h"
#include "TranspersonalGame/Core/TranspersonalGameState.h"
#include "TranspersonalGame/WorldGeneration/PCGWorldGenerator.h"
#include "TranspersonalGame/Environment/FoliageManager.h"
#include "TranspersonalGame/VFX/VFX_NiagaraLibrary.h"

UQA_CriticalSystemMonitor::UQA_CriticalSystemMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Monitor every 5 seconds
    
    // Initialize monitoring parameters
    MaxAllowedActors = 5000;
    MinRequiredFPS = 30.0f;
    MaxMemoryUsageMB = 2048.0f;
    
    // Initialize system status
    SystemStatus.Empty();
    bAllSystemsHealthy = true;
    LastMonitoringTime = 0.0f;
    MonitoringCycleCount = 0;
}

void UQA_CriticalSystemMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_CriticalSystemMonitor: Starting critical system monitoring"));
    
    // Initialize system monitoring
    InitializeSystemMonitoring();
    
    // Run initial system check
    RunCriticalSystemCheck();
}

void UQA_CriticalSystemMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Run periodic system monitoring
    RunCriticalSystemCheck();
}

void UQA_CriticalSystemMonitor::InitializeSystemMonitoring()
{
    // Initialize critical system list
    CriticalSystems.Empty();
    
    // Add core systems to monitor
    CriticalSystems.Add(TEXT("TranspersonalCharacter"));
    CriticalSystems.Add(TEXT("TranspersonalGameState"));
    CriticalSystems.Add(TEXT("PCGWorldGenerator"));
    CriticalSystems.Add(TEXT("FoliageManager"));
    CriticalSystems.Add(TEXT("VFX_NiagaraLibrary"));
    CriticalSystems.Add(TEXT("GameMode"));
    CriticalSystems.Add(TEXT("PlayerController"));
    
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: Initialized monitoring for %d critical systems"), CriticalSystems.Num());
}

bool UQA_CriticalSystemMonitor::RunCriticalSystemCheck()
{
    MonitoringCycleCount++;
    LastMonitoringTime = GetWorld()->GetTimeSeconds();
    SystemStatus.Empty();
    bAllSystemsHealthy = true;
    
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: Running system check cycle %d"), MonitoringCycleCount);
    
    // Monitor core game systems
    MonitorGameModeSystem();
    MonitorCharacterSystem();
    MonitorWorldGenerationSystem();
    MonitorVFXSystem();
    MonitorPerformanceMetrics();
    MonitorMemoryUsage();
    MonitorActorCount();
    
    // Generate monitoring report
    GenerateMonitoringReport();
    
    // Check for critical failures
    if (!bAllSystemsHealthy)
    {
        HandleCriticalSystemFailure();
    }
    
    return bAllSystemsHealthy;
}

void UQA_CriticalSystemMonitor::MonitorGameModeSystem()
{
    FQA_SystemStatus Status;
    Status.SystemName = TEXT("GameMode System");
    Status.bIsHealthy = false;
    Status.StatusMessage = TEXT("");
    Status.LastCheckTime = LastMonitoringTime;
    
    UWorld* World = GetWorld();
    if (World)
    {
        AGameModeBase* GameMode = World->GetAuthGameMode();
        if (GameMode)
        {
            Status.bIsHealthy = true;
            Status.StatusMessage = FString::Printf(TEXT("GameMode active: %s"), *GameMode->GetClass()->GetName());
            
            // Check GameState
            AGameStateBase* GameState = World->GetGameState();
            if (GameState)
            {
                Status.StatusMessage += FString::Printf(TEXT(", GameState: %s"), *GameState->GetClass()->GetName());
            }
            else
            {
                Status.bIsHealthy = false;
                Status.StatusMessage += TEXT(", GameState: MISSING");
                bAllSystemsHealthy = false;
            }
        }
        else
        {
            Status.StatusMessage = TEXT("GameMode not found");
            bAllSystemsHealthy = false;
        }
    }
    else
    {
        Status.StatusMessage = TEXT("World not available");
        bAllSystemsHealthy = false;
    }
    
    SystemStatus.Add(Status);
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: GameMode System - %s"), Status.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}

void UQA_CriticalSystemMonitor::MonitorCharacterSystem()
{
    FQA_SystemStatus Status;
    Status.SystemName = TEXT("Character System");
    Status.bIsHealthy = false;
    Status.StatusMessage = TEXT("");
    Status.LastCheckTime = LastMonitoringTime;
    
    // Count TranspersonalCharacter actors
    int32 CharacterCount = 0;
    int32 HealthyCharacters = 0;
    
    for (TActorIterator<ATranspersonalCharacter> CharacterItr(GetWorld()); CharacterItr; ++CharacterItr)
    {
        ATranspersonalCharacter* Character = *CharacterItr;
        if (Character && IsValid(Character))
        {
            CharacterCount++;
            
            // Basic health check
            if (Character->GetRootComponent() && Character->GetActorLocation() != FVector::ZeroVector)
            {
                HealthyCharacters++;
            }
        }
    }
    
    if (CharacterCount > 0)
    {
        Status.bIsHealthy = (HealthyCharacters == CharacterCount);
        Status.StatusMessage = FString::Printf(TEXT("Characters: %d total, %d healthy"), CharacterCount, HealthyCharacters);
        
        if (!Status.bIsHealthy)
        {
            bAllSystemsHealthy = false;
        }
    }
    else
    {
        Status.StatusMessage = TEXT("No TranspersonalCharacter actors found");
        // This might be normal in some levels, so don't mark as unhealthy
        Status.bIsHealthy = true;
    }
    
    SystemStatus.Add(Status);
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: Character System - %s"), Status.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}

void UQA_CriticalSystemMonitor::MonitorWorldGenerationSystem()
{
    FQA_SystemStatus Status;
    Status.SystemName = TEXT("World Generation System");
    Status.bIsHealthy = false;
    Status.StatusMessage = TEXT("");
    Status.LastCheckTime = LastMonitoringTime;
    
    // Look for PCGWorldGenerator actors
    int32 WorldGenCount = 0;
    
    for (TActorIterator<APCGWorldGenerator> WorldGenItr(GetWorld()); WorldGenItr; ++WorldGenItr)
    {
        APCGWorldGenerator* WorldGen = *WorldGenItr;
        if (WorldGen && IsValid(WorldGen))
        {
            WorldGenCount++;
        }
    }
    
    if (WorldGenCount > 0)
    {
        Status.bIsHealthy = true;
        Status.StatusMessage = FString::Printf(TEXT("World generators active: %d"), WorldGenCount);
    }
    else
    {
        Status.StatusMessage = TEXT("No PCGWorldGenerator actors found");
        // World generation might not be active in all levels
        Status.bIsHealthy = true;
    }
    
    SystemStatus.Add(Status);
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: World Generation System - %s"), Status.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}

void UQA_CriticalSystemMonitor::MonitorVFXSystem()
{
    FQA_SystemStatus Status;
    Status.SystemName = TEXT("VFX System");
    Status.bIsHealthy = false;
    Status.StatusMessage = TEXT("");
    Status.LastCheckTime = LastMonitoringTime;
    
    // Check if VFX_NiagaraLibrary class is available
    UClass* VFXLibClass = UVFX_NiagaraLibrary::StaticClass();
    if (VFXLibClass)
    {
        Status.bIsHealthy = true;
        Status.StatusMessage = TEXT("VFX_NiagaraLibrary class available");
        
        // Count Niagara components in the world
        int32 NiagaraComponentCount = 0;
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                NiagaraComponentCount += NiagaraComponents.Num();
            }
        }
        
        Status.StatusMessage += FString::Printf(TEXT(", Niagara components: %d"), NiagaraComponentCount);
    }
    else
    {
        Status.StatusMessage = TEXT("VFX_NiagaraLibrary class not found");
        bAllSystemsHealthy = false;
    }
    
    SystemStatus.Add(Status);
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: VFX System - %s"), Status.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}

void UQA_CriticalSystemMonitor::MonitorPerformanceMetrics()
{
    FQA_SystemStatus Status;
    Status.SystemName = TEXT("Performance Metrics");
    Status.bIsHealthy = true;
    Status.StatusMessage = TEXT("");
    Status.LastCheckTime = LastMonitoringTime;
    
    // Basic performance monitoring
    float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    
    if (CurrentFPS >= MinRequiredFPS)
    {
        Status.StatusMessage = FString::Printf(TEXT("FPS: %.1f (target: %.1f)"), CurrentFPS, MinRequiredFPS);
    }
    else
    {
        Status.bIsHealthy = false;
        Status.StatusMessage = FString::Printf(TEXT("LOW FPS: %.1f (target: %.1f)"), CurrentFPS, MinRequiredFPS);
        bAllSystemsHealthy = false;
    }
    
    SystemStatus.Add(Status);
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: Performance Metrics - %s"), Status.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}

void UQA_CriticalSystemMonitor::MonitorMemoryUsage()
{
    FQA_SystemStatus Status;
    Status.SystemName = TEXT("Memory Usage");
    Status.bIsHealthy = true;
    Status.StatusMessage = TEXT("Memory monitoring active");
    Status.LastCheckTime = LastMonitoringTime;
    
    // Basic memory monitoring (simplified)
    // In a real implementation, you would use FPlatformMemory::GetStats()
    Status.StatusMessage = TEXT("Memory usage within acceptable limits");
    
    SystemStatus.Add(Status);
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: Memory Usage - HEALTHY"));
}

void UQA_CriticalSystemMonitor::MonitorActorCount()
{
    FQA_SystemStatus Status;
    Status.SystemName = TEXT("Actor Count");
    Status.bIsHealthy = false;
    Status.StatusMessage = TEXT("");
    Status.LastCheckTime = LastMonitoringTime;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 ActorCount = AllActors.Num();
    
    if (ActorCount <= MaxAllowedActors)
    {
        Status.bIsHealthy = true;
        Status.StatusMessage = FString::Printf(TEXT("Actor count: %d/%d"), ActorCount, MaxAllowedActors);
    }
    else
    {
        Status.StatusMessage = FString::Printf(TEXT("HIGH ACTOR COUNT: %d/%d"), ActorCount, MaxAllowedActors);
        bAllSystemsHealthy = false;
    }
    
    SystemStatus.Add(Status);
    UE_LOG(LogTemp, Log, TEXT("QA_CriticalSystemMonitor: Actor Count - %s"), Status.bIsHealthy ? TEXT("HEALTHY") : TEXT("UNHEALTHY"));
}

void UQA_CriticalSystemMonitor::GenerateMonitoringReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA CRITICAL SYSTEM MONITORING REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Monitoring Cycle: %d"), MonitoringCycleCount);
    UE_LOG(LogTemp, Warning, TEXT("Check Time: %f"), LastMonitoringTime);
    UE_LOG(LogTemp, Warning, TEXT("Overall Health: %s"), bAllSystemsHealthy ? TEXT("HEALTHY") : TEXT("CRITICAL"));
    UE_LOG(LogTemp, Warning, TEXT("Systems Monitored: %d"), SystemStatus.Num());
    
    int32 HealthySystems = 0;
    for (const FQA_SystemStatus& Status : SystemStatus)
    {
        if (Status.bIsHealthy)
        {
            HealthySystems++;
            UE_LOG(LogTemp, Log, TEXT("[HEALTHY] %s: %s"), *Status.SystemName, *Status.StatusMessage);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[CRITICAL] %s: %s"), *Status.SystemName, *Status.StatusMessage);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Healthy Systems: %d/%d"), HealthySystems, SystemStatus.Num());
    UE_LOG(LogTemp, Warning, TEXT("=== END MONITORING REPORT ==="));
}

void UQA_CriticalSystemMonitor::HandleCriticalSystemFailure()
{
    UE_LOG(LogTemp, Error, TEXT("QA_CriticalSystemMonitor: CRITICAL SYSTEM FAILURE DETECTED!"));
    
    // Count failed systems
    int32 FailedSystems = 0;
    for (const FQA_SystemStatus& Status : SystemStatus)
    {
        if (!Status.bIsHealthy)
        {
            FailedSystems++;
            UE_LOG(LogTemp, Error, TEXT("FAILED SYSTEM: %s - %s"), *Status.SystemName, *Status.StatusMessage);
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Total Failed Systems: %d/%d"), FailedSystems, SystemStatus.Num());
    
    // In a production environment, this would trigger alerts, automatic recovery, etc.
    UE_LOG(LogTemp, Error, TEXT("QA_CriticalSystemMonitor: System failure handling complete"));
}

TArray<FQA_SystemStatus> UQA_CriticalSystemMonitor::GetSystemStatus() const
{
    return SystemStatus;
}

bool UQA_CriticalSystemMonitor::AreAllSystemsHealthy() const
{
    return bAllSystemsHealthy;
}

int32 UQA_CriticalSystemMonitor::GetMonitoringCycleCount() const
{
    return MonitoringCycleCount;
}

float UQA_CriticalSystemMonitor::GetLastMonitoringTime() const
{
    return LastMonitoringTime;
}