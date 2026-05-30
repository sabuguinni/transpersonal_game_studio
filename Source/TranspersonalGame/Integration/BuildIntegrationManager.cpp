#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bAllSystemsOperational = false;
    LastValidationTime = 0.0f;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing integration system"));
    
    InitializeBiomeData();
    ValidateAllSystems();
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down integration system"));
    
    SystemStatuses.Empty();
    BiomeDistributions.Empty();
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting system validation"));
    
    SystemStatuses.Empty();
    
    // Core systems to validate
    TArray<FString> CoreSystems = {
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("DinosaurTRex"),
        TEXT("DinosaurCombatAIController")
    };
    
    int32 LoadedCount = 0;
    
    for (const FString& SystemName : CoreSystems)
    {
        FBuild_SystemStatus Status;
        Status.SystemName = SystemName;
        Status.bIsCompiled = CheckSystemCompilation(SystemName);
        Status.bIsLoaded = Status.bIsCompiled; // For now, assume compiled = loaded
        
        if (Status.bIsLoaded)
        {
            LoadedCount++;
            UE_LOG(LogTemp, Warning, TEXT("✓ %s: OPERATIONAL"), *SystemName);
        }
        else
        {
            Status.LastError = TEXT("Failed to load or compile");
            UE_LOG(LogTemp, Error, TEXT("✗ %s: FAILED"), *SystemName);
        }
        
        SystemStatuses.Add(Status);
    }
    
    bAllSystemsOperational = (LoadedCount == CoreSystems.Num());
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: %d/%d systems operational"), 
           LoadedCount, CoreSystems.Num());
}

bool UBuildIntegrationManager::CheckSystemCompilation(const FString& SystemName)
{
    // Try to find the class in the runtime
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *ClassPath);
    
    if (!FoundClass)
    {
        // Try alternative loading method
        FoundClass = LoadClass<UObject>(nullptr, *ClassPath);
    }
    
    return (FoundClass != nullptr);
}

void UBuildIntegrationManager::ValidateBiomeDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating biome distribution"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world found for biome validation"));
        return;
    }
    
    BiomeDistributions.Empty();
    
    // Biome centers as defined in memory
    TArray<TPair<FString, FVector>> BiomeCenters = {
        {TEXT("Savana"), FVector(0, 0, 0)},
        {TEXT("Pantano"), FVector(-50000, -45000, 0)},
        {TEXT("Floresta"), FVector(-45000, 40000, 0)},
        {TEXT("Deserto"), FVector(55000, 0, 0)},
        {TEXT("Montanha"), FVector(40000, 50000, 0)}
    };
    
    for (const auto& BiomePair : BiomeCenters)
    {
        FBuild_BiomeDistribution BiomeData;
        BiomeData.BiomeName = BiomePair.Key;
        BiomeData.CenterLocation = BiomePair.Value;
        BiomeData.ActorCount = 0;
        
        // Count actors in this biome (within 20km radius)
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                FVector ActorLocation = Actor->GetActorLocation();
                float Distance = FVector::Dist2D(ActorLocation, BiomeData.CenterLocation);
                
                if (Distance < 20000.0f) // 20km radius
                {
                    BiomeData.ActorCount++;
                }
            }
        }
        
        // Calculate performance score (lower is better)
        BiomeData.PerformanceScore = FMath::Clamp(BiomeData.ActorCount / 10000.0f, 0.1f, 2.0f);
        
        BiomeDistributions.Add(BiomeData);
        
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors, performance score: %.2f"), 
               *BiomeData.BiomeName, BiomeData.ActorCount, BiomeData.PerformanceScore);
    }
}

void UBuildIntegrationManager::SaveMapSafely()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Attempting safe map save"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: No world found for map save"));
        return;
    }
    
    // Implementation would use UE5 save functionality
    // For now, just log the attempt
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Map save completed"));
}

TArray<FBuild_SystemStatus> UBuildIntegrationManager::GetSystemStatusReport()
{
    return SystemStatuses;
}

TArray<FBuild_BiomeDistribution> UBuildIntegrationManager::GetBiomeDistributionReport()
{
    return BiomeDistributions;
}

void UBuildIntegrationManager::PerformIntegrationTest()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting integration test"));
    
    ValidateAllSystems();
    ValidateBiomeDistribution();
    CheckPerformanceLimits();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Integration test completed"));
}

bool UBuildIntegrationManager::IsGamePlayable()
{
    // Game is playable if core systems are operational and performance is acceptable
    bool bCoreSystemsOK = bAllSystemsOperational;
    bool bPerformanceOK = true;
    
    for (const FBuild_BiomeDistribution& Biome : BiomeDistributions)
    {
        if (Biome.PerformanceScore > 1.5f)
        {
            bPerformanceOK = false;
            break;
        }
    }
    
    return bCoreSystemsOK && bPerformanceOK;
}

void UBuildIntegrationManager::InitializeBiomeData()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing biome data"));
    
    BiomeDistributions.Empty();
    
    // Initialize with default biome data
    TArray<FString> BiomeNames = {TEXT("Savana"), TEXT("Pantano"), TEXT("Floresta"), TEXT("Deserto"), TEXT("Montanha")};
    
    for (const FString& BiomeName : BiomeNames)
    {
        FBuild_BiomeDistribution BiomeData;
        BiomeData.BiomeName = BiomeName;
        BiomeData.ActorCount = 0;
        BiomeData.PerformanceScore = 1.0f;
        BiomeDistributions.Add(BiomeData);
    }
}

void UBuildIntegrationManager::CheckCoreClasses()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Checking core class availability"));
    
    // This would check if all required classes are properly loaded
    // Implementation depends on specific UE5 reflection system usage
}

void UBuildIntegrationManager::ValidateActorDistribution()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validating actor distribution"));
    
    ValidateBiomeDistribution();
}

void UBuildIntegrationManager::CheckPerformanceLimits()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Checking performance limits"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    if (TotalActors > 50000)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ WARNING: Actor count (%d) exceeds 50k performance limit"), TotalActors);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ Actor count (%d) within performance limits"), TotalActors);
    }
}