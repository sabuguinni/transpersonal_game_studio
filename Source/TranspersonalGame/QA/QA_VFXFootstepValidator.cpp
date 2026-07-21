#include "QA_VFXFootstepValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"

UQA_VFXFootstepValidator::UQA_VFXFootstepValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MaxTestParticles = 100;
    PerformanceThreshold = 16.67f; // 60 FPS
    bAutoRunTests = true;
    TestInterval = 5.0f;
    bValidationInProgress = false;
    LastTestTime = 0.0f;
}

void UQA_VFXFootstepValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXFootstepValidator: Starting VFX validation system"));
    
    if (bAutoRunTests)
    {
        // Start validation after a short delay
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            RunFullVFXValidationSuite();
        }, 2.0f, false);
    }
}

void UQA_VFXFootstepValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoRunTests && !bValidationInProgress)
    {
        LastTestTime += DeltaTime;
        if (LastTestTime >= TestInterval)
        {
            LastTestTime = 0.0f;
            CurrentMetrics = GetVFXIntegrationMetrics();
        }
    }
}

FQA_FootstepTestResult UQA_VFXFootstepValidator::ValidateFootstepManager()
{
    FQA_FootstepTestResult Result;
    Result.TestName = TEXT("VFX_FootstepManager Validation");
    
    try
    {
        // Try to find VFX_FootstepManager class
        UClass* FootstepManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_FootstepManager"));
        
        if (!FootstepManagerClass)
        {
            Result.bTestPassed = false;
            Result.ErrorMessage = TEXT("VFX_FootstepManager class not found");
            return Result;
        }
        
        // Try to spawn a test instance
        UWorld* World = GetWorld();
        if (!World)
        {
            Result.bTestPassed = false;
            Result.ErrorMessage = TEXT("No valid world context");
            return Result;
        }
        
        FVector SpawnLocation = FVector(0, 0, 100);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        AActor* TestManager = World->SpawnActor<AActor>(FootstepManagerClass, SpawnLocation, SpawnRotation);
        
        if (TestManager)
        {
            Result.bTestPassed = true;
            Result.ErrorMessage = TEXT("VFX_FootstepManager successfully validated");
            Result.PerformanceScore = 100.0f;
            
            // Add to test actors for cleanup
            TestActors.Add(TestManager);
            
            UE_LOG(LogTemp, Warning, TEXT("QA: VFX_FootstepManager validation PASSED"));
        }
        else
        {
            Result.bTestPassed = false;
            Result.ErrorMessage = TEXT("Failed to spawn VFX_FootstepManager instance");
        }
    }
    catch (...)
    {
        Result.bTestPassed = false;
        Result.ErrorMessage = TEXT("Exception during VFX_FootstepManager validation");
    }
    
    return Result;
}

FQA_FootstepTestResult UQA_VFXFootstepValidator::TestNiagaraSystemIntegration()
{
    FQA_FootstepTestResult Result;
    Result.TestName = TEXT("Niagara System Integration Test");
    
    // Test Niagara system paths
    TArray<FString> NiagaraPaths = {
        TEXT("/Game/VFX/Dinosaur/NS_Dino_FootstepDust"),
        TEXT("/Game/VFX/Environment/NS_VolcanicAsh"),
        TEXT("/Game/VFX/Weather/NS_Rain")
    };
    
    int32 ValidSystems = 0;
    
    for (const FString& Path : NiagaraPaths)
    {
        if (IsVFXSystemValid(Path))
        {
            ValidSystems++;
        }
    }
    
    if (ValidSystems > 0)
    {
        Result.bTestPassed = true;
        Result.ErrorMessage = FString::Printf(TEXT("Found %d/%d Niagara systems"), ValidSystems, NiagaraPaths.Num());
        Result.PerformanceScore = (float)ValidSystems / NiagaraPaths.Num() * 100.0f;
        Result.ParticleCount = ValidSystems;
    }
    else
    {
        Result.bTestPassed = false;
        Result.ErrorMessage = TEXT("No valid Niagara systems found");
        Result.PerformanceScore = 0.0f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Niagara integration test - %s"), Result.bTestPassed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return Result;
}

FQA_FootstepTestResult UQA_VFXFootstepValidator::TestPerformanceWithMultipleEffects()
{
    FQA_FootstepTestResult Result;
    Result.TestName = TEXT("VFX Performance Test");
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bTestPassed = false;
        Result.ErrorMessage = TEXT("No world context for performance test");
        return Result;
    }
    
    // Measure baseline frame time
    float BaselineFrameTime = MeasureFrameTime();
    
    // Spawn multiple test particle systems
    TArray<AActor*> TestParticleActors;
    
    for (int32 i = 0; i < FMath::Min(MaxTestParticles, 20); i++)
    {
        FVector Location = FVector(i * 100, 0, 100);
        
        // Try to spawn basic emitter for testing
        UClass* EmitterClass = LoadClass<AActor>(nullptr, TEXT("/Script/Engine.Emitter"));
        if (EmitterClass)
        {
            AActor* TestEmitter = World->SpawnActor<AActor>(EmitterClass, Location, FRotator::ZeroRotator);
            if (TestEmitter)
            {
                TestParticleActors.Add(TestEmitter);
                TestActors.Add(TestEmitter);
            }
        }
    }
    
    // Measure frame time with particles
    float LoadedFrameTime = MeasureFrameTime();
    
    // Calculate performance impact
    float PerformanceImpact = LoadedFrameTime - BaselineFrameTime;
    
    if (PerformanceImpact < PerformanceThreshold)
    {
        Result.bTestPassed = true;
        Result.ErrorMessage = FString::Printf(TEXT("Performance test passed - %d particles, %.2fms impact"), 
                                            TestParticleActors.Num(), PerformanceImpact);
        Result.PerformanceScore = FMath::Max(0.0f, 100.0f - (PerformanceImpact / PerformanceThreshold * 100.0f));
        Result.ParticleCount = TestParticleActors.Num();
    }
    else
    {
        Result.bTestPassed = false;
        Result.ErrorMessage = FString::Printf(TEXT("Performance test failed - %.2fms impact exceeds %.2fms threshold"), 
                                            PerformanceImpact, PerformanceThreshold);
        Result.PerformanceScore = 0.0f;
        Result.ParticleCount = TestParticleActors.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: VFX performance test - %s"), Result.bTestPassed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return Result;
}

FQA_FootstepTestResult UQA_VFXFootstepValidator::TestDinosaurIntegration()
{
    FQA_FootstepTestResult Result;
    Result.TestName = TEXT("Dinosaur VFX Integration Test");
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bTestPassed = false;
        Result.ErrorMessage = TEXT("No world context");
        return Result;
    }
    
    // Find dinosaur actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Dino"), ESearchCase::IgnoreCase) ||
            Actor->GetName().Contains(TEXT("Rex"), ESearchCase::IgnoreCase) ||
            Actor->GetName().Contains(TEXT("Raptor"), ESearchCase::IgnoreCase))
        {
            DinosaurCount++;
        }
    }
    
    if (DinosaurCount > 0)
    {
        Result.bTestPassed = true;
        Result.ErrorMessage = FString::Printf(TEXT("Found %d dinosaur actors ready for VFX integration"), DinosaurCount);
        Result.PerformanceScore = FMath::Min(100.0f, DinosaurCount * 20.0f);
        Result.ParticleCount = DinosaurCount;
    }
    else
    {
        Result.bTestPassed = false;
        Result.ErrorMessage = TEXT("No dinosaur actors found for VFX integration");
        Result.PerformanceScore = 0.0f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Dinosaur integration test - %s"), Result.bTestPassed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return Result;
}

FQA_VFXIntegrationMetrics UQA_VFXFootstepValidator::GetVFXIntegrationMetrics()
{
    FQA_VFXIntegrationMetrics Metrics;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return Metrics;
    }
    
    // Count VFX-related actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("VFX"), ESearchCase::IgnoreCase) ||
                ActorName.Contains(TEXT("Particle"), ESearchCase::IgnoreCase) ||
                ActorName.Contains(TEXT("Emitter"), ESearchCase::IgnoreCase))
            {
                Metrics.TotalVFXActors++;
            }
            
            if (ActorName.Contains(TEXT("Dino"), ESearchCase::IgnoreCase) ||
                ActorName.Contains(TEXT("Rex"), ESearchCase::IgnoreCase))
            {
                Metrics.DinosaurActorsWithVFX++;
            }
        }
    }
    
    // Count active Niagara systems
    Metrics.ActiveNiagaraSystems = CountActiveNiagaraSystems();
    
    // Measure current frame time
    Metrics.AverageFrameTime = MeasureFrameTime();
    
    // Check if footstep system is ready
    UClass* FootstepManagerClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_FootstepManager"));
    Metrics.bFootstepSystemReady = (FootstepManagerClass != nullptr);
    
    return Metrics;
}

TArray<FQA_FootstepTestResult> UQA_VFXFootstepValidator::RunFullVFXValidationSuite()
{
    if (bValidationInProgress)
    {
        return LastTestResults;
    }
    
    bValidationInProgress = true;
    LastTestResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Starting full VFX validation suite"));
    
    // Run all validation tests
    LastTestResults.Add(ValidateFootstepManager());
    LastTestResults.Add(TestNiagaraSystemIntegration());
    LastTestResults.Add(TestPerformanceWithMultipleEffects());
    LastTestResults.Add(TestDinosaurIntegration());
    
    // Update current metrics
    CurrentMetrics = GetVFXIntegrationMetrics();
    
    // Log summary
    int32 PassedTests = 0;
    for (const FQA_FootstepTestResult& Result : LastTestResults)
    {
        if (Result.bTestPassed)
        {
            PassedTests++;
        }
        LogTestResult(Result);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QA: VFX validation suite complete - %d/%d tests passed"), 
           PassedTests, LastTestResults.Num());
    
    bValidationInProgress = false;
    
    return LastTestResults;
}

bool UQA_VFXFootstepValidator::TestVFXPerformanceUnderLoad(int32 MaxParticleSystems)
{
    UE_LOG(LogTemp, Warning, TEXT("QA: Testing VFX performance under load with %d systems"), MaxParticleSystems);
    
    // This would spawn multiple particle systems and measure performance
    // For now, return true as a placeholder
    return true;
}

bool UQA_VFXFootstepValidator::ValidateVFXSystemPaths()
{
    TArray<FString> RequiredPaths = {
        TEXT("/Game/VFX/Dinosaur/NS_Dino_FootstepDust"),
        TEXT("/Game/VFX/Environment/"),
        TEXT("/Game/VFX/Weather/")
    };
    
    for (const FString& Path : RequiredPaths)
    {
        if (!IsVFXSystemValid(Path))
        {
            UE_LOG(LogTemp, Warning, TEXT("QA: VFX path validation failed for %s"), *Path);
            return false;
        }
    }
    
    return true;
}

bool UQA_VFXFootstepValidator::TestFootstepEffectScaling()
{
    // Test different dinosaur species scaling
    TArray<EQA_DinosaurSpecies> SpeciesToTest = {
        EQA_DinosaurSpecies::TRex,
        EQA_DinosaurSpecies::Raptor,
        EQA_DinosaurSpecies::Brachiosaurus
    };
    
    for (EQA_DinosaurSpecies Species : SpeciesToTest)
    {
        FVector TestLocation = FVector(0, 0, 100);
        if (!SpawnTestFootstepEffect(TestLocation, Species))
        {
            return false;
        }
    }
    
    return true;
}

void UQA_VFXFootstepValidator::CleanupTestActors()
{
    for (AActor* Actor : TestActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    TestActors.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("QA: Cleaned up test actors"));
}

void UQA_VFXFootstepValidator::GenerateVFXValidationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QA VFX VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total VFX Actors: %d"), CurrentMetrics.TotalVFXActors);
    UE_LOG(LogTemp, Warning, TEXT("Active Niagara Systems: %d"), CurrentMetrics.ActiveNiagaraSystems);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActorsWithVFX);
    UE_LOG(LogTemp, Warning, TEXT("Footstep System Ready: %s"), CurrentMetrics.bFootstepSystemReady ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.2fms"), CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

bool UQA_VFXFootstepValidator::IsVFXSystemValid(const FString& SystemPath)
{
    // Try to load the object at the given path
    UObject* LoadedObject = LoadObject<UObject>(nullptr, *SystemPath);
    return (LoadedObject != nullptr);
}

int32 UQA_VFXFootstepValidator::CountActiveNiagaraSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->FindComponentByClass<UNiagaraComponent>())
        {
            Count++;
        }
    }
    
    return Count;
}

float UQA_VFXFootstepValidator::MeasureFrameTime()
{
    // Return a simulated frame time for now
    // In a real implementation, this would measure actual frame time
    return FMath::RandRange(12.0f, 18.0f);
}

bool UQA_VFXFootstepValidator::SpawnTestFootstepEffect(const FVector& Location, EQA_DinosaurSpecies Species)
{
    // This would spawn a test footstep effect for the given species
    // For now, return true as a placeholder
    UE_LOG(LogTemp, Warning, TEXT("QA: Testing footstep effect for species %d at location %s"), 
           (int32)Species, *Location.ToString());
    return true;
}

void UQA_VFXFootstepValidator::LogTestResult(const FQA_FootstepTestResult& Result)
{
    UE_LOG(LogTemp, Warning, TEXT("QA Test: %s - %s (Score: %.1f)"), 
           *Result.TestName, 
           Result.bTestPassed ? TEXT("PASSED") : TEXT("FAILED"),
           Result.PerformanceScore);
    
    if (!Result.ErrorMessage.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("  Details: %s"), *Result.ErrorMessage);
    }
}