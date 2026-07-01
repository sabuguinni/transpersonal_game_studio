// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — PROD_CYCLE_AUTO_20260701_001
// Manages build validation, module integrity checks, and integration status reporting.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bBuildIsGreen = false;
    ActiveClassCount = 0;
    LastBuildCycle = TEXT("PROD_CYCLE_AUTO_20260701_001");
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationCheck();
}

void ABuildIntegrationManager::RunIntegrationCheck()
{
    ActiveClassCount = 0;
    BuildErrors.Empty();
    bBuildIsGreen = true;

    // Verify world is valid
    UWorld* World = GetWorld();
    if (!World)
    {
        BuildErrors.Add(TEXT("ERROR: World is null during integration check"));
        bBuildIsGreen = false;
        return;
    }

    // Count active actors as a proxy for world health
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    ActiveClassCount = AllActors.Num();

    if (ActiveClassCount < 5)
    {
        BuildErrors.Add(FString::Printf(TEXT("WARNING: Only %d actors in world — expected ≥5 for MinPlayableMap"), ActiveClassCount));
    }

    // Log integration status
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Integration check complete: %d actors, build=%s"),
        ActiveClassCount, bBuildIsGreen ? TEXT("GREEN") : TEXT("RED"));
}

bool ABuildIntegrationManager::IsBuildGreen() const
{
    return bBuildIsGreen;
}

TArray<FString> ABuildIntegrationManager::GetBuildErrors() const
{
    return BuildErrors;
}

FString ABuildIntegrationManager::GetLastBuildCycle() const
{
    return LastBuildCycle;
}

int32 ABuildIntegrationManager::GetActiveClassCount() const
{
    return ActiveClassCount;
}

void ABuildIntegrationManager::SetBuildCycle(const FString& CycleId)
{
    LastBuildCycle = CycleId;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Build cycle set to: %s"), *CycleId);
}
