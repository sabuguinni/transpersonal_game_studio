#include "IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/UObjectGlobals.h"

AIntegrationValidator::AIntegrationValidator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize default values
    MaxActorCount = 8000;
    MaxDinosaurCount = 150;
    MaxActorsPerBiome = 4000;
    bEnableContinuousMonitoring = true;
    MonitoringInterval = 30.0f;
    MonitoringTimer = 0.0f;
    
    // Initialize core systems and biome data
    InitializeCoreSystemsList();
    InitializeBiomeCoordinates();
}

void AIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Run initial validation
    UE_LOG(LogTemp, Warning, TEXT("IntegrationValidator: Starting initial validation"));
    RunFullIntegrationTest();
}

void AIntegrationValidator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableContinuousMonitoring)
    {
        MonitoringTimer += DeltaTime;
        if (MonitoringTimer >= MonitoringInterval)
        {
            MonitoringTimer = 0.0f;
            
            // Run quick validation checks
            ValidateActorLimits();
            ValidateBiomeDistribution();
        }
    }
}

void AIntegrationValidator::InitializeCoreSystemsList()
{
    CoreSystemClasses.Empty();
    CoreSystemClasses.Add(TEXT("TranspersonalCharacter"));
    CoreSystemClasses.Add(TEXT("TranspersonalGameState"));
    CoreSystemClasses.Add(TEXT("BuildIntegrationManager"));
    CoreSystemClasses.Add(TEXT("PCGWorldGenerator"));
    CoreSystemClasses.Add(TEXT("FoliageManager"));
    CoreSystemClasses.Add(TEXT("CrowdSimulationManager"));
}

void AIntegrationValidator::InitializeBiomeCoordinates()
{
    BiomeCenters.Empty();
    BiomeCenters.Add(FVector2D(0.0f, 0.0f));           // Savana
    BiomeCenters.Add(FVector2D(-50000.0f, -45000.0f)); // Pantano
    BiomeCenters.Add(FVector2D(-45000.0f, 40000.0f));  // Floresta
    BiomeCenters.Add(FVector2D(55000.0f, 0.0f));       // Deserto
    BiomeCenters.Add(FVector2D(40000.0f, 50000.0f));   // Montanha
}

bool AIntegrationValidator::ValidateCoreSystemsLoaded()
{
    bool bAllSystemsLoaded = true;
    
    for (const FString& ClassName : CoreSystemClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LogValidationResult(ClassName, true, TEXT("Class loaded successfully"));
        }
        else
        {
            LogValidationResult(ClassName, false, TEXT("Class failed to load"));
            bAllSystemsLoaded = false;
        }
    }
    
    return bAllSystemsLoaded;
}

bool AIntegrationValidator::ValidateActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count all actors
    CurrentActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        CurrentActorCount++;
    }
    
    // Count dinosaurs
    CurrentDinosaurCount = CountDinosaurActors();
    
    // Check limits
    bool bActorLimitOK = CurrentActorCount <= MaxActorCount;
    bool bDinoLimitOK = CurrentDinosaurCount <= MaxDinosaurCount;
    
    LogValidationResult(TEXT("ActorLimit"), bActorLimitOK, 
        FString::Printf(TEXT("Current: %d, Max: %d"), CurrentActorCount, MaxActorCount));
    
    LogValidationResult(TEXT("DinosaurLimit"), bDinoLimitOK,
        FString::Printf(TEXT("Current: %d, Max: %d"), CurrentDinosaurCount, MaxDinosaurCount));
    
    return bActorLimitOK && bDinoLimitOK;
}

bool AIntegrationValidator::ValidateBiomeDistribution()
{
    bool bDistributionOK = true;
    
    for (int32 i = 0; i < BiomeCenters.Num(); i++)
    {
        int32 ActorsInBiome = CountActorsInBiome(BiomeCenters[i]);
        bool bBiomeOK = ActorsInBiome <= MaxActorsPerBiome;
        
        if (!bBiomeOK)
        {
            bDistributionOK = false;
        }
        
        FString BiomeName = FString::Printf(TEXT("Biome_%d"), i);
        LogValidationResult(BiomeName, bBiomeOK,
            FString::Printf(TEXT("Actors: %d, Max: %d"), ActorsInBiome, MaxActorsPerBiome));
    }
    
    return bDistributionOK;
}

bool AIntegrationValidator::ValidateCrossSystemIntegration()
{
    // Test basic cross-system compatibility
    bool bIntegrationOK = true;
    
    // Check if PlayerStart exists (basic level setup)
    UWorld* World = GetWorld();
    if (World)
    {
        bool bPlayerStartFound = false;
        for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
        {
            bPlayerStartFound = true;
            break;
        }
        
        LogValidationResult(TEXT("PlayerStart"), bPlayerStartFound, TEXT("Required for gameplay"));
        if (!bPlayerStartFound)
        {
            bIntegrationOK = false;
        }
    }
    
    return bIntegrationOK;
}

void AIntegrationValidator::RunFullIntegrationTest()
{
    UE_LOG(LogTemp, Warning, TEXT("IntegrationValidator: Running full integration test"));
    
    bool bCoreSystemsOK = ValidateCoreSystemsLoaded();
    bool bActorLimitsOK = ValidateActorLimits();
    bool bBiomeDistributionOK = ValidateBiomeDistribution();
    bool bCrossSystemOK = ValidateCrossSystemIntegration();
    
    bLastValidationPassed = bCoreSystemsOK && bActorLimitsOK && bBiomeDistributionOK && bCrossSystemOK;
    
    FString OverallResult = bLastValidationPassed ? TEXT("PASS") : TEXT("FAIL");
    UE_LOG(LogTemp, Warning, TEXT("IntegrationValidator: Full test result: %s"), *OverallResult);
    
    // Generate and log health report
    FString HealthReport = GenerateHealthReport();
    UE_LOG(LogTemp, Warning, TEXT("Health Report:\n%s"), *HealthReport);
}

FString AIntegrationValidator::GenerateHealthReport()
{
    FString Report;
    Report += TEXT("=== INTEGRATION HEALTH REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Status: %s\n"), bLastValidationPassed ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Total Actors: %d / %d\n"), CurrentActorCount, MaxActorCount);
    Report += FString::Printf(TEXT("Dinosaurs: %d / %d\n"), CurrentDinosaurCount, MaxDinosaurCount);
    Report += TEXT("Core Systems: ");
    
    int32 LoadedSystems = 0;
    for (const FString& ClassName : CoreSystemClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            LoadedSystems++;
        }
    }
    
    Report += FString::Printf(TEXT("%d / %d\n"), LoadedSystems, CoreSystemClasses.Num());
    Report += TEXT("Biome Distribution:\n");
    
    for (int32 i = 0; i < BiomeCenters.Num(); i++)
    {
        int32 ActorsInBiome = CountActorsInBiome(BiomeCenters[i]);
        Report += FString::Printf(TEXT("  Biome %d: %d actors\n"), i, ActorsInBiome);
    }
    
    return Report;
}

int32 AIntegrationValidator::CountActorsInBiome(const FVector2D& BiomeCenter, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FVector ActorLocation = Actor->GetActorLocation();
            FVector2D ActorLocation2D(ActorLocation.X, ActorLocation.Y);
            
            float Distance = FVector2D::Distance(ActorLocation2D, BiomeCenter);
            if (Distance <= Radius)
            {
                Count++;
            }
        }
    }
    
    return Count;
}

int32 AIntegrationValidator::CountDinosaurActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    TArray<FString> DinosaurKeywords = {
        TEXT("trex"), TEXT("veloci"), TEXT("tricera"), TEXT("brachi"), 
        TEXT("ankylo"), TEXT("parasauro"), TEXT("pachy"), TEXT("proto"), TEXT("tsinta")
    };
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            for (const FString& Keyword : DinosaurKeywords)
            {
                if (ActorName.Contains(Keyword))
                {
                    Count++;
                    break;
                }
            }
        }
    }
    
    return Count;
}

void AIntegrationValidator::LogValidationResult(const FString& TestName, bool bPassed, const FString& Details)
{
    FString Status = bPassed ? TEXT("PASS") : TEXT("FAIL");
    FString LogMessage = FString::Printf(TEXT("Validation [%s]: %s"), *TestName, *Status);
    
    if (!Details.IsEmpty())
    {
        LogMessage += FString::Printf(TEXT(" - %s"), *Details);
    }
    
    if (bPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
    }
}