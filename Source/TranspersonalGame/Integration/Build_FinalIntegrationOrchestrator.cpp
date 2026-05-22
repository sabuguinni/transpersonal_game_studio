#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameModeBase.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Static member definitions
const TMap<FString, FVector> UBuild_FinalIntegrationOrchestrator::BiomeCoordinates = {
    {TEXT("Savana"), FVector(0.0f, 0.0f, 0.0f)},
    {TEXT("Floresta"), FVector(-45000.0f, 40000.0f, 0.0f)},
    {TEXT("Pantano"), FVector(-50000.0f, -45000.0f, 0.0f)},
    {TEXT("Deserto"), FVector(55000.0f, 0.0f, 0.0f)},
    {TEXT("Montanha"), FVector(40000.0f, 50000.0f, 0.0f)}
};

const TArray<FString> UBuild_FinalIntegrationOrchestrator::CriticalModules = {
    TEXT("TranspersonalCharacter"),
    TEXT("TranspersonalGameState"),
    TEXT("PCGWorldGenerator"),
    TEXT("FoliageManager"),
    TEXT("CrowdSimulationManager"),
    TEXT("ProceduralWorldManager"),
    TEXT("BuildIntegrationManager")
};

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    bLastValidationPassed = false;
    LastValidationTime = FDateTime::Now();
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator initialized"));
    
    // Perform initial health check
    LastHealthReport = PerformSystemHealthCheck();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator shutting down"));
    Super::Deinitialize();
}

FBuild_SystemHealthReport UBuild_FinalIntegrationOrchestrator::PerformSystemHealthCheck()
{
    FBuild_SystemHealthReport Report;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world for health check"));
        return Report;
    }

    // Count total actors
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    Report.TotalActors = ActorCount;

    // Check biome population
    BiomeStatus = CheckBiomePopulation();
    Report.PopulatedBiomes = 0;
    for (const FBuild_BiomePopulation& Biome : BiomeStatus)
    {
        if (Biome.bIsPopulated)
        {
            Report.PopulatedBiomes++;
        }
    }

    // Test module compilation
    Report.bCompilationSuccess = TestModuleCompilation(Report.WorkingModules, Report.FailedModules);

    // Calculate integration score
    Report.IntegrationScore = CalculateIntegrationScore();
    
    // Update timestamp
    Report.LastValidationTime = FDateTime::Now().ToString();
    LastValidationTime = FDateTime::Now();

    // Store report
    LastHealthReport = Report;
    
    UE_LOG(LogTemp, Warning, TEXT("System Health Check: %d actors, %d biomes, score %d/100"), 
           Report.TotalActors, Report.PopulatedBiomes, Report.IntegrationScore);

    return Report;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateCrossSystemIntegrations()
{
    bool bAllValid = true;

    // Validate player systems
    if (!ValidatePlayerSystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Player system validation failed"));
        bAllValid = false;
    }

    // Validate dinosaur systems
    if (!ValidateDinosaurSystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Dinosaur system validation failed"));
        bAllValid = false;
    }

    // Validate environment systems
    if (!ValidateEnvironmentSystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Environment system validation failed"));
        bAllValid = false;
    }

    // Validate physics systems
    if (!ValidatePhysicsSystems())
    {
        UE_LOG(LogTemp, Error, TEXT("Physics system validation failed"));
        bAllValid = false;
    }

    bLastValidationPassed = bAllValid;
    return bAllValid;
}

TArray<FBuild_BiomePopulation> UBuild_FinalIntegrationOrchestrator::CheckBiomePopulation()
{
    TArray<FBuild_BiomePopulation> BiomeData;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return BiomeData;
    }

    // Initialize biome data
    for (const auto& BiomePair : BiomeCoordinates)
    {
        FBuild_BiomePopulation BiomeInfo;
        BiomeInfo.BiomeName = BiomePair.Key;
        BiomeInfo.CenterLocation = BiomePair.Value;
        BiomeInfo.ActorCount = 0;
        BiomeInfo.bIsPopulated = false;
        BiomeData.Add(BiomeInfo);
    }

    // Count actors in each biome
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }

        FVector ActorLocation = Actor->GetActorLocation();
        
        // Check which biome this actor belongs to
        for (int32 i = 0; i < BiomeData.Num(); i++)
        {
            FVector BiomeCenter = BiomeData[i].CenterLocation;
            float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
            
            // Consider actors within 10km of biome center
            if (Distance <= 10000.0f)
            {
                BiomeData[i].ActorCount++;
                break;
            }
        }
    }

    // Mark biomes as populated if they have enough actors
    for (int32 i = 0; i < BiomeData.Num(); i++)
    {
        BiomeData[i].bIsPopulated = (BiomeData[i].ActorCount >= 10);
    }

    return BiomeData;
}

bool UBuild_FinalIntegrationOrchestrator::TestModuleCompilation(TArray<FString>& OutWorkingModules, TArray<FString>& OutFailedModules)
{
    OutWorkingModules.Empty();
    OutFailedModules.Empty();

    bool bAllModulesWorking = true;

    for (const FString& ModuleName : CriticalModules)
    {
        // Try to find the class
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
        UClass* TestClass = FindObject<UClass>(ANY_PACKAGE, *ClassPath);
        
        if (TestClass)
        {
            OutWorkingModules.Add(ModuleName);
            UE_LOG(LogTemp, Log, TEXT("Module %s: WORKING"), *ModuleName);
        }
        else
        {
            OutFailedModules.Add(ModuleName);
            UE_LOG(LogTemp, Error, TEXT("Module %s: FAILED"), *ModuleName);
            bAllModulesWorking = false;
        }
    }

    return bAllModulesWorking;
}

int32 UBuild_FinalIntegrationOrchestrator::CalculateIntegrationScore()
{
    int32 Score = 0;

    // Base score from actor count (max 30 points)
    Score += FMath::Min(30, LastHealthReport.TotalActors / 10);

    // Biome population score (max 30 points)
    Score += LastHealthReport.PopulatedBiomes * 6;

    // Module compilation score (max 35 points)
    if (LastHealthReport.bCompilationSuccess)
    {
        Score += 35;
    }
    else
    {
        Score += (LastHealthReport.WorkingModules.Num() * 35) / CriticalModules.Num();
    }

    // Cross-system integration bonus (max 5 points)
    if (bLastValidationPassed)
    {
        Score += 5;
    }

    return FMath::Clamp(Score, 0, 100);
}

bool UBuild_FinalIntegrationOrchestrator::SaveMapState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot save map: No valid world"));
        return false;
    }

    // Use the safe map saving method
    FString MapPath = TEXT("/Game/Maps/MinPlayableMap");
    
    // TODO: Implement safe map saving
    // For now, just log the attempt
    UE_LOG(LogTemp, Warning, TEXT("Map save requested for: %s"), *MapPath);
    
    return true;
}

bool UBuild_FinalIntegrationOrchestrator::PerformCompilationGate()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION GATE ==="));
    
    // Perform final health check
    FBuild_SystemHealthReport FinalReport = PerformSystemHealthCheck();
    
    // Validate cross-system integrations
    bool bIntegrationsValid = ValidateCrossSystemIntegrations();
    
    // Check minimum requirements
    bool bPassesGate = true;
    
    if (FinalReport.TotalActors < 50)
    {
        UE_LOG(LogTemp, Error, TEXT("GATE FAIL: Insufficient actors (%d < 50)"), FinalReport.TotalActors);
        bPassesGate = false;
    }
    
    if (FinalReport.PopulatedBiomes < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("GATE FAIL: Insufficient biomes (%d < 2)"), FinalReport.PopulatedBiomes);
        bPassesGate = false;
    }
    
    if (FinalReport.FailedModules.Num() > 2)
    {
        UE_LOG(LogTemp, Error, TEXT("GATE FAIL: Too many failed modules (%d > 2)"), FinalReport.FailedModules.Num());
        bPassesGate = false;
    }
    
    if (!bIntegrationsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("GATE FAIL: Cross-system integration failures"));
        bPassesGate = false;
    }

    if (bPassesGate)
    {
        UE_LOG(LogTemp, Warning, TEXT("COMPILATION GATE: PASSED (Score: %d/100)"), FinalReport.IntegrationScore);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("COMPILATION GATE: FAILED (Score: %d/100)"), FinalReport.IntegrationScore);
    }

    return bPassesGate;
}

// Private validation helpers
bool UBuild_FinalIntegrationOrchestrator::ValidatePlayerSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Check for PlayerStart
    bool bHasPlayerStart = false;
    for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
    {
        bHasPlayerStart = true;
        break;
    }

    // Check for GameMode
    AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World);
    bool bHasGameMode = (GameMode != nullptr);

    return bHasPlayerStart && bHasGameMode;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateDinosaurSystems()
{
    // TODO: Implement dinosaur system validation
    // Check for dinosaur actors, AI components, etc.
    return true;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateEnvironmentSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Check for environment actors (static meshes, foliage, etc.)
    int32 EnvironmentActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            EnvironmentActorCount++;
        }
    }

    return EnvironmentActorCount > 10;
}

bool UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystems()
{
    // TODO: Implement physics system validation
    // Check for collision components, physics bodies, etc.
    return true;
}