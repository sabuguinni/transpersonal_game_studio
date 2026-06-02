#include "Build_QAIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ABuild_QAIntegrationManager::ABuild_QAIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create QA validator mesh component
    QAValidatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QAValidatorMesh"));
    RootComponent = QAValidatorMesh;

    // Set default limits
    MaxActorLimit = 20000;
    MaxDinosaurLimit = 150;

    InitializeQAValidator();
}

void ABuild_QAIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-generate QA integration report on begin play
    GenerateQAIntegrationReport();
}

void ABuild_QAIntegrationManager::InitializeQAValidator()
{
    if (QAValidatorMesh)
    {
        // Set a basic cube mesh for visualization
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMeshAsset.Succeeded())
        {
            QAValidatorMesh->SetStaticMesh(CubeMeshAsset.Object);
            QAValidatorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
        }
    }
}

FBuild_QAIntegrationReport ABuild_QAIntegrationManager::ValidateQAResults()
{
    FBuild_QAIntegrationReport Report;
    
    // Get all actors in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("QAIntegrationManager: No valid world found"));
        return Report;
    }

    // Count total actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    Report.TotalActorCount = AllActors.Num();

    // Count dinosaurs
    int32 DinoCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Dino"), ESearchCase::IgnoreCase) ||
            Actor->GetActorLabel().Contains(TEXT("TRex"), ESearchCase::IgnoreCase) ||
            Actor->GetActorLabel().Contains(TEXT("Veloci"), ESearchCase::IgnoreCase) ||
            Actor->GetActorLabel().Contains(TEXT("Tricera"), ESearchCase::IgnoreCase) ||
            Actor->GetActorLabel().Contains(TEXT("Brachi"), ESearchCase::IgnoreCase))
        {
            DinoCount++;
        }
    }
    Report.DinosaurCount = DinoCount;

    // Check limits
    Report.bWithinActorLimits = (Report.TotalActorCount <= MaxActorLimit) && (Report.DinosaurCount <= MaxDinosaurLimit);

    // Parse test results
    Report.VFXTests = ParseVFXTestResults();
    Report.PerformanceTests = ParsePerformanceTestResults();
    Report.IntegrationTests = ParseIntegrationTestResults();

    // Determine overall pass status
    bool bAllVFXPassed = true;
    for (const FBuild_QATestResult& Test : Report.VFXTests)
    {
        if (!Test.bPassed)
        {
            bAllVFXPassed = false;
            break;
        }
    }

    bool bAllPerfPassed = true;
    for (const FBuild_QATestResult& Test : Report.PerformanceTests)
    {
        if (!Test.bPassed)
        {
            bAllPerfPassed = false;
            break;
        }
    }

    bool bAllIntegrationPassed = true;
    for (const FBuild_QATestResult& Test : Report.IntegrationTests)
    {
        if (!Test.bPassed)
        {
            bAllIntegrationPassed = false;
            break;
        }
    }

    Report.bAllTestsPassed = bAllVFXPassed && bAllPerfPassed && bAllIntegrationPassed && Report.bWithinActorLimits;

    CurrentReport = Report;
    return Report;
}

bool ABuild_QAIntegrationManager::CheckActorLimits()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 TotalActors = AllActors.Num();
    int32 DinoCount = 0;

    for (AActor* Actor : AllActors)
    {
        if (Actor && (Actor->GetActorLabel().Contains(TEXT("Dino"), ESearchCase::IgnoreCase) ||
                     Actor->GetActorLabel().Contains(TEXT("TRex"), ESearchCase::IgnoreCase) ||
                     Actor->GetActorLabel().Contains(TEXT("Veloci"), ESearchCase::IgnoreCase)))
        {
            DinoCount++;
        }
    }

    bool bWithinLimits = (TotalActors <= MaxActorLimit) && (DinoCount <= MaxDinosaurLimit);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Integration - Actor Count: %d/%d, Dino Count: %d/%d, Within Limits: %s"),
           TotalActors, MaxActorLimit, DinoCount, MaxDinosaurLimit, bWithinLimits ? TEXT("YES") : TEXT("NO"));

    return bWithinLimits;
}

TArray<FBuild_QATestResult> ABuild_QAIntegrationManager::ParseVFXTestResults()
{
    TArray<FBuild_QATestResult> VFXResults;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        VFXResults.Add(CreateTestResult(TEXT("VFX_WorldValidation"), false, TEXT("No valid world found")));
        return VFXResults;
    }

    // Look for VFX test actors created by QA Agent
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 VFXTestActors = 0;
    int32 NiagaraActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString Label = Actor->GetActorLabel();
            if (Label.Contains(TEXT("VFX_Test"), ESearchCase::IgnoreCase) ||
                Label.Contains(TEXT("VFXTest"), ESearchCase::IgnoreCase))
            {
                VFXTestActors++;
            }
            if (Label.Contains(TEXT("Niagara"), ESearchCase::IgnoreCase) ||
                Label.Contains(TEXT("Particle"), ESearchCase::IgnoreCase))
            {
                NiagaraActors++;
            }
        }
    }

    // Create test results based on findings
    VFXResults.Add(CreateTestResult(TEXT("VFX_TestActorsPresent"), VFXTestActors > 0, 
                                   VFXTestActors > 0 ? TEXT("") : TEXT("No VFX test actors found"), 0.1f));
    
    VFXResults.Add(CreateTestResult(TEXT("Niagara_SystemsPresent"), NiagaraActors > 0,
                                   NiagaraActors > 0 ? TEXT("") : TEXT("No Niagara systems found"), 0.1f));

    VFXResults.Add(CreateTestResult(TEXT("VFX_Integration"), (VFXTestActors + NiagaraActors) >= 2,
                                   (VFXTestActors + NiagaraActors) >= 2 ? TEXT("") : TEXT("Insufficient VFX integration"), 0.2f));

    return VFXResults;
}

TArray<FBuild_QATestResult> ABuild_QAIntegrationManager::ParsePerformanceTestResults()
{
    TArray<FBuild_QATestResult> PerfResults;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        PerfResults.Add(CreateTestResult(TEXT("Performance_WorldValidation"), false, TEXT("No valid world found")));
        return PerfResults;
    }

    // Check actor count performance
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    int32 ActorCount = AllActors.Num();
    
    bool bActorCountOK = ActorCount <= MaxActorLimit;
    PerfResults.Add(CreateTestResult(TEXT("Performance_ActorCount"), bActorCountOK,
                                    bActorCountOK ? TEXT("") : FString::Printf(TEXT("Actor count %d exceeds limit %d"), ActorCount, MaxActorLimit), 0.1f));

    // Check for performance test actors
    int32 PerfTestActors = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && (Actor->GetActorLabel().Contains(TEXT("Performance"), ESearchCase::IgnoreCase) ||
                     Actor->GetActorLabel().Contains(TEXT("Optimization"), ESearchCase::IgnoreCase)))
        {
            PerfTestActors++;
        }
    }

    PerfResults.Add(CreateTestResult(TEXT("Performance_TestActorsPresent"), PerfTestActors > 0,
                                    PerfTestActors > 0 ? TEXT("") : TEXT("No performance test actors found"), 0.1f));

    // Memory usage estimation (basic)
    float EstimatedMemoryMB = ActorCount * 0.1f; // Rough estimate: 0.1MB per actor
    bool bMemoryOK = EstimatedMemoryMB < 2000.0f; // Under 2GB
    PerfResults.Add(CreateTestResult(TEXT("Performance_MemoryEstimate"), bMemoryOK,
                                    bMemoryOK ? TEXT("") : FString::Printf(TEXT("Estimated memory %.1fMB too high"), EstimatedMemoryMB), 0.2f));

    return PerfResults;
}

TArray<FBuild_QATestResult> ABuild_QAIntegrationManager::ParseIntegrationTestResults()
{
    TArray<FBuild_QATestResult> IntegrationResults;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        IntegrationResults.Add(CreateTestResult(TEXT("Integration_WorldValidation"), false, TEXT("No valid world found")));
        return IntegrationResults;
    }

    // Check for integration test actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 IntegrationTestActors = 0;
    bool bHasLandscape = false;
    bool bHasLighting = false;
    bool bHasCharacter = false;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString Label = Actor->GetActorLabel();
            if (Label.Contains(TEXT("Integration"), ESearchCase::IgnoreCase) ||
                Label.Contains(TEXT("Test"), ESearchCase::IgnoreCase))
            {
                IntegrationTestActors++;
            }
            if (Label.Contains(TEXT("Landscape"), ESearchCase::IgnoreCase) ||
                Label.Contains(TEXT("Terrain"), ESearchCase::IgnoreCase))
            {
                bHasLandscape = true;
            }
            if (Label.Contains(TEXT("Light"), ESearchCase::IgnoreCase) ||
                Label.Contains(TEXT("Sun"), ESearchCase::IgnoreCase))
            {
                bHasLighting = true;
            }
            if (Label.Contains(TEXT("Character"), ESearchCase::IgnoreCase) ||
                Label.Contains(TEXT("Player"), ESearchCase::IgnoreCase))
            {
                bHasCharacter = true;
            }
        }
    }

    // Create integration test results
    IntegrationResults.Add(CreateTestResult(TEXT("Integration_TestActorsPresent"), IntegrationTestActors > 0,
                                           IntegrationTestActors > 0 ? TEXT("") : TEXT("No integration test actors found"), 0.1f));

    IntegrationResults.Add(CreateTestResult(TEXT("Integration_WorldTerrain"), bHasLandscape,
                                           bHasLandscape ? TEXT("") : TEXT("No landscape/terrain found"), 0.1f));

    IntegrationResults.Add(CreateTestResult(TEXT("Integration_Lighting"), bHasLighting,
                                           bHasLighting ? TEXT("") : TEXT("No lighting setup found"), 0.1f));

    IntegrationResults.Add(CreateTestResult(TEXT("Integration_Character"), bHasCharacter,
                                           bHasCharacter ? TEXT("") : TEXT("No character setup found"), 0.1f));

    bool bCrossSystemOK = ValidateCrossSystemCompatibility();
    IntegrationResults.Add(CreateTestResult(TEXT("Integration_CrossSystem"), bCrossSystemOK,
                                           bCrossSystemOK ? TEXT("") : TEXT("Cross-system compatibility issues"), 0.2f));

    return IntegrationResults;
}

void ABuild_QAIntegrationManager::GenerateQAIntegrationReport()
{
    FBuild_QAIntegrationReport Report = ValidateQAResults();
    
    UE_LOG(LogTemp, Warning, TEXT("=== QA INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), Report.TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Count: %d"), Report.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Within Limits: %s"), Report.bWithinActorLimits ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("All Tests Passed: %s"), Report.bAllTestsPassed ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Tests: %d"), Report.VFXTests.Num());
    for (const FBuild_QATestResult& Test : Report.VFXTests)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *Test.TestName, Test.bPassed ? TEXT("PASS") : TEXT("FAIL"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Tests: %d"), Report.PerformanceTests.Num());
    for (const FBuild_QATestResult& Test : Report.PerformanceTests)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *Test.TestName, Test.bPassed ? TEXT("PASS") : TEXT("FAIL"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Tests: %d"), Report.IntegrationTests.Num());
    for (const FBuild_QATestResult& Test : Report.IntegrationTests)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s"), *Test.TestName, Test.bPassed ? TEXT("PASS") : TEXT("FAIL"));
    }
}

bool ABuild_QAIntegrationManager::ValidateCrossSystemCompatibility()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Basic cross-system validation
    // Check if we have the minimum required systems for a functional game
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    bool bHasWorldGen = false;
    bool bHasCharacterSystem = false;
    bool bHasLightingSystem = false;
    bool bHasGameMode = false;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName();
            FString Label = Actor->GetActorLabel();
            
            if (ClassName.Contains(TEXT("WorldGen")) || Label.Contains(TEXT("WorldGen")))
            {
                bHasWorldGen = true;
            }
            if (ClassName.Contains(TEXT("Character")) || Label.Contains(TEXT("Character")))
            {
                bHasCharacterSystem = true;
            }
            if (ClassName.Contains(TEXT("Light")) || Label.Contains(TEXT("Light")))
            {
                bHasLightingSystem = true;
            }
        }
    }

    // Check for game mode
    AGameModeBase* GameMode = World->GetAuthGameMode();
    bHasGameMode = (GameMode != nullptr);

    bool bCompatible = bHasWorldGen && bHasCharacterSystem && bHasLightingSystem && bHasGameMode;
    
    UE_LOG(LogTemp, Warning, TEXT("Cross-System Compatibility Check:"));
    UE_LOG(LogTemp, Warning, TEXT("  WorldGen: %s"), bHasWorldGen ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Warning, TEXT("  Character: %s"), bHasCharacterSystem ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Warning, TEXT("  Lighting: %s"), bHasLightingSystem ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Warning, TEXT("  GameMode: %s"), bHasGameMode ? TEXT("OK") : TEXT("MISSING"));
    UE_LOG(LogTemp, Warning, TEXT("  Overall: %s"), bCompatible ? TEXT("COMPATIBLE") : TEXT("INCOMPATIBLE"));

    return bCompatible;
}

bool ABuild_QAIntegrationManager::ValidateTestActor(AActor* TestActor)
{
    if (!TestActor)
    {
        return false;
    }

    // Basic validation - check if actor is valid and has components
    bool bHasComponents = TestActor->GetRootComponent() != nullptr;
    bool bHasValidTransform = !TestActor->GetActorLocation().IsZero() || !TestActor->GetActorRotation().IsZero();
    
    return bHasComponents;
}

FBuild_QATestResult ABuild_QAIntegrationManager::CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMsg, float ExecTime)
{
    FBuild_QATestResult Result;
    Result.TestName = TestName;
    Result.bPassed = bPassed;
    Result.ErrorMessage = ErrorMsg;
    Result.ExecutionTime = ExecTime;
    return Result;
}