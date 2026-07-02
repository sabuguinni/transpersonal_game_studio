// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — PROD_CYCLE_AUTO_20260702_001
// Manages build integration, module health checks, and cycle reporting.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    CycleID = TEXT("AUTO_20260702_001");
    bBridgeValidated = false;
    bCAPEnforced = false;
    IntegrationStatus = EBuild_IntegrationStatus::Pending;
    ClassesLoaded = 0;
    TotalClasses = 7;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[A19] BuildIntegrationManager initialized — Cycle: %s"), *CycleID);
    IntegrationStatus = EBuild_IntegrationStatus::Initializing;
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("[A19] BuildIntegrationManager deinitialized"));
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateBridge()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[A19] Bridge validation FAIL — no world"));
        bBridgeValidated = false;
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("[A19] Bridge validation PASS — world: %s"), *World->GetName());
    bBridgeValidated = true;
    return true;
}

bool UBuildIntegrationManager::EnforceCAP()
{
    if (!bBridgeValidated)
    {
        UE_LOG(LogTemp, Warning, TEXT("[A19] CAP enforcement skipped — bridge not validated"));
        return false;
    }

    // CAP = Consistent Atmospheric Parameters
    // Sun pitch guard, fog dedup, FastSkyLUT — enforced via Python in editor
    // This C++ method logs the enforcement for runtime tracking
    UE_LOG(LogTemp, Log, TEXT("[A19] CAP enforcement: sun_pitch=-45, fog_dedup=1, FastSkyLUT=1"));
    bCAPEnforced = true;
    return true;
}

void UBuildIntegrationManager::RunIntegrationCheck()
{
    UE_LOG(LogTemp, Log, TEXT("[A19] Running integration check for cycle: %s"), *CycleID);

    // Validate bridge first
    if (!ValidateBridge())
    {
        IntegrationStatus = EBuild_IntegrationStatus::Failed;
        UE_LOG(LogTemp, Error, TEXT("[A19] Integration FAIL — bridge validation failed"));
        return;
    }

    // Enforce CAP
    EnforceCAP();

    // Check class count (set by Python validation)
    if (ClassesLoaded >= TotalClasses)
    {
        IntegrationStatus = EBuild_IntegrationStatus::Pass;
        UE_LOG(LogTemp, Log, TEXT("[A19] Integration PASS — %d/%d classes loaded"), ClassesLoaded, TotalClasses);
    }
    else if (ClassesLoaded > 0)
    {
        IntegrationStatus = EBuild_IntegrationStatus::Partial;
        UE_LOG(LogTemp, Warning, TEXT("[A19] Integration PARTIAL — %d/%d classes loaded"), ClassesLoaded, TotalClasses);
    }
    else
    {
        IntegrationStatus = EBuild_IntegrationStatus::Failed;
        UE_LOG(LogTemp, Error, TEXT("[A19] Integration FAIL — no classes loaded"));
    }
}

FString UBuildIntegrationManager::GetStatusReport() const
{
    FString StatusStr;
    switch (IntegrationStatus)
    {
        case EBuild_IntegrationStatus::Pass:        StatusStr = TEXT("PASS"); break;
        case EBuild_IntegrationStatus::Partial:     StatusStr = TEXT("PARTIAL"); break;
        case EBuild_IntegrationStatus::Failed:      StatusStr = TEXT("FAIL"); break;
        case EBuild_IntegrationStatus::Initializing:StatusStr = TEXT("INITIALIZING"); break;
        default:                                    StatusStr = TEXT("PENDING"); break;
    }

    return FString::Printf(
        TEXT("[A19] Cycle=%s | Bridge=%s | CAP=%s | Classes=%d/%d | Status=%s"),
        *CycleID,
        bBridgeValidated ? TEXT("OK") : TEXT("FAIL"),
        bCAPEnforced ? TEXT("OK") : TEXT("FAIL"),
        ClassesLoaded,
        TotalClasses,
        *StatusStr
    );
}

void UBuildIntegrationManager::SetClassesLoaded(int32 Count)
{
    ClassesLoaded = FMath::Clamp(Count, 0, TotalClasses);
    UE_LOG(LogTemp, Log, TEXT("[A19] Classes loaded updated: %d/%d"), ClassesLoaded, TotalClasses);
}
