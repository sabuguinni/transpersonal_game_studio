// BuildIntegrationManager.cpp
// Agent #19 — Integration & Build Agent
// PROD_CYCLE_AUTO_20260628_007
// Manages build integration, module dependency validation, and cycle reporting.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bBuildApproved = false;
    CoreClassesLoaded = 0;
    TotalActorCount = 0;
    CurrentCycleID = TEXT("PROD_CYCLE_AUTO_20260628_007");
    BuildStatus = TEXT("PENDING");
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationCheck();
}

void ABuildIntegrationManager::RunIntegrationCheck()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildIntegration] No world available for integration check."));
        return;
    }

    // Count all actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();

    // Validate gameplay readiness
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    bool bHasTerrain = false;
    int32 StaticMeshCount = 0;

    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        FString ClassName = Actor->GetClass()->GetName();

        if (ClassName.Contains(TEXT("PlayerStart"))) bHasPlayerStart = true;
        if (ClassName.Contains(TEXT("DirectionalLight"))) bHasLighting = true;
        if (ClassName.Contains(TEXT("Landscape"))) bHasTerrain = true;
        if (ClassName.Contains(TEXT("StaticMesh"))) StaticMeshCount++;
    }

    bool bGameplayReady = bHasPlayerStart && bHasLighting && bHasTerrain && (StaticMeshCount >= 5);

    // Set build status
    bBuildApproved = bGameplayReady && (CoreClassesLoaded >= 6);
    BuildStatus = bBuildApproved ? TEXT("APPROVED") : TEXT("NEEDS_REVIEW");

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Cycle: %s"), *CurrentCycleID);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Total actors: %d"), TotalActorCount);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Gameplay ready: %s"), bGameplayReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Build status: %s"), *BuildStatus);
}

bool ABuildIntegrationManager::IsBuildApproved() const
{
    return bBuildApproved;
}

FString ABuildIntegrationManager::GetBuildStatus() const
{
    return BuildStatus;
}

int32 ABuildIntegrationManager::GetTotalActorCount() const
{
    return TotalActorCount;
}

void ABuildIntegrationManager::SetCoreClassesLoaded(int32 Count)
{
    CoreClassesLoaded = Count;
}

FString ABuildIntegrationManager::GetCycleID() const
{
    return CurrentCycleID;
}
