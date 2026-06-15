#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Stats/Stats.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIsMonitoringPerformance = false;
    LastFrameRateCheck = 0.0f;
    MaxAllowedActors = 8000;
    MaxAllowedDinosaurs = 150;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing build integration system"));
    
    // Initialize module tracking
    ModuleStatuses.Empty();
    
    // Set up initial system health
    CurrentSystemHealth = FBuild_SystemHealth();
    
    // Start monitoring
    StartPerformanceMonitoring();
    
    // Validate initial state
    ValidateAllModules();
    UpdateSystemHealth();
}

void UBuildIntegrationManager::Deinitialize()
{
    StopPerformanceMonitoring();
    LogBuildReport();
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::ValidateAllModules()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating all modules"));
    
    ModuleStatuses.Empty();
    
    // Core modules to validate
    TArray<FString> CoreModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("Engine"),
        TEXT("UnrealEd")
    };
    
    for (const FString& ModuleName : CoreModules)
    {
        FBuild_ModuleStatus Status;
        Status.ModuleName = ModuleName;
        Status.bIsCompiled = true; // Assume compiled if we're running
        Status.bIsLoaded = true;
        Status.ClassCount = 0;
        
        // Count classes in this module (simplified)
        if (ModuleName == TEXT("TranspersonalGame"))
        {
            Status.ClassCount = 25; // Estimated based on current codebase
        }
        
        ModuleStatuses.Add(Status);
    }
    
    CheckModuleCompilation();
}

FBuild_ModuleStatus UBuildIntegrationManager::GetModuleStatus(const FString& ModuleName)
{
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.ModuleName == ModuleName)
        {
            return Status;
        }
    }
    
    // Return empty status if not found
    return FBuild_ModuleStatus();
}

TArray<FBuild_ModuleStatus> UBuildIntegrationManager::GetAllModuleStatuses()
{
    return ModuleStatuses;
}

FBuild_SystemHealth UBuildIntegrationManager::GetSystemHealth()
{
    UpdateSystemHealth();
    return CurrentSystemHealth;
}

void UBuildIntegrationManager::EnforceActorCaps()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Count different types of actors
    TArray<AActor*> DinosaurActors;
    TArray<AActor*> PropActors;
    TArray<AActor*> EssentialActors;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        // Check for dinosaurs
        if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("dino")) || ActorLabel.Contains(TEXT("trex")) ||
            ActorLabel.Contains(TEXT("raptor")) || ActorLabel.Contains(TEXT("brachi")))
        {
            DinosaurActors.Add(Actor);
        }
        // Check for essential actors
        else if (ActorName.Contains(TEXT("playerstart")) || ActorName.Contains(TEXT("light")) ||
                 ActorName.Contains(TEXT("sky")) || ActorName.Contains(TEXT("fog")) ||
                 ActorLabel.Contains(TEXT("playerstart")) || ActorLabel.Contains(TEXT("light")) ||
                 ActorLabel.Contains(TEXT("sky")) || ActorLabel.Contains(TEXT("fog")))
        {
            EssentialActors.Add(Actor);
        }
        else
        {
            PropActors.Add(Actor);
        }
    }
    
    // Enforce dinosaur cap
    if (DinosaurActors.Num() > MaxAllowedDinosaurs)
    {
        int32 ToRemove = DinosaurActors.Num() - MaxAllowedDinosaurs;
        for (int32 i = 0; i < ToRemove && i < DinosaurActors.Num(); i++)
        {
            if (DinosaurActors[i])
            {
                DinosaurActors[i]->Destroy();
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Removed %d excess dinosaurs"), ToRemove);
    }
    
    // Enforce total actor cap
    int32 TotalActors = AllActors.Num();
    if (TotalActors > MaxAllowedActors)
    {
        int32 ToRemove = TotalActors - MaxAllowedActors;
        int32 Removed = 0;
        
        // Remove props first
        for (AActor* PropActor : PropActors)
        {
            if (Removed >= ToRemove)
            {
                break;
            }
            
            if (PropActor)
            {
                PropActor->Destroy();
                Removed++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Removed %d excess actors"), Removed);
    }
    
    UpdateSystemHealth();
}

void UBuildIntegrationManager::ValidateGameplayReadiness()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating gameplay readiness"));
    
    bool bIsReady = IsMinimumViablePrototypeReady();
    
    if (bIsReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Minimum viable prototype is READY"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Minimum viable prototype is NOT READY"));
    }
}

bool UBuildIntegrationManager::IsMinimumViablePrototypeReady()
{
    return ValidateMinimumGameplayElements();
}

void UBuildIntegrationManager::StartPerformanceMonitoring()
{
    bIsMonitoringPerformance = true;
    LastFrameRateCheck = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Started performance monitoring"));
}

void UBuildIntegrationManager::StopPerformanceMonitoring()
{
    bIsMonitoringPerformance = false;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Stopped performance monitoring"));
}

