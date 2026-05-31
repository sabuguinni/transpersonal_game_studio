#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bBuildValid = false;
    CurrentHealth = FBuild_SystemHealth();
    LastValidationTime = FDateTime::Now();
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager initialized - Agent #19"));
    
    // Initial validation
    ValidateBuildIntegrity();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager shutting down"));
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateBuildIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting build integrity validation"));
    
    ValidateModules();
    CheckActorCounts();
    MonitorPerformance();
    UpdateHealthStatus();
    
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Build validation complete - Valid: %s"), 
           bBuildValid ? TEXT("TRUE") : TEXT("FALSE"));
    
    return bBuildValid;
}

FBuild_SystemHealth UBuildIntegrationManager::GetSystemHealth()
{
    // Update health before returning
    CheckActorCounts();
    MonitorPerformance();
    UpdateHealthStatus();
    
    return CurrentHealth;
}

TArray<FBuild_ModuleStatus> UBuildIntegrationManager::GetModuleStatuses()
{
    ValidateModules();
    return ModuleStatuses;
}

bool UBuildIntegrationManager::EnforceActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No world found for actor limit enforcement"));
        return false;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 TotalActors = AllActors.Num();
    UE_LOG(LogTemp, Warning, TEXT("Current actor count: %d (limit: %d)"), TotalActors, MAX_TOTAL_ACTORS);
    
    if (TotalActors > MAX_TOTAL_ACTORS)
    {
        UE_LOG(LogTemp, Error, TEXT("Actor limit exceeded! Triggering cleanup"));
        CleanupExcessActors();
        return false;
    }
    
    return true;
}

void UBuildIntegrationManager::CleanupExcessActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Count dinosaurs and props separately
    TArray<AActor*> DinosaurActors;
    TArray<AActor*> PropActors;
    TArray<AActor*> OtherActors;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
            ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")))
        {
            DinosaurActors.Add(Actor);
        }
        else if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("tree")) || 
                 ActorName.Contains(TEXT("bush")) || ActorName.Contains(TEXT("prop")))
        {
            PropActors.Add(Actor);
        }
        else
        {
            OtherActors.Add(Actor);
        }
    }
    
    // Remove excess dinosaurs first
    if (DinosaurActors.Num() > MAX_DINOSAURS)
    {
        int32 ToRemove = DinosaurActors.Num() - MAX_DINOSAURS;
        UE_LOG(LogTemp, Warning, TEXT("Removing %d excess dinosaurs"), ToRemove);
        
        for (int32 i = 0; i < ToRemove && i < DinosaurActors.Num(); i++)
        {
            if (DinosaurActors[i])
            {
                DinosaurActors[i]->Destroy();
            }
        }
    }
    
    // Remove excess props
    if (PropActors.Num() > MAX_TOTAL_PROPS)
    {
        int32 ToRemove = PropActors.Num() - MAX_TOTAL_PROPS;
        UE_LOG(LogTemp, Warning, TEXT("Removing %d excess props"), ToRemove);
        
        for (int32 i = 0; i < ToRemove && i < PropActors.Num(); i++)
        {
            if (PropActors[i])
            {
                PropActors[i]->Destroy();
            }
        }
    }
}

float UBuildIntegrationManager::GetCurrentMemoryUsage()
{
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    return UsedMemoryMB;
}

bool UBuildIntegrationManager::IsPerformanceAcceptable()
{
    float MemoryUsage = GetCurrentMemoryUsage();
    
    // Memory limit: 50GB = 51200MB
    bool bMemoryOK = MemoryUsage < 51200.0f;
    
    UWorld* World = GetWorld();
    bool bActorCountOK = true;
    
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        bActorCountOK = AllActors.Num() <= MAX_TOTAL_ACTORS;
    }
    
    return bMemoryOK && bActorCountOK;
}

FString UBuildIntegrationManager::GenerateBuildReport()
{
    FString Report = TEXT("=== BUILD INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Time: %s\n"), *LastValidationTime.ToString());
    Report += FString::Printf(TEXT("Build Valid: %s\n"), bBuildValid ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\n--- SYSTEM HEALTH ---\n");
    Report += FString::Printf(TEXT("Total Actors: %d / %d\n"), CurrentHealth.TotalActors, MAX_TOTAL_ACTORS);
    Report += FString::Printf(TEXT("Dinosaurs: %d / %d\n"), CurrentHealth.DinosaurCount, MAX_DINOSAURS);
    Report += FString::Printf(TEXT("Props: %d / %d\n"), CurrentHealth.PropCount, MAX_TOTAL_PROPS);
    Report += FString::Printf(TEXT("Memory Usage: %.1f MB\n"), CurrentHealth.MemoryUsageMB);
    Report += FString::Printf(TEXT("Within Limits: %s\n"), CurrentHealth.bWithinLimits ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("Performance Good: %s\n"), CurrentHealth.bPerformanceGood ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\n--- MODULE STATUS ---\n");
    for (const FBuild_ModuleStatus& Module : ModuleStatuses)
    {
        Report += FString::Printf(TEXT("%s: %s (Errors: %d)\n"), 
                                  *Module.ModuleName,
                                  Module.bIsCompiled ? TEXT("COMPILED") : TEXT("FAILED"),
                                  Module.ErrorCount);
        if (!Module.LastError.IsEmpty())
        {
            Report += FString::Printf(TEXT("  Last Error: %s\n"), *Module.LastError);
        }
    }
    
    return Report;
}

void UBuildIntegrationManager::SaveBuildSnapshot()
{
    FString Report = GenerateBuildReport();
    FString FileName = FString::Printf(TEXT("BuildSnapshot_%s.txt"), *FDateTime::Now().ToString());
    FString FilePath = FPaths::ProjectLogDir() / FileName;
    
    FFileHelper::SaveStringToFile(Report, *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("Build snapshot saved to: %s"), *FilePath);
}

void UBuildIntegrationManager::ValidateModules()
{
    ModuleStatuses.Empty();
    
    // Core modules to validate
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Environment"),
        TEXT("Character"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("QA"),
        TEXT("Integration")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FBuild_ModuleStatus Status;
        Status.ModuleName = ModuleName;
        
        // For now, assume modules are compiled if we can create this manager
        // In a real implementation, we'd check compilation logs
        Status.bIsCompiled = true;
        Status.bHasErrors = false;
        Status.ErrorCount = 0;
        Status.LastError = TEXT("");
        
        ModuleStatuses.Add(Status);
    }
}

void UBuildIntegrationManager::CheckActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        CurrentHealth.TotalActors = 0;
        CurrentHealth.DinosaurCount = 0;
        CurrentHealth.PropCount = 0;
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    CurrentHealth.TotalActors = AllActors.Num();
    CurrentHealth.DinosaurCount = 0;
    CurrentHealth.PropCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("veloci")) || 
            ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachi")) ||
            ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("parasauro")))
        {
            CurrentHealth.DinosaurCount++;
        }
        else if (ActorName.Contains(TEXT("rock")) || ActorName.Contains(TEXT("tree")) || 
                 ActorName.Contains(TEXT("bush")) || ActorName.Contains(TEXT("prop")))
        {
            CurrentHealth.PropCount++;
        }
    }
}

void UBuildIntegrationManager::MonitorPerformance()
{
    CurrentHealth.MemoryUsageMB = GetCurrentMemoryUsage();
    CurrentHealth.bPerformanceGood = IsPerformanceAcceptable();
}

void UBuildIntegrationManager::UpdateHealthStatus()
{
    CurrentHealth.bWithinLimits = (CurrentHealth.TotalActors <= MAX_TOTAL_ACTORS) &&
                                  (CurrentHealth.DinosaurCount <= MAX_DINOSAURS) &&
                                  (CurrentHealth.PropCount <= MAX_TOTAL_PROPS);
    
    bBuildValid = CurrentHealth.bWithinLimits && CurrentHealth.bPerformanceGood;
}