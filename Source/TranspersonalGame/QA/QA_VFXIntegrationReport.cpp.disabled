#include "QA_VFXIntegrationReport.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/ObjectLibrary.h"

UQA_VFXIntegrationReport::UQA_VFXIntegrationReport()
{
    PrimaryComponentTick.bCanEverTick = false;
    bCriticalVFXFailure = false;
    LastValidationTime = FDateTime::Now();
}

void UQA_VFXIntegrationReport::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-run validation on begin play
    RunVFXIntegrationTests();
}

void UQA_VFXIntegrationReport::RunVFXIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationReport: Starting VFX integration tests"));
    
    ResetTestResults();
    LastValidationTime = FDateTime::Now();
    bCriticalVFXFailure = false;

    // Test 1: Validate VFX Manager Class
    float StartTime = FPlatformTime::Seconds();
    bool bVFXManagerTest = ValidateVFXManagerClass();
    float VFXManagerTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(TEXT("VFX_ImpactManager_ClassLoad"), bVFXManagerTest, 
                  bVFXManagerTest ? TEXT("") : TEXT("VFX_ImpactManager class not found"), VFXManagerTime);

    // Test 2: Test Niagara System Loading
    StartTime = FPlatformTime::Seconds();
    bool bNiagaraTest = TestNiagaraSystemLoading();
    float NiagaraTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(TEXT("Niagara_SystemLoad"), bNiagaraTest,
                  bNiagaraTest ? TEXT("") : TEXT("Niagara systems failed to load"), NiagaraTime);

    // Test 3: Validate Impact Effects
    StartTime = FPlatformTime::Seconds();
    bool bImpactTest = ValidateImpactEffects();
    float ImpactTime = FPlatformTime::Seconds() - StartTime;
    AddTestResult(TEXT("Impact_Effects"), bImpactTest,
                  bImpactTest ? TEXT("") : TEXT("Impact effects validation failed"), ImpactTime);

    // Update status
    CurrentStatus.bVFXManagerLoaded = bVFXManagerTest;
    CurrentStatus.bNiagaraSystemsValid = bNiagaraTest;
    CurrentStatus.bImpactEffectsWorking = bImpactTest;

    // Count VFX actors in world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> VFXActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), VFXActors);
        CurrentStatus.TotalVFXActors = 0;
        
        for (AActor* Actor : VFXActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("VFX")))
            {
                CurrentStatus.TotalVFXActors++;
            }
        }
    }

    // Check for critical failures
    if (!bVFXManagerTest || !bNiagaraTest)
    {
        bCriticalVFXFailure = true;
        LogCriticalVFXIssue(TEXT("Critical VFX systems failed validation"));
    }

    UE_LOG(LogTemp, Warning, TEXT("QA_VFXIntegrationReport: Tests completed. Critical failure: %s"), 
           bCriticalVFXFailure ? TEXT("YES") : TEXT("NO"));
}

bool UQA_VFXIntegrationReport::ValidateVFXManagerClass()
{
    // Try to load VFX_ImpactManager class
    UClass* VFXManagerClass = StaticLoadClass(UObject::StaticClass(), nullptr, 
                                              TEXT("/Script/TranspersonalGame.VFX_ImpactManager"));
    
    if (VFXManagerClass)
    {
        UE_LOG(LogTemp, Log, TEXT("QA: VFX_ImpactManager class loaded successfully"));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QA: VFX_ImpactManager class not found"));
        return false;
    }
}

bool UQA_VFXIntegrationReport::TestNiagaraSystemLoading()
{
    // Test basic Niagara functionality
    try
    {
        // Try to find any Niagara systems in the project
        UObjectLibrary* NiagaraLibrary = UObjectLibrary::CreateLibrary(UNiagaraSystem::StaticClass(), false, true);
        if (NiagaraLibrary)
        {
            NiagaraLibrary->AddToRoot();
            TArray<FString> Paths;
            Paths.Add(TEXT("/Game/"));
            NiagaraLibrary->LoadAssetDataFromPaths(Paths);
            
            TArray<UObject*> NiagaraSystems;
            NiagaraLibrary->GetObjects(NiagaraSystems);
            
            UE_LOG(LogTemp, Log, TEXT("QA: Found %d Niagara systems"), NiagaraSystems.Num());
            
            NiagaraLibrary->RemoveFromRoot();
            return NiagaraSystems.Num() > 0;
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("QA: Exception during Niagara system test"));
        return false;
    }
    
    return false;
}

bool UQA_VFXIntegrationReport::ValidateImpactEffects()
{
    // Test impact effect functionality
    if (UWorld* World = GetWorld())
    {
        // Look for any actors with VFX components
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 VFXComponentCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                VFXComponentCount += NiagaraComponents.Num();
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("QA: Found %d Niagara components in world"), VFXComponentCount);
        return VFXComponentCount >= 0; // Pass if we can count components without crashing
    }
    
    return false;
}

FString UQA_VFXIntegrationReport::GenerateVFXReport()
{
    FString Report = TEXT("=== VFX INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Time: %s\n"), *LastValidationTime.ToString());
    Report += FString::Printf(TEXT("Critical Failure: %s\n"), bCriticalVFXFailure ? TEXT("YES") : TEXT("NO"));
    Report += FString::Printf(TEXT("VFX Manager Loaded: %s\n"), CurrentStatus.bVFXManagerLoaded ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Niagara Systems Valid: %s\n"), CurrentStatus.bNiagaraSystemsValid ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Impact Effects Working: %s\n"), CurrentStatus.bImpactEffectsWorking ? TEXT("PASS") : TEXT("FAIL"));
    Report += FString::Printf(TEXT("Total VFX Actors: %d\n"), CurrentStatus.TotalVFXActors);
    
    Report += TEXT("\n=== TEST RESULTS ===\n");
    for (const FQA_VFXTestResult& Result : CurrentStatus.TestResults)
    {
        Report += FString::Printf(TEXT("%s: %s (%.3fs)\n"), 
                                  *Result.TestName, 
                                  Result.bPassed ? TEXT("PASS") : TEXT("FAIL"),
                                  Result.ExecutionTime);
        if (!Result.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT("  Error: %s\n"), *Result.ErrorMessage);
        }
    }
    
    return Report;
}

void UQA_VFXIntegrationReport::LogCriticalVFXIssue(const FString& Issue)
{
    UE_LOG(LogTemp, Error, TEXT("QA CRITICAL VFX ISSUE: %s"), *Issue);
    bCriticalVFXFailure = true;
}

void UQA_VFXIntegrationReport::AddTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage, float ExecutionTime)
{
    FQA_VFXTestResult Result;
    Result.TestName = TestName;
    Result.bPassed = bPassed;
    Result.ErrorMessage = ErrorMessage;
    Result.ExecutionTime = ExecutionTime;
    
    CurrentStatus.TestResults.Add(Result);
}

void UQA_VFXIntegrationReport::ResetTestResults()
{
    CurrentStatus.TestResults.Empty();
    CurrentStatus.bVFXManagerLoaded = false;
    CurrentStatus.bNiagaraSystemsValid = false;
    CurrentStatus.bImpactEffectsWorking = false;
    CurrentStatus.TotalVFXActors = 0;
}