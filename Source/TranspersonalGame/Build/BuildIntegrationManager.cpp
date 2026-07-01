// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — Cycle AUTO_20260701_006
// Concrete implementation of build integration, module health checks, and cycle reporting.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;

    CycleID = TEXT("AUTO_20260701_006");
    bIntegrationPassed = false;
    TotalActorsInMap = 0;
    LoadedClassCount = 0;
    FailedClassCount = 0;
    IntegrationHealthScore = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationChecks();
}

// ─────────────────────────────────────────────────────────────────────────────
// Core Integration Methods
// ─────────────────────────────────────────────────────────────────────────────

void ABuildIntegrationManager::RunIntegrationChecks()
{
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] === Cycle %s Integration Checks ==="), *CycleID);

    ValidateModuleHealth();
    ValidateMapActors();
    ComputeHealthScore();
    WriteIntegrationReport();

    bIntegrationPassed = (IntegrationHealthScore >= 4);

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Health Score: %d/6 — %s"),
        IntegrationHealthScore,
        bIntegrationPassed ? TEXT("INTEGRATION PASS") : TEXT("INTEGRATION FAIL"));
}

void ABuildIntegrationManager::ValidateModuleHealth()
{
    // List of expected core module classes
    static const TArray<FString> ExpectedClasses = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager"),
    };

    LoadedClassCount = 0;
    FailedClassCount = 0;
    FailedClasses.Empty();

    for (const FString& ClassName : ExpectedClasses)
    {
        // Attempt to find the class by name in the TranspersonalGame module
        FString FullPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);

        if (FoundClass)
        {
            LoadedClassCount++;
            UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager]   OK: %s"), *ClassName);
        }
        else
        {
            FailedClassCount++;
            FailedClasses.Add(ClassName);
            UE_LOG(LogTemp, Warning, TEXT("[BuildIntegrationManager]   FAIL: %s not found"), *ClassName);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Module health: %d/%d classes loaded"),
        LoadedClassCount, ExpectedClasses.Num());
}

void ABuildIntegrationManager::ValidateMapActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildIntegrationManager] No world found!"));
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorsInMap = AllActors.Num();

    // Check for critical actor types
    bool bHasPlayerStart = false;
    bool bHasDirectionalLight = false;
    bool bHasSkyAtmosphere = false;
    bool bHasFog = false;
    bool bHasSkyLight = false;
    int32 StaticMeshCount = 0;

    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;

        FString ClassName = Actor->GetClass()->GetName();

        if (ClassName.Contains(TEXT("PlayerStart")))       bHasPlayerStart = true;
        if (ClassName.Contains(TEXT("DirectionalLight")))  bHasDirectionalLight = true;
        if (ClassName.Contains(TEXT("SkyAtmosphere")))     bHasSkyAtmosphere = true;
        if (ClassName.Contains(TEXT("ExponentialHeightFog"))) bHasFog = true;
        if (ClassName.Contains(TEXT("SkyLight")))          bHasSkyLight = true;
        if (ClassName.Contains(TEXT("StaticMeshActor")))   StaticMeshCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Map actors: %d total"), TotalActorsInMap);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager]   PlayerStart:      %s"), bHasPlayerStart ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager]   DirectionalLight: %s"), bHasDirectionalLight ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager]   SkyAtmosphere:    %s"), bHasSkyAtmosphere ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager]   ExponentialFog:   %s"), bHasFog ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager]   SkyLight:         %s"), bHasSkyLight ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager]   StaticMeshActors: %d"), StaticMeshCount);

    // Store for health score
    MapChecks.Empty();
    MapChecks.Add(TEXT("PlayerStart"),      bHasPlayerStart);
    MapChecks.Add(TEXT("DirectionalLight"), bHasDirectionalLight);
    MapChecks.Add(TEXT("SkyAtmosphere"),    bHasSkyAtmosphere);
    MapChecks.Add(TEXT("ExponentialFog"),   bHasFog);
    MapChecks.Add(TEXT("SkyLight"),         bHasSkyLight);
    MapChecks.Add(TEXT("StaticMeshActors"), StaticMeshCount >= 5);
}

void ABuildIntegrationManager::ComputeHealthScore()
{
    IntegrationHealthScore = 0;
    for (const auto& Check : MapChecks)
    {
        if (Check.Value)
        {
            IntegrationHealthScore++;
        }
    }
}

void ABuildIntegrationManager::WriteIntegrationReport()
{
    FString ReportPath = FPaths::ProjectSavedDir() / TEXT("Logs") / TEXT("BuildIntegrationReport_AUTO006.txt");

    FString Report;
    Report += FString::Printf(TEXT("=== BuildIntegrationManager Report ===\n"));
    Report += FString::Printf(TEXT("Cycle: %s\n"), *CycleID);
    Report += FString::Printf(TEXT("Total actors in map: %d\n"), TotalActorsInMap);
    Report += FString::Printf(TEXT("Classes loaded: %d\n"), LoadedClassCount);
    Report += FString::Printf(TEXT("Classes failed: %d\n"), FailedClassCount);
    Report += FString::Printf(TEXT("Health score: %d/6\n"), IntegrationHealthScore);
    Report += FString::Printf(TEXT("Integration passed: %s\n"), bIntegrationPassed ? TEXT("YES") : TEXT("NO"));

    if (FailedClasses.Num() > 0)
    {
        Report += TEXT("Failed classes:\n");
        for (const FString& FC : FailedClasses)
        {
            Report += FString::Printf(TEXT("  - %s\n"), *FC);
        }
    }

    FFileHelper::SaveStringToFile(Report, *ReportPath);
    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Report written to: %s"), *ReportPath);
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint-callable helpers
// ─────────────────────────────────────────────────────────────────────────────

bool ABuildIntegrationManager::IsIntegrationHealthy() const
{
    return bIntegrationPassed;
}

int32 ABuildIntegrationManager::GetHealthScore() const
{
    return IntegrationHealthScore;
}

FString ABuildIntegrationManager::GetCycleID() const
{
    return CycleID;
}

TArray<FString> ABuildIntegrationManager::GetFailedClasses() const
{
    return FailedClasses;
}
