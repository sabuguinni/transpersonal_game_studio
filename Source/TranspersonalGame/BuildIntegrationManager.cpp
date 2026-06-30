// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — Cycle AUTO_20260630_011
// Manages build state, rollback points, and integration validation

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    CurrentBuildVersion = TEXT("AUTO_20260630_011");
    bIntegrationValid = false;
    LastValidatedActorCount = 0;
    MaxRollbackBuilds = 10;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bIntegrationValid = false;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Initialized — Build: %s"), *CurrentBuildVersion);
}

void UBuildIntegrationManager::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Deinitialized"));
}

bool UBuildIntegrationManager::ValidateIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildIntegrationManager] ValidateIntegration: No world"));
        return false;
    }

    // Count actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    LastValidatedActorCount = AllActors.Num();

    // Validate PlayerStart exists
    bool bHasPlayerStart = false;
    bool bHasDirectionalLight = false;
    bool bHasLandscape = false;

    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        FString ClassName = Actor->GetClass()->GetName();
        if (ClassName.Contains(TEXT("PlayerStart"))) bHasPlayerStart = true;
        if (ClassName.Contains(TEXT("DirectionalLight"))) bHasDirectionalLight = true;
        if (ClassName.Contains(TEXT("Landscape"))) bHasLandscape = true;
    }

    bIntegrationValid = bHasPlayerStart && bHasDirectionalLight;

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Validation: actors=%d, PlayerStart=%d, Light=%d, Landscape=%d, Valid=%d"),
        LastValidatedActorCount, bHasPlayerStart, bHasDirectionalLight, bHasLandscape, bIntegrationValid);

    return bIntegrationValid;
}

FString UBuildIntegrationManager::GetBuildVersion() const
{
    return CurrentBuildVersion;
}

void UBuildIntegrationManager::SetBuildVersion(const FString& NewVersion)
{
    CurrentBuildVersion = NewVersion;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Build version set: %s"), *CurrentBuildVersion);
}

bool UBuildIntegrationManager::IsIntegrationValid() const
{
    return bIntegrationValid;
}

int32 UBuildIntegrationManager::GetLastValidatedActorCount() const
{
    return LastValidatedActorCount;
}

void UBuildIntegrationManager::LogBuildStatus() const
{
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] === BUILD STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Version: %s"), *CurrentBuildVersion);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Valid: %d"), bIntegrationValid);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Actors: %d"), LastValidatedActorCount);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] MaxRollbacks: %d"), MaxRollbackBuilds);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] =================="));
}
