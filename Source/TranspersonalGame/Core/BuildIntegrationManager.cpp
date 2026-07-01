// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — PROD_CYCLE_AUTO_20260701_007
// Manages build integration, module health checks, and cross-agent dependency validation.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f; // Check every 5 seconds

    bIntegrationHealthy = false;
    LastBuildCycle = TEXT("PROD_CYCLE_AUTO_20260701_007");
    ModulesLoaded = 0;
    TotalModules = 7;
    bAutoValidateOnStart = true;
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager: Starting cycle %s"), *LastBuildCycle);

    if (bAutoValidateOnStart)
    {
        ValidateAllModules();
    }
}

void ABuildIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Periodic health check — runs every 5 seconds
    TicksSinceLastCheck += DeltaTime;
    if (TicksSinceLastCheck >= 5.0f)
    {
        TicksSinceLastCheck = 0.0f;
        PerformHealthCheck();
    }
}

void ABuildIntegrationManager::ValidateAllModules()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("=== BUILD INTEGRATION VALIDATION START ==="));

    ModulesLoaded = 0;
    IntegrationErrors.Empty();

    // Validate core module list
    TArray<FString> RequiredModules = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };

    for (const FString& ModuleName : RequiredModules)
    {
        bool bModuleOk = CheckModuleExists(ModuleName);
        if (bModuleOk)
        {
            ModulesLoaded++;
            UE_LOG(LogBuildIntegration, Log, TEXT("  [OK] Module: %s"), *ModuleName);
        }
        else
        {
            FString Error = FString::Printf(TEXT("MISSING_MODULE: %s"), *ModuleName);
            IntegrationErrors.Add(Error);
            UE_LOG(LogBuildIntegration, Warning, TEXT("  [FAIL] Module: %s"), *ModuleName);
        }
    }

    bIntegrationHealthy = (IntegrationErrors.Num() == 0);

    UE_LOG(LogBuildIntegration, Log, TEXT("Modules loaded: %d/%d"), ModulesLoaded, TotalModules);
    UE_LOG(LogBuildIntegration, Log, TEXT("Integration healthy: %s"), bIntegrationHealthy ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogBuildIntegration, Log, TEXT("=== BUILD INTEGRATION VALIDATION END ==="));

    OnValidationComplete(bIntegrationHealthy, ModulesLoaded, TotalModules);
}

bool ABuildIntegrationManager::CheckModuleExists(const FString& ModuleName)
{
    // In a real build, this would check UClass registry
    // For now, we verify the class is registered in the UObject system
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
    UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *ModuleName);
    return (FoundClass != nullptr);
}

void ABuildIntegrationManager::PerformHealthCheck()
{
    if (!GetWorld())
    {
        return;
    }

    // Count actors in world as a basic health metric
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    int32 ActorCount = AllActors.Num();

    if (ActorCount < 5)
    {
        UE_LOG(LogBuildIntegration, Warning,
            TEXT("BuildIntegrationManager: Low actor count (%d) — world may not be fully loaded"), ActorCount);
    }
}

void ABuildIntegrationManager::OnValidationComplete(bool bHealthy, int32 LoadedCount, int32 TotalCount)
{
    // Blueprint-callable event — override in BP for UI feedback
    if (GEngine)
    {
        FString StatusMsg = FString::Printf(
            TEXT("Build Integration [%s]: %d/%d modules OK — %s"),
            *LastBuildCycle,
            LoadedCount,
            TotalCount,
            bHealthy ? TEXT("HEALTHY") : TEXT("ERRORS FOUND")
        );
        GEngine->AddOnScreenDebugMessage(-1, 10.0f,
            bHealthy ? FColor::Green : FColor::Red,
            StatusMsg);
    }
}

FString ABuildIntegrationManager::GetBuildStatus() const
{
    return FString::Printf(
        TEXT("Cycle=%s | Modules=%d/%d | Healthy=%s | Errors=%d"),
        *LastBuildCycle,
        ModulesLoaded,
        TotalModules,
        bIntegrationHealthy ? TEXT("YES") : TEXT("NO"),
        IntegrationErrors.Num()
    );
}

TArray<FString> ABuildIntegrationManager::GetIntegrationErrors() const
{
    return IntegrationErrors;
}
