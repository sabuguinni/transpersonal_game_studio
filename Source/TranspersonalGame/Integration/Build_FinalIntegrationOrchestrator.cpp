#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bIntegrationComplete = false;
    IntegrationProgress = 0.0f;
    LastErrorMessage = TEXT("");
    StartTime = 0.0f;
    ValidationSteps = 6; // Character, WorldGen, Foliage, Crowd, GameState, Performance
    CompletedSteps = 0;
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LogIntegrationStatus(TEXT("Final Integration Orchestrator initialized"));
    StartTime = FPlatformTime::Seconds();
    
    // Initialize system info array
    SystemInfos.Empty();
    SystemInfos.Add(CreateSystemInfo(TEXT("Character System"), EBuild_SystemStatus::Loading));
    SystemInfos.Add(CreateSystemInfo(TEXT("World Generation"), EBuild_SystemStatus::Loading));
    SystemInfos.Add(CreateSystemInfo(TEXT("Foliage System"), EBuild_SystemStatus::Loading));
    SystemInfos.Add(CreateSystemInfo(TEXT("Crowd Simulation"), EBuild_SystemStatus::Loading));
    SystemInfos.Add(CreateSystemInfo(TEXT("Game State"), EBuild_SystemStatus::Loading));
    SystemInfos.Add(CreateSystemInfo(TEXT("Performance Metrics"), EBuild_SystemStatus::Loading));
    
    // Start integration process automatically
    StartIntegrationProcess();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    LogIntegrationStatus(TEXT("Final Integration Orchestrator shutting down"));
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::StartIntegrationProcess()
{
    LogIntegrationStatus(TEXT("Starting final build integration process"));
    
    CompletedSteps = 0;
    IntegrationProgress = 0.0f;
    bIntegrationComplete = false;
    
    // Validate all systems
    ValidateAllSystems();
    
    // Calculate final progress
    IntegrationProgress = (float)CompletedSteps / (float)ValidationSteps;
    bIntegrationComplete = (CompletedSteps == ValidationSteps);
    
    if (bIntegrationComplete)
    {
        LogIntegrationStatus(TEXT("Build integration completed successfully"));
    }
    else
    {
        LogIntegrationStatus(FString::Printf(TEXT("Build integration incomplete: %d/%d steps"), CompletedSteps, ValidationSteps));
    }
}

bool UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    LogIntegrationStatus(TEXT("Validating all game systems"));
    
    ValidateCharacterSystem();
    ValidateWorldGeneration();
    ValidateFoliageSystem();
    ValidateCrowdSimulation();
    ValidateGameState();
    ValidatePerformanceMetrics();
    
    return bIntegrationComplete;
}

void UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    try
    {
        // Check if TranspersonalCharacter class exists
        UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
        
        if (CharacterClass)
        {
            SystemInfos[0] = CreateSystemInfo(TEXT("Character System"), EBuild_SystemStatus::Ready);
            CompletedSteps++;
            LogIntegrationStatus(TEXT("Character System: VALIDATED"));
        }
        else
        {
            SystemInfos[0] = CreateSystemInfo(TEXT("Character System"), EBuild_SystemStatus::Error, TEXT("TranspersonalCharacter class not found"));
            LogIntegrationStatus(TEXT("Character System: ERROR - Class not found"));
        }
    }
    catch (...)
    {
        SystemInfos[0] = CreateSystemInfo(TEXT("Character System"), EBuild_SystemStatus::Error, TEXT("Validation exception"));
        LogIntegrationStatus(TEXT("Character System: ERROR - Exception during validation"));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateWorldGeneration()
{
    try
    {
        UClass* WorldGenClass = FindObject<UClass>(ANY_PACKAGE, TEXT("PCGWorldGenerator"));
        
        if (WorldGenClass)
        {
            SystemInfos[1] = CreateSystemInfo(TEXT("World Generation"), EBuild_SystemStatus::Ready);
            CompletedSteps++;
            LogIntegrationStatus(TEXT("World Generation: VALIDATED"));
        }
        else
        {
            SystemInfos[1] = CreateSystemInfo(TEXT("World Generation"), EBuild_SystemStatus::Error, TEXT("PCGWorldGenerator class not found"));
            LogIntegrationStatus(TEXT("World Generation: ERROR - Class not found"));
        }
    }
    catch (...)
    {
        SystemInfos[1] = CreateSystemInfo(TEXT("World Generation"), EBuild_SystemStatus::Error, TEXT("Validation exception"));
        LogIntegrationStatus(TEXT("World Generation: ERROR - Exception during validation"));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateFoliageSystem()
{
    try
    {
        UClass* FoliageClass = FindObject<UClass>(ANY_PACKAGE, TEXT("FoliageManager"));
        
        if (FoliageClass)
        {
            SystemInfos[2] = CreateSystemInfo(TEXT("Foliage System"), EBuild_SystemStatus::Ready);
            CompletedSteps++;
            LogIntegrationStatus(TEXT("Foliage System: VALIDATED"));
        }
        else
        {
            SystemInfos[2] = CreateSystemInfo(TEXT("Foliage System"), EBuild_SystemStatus::Error, TEXT("FoliageManager class not found"));
            LogIntegrationStatus(TEXT("Foliage System: ERROR - Class not found"));
        }
    }
    catch (...)
    {
        SystemInfos[2] = CreateSystemInfo(TEXT("Foliage System"), EBuild_SystemStatus::Error, TEXT("Validation exception"));
        LogIntegrationStatus(TEXT("Foliage System: ERROR - Exception during validation"));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateCrowdSimulation()
{
    try
    {
        UClass* CrowdClass = FindObject<UClass>(ANY_PACKAGE, TEXT("CrowdSimulationManager"));
        
        if (CrowdClass)
        {
            SystemInfos[3] = CreateSystemInfo(TEXT("Crowd Simulation"), EBuild_SystemStatus::Ready);
            CompletedSteps++;
            LogIntegrationStatus(TEXT("Crowd Simulation: VALIDATED"));
        }
        else
        {
            SystemInfos[3] = CreateSystemInfo(TEXT("Crowd Simulation"), EBuild_SystemStatus::Error, TEXT("CrowdSimulationManager class not found"));
            LogIntegrationStatus(TEXT("Crowd Simulation: ERROR - Class not found"));
        }
    }
    catch (...)
    {
        SystemInfos[3] = CreateSystemInfo(TEXT("Crowd Simulation"), EBuild_SystemStatus::Error, TEXT("Validation exception"));
        LogIntegrationStatus(TEXT("Crowd Simulation: ERROR - Exception during validation"));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateGameState()
{
    try
    {
        UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
        
        if (GameStateClass)
        {
            SystemInfos[4] = CreateSystemInfo(TEXT("Game State"), EBuild_SystemStatus::Ready);
            CompletedSteps++;
            LogIntegrationStatus(TEXT("Game State: VALIDATED"));
        }
        else
        {
            SystemInfos[4] = CreateSystemInfo(TEXT("Game State"), EBuild_SystemStatus::Error, TEXT("TranspersonalGameState class not found"));
            LogIntegrationStatus(TEXT("Game State: ERROR - Class not found"));
        }
    }
    catch (...)
    {
        SystemInfos[4] = CreateSystemInfo(TEXT("Game State"), EBuild_SystemStatus::Error, TEXT("Validation exception"));
        LogIntegrationStatus(TEXT("Game State: ERROR - Exception during validation"));
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidatePerformanceMetrics()
{
    try
    {
        // Check basic performance metrics
        UWorld* World = GetWorld();
        if (World)
        {
            int32 ActorCount = World->GetActorCount();
            SystemInfos[5] = CreateSystemInfo(TEXT("Performance Metrics"), EBuild_SystemStatus::Ready);
            SystemInfos[5].ActorCount = ActorCount;
            CompletedSteps++;
            LogIntegrationStatus(FString::Printf(TEXT("Performance Metrics: VALIDATED - %d actors"), ActorCount));
        }
        else
        {
            SystemInfos[5] = CreateSystemInfo(TEXT("Performance Metrics"), EBuild_SystemStatus::Error, TEXT("World not available"));
            LogIntegrationStatus(TEXT("Performance Metrics: ERROR - No world"));
        }
    }
    catch (...)
    {
        SystemInfos[5] = CreateSystemInfo(TEXT("Performance Metrics"), EBuild_SystemStatus::Error, TEXT("Validation exception"));
        LogIntegrationStatus(TEXT("Performance Metrics: ERROR - Exception during validation"));
    }
}

FBuild_IntegrationReport UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    
    Report.SystemStatuses = SystemInfos;
    Report.bAllSystemsReady = bIntegrationComplete;
    Report.BuildVersion = TEXT("PROD_CYCLE_AUTO_20260516_009");
    Report.LastIntegrationTime = FDateTime::Now();
    
    // Calculate total actors
    Report.TotalActors = 0;
    for (const FBuild_SystemInfo& Info : SystemInfos)
    {
        Report.TotalActors += Info.ActorCount;
    }
    
    // Estimate memory usage (simplified)
    Report.MemoryUsagePercent = FMath::Clamp((float)Report.TotalActors / 1000.0f * 100.0f, 0.0f, 100.0f);
    
    return Report;
}

void UBuild_FinalIntegrationOrchestrator::RefreshSystemStatuses()
{
    LogIntegrationStatus(TEXT("Refreshing system statuses"));
    StartIntegrationProcess();
}

FBuild_SystemInfo UBuild_FinalIntegrationOrchestrator::CreateSystemInfo(const FString& SystemName, EBuild_SystemStatus Status, const FString& Error)
{
    FBuild_SystemInfo Info;
    Info.SystemName = SystemName;
    Info.Status = Status;
    Info.LastError = Error;
    Info.LastUpdateTime = FPlatformTime::Seconds() - StartTime;
    Info.ActorCount = 0;
    
    return Info;
}

void UBuild_FinalIntegrationOrchestrator::LogIntegrationStatus(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("[Build Integration] %s"), *Message);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("[Integration] %s"), *Message));
    }
}