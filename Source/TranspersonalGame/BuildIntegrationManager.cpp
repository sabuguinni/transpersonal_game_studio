// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — PROD_CYCLE_AUTO_20260630_010
// Manages build integration state, module health checks, and cycle reporting.

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
    CoreClassesLoaded = 0;
    TotalCoreClasses = 7;
    LastBuildCycle = TEXT("PROD_CYCLE_AUTO_20260630_010");
    bCompilationGatePassed = false;
    bCAPEnforcementApplied = false;
    bBridgeValidated = false;
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationCheck();
}

void ABuildIntegrationManager::RunIntegrationCheck()
{
    // Validate world state
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildIntegration] World is null — integration check aborted"));
        bIntegrationValid = false;
        return;
    }

    bBridgeValidated = true;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Bridge validated — world: %s"), *World->GetName());

    // Count actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    ActorCount = AllActors.Num();
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Actor count: %d"), ActorCount);

    // Mark integration as valid if basic checks pass
    bIntegrationValid = (ActorCount > 0) && bBridgeValidated;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Integration valid: %s"), bIntegrationValid ? TEXT("true") : TEXT("false"));
}

void ABuildIntegrationManager::ReportBuildStatus()
{
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] === BUILD STATUS REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Cycle: %s"), *LastBuildCycle);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Bridge validated: %s"), bBridgeValidated ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] CAP enforcement: %s"), bCAPEnforcementApplied ? TEXT("PASS") : TEXT("PENDING"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Core classes: %d/%d"), CoreClassesLoaded, TotalCoreClasses);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Compilation gate: %s"), bCompilationGatePassed ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Actor count: %d"), ActorCount);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Integration valid: %s"), bIntegrationValid ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] === END REPORT ==="));
}

void ABuildIntegrationManager::SetCoreClassesLoaded(int32 Count)
{
    CoreClassesLoaded = FMath::Clamp(Count, 0, TotalCoreClasses);
    bCompilationGatePassed = (CoreClassesLoaded == TotalCoreClasses);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] Core classes loaded: %d/%d — gate: %s"),
        CoreClassesLoaded, TotalCoreClasses, bCompilationGatePassed ? TEXT("PASS") : TEXT("FAIL"));
}

void ABuildIntegrationManager::MarkCAPEnforced()
{
    bCAPEnforcementApplied = true;
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegration] CAP enforcement marked as applied"));
}

FString ABuildIntegrationManager::GetIntegrationSummary() const
{
    return FString::Printf(
        TEXT("Cycle=%s | Bridge=%s | CAP=%s | Classes=%d/%d | Gate=%s | Actors=%d | Valid=%s"),
        *LastBuildCycle,
        bBridgeValidated ? TEXT("OK") : TEXT("FAIL"),
        bCAPEnforcementApplied ? TEXT("OK") : TEXT("PENDING"),
        CoreClassesLoaded, TotalCoreClasses,
        bCompilationGatePassed ? TEXT("PASS") : TEXT("FAIL"),
        ActorCount,
        bIntegrationValid ? TEXT("PASS") : TEXT("FAIL")
    );
}
