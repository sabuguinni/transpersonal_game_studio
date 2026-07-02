// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — Cycle 003
// Manages build integration state, module dependency tracking, and playability validation.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "NavigationSystem.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default integration state
    bBridgeValidated = false;
    bCAPEnforced = false;
    IntegrationScore = 0.0f;
    LastCycleID = TEXT("PROD_CYCLE_AUTO_20260702_003");
    bPlayabilityVerified = false;
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationChecks();
}

void ABuildIntegrationManager::RunIntegrationChecks()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: No world available"));
        return;
    }

    int32 PassedChecks = 0;
    int32 TotalChecks = 0;

    // Check 1: DirectionalLight present
    TotalChecks++;
    TArray<AActor*> DirLights;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), DirLights);
    if (DirLights.Num() > 0)
    {
        PassedChecks++;
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: [PASS] DirectionalLight found (%d)"), DirLights.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: [FAIL] No DirectionalLight in level"));
    }

    // Check 2: ExponentialHeightFog present (exactly 1)
    TotalChecks++;
    TArray<AActor*> Fogs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), Fogs);
    if (Fogs.Num() == 1)
    {
        PassedChecks++;
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: [PASS] ExponentialHeightFog count = 1"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: [FAIL] ExponentialHeightFog count = %d (expected 1)"), Fogs.Num());
    }

    // Check 3: Navigation system available
    TotalChecks++;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        PassedChecks++;
        UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: [PASS] NavigationSystem available"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: [FAIL] NavigationSystem not found"));
    }

    // Compute score
    IntegrationScore = TotalChecks > 0 ? (float)PassedChecks / (float)TotalChecks * 100.0f : 0.0f;
    bPlayabilityVerified = (PassedChecks == TotalChecks);

    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Integration score = %.0f%% (%d/%d)"),
        IntegrationScore, PassedChecks, TotalChecks);
}

float ABuildIntegrationManager::GetIntegrationScore() const
{
    return IntegrationScore;
}

bool ABuildIntegrationManager::IsPlayabilityVerified() const
{
    return bPlayabilityVerified;
}

FString ABuildIntegrationManager::GetLastCycleID() const
{
    return LastCycleID;
}

void ABuildIntegrationManager::SetBridgeValidated(bool bValidated)
{
    bBridgeValidated = bValidated;
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: Bridge validated = %s"),
        bValidated ? TEXT("true") : TEXT("false"));
}

void ABuildIntegrationManager::SetCAPEnforced(bool bEnforced)
{
    bCAPEnforced = bEnforced;
    UE_LOG(LogTemp, Log, TEXT("BuildIntegrationManager: CAP enforced = %s"),
        bEnforced ? TEXT("true") : TEXT("false"));
}