void UBuildIntegrationManager::LogBuildReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION REPORT ==="));
    
    // Module status
    UE_LOG(LogTemp, Warning, TEXT("Modules: %d total"), ModuleStatuses.Num());
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: Compiled=%s, Loaded=%s, Classes=%d"), 
               *Status.ModuleName, 
               Status.bIsCompiled ? TEXT("YES") : TEXT("NO"),
               Status.bIsLoaded ? TEXT("YES") : TEXT("NO"),
               Status.ClassCount);
    }
    
    // System health
    UE_LOG(LogTemp, Warning, TEXT("System Health:"));
    UE_LOG(LogTemp, Warning, TEXT("  Total Actors: %d"), CurrentSystemHealth.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("  Dinosaurs: %d"), CurrentSystemHealth.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("  Props: %d"), CurrentSystemHealth.PropCount);
    UE_LOG(LogTemp, Warning, TEXT("  Frame Rate: %.1f"), CurrentSystemHealth.FrameRate);
    UE_LOG(LogTemp, Warning, TEXT("  Memory: %.1f MB"), CurrentSystemHealth.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("  Performance Healthy: %s"), 
           CurrentSystemHealth.bIsPerformanceHealthy ? TEXT("YES") : TEXT("NO"));
    
    // Gameplay readiness
    bool bReady = IsMinimumViablePrototypeReady();
    UE_LOG(LogTemp, Warning, TEXT("Minimum Viable Prototype: %s"), bReady ? TEXT("READY") : TEXT("NOT READY"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== END BUILD REPORT ==="));
}

void UBuildIntegrationManager::CheckModuleCompilation()
{
    // This would normally check compilation logs, but for now we assume success
    // if the subsystem is running
    
    for (FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        Status.bIsCompiled = true;
        Status.bIsLoaded = true;
    }
}

void UBuildIntegrationManager::UpdateSystemHealth()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    ValidateActorCounts();
    CheckPerformanceMetrics();
}

void UBuildIntegrationManager::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    CurrentSystemHealth.TotalActors = AllActors.Num();
    CurrentSystemHealth.DinosaurCount = 0;
    CurrentSystemHealth.PropCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("trex")) || 
            ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachi")) ||
            ActorLabel.Contains(TEXT("dino")) || ActorLabel.Contains(TEXT("trex")) ||
            ActorLabel.Contains(TEXT("raptor")) || ActorLabel.Contains(TEXT("brachi")))
        {
            CurrentSystemHealth.DinosaurCount++;
        }
        else if (!ActorName.Contains(TEXT("playerstart")) && !ActorName.Contains(TEXT("light")) &&
                 !ActorName.Contains(TEXT("sky")) && !ActorName.Contains(TEXT("fog")) &&
                 !ActorLabel.Contains(TEXT("playerstart")) && !ActorLabel.Contains(TEXT("light")) &&
                 !ActorLabel.Contains(TEXT("sky")) && !ActorLabel.Contains(TEXT("fog")))
        {
            CurrentSystemHealth.PropCount++;
        }
    }
}

void UBuildIntegrationManager::CheckPerformanceMetrics()
{
    if (!bIsMonitoringPerformance)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get frame rate (simplified)
    float CurrentTime = World->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastFrameRateCheck;
    
    if (DeltaTime > 0.0f)
    {
        CurrentSystemHealth.FrameRate = 1.0f / DeltaTime;
    }
    
    LastFrameRateCheck = CurrentTime;
    
    // Estimate memory usage (simplified)
    CurrentSystemHealth.MemoryUsageMB = CurrentSystemHealth.TotalActors * 0.5f; // Rough estimate
    
    // Check if performance is healthy
    CurrentSystemHealth.bIsPerformanceHealthy = 
        (CurrentSystemHealth.FrameRate > 30.0f) && 
        (CurrentSystemHealth.TotalActors < MaxAllowedActors) &&
        (CurrentSystemHealth.DinosaurCount < MaxAllowedDinosaurs);
}

bool UBuildIntegrationManager::ValidateMinimumGameplayElements()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for essential gameplay elements
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        if (ActorName.Contains(TEXT("playerstart")) || ActorLabel.Contains(TEXT("playerstart")))
        {
            bHasPlayerStart = true;
        }
        else if (ActorName.Contains(TEXT("light")) || ActorLabel.Contains(TEXT("light")))
        {
            bHasLighting = true;
        }
        else if (ActorName.Contains(TEXT("landscape")) || ActorName.Contains(TEXT("terrain")) ||
                 ActorLabel.Contains(TEXT("landscape")) || ActorLabel.Contains(TEXT("terrain")))
        {
            bHasTerrain = true;
        }
        else if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("trex")) || 
                 ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachi")) ||
                 ActorLabel.Contains(TEXT("dino")) || ActorLabel.Contains(TEXT("trex")) ||
                 ActorLabel.Contains(TEXT("raptor")) || ActorLabel.Contains(TEXT("brachi")))
        {
            bHasDinosaurs = true;
        }
    }
    
    return bHasPlayerStart && bHasLighting && bHasTerrain && bHasDinosaurs;
}