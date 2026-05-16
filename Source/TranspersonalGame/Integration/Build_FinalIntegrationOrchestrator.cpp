#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bIntegrationComplete = false;
    ValidationInterval = 30.0f; // Validate every 30 seconds
    LastFullValidation = FDateTime::Now();
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing final build integration system"));
    
    // Initialize system health reports
    SystemHealthReports.Empty();
    
    // Add core systems to monitor
    TArray<FString> CoreSystems = {
        TEXT("WorldGeneration"),
        TEXT("CharacterSystem"),
        TEXT("DinosaurAI"),
        TEXT("Environment"),
        TEXT("Physics"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("CrowdSimulation")
    };
    
    for (const FString& SystemName : CoreSystems)
    {
        FBuild_SystemHealthReport Report;
        Report.SystemName = SystemName;
        Report.Status = EBuild_SystemStatus::Unknown;
        Report.LastChecked = FDateTime::Now();
        SystemHealthReports.Add(Report);
    }
    
    // Start periodic validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuild_FinalIntegrationOrchestrator::PerformFullSystemValidation,
            ValidationInterval,
            true
        );
    }
    
    // Perform initial validation
    PerformFullSystemValidation();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Deinitializing integration system"));
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::PerformFullSystemValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting full system validation"));
    
    LastFullValidation = FDateTime::Now();
    
    // Collect current metrics
    CollectPerformanceMetrics();
    
    // Validate actor counts
    ValidateActorCounts();
    
    // Check individual systems
    UpdateSystemHealthReport(TEXT("WorldGeneration"), CheckWorldGenerationHealth());
    UpdateSystemHealthReport(TEXT("CharacterSystem"), CheckCharacterSystemHealth());
    UpdateSystemHealthReport(TEXT("DinosaurAI"), CheckDinosaurAIHealth());
    UpdateSystemHealthReport(TEXT("Environment"), CheckEnvironmentHealth());
    
    // Check system dependencies
    CheckSystemDependencies();
    
    // Determine overall integration status
    bool bAllSystemsHealthy = true;
    for (const FBuild_SystemHealthReport& Report : SystemHealthReports)
    {
        if (Report.Status == EBuild_SystemStatus::Critical || Report.Status == EBuild_SystemStatus::Failed)
        {
            bAllSystemsHealthy = false;
            break;
        }
    }
    
    CurrentMetrics.bAllSystemsOperational = bAllSystemsHealthy;
    bIntegrationComplete = bAllSystemsHealthy;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validation complete - All systems operational: %s"), 
           bAllSystemsHealthy ? TEXT("YES") : TEXT("NO"));
}

FBuild_IntegrationMetrics UBuild_FinalIntegrationOrchestrator::GetCurrentIntegrationMetrics()
{
    return CurrentMetrics;
}

TArray<FBuild_SystemHealthReport> UBuild_FinalIntegrationOrchestrator::GetAllSystemHealthReports()
{
    return SystemHealthReports;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateSystemIntegrity(const FString& SystemName)
{
    for (const FBuild_SystemHealthReport& Report : SystemHealthReports)
    {
        if (Report.SystemName == SystemName)
        {
            return Report.Status == EBuild_SystemStatus::Healthy || Report.Status == EBuild_SystemStatus::Warning;
        }
    }
    return false;
}

void UBuild_FinalIntegrationOrchestrator::ForceSystemReconciliation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Forcing system reconciliation"));
    
    // Clear all cached data
    SystemHealthReports.Empty();
    
    // Reinitialize
    Initialize(FSubsystemCollectionBase());
}

float UBuild_FinalIntegrationOrchestrator::GetCurrentFrameTime()
{
    return FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
}

float UBuild_FinalIntegrationOrchestrator::GetMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // Convert to MB
}

bool UBuild_FinalIntegrationOrchestrator::IsPerformanceWithinLimits()
{
    float FrameTime = GetCurrentFrameTime();
    float MemoryUsage = GetMemoryUsage();
    
    // Performance limits: 33ms frame time (30 FPS), 4GB memory
    return FrameTime < 33.0f && MemoryUsage < 4096.0f;
}

EBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::CheckWorldGenerationHealth()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 EnvironmentActors = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && (Actor->GetName().Contains(TEXT("Tree")) || 
                         Actor->GetName().Contains(TEXT("Rock")) ||
                         Actor->GetName().Contains(TEXT("Foliage"))))
            {
                EnvironmentActors++;
            }
        }
        
        if (EnvironmentActors > 100)
        {
            return EBuild_SystemStatus::Healthy;
        }
        else if (EnvironmentActors > 10)
        {
            return EBuild_SystemStatus::Warning;
        }
    }
    
    return EBuild_SystemStatus::Critical;
}

EBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::CheckCharacterSystemHealth()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> Characters;
        UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
        
        if (Characters.Num() > 0)
        {
            return EBuild_SystemStatus::Healthy;
        }
    }
    
    return EBuild_SystemStatus::Warning;
}

EBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::CheckDinosaurAIHealth()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 DinosaurActors = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                         Actor->GetName().Contains(TEXT("TRex")) ||
                         Actor->GetName().Contains(TEXT("Raptor"))))
            {
                DinosaurActors++;
            }
        }
        
        if (DinosaurActors > 3)
        {
            return EBuild_SystemStatus::Healthy;
        }
        else if (DinosaurActors > 0)
        {
            return EBuild_SystemStatus::Warning;
        }
    }
    
    return EBuild_SystemStatus::Critical;
}

EBuild_SystemStatus UBuild_FinalIntegrationOrchestrator::CheckEnvironmentHealth()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        bool bHasLighting = false;
        bool bHasLandscape = false;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("Light")) || ActorName.Contains(TEXT("Sun")))
                {
                    bHasLighting = true;
                }
                if (ActorName.Contains(TEXT("Landscape")) || ActorName.Contains(TEXT("Terrain")))
                {
                    bHasLandscape = true;
                }
            }
        }
        
        if (bHasLighting && bHasLandscape)
        {
            return EBuild_SystemStatus::Healthy;
        }
        else if (bHasLighting || bHasLandscape)
        {
            return EBuild_SystemStatus::Warning;
        }
    }
    
    return EBuild_SystemStatus::Critical;
}

void UBuild_FinalIntegrationOrchestrator::TriggerEmergencyShutdown(const FString& Reason)
{
    UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationOrchestrator: EMERGENCY SHUTDOWN - %s"), *Reason);
    
    // Log critical error
    for (FBuild_SystemHealthReport& Report : SystemHealthReports)
    {
        Report.Status = EBuild_SystemStatus::Failed;
        Report.LastError = Reason;
    }
    
    bIntegrationComplete = false;
}

bool UBuild_FinalIntegrationOrchestrator::AttemptSystemRecovery(const FString& SystemName)
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Attempting recovery for system: %s"), *SystemName);
    
    // Basic recovery attempt - force re-validation
    PerformFullSystemValidation();
    
    return ValidateSystemIntegrity(SystemName);
}

void UBuild_FinalIntegrationOrchestrator::UpdateSystemHealthReport(const FString& SystemName, EBuild_SystemStatus Status, const FString& ErrorMessage)
{
    for (FBuild_SystemHealthReport& Report : SystemHealthReports)
    {
        if (Report.SystemName == SystemName)
        {
            Report.Status = Status;
            Report.LastChecked = FDateTime::Now();
            if (!ErrorMessage.IsEmpty())
            {
                Report.LastError = ErrorMessage;
            }
            break;
        }
    }
}

void UBuild_FinalIntegrationOrchestrator::CollectPerformanceMetrics()
{
    CurrentMetrics.AverageFrameTime = GetCurrentFrameTime();
    CurrentMetrics.MemoryUsageMB = GetMemoryUsage();
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        CurrentMetrics.TotalActors = AllActors.Num();
        CurrentMetrics.CharacterActors = 0;
        CurrentMetrics.DinosaurActors = 0;
        CurrentMetrics.EnvironmentActors = 0;
        CurrentMetrics.SystemActors = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorName = Actor->GetName();
                FString ClassName = Actor->GetClass()->GetName();
                
                if (ClassName.Contains(TEXT("Character")) || ClassName.Contains(TEXT("Pawn")))
                {
                    CurrentMetrics.CharacterActors++;
                }
                else if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
                {
                    CurrentMetrics.DinosaurActors++;
                }
                else if (ActorName.Contains(TEXT("Tree")) || ActorName.Contains(TEXT("Rock")) || ActorName.Contains(TEXT("Foliage")))
                {
                    CurrentMetrics.EnvironmentActors++;
                }
                else if (ClassName.Contains(TEXT("Manager")) || ClassName.Contains(TEXT("System")))
                {
                    CurrentMetrics.SystemActors++;
                }
            }
        }
    }
}

void UBuild_FinalIntegrationOrchestrator::ValidateActorCounts()
{
    // Validate that we have reasonable actor counts
    if (CurrentMetrics.TotalActors < 10)
    {
        UpdateSystemHealthReport(TEXT("Environment"), EBuild_SystemStatus::Critical, TEXT("Too few actors in scene"));
    }
    else if (CurrentMetrics.TotalActors > 10000)
    {
        UpdateSystemHealthReport(TEXT("Environment"), EBuild_SystemStatus::Warning, TEXT("High actor count may impact performance"));
    }
}

void UBuild_FinalIntegrationOrchestrator::CheckSystemDependencies()
{
    // Check that critical systems have their dependencies
    bool bHasCharacters = CurrentMetrics.CharacterActors > 0;
    bool bHasEnvironment = CurrentMetrics.EnvironmentActors > 0;
    bool bHasDinosaurs = CurrentMetrics.DinosaurActors > 0;
    
    if (!bHasCharacters)
    {
        UpdateSystemHealthReport(TEXT("CharacterSystem"), EBuild_SystemStatus::Critical, TEXT("No character actors found"));
    }
    
    if (!bHasEnvironment)
    {
        UpdateSystemHealthReport(TEXT("Environment"), EBuild_SystemStatus::Critical, TEXT("No environment actors found"));
    }
    
    if (!bHasDinosaurs)
    {
        UpdateSystemHealthReport(TEXT("DinosaurAI"), EBuild_SystemStatus::Warning, TEXT("No dinosaur actors found"));
    }
}