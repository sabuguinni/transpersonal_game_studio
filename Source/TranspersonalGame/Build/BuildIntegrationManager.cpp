// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — Cycle 019
// Manages build integration, module health checks, and rollback registry

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    CurrentBuildVersion = TEXT("Build_019");
    bIntegrationComplete = false;
    LastIntegrationCycle = 19;
    MaxRollbackBuilds = 10;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register this build in the rollback registry
    RegisterBuild(CurrentBuildVersion, LastIntegrationCycle);
    
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Initialized — Build: %s, Cycle: %d"),
        *CurrentBuildVersion, LastIntegrationCycle);
}

void UBuildIntegrationManager::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Deinitialized"));
}

bool UBuildIntegrationManager::RunIntegrationCheck()
{
    int32 PassCount = 0;
    int32 FailCount = 0;
    
    // Check 1: World valid
    UWorld* World = GetWorld();
    if (World)
    {
        PassCount++;
        UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Check 1 PASS: World valid — %s"), *World->GetName());
    }
    else
    {
        FailCount++;
        UE_LOG(LogTemp, Warning, TEXT("[BuildIntegration] Check 1 FAIL: World is null"));
    }
    
    // Check 2: PlayerStart exists
    if (World)
    {
        TArray<AActor*> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
        if (PlayerStarts.Num() > 0)
        {
            PassCount++;
            UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Check 2 PASS: %d PlayerStart(s) found"), PlayerStarts.Num());
        }
        else
        {
            FailCount++;
            UE_LOG(LogTemp, Warning, TEXT("[BuildIntegration] Check 2 FAIL: No PlayerStart in level"));
        }
    }
    
    // Check 3: Module version consistent
    if (!CurrentBuildVersion.IsEmpty())
    {
        PassCount++;
        UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Check 3 PASS: Build version set — %s"), *CurrentBuildVersion);
    }
    else
    {
        FailCount++;
        UE_LOG(LogTemp, Warning, TEXT("[BuildIntegration] Check 3 FAIL: Build version empty"));
    }
    
    bIntegrationComplete = (FailCount == 0);
    
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Integration check complete — PASS: %d, FAIL: %d, Status: %s"),
        PassCount, FailCount, bIntegrationComplete ? TEXT("OK") : TEXT("DEGRADED"));
    
    return bIntegrationComplete;
}

void UBuildIntegrationManager::RegisterBuild(const FString& BuildVersion, int32 CycleNumber)
{
    FBuild_RollbackEntry Entry;
    Entry.BuildVersion = BuildVersion;
    Entry.CycleNumber = CycleNumber;
    Entry.Timestamp = FDateTime::Now();
    Entry.bIsValid = true;
    
    RollbackRegistry.Insert(Entry, 0);
    
    // Trim to max rollback count
    while (RollbackRegistry.Num() > MaxRollbackBuilds)
    {
        RollbackRegistry.RemoveAt(RollbackRegistry.Num() - 1);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Registered build %s (Cycle %d) — Registry size: %d"),
        *BuildVersion, CycleNumber, RollbackRegistry.Num());
}

FString UBuildIntegrationManager::GetCurrentBuildVersion() const
{
    return CurrentBuildVersion;
}

int32 UBuildIntegrationManager::GetRollbackCount() const
{
    return RollbackRegistry.Num();
}

bool UBuildIntegrationManager::IsIntegrationComplete() const
{
    return bIntegrationComplete;
}

TArray<FBuild_RollbackEntry> UBuildIntegrationManager::GetRollbackRegistry() const
{
    return RollbackRegistry;
}

void UBuildIntegrationManager::LogModuleHealth(const FString& ModuleName, bool bHealthy)
{
    if (bHealthy)
    {
        UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Module HEALTHY: %s"), *ModuleName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildIntegration] Module DEGRADED: %s"), *ModuleName);
    }
    
    FBuild_ModuleHealthEntry HealthEntry;
    HealthEntry.ModuleName = ModuleName;
    HealthEntry.bIsHealthy = bHealthy;
    HealthEntry.CheckTime = FDateTime::Now();
    ModuleHealthLog.Add(HealthEntry);
}

int32 UBuildIntegrationManager::GetHealthyModuleCount() const
{
    int32 Count = 0;
    for (const FBuild_ModuleHealthEntry& Entry : ModuleHealthLog)
    {
        if (Entry.bIsHealthy) Count++;
    }
    return Count;
}
