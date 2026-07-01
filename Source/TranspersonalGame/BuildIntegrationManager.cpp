// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — CYCLE AUTO_20260701_004
// Manages build integration state, module health, and cycle reporting.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default integration state
    bIntegrationValid = false;
    LastBuildCycle = TEXT("CYCLE_AUTO_20260701_004");
    CoreClassesLoaded = 0;
    TotalCoreClasses = 7;
    GameplayReadinessScore = 0;
    MaxReadinessScore = 5;
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationCheck();
}

void ABuildIntegrationManager::RunIntegrationCheck()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[A19] BuildIntegrationManager: No world available for integration check."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[A19] BuildIntegrationManager: Running integration check for cycle %s"), *LastBuildCycle);

    // Check actor inventory
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorsInLevel = AllActors.Num();

    UE_LOG(LogTemp, Log, TEXT("[A19] Total actors in level: %d"), TotalActorsInLevel);

    // Validate gameplay readiness
    GameplayReadinessScore = 0;
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    bool bHasTerrain = false;
    bool bHasDinosaurs = false;
    bool bHasNavMesh = false;

    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        FString ClassName = Actor->GetClass()->GetName();
        FString Label = Actor->GetActorLabel();

        if (ClassName.Contains(TEXT("PlayerStart"))) bHasPlayerStart = true;
        if (ClassName.Contains(TEXT("DirectionalLight")) || ClassName.Contains(TEXT("SkyLight"))) bHasLighting = true;
        if (Label.Contains(TEXT("Landscape")) || Label.Contains(TEXT("Terrain"))) bHasTerrain = true;
        if (Label.Contains(TEXT("Dino")) || Label.Contains(TEXT("Rex")) || Label.Contains(TEXT("Raptor"))) bHasDinosaurs = true;
        if (ClassName.Contains(TEXT("NavMesh"))) bHasNavMesh = true;
    }

    if (bHasPlayerStart) GameplayReadinessScore++;
    if (bHasLighting) GameplayReadinessScore++;
    if (bHasTerrain) GameplayReadinessScore++;
    if (bHasDinosaurs) GameplayReadinessScore++;
    if (bHasNavMesh) GameplayReadinessScore++;

    bIntegrationValid = (GameplayReadinessScore >= 4);

    UE_LOG(LogTemp, Log, TEXT("[A19] Gameplay readiness: %d/%d — Integration valid: %s"),
        GameplayReadinessScore, MaxReadinessScore,
        bIntegrationValid ? TEXT("YES") : TEXT("NO"));
}

FString ABuildIntegrationManager::GetIntegrationStatus() const
{
    return FString::Printf(TEXT("Cycle: %s | Classes: %d/%d | Readiness: %d/%d | Valid: %s"),
        *LastBuildCycle,
        CoreClassesLoaded, TotalCoreClasses,
        GameplayReadinessScore, MaxReadinessScore,
        bIntegrationValid ? TEXT("YES") : TEXT("NO"));
}

void ABuildIntegrationManager::ReportBuildStatus()
{
    UE_LOG(LogTemp, Log, TEXT("[A19] === BUILD STATUS REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("[A19] %s"), *GetIntegrationStatus());
    UE_LOG(LogTemp, Log, TEXT("[A19] Total actors: %d"), TotalActorsInLevel);
    UE_LOG(LogTemp, Log, TEXT("[A19] === END BUILD STATUS ==="));
}
