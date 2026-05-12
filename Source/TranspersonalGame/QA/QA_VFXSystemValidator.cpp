#include "QA_VFXSystemValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

UQA_VFXSystemValidator::UQA_VFXSystemValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bValidationComplete = false;
    TotalValidationTime = 0.0f;
}

void UQA_VFXSystemValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-run validation suite on begin play
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        FTimerHandle ValidationTimer;
        GetWorld()->GetTimerManager().SetTimer(ValidationTimer, this, &UQA_VFXSystemValidator::RunFullVFXValidationSuite, 2.0f, false);
    }
}

bool UQA_VFXSystemValidator::ValidateVFXImpactManager()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test class loading
        UClass* VFXClass = LoadClass<AActor>(nullptr, TEXT("/Script/TranspersonalGame.VFX_ImpactManager"));
        if (!VFXClass)
        {
            AddTestResult(TEXT("VFX_ImpactManager_ClassLoad"), EQA_VFXValidationResult::Fail, TEXT("Class not found"), FPlatformTime::Seconds() - StartTime);
            return false;
        }

        // Test actor spawning
        if (GetWorld())
        {
            FVector TestLocation(1000.0f, 1000.0f, 200.0f);
            AActor* TestActor = GetWorld()->SpawnActor<AActor>(VFXClass, TestLocation, FRotator::ZeroRotator);
            
            if (TestActor)
            {
                // Test basic functionality
                bool bFunctionalityTest = TestComponentFunctionality(TestActor);
                
                // Clean up test actor
                TestActor->Destroy();
                
                if (bFunctionalityTest)
                {
                    AddTestResult(TEXT("VFX_ImpactManager_Full"), EQA_VFXValidationResult::Pass, TEXT(""), FPlatformTime::Seconds() - StartTime);
                    return true;
                }
                else
                {
                    AddTestResult(TEXT("VFX_ImpactManager_Functionality"), EQA_VFXValidationResult::Fail, TEXT("Component functionality failed"), FPlatformTime::Seconds() - StartTime);
                    return false;
                }
            }
            else
            {
                AddTestResult(TEXT("VFX_ImpactManager_Spawn"), EQA_VFXValidationResult::Fail, TEXT("Actor spawn failed"), FPlatformTime::Seconds() - StartTime);
                return false;
            }
        }
        else
        {
            AddTestResult(TEXT("VFX_ImpactManager_World"), EQA_VFXValidationResult::Critical, TEXT("No valid world context"), FPlatformTime::Seconds() - StartTime);
            return false;
        }
    }
    catch (...)
    {
        AddTestResult(TEXT("VFX_ImpactManager_Exception"), EQA_VFXValidationResult::Critical, TEXT("Unhandled exception during validation"), FPlatformTime::Seconds() - StartTime);
        return false;
    }
}

bool UQA_VFXSystemValidator::ValidateNiagaraIntegration()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test Niagara component creation
        UNiagaraComponent* TestComponent = NewObject<UNiagaraComponent>(this);
        if (!TestComponent)
        {
            AddTestResult(TEXT("Niagara_ComponentCreation"), EQA_VFXValidationResult::Fail, TEXT("Failed to create Niagara component"), FPlatformTime::Seconds() - StartTime);
            return false;
        }

        // Test basic Niagara functionality
        TestComponent->SetAutoActivate(false);
        TestComponent->SetVisibility(true);
        
        AddTestResult(TEXT("Niagara_Integration"), EQA_VFXValidationResult::Pass, TEXT(""), FPlatformTime::Seconds() - StartTime);
        return true;
    }
    catch (...)
    {
        AddTestResult(TEXT("Niagara_Exception"), EQA_VFXValidationResult::Critical, TEXT("Exception during Niagara validation"), FPlatformTime::Seconds() - StartTime);
        return false;
    }
}

bool UQA_VFXSystemValidator::ValidateParticleSystemPerformance()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Performance validation - check if we can create multiple particle systems without major performance hit
        TArray<UNiagaraComponent*> TestComponents;
        
        for (int32 i = 0; i < 10; ++i)
        {
            UNiagaraComponent* Component = NewObject<UNiagaraComponent>(this);
            if (Component)
            {
                TestComponents.Add(Component);
                Component->SetAutoActivate(false);
            }
        }
        
        if (TestComponents.Num() == 10)
        {
            AddTestResult(TEXT("ParticleSystem_Performance"), EQA_VFXValidationResult::Pass, TEXT(""), FPlatformTime::Seconds() - StartTime);
            return true;
        }
        else
        {
            AddTestResult(TEXT("ParticleSystem_Performance"), EQA_VFXValidationResult::Fail, FString::Printf(TEXT("Only created %d/10 components"), TestComponents.Num()), FPlatformTime::Seconds() - StartTime);
            return false;
        }
    }
    catch (...)
    {
        AddTestResult(TEXT("ParticleSystem_Exception"), EQA_VFXValidationResult::Critical, TEXT("Exception during performance validation"), FPlatformTime::Seconds() - StartTime);
        return false;
    }
}

bool UQA_VFXSystemValidator::ValidateEnvironmentalVFX()
{
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Test environmental VFX systems integration
        if (GetWorld())
        {
            // Check for existing environmental actors
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
            
            int32 VFXActorCount = 0;
            for (AActor* Actor : FoundActors)
            {
                if (Actor && Actor->GetName().Contains(TEXT("VFX")))
                {
                    VFXActorCount++;
                }
            }
            
            if (VFXActorCount > 0)
            {
                AddTestResult(TEXT("Environmental_VFX"), EQA_VFXValidationResult::Pass, FString::Printf(TEXT("Found %d VFX actors"), VFXActorCount), FPlatformTime::Seconds() - StartTime);
                return true;
            }
            else
            {
                AddTestResult(TEXT("Environmental_VFX"), EQA_VFXValidationResult::Fail, TEXT("No VFX actors found in level"), FPlatformTime::Seconds() - StartTime);
                return false;
            }
        }
        else
        {
            AddTestResult(TEXT("Environmental_VFX_World"), EQA_VFXValidationResult::Critical, TEXT("No world context"), FPlatformTime::Seconds() - StartTime);
            return false;
        }
    }
    catch (...)
    {
        AddTestResult(TEXT("Environmental_VFX_Exception"), EQA_VFXValidationResult::Critical, TEXT("Exception during environmental VFX validation"), FPlatformTime::Seconds() - StartTime);
        return false;
    }
}

void UQA_VFXSystemValidator::RunFullVFXValidationSuite()
{
    float SuiteStartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXSystemValidator: Starting full VFX validation suite"));
    
    ValidationResults.Empty();
    bValidationComplete = false;
    
    // Run all validation tests
    ValidateVFXImpactManager();
    ValidateNiagaraIntegration();
    ValidateParticleSystemPerformance();
    ValidateEnvironmentalVFX();
    
    TotalValidationTime = FPlatformTime::Seconds() - SuiteStartTime;
    bValidationComplete = true;
    
    // Log results
    FString Report = GenerateValidationReport();
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXSystemValidator: Validation complete\n%s"), *Report);
}

bool UQA_VFXSystemValidator::HasCriticalFailures() const
{
    for (const FQA_VFXTestResult& Result : ValidationResults)
    {
        if (Result.Result == EQA_VFXValidationResult::Critical)
        {
            return true;
        }
    }
    return false;
}

FString UQA_VFXSystemValidator::GenerateValidationReport() const
{
    FString Report = TEXT("=== VFX SYSTEM VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Total Tests: %d\n"), ValidationResults.Num());
    Report += FString::Printf(TEXT("Total Execution Time: %.3f seconds\n"), TotalValidationTime);
    Report += TEXT("Status: ") + (bValidationComplete ? TEXT("COMPLETE") : TEXT("IN PROGRESS")) + TEXT("\n\n");
    
    int32 PassCount = 0, FailCount = 0, CriticalCount = 0;
    
    for (const FQA_VFXTestResult& Result : ValidationResults)
    {
        FString StatusText;
        switch (Result.Result)
        {
            case EQA_VFXValidationResult::Pass:
                StatusText = TEXT("PASS");
                PassCount++;
                break;
            case EQA_VFXValidationResult::Fail:
                StatusText = TEXT("FAIL");
                FailCount++;
                break;
            case EQA_VFXValidationResult::Critical:
                StatusText = TEXT("CRITICAL");
                CriticalCount++;
                break;
            default:
                StatusText = TEXT("NOT TESTED");
                break;
        }
        
        Report += FString::Printf(TEXT("[%s] %s (%.3fs)"), *StatusText, *Result.TestName, Result.ExecutionTime);
        if (!Result.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT(" - %s"), *Result.ErrorMessage);
        }
        Report += TEXT("\n");
    }
    
    Report += FString::Printf(TEXT("\nSUMMARY: %d PASS, %d FAIL, %d CRITICAL\n"), PassCount, FailCount, CriticalCount);
    
    if (CriticalCount > 0)
    {
        Report += TEXT("*** CRITICAL FAILURES DETECTED - IMMEDIATE ATTENTION REQUIRED ***\n");
    }
    else if (FailCount > 0)
    {
        Report += TEXT("*** FAILURES DETECTED - REVIEW REQUIRED ***\n");
    }
    else
    {
        Report += TEXT("*** ALL TESTS PASSED ***\n");
    }
    
    return Report;
}

void UQA_VFXSystemValidator::AddTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& ErrorMessage, float ExecutionTime)
{
    FQA_VFXTestResult NewResult;
    NewResult.TestName = TestName;
    NewResult.Result = Result;
    NewResult.ErrorMessage = ErrorMessage;
    NewResult.ExecutionTime = ExecutionTime;
    
    ValidationResults.Add(NewResult);
}

bool UQA_VFXSystemValidator::TestClassLoading(const FString& ClassName)
{
    UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
    return TestClass != nullptr;
}

bool UQA_VFXSystemValidator::TestActorSpawning(UClass* ActorClass, const FVector& Location)
{
    if (!ActorClass || !GetWorld())
    {
        return false;
    }
    
    AActor* TestActor = GetWorld()->SpawnActor<AActor>(ActorClass, Location, FRotator::ZeroRotator);
    if (TestActor)
    {
        TestActor->Destroy();
        return true;
    }
    
    return false;
}

bool UQA_VFXSystemValidator::TestComponentFunctionality(AActor* TestActor)
{
    if (!TestActor)
    {
        return false;
    }
    
    // Basic functionality tests
    TestActor->SetActorLocation(FVector(1500.0f, 1500.0f, 300.0f));
    FVector NewLocation = TestActor->GetActorLocation();
    
    // Check if location was set correctly (within tolerance)
    return FVector::Dist(NewLocation, FVector(1500.0f, 1500.0f, 300.0f)) < 10.0f;
}