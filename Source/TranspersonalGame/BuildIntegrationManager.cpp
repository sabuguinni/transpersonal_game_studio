// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — Cycle AUTO_20260702_006
// Manages build integration, module dependency tracking, and integration health reporting.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIsInitialized = false;
    IntegrationScore = 0.0f;
    LastBuildCycle = TEXT("PROD_CYCLE_AUTO_20260702_006");
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bIsInitialized = true;
    IntegrationScore = 100.0f;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Initialized — Cycle %s"), *LastBuildCycle);
}

void UBuildIntegrationManager::Deinitialize()
{
    bIsInitialized = false;
    Super::Deinitialize();
}

float UBuildIntegrationManager::GetIntegrationScore() const
{
    return IntegrationScore;
}

FString UBuildIntegrationManager::GetLastBuildCycle() const
{
    return LastBuildCycle;
}

void UBuildIntegrationManager::SetLastBuildCycle(const FString& CycleID)
{
    LastBuildCycle = CycleID;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Build cycle updated: %s"), *CycleID);
}

bool UBuildIntegrationManager::RunIntegrationHealthCheck(UObject* WorldContext)
{
    if (!WorldContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildIntegrationManager] HealthCheck: null WorldContext"));
        return false;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildIntegrationManager] HealthCheck: no world found"));
        return false;
    }

    int32 ActorCount = 0;
    bool bHasPlayerStart = false;
    bool bHasDirectionalLight = false;
    bool bHasFog = false;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;
        ActorCount++;

        FString ClassName = Actor->GetClass()->GetName();
        if (ClassName.Contains(TEXT("PlayerStart"))) bHasPlayerStart = true;
        if (ClassName.Contains(TEXT("DirectionalLight"))) bHasDirectionalLight = true;
        if (ClassName.Contains(TEXT("ExponentialHeightFog"))) bHasFog = true;
    }

    int32 PassCount = (bHasPlayerStart ? 1 : 0) + (bHasDirectionalLight ? 1 : 0) + (bHasFog ? 1 : 0);
    IntegrationScore = (PassCount / 3.0f) * 100.0f;

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] HealthCheck: %d actors, score=%.0f%%"),
        ActorCount, IntegrationScore);

    return IntegrationScore >= 66.0f;
}

void UBuildIntegrationManager::LogBuildStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] === BUILD STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Cycle: %s"), *LastBuildCycle);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Integration Score: %.0f%%"), IntegrationScore);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Initialized: %s"), bIsInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] === END STATUS ==="));
}
