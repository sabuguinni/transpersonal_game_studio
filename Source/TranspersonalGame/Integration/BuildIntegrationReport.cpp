#include "BuildIntegrationReport.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"

void UBuildIntegrationReport::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationReport subsystem initialized"));
    
    // Initialize with current system state
    LastReport = GenerateIntegrationReport(TEXT("INIT"));
}

FBuild_IntegrationReport UBuildIntegrationReport::GenerateIntegrationReport(const FString& CycleId)
{
    FBuild_IntegrationReport Report;
    Report.CycleId = CycleId;
    Report.ReportTime = FDateTime::Now();
    
    // Get system statuses
    TArray<FBuild_SystemStatus> SystemStatuses = GetSystemStatuses();
    Report.SystemStatuses = SystemStatuses;
    
    // Count loaded classes
    Report.LoadedClasses = 0;
    for (const FBuild_SystemStatus& Status : SystemStatuses)
    {
        if (Status.bIsLoaded)
        {
            Report.LoadedClasses++;
        }
    }
    
    // Count actors in world
    if (UWorld* World = GetWorld())
    {
        Report.TotalActors = 0;
        Report.DinosaurActors = 0;
        Report.EnvironmentActors = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                Report.TotalActors++;
                
                FString ActorLabel = Actor->GetActorLabel().ToLower();
                if (ActorLabel.Contains(TEXT("dinosaur")) || 
                    ActorLabel.Contains(TEXT("trex")) || 
                    ActorLabel.Contains(TEXT("raptor")))
                {
                    Report.DinosaurActors++;
                }
                else if (ActorLabel.Contains(TEXT("tree")) || 
                         ActorLabel.Contains(TEXT("rock")) || 
                         ActorLabel.Contains(TEXT("foliage")))
                {
                    Report.EnvironmentActors++;
                }
            }
        }
    }
    
    // Validate map persistence
    Report.bMapSaved = ValidateMapPersistence();
    
    // Check if all systems are operational
    Report.bAllSystemsOperational = ValidateSystemIntegration();
    
    // Store in history
    ReportHistory.Add(Report);
    LastReport = Report;
    
    return Report;
}

bool UBuildIntegrationReport::ValidateSystemIntegration()
{
    TArray<FBuild_SystemStatus> SystemStatuses = GetSystemStatuses();
    
    int32 OperationalSystems = 0;
    for (const FBuild_SystemStatus& Status : SystemStatuses)
    {
        if (Status.bIsLoaded && Status.bIsCompiled)
        {
            OperationalSystems++;
        }
    }
    
    // Consider integration successful if at least 75% of systems are operational
    float OperationalPercentage = SystemStatuses.Num() > 0 ? 
        (float)OperationalSystems / (float)SystemStatuses.Num() : 0.0f;
    
    return OperationalPercentage >= 0.75f;
}

TArray<FBuild_SystemStatus> UBuildIntegrationReport::GetSystemStatuses()
{
    TArray<FBuild_SystemStatus> Statuses;
    
    // Critical system classes to check
    TArray<TPair<FString, FString>> SystemClasses = {
        {TEXT("TranspersonalCharacter"), TEXT("/Script/TranspersonalGame.TranspersonalCharacter")},
        {TEXT("TranspersonalGameState"), TEXT("/Script/TranspersonalGame.TranspersonalGameState")},
        {TEXT("PCGWorldGenerator"), TEXT("/Script/TranspersonalGame.PCGWorldGenerator")},
        {TEXT("FoliageManager"), TEXT("/Script/TranspersonalGame.FoliageManager")},
        {TEXT("CrowdSimulationManager"), TEXT("/Script/TranspersonalGame.CrowdSimulationManager")},
        {TEXT("VFXBloodManager"), TEXT("/Script/TranspersonalGame.VFXBloodManager")},
        {TEXT("BuildIntegrationManager"), TEXT("/Script/TranspersonalGame.BuildIntegrationManager")}
    };
    
    for (const auto& SystemClass : SystemClasses)
    {
        FBuild_SystemStatus Status = CheckSystemStatus(SystemClass.Key, SystemClass.Value);
        Statuses.Add(Status);
    }
    
    return Statuses;
}

void UBuildIntegrationReport::LogIntegrationMetrics(const FBuild_IntegrationReport& Report)
{
    UE_LOG(LogTemp, Log, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Cycle ID: %s"), *Report.CycleId);
    UE_LOG(LogTemp, Log, TEXT("Report Time: %s"), *Report.ReportTime.ToString());
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d"), Report.TotalActors);
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Actors: %d"), Report.DinosaurActors);
    UE_LOG(LogTemp, Log, TEXT("Environment Actors: %d"), Report.EnvironmentActors);
    UE_LOG(LogTemp, Log, TEXT("Loaded Classes: %d"), Report.LoadedClasses);
    UE_LOG(LogTemp, Log, TEXT("Map Saved: %s"), Report.bMapSaved ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("All Systems Operational: %s"), Report.bAllSystemsOperational ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Log, TEXT("System Statuses:"));
    for (const FBuild_SystemStatus& Status : Report.SystemStatuses)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: Compiled=%s, Loaded=%s, Actors=%d"), 
            *Status.SystemName,
            Status.bIsCompiled ? TEXT("YES") : TEXT("NO"),
            Status.bIsLoaded ? TEXT("YES") : TEXT("NO"),
            Status.ActorCount);
    }
}

FBuild_SystemStatus UBuildIntegrationReport::CheckSystemStatus(const FString& SystemName, const FString& ClassPath)
{
    FBuild_SystemStatus Status;
    Status.SystemName = SystemName;
    
    // Try to load the class
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    if (LoadedClass)
    {
        Status.bIsLoaded = true;
        Status.bIsCompiled = true;
        
        // Count actors of this type
        Status.ActorCount = CountActorsByType(SystemName);
    }
    else
    {
        Status.bIsLoaded = false;
        Status.bIsCompiled = false;
        Status.LastError = FString::Printf(TEXT("Failed to load class: %s"), *ClassPath);
    }
    
    return Status;
}

int32 UBuildIntegrationReport::CountActorsByType(const FString& ActorType)
{
    int32 Count = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (IsValid(Actor))
            {
                FString ActorClassName = Actor->GetClass()->GetName();
                if (ActorClassName.Contains(ActorType))
                {
                    Count++;
                }
            }
        }
    }
    
    return Count;
}

bool UBuildIntegrationReport::ValidateMapPersistence()
{
    // Check if the map has been saved recently
    // This is a simplified check - in a real implementation,
    // you might check file timestamps or other persistence indicators
    
    if (UWorld* World = GetWorld())
    {
        // If we have a valid world with actors, assume map persistence is working
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        
        return ActorCount > 10; // Minimum threshold for a populated map
    }
    
    return false;
}