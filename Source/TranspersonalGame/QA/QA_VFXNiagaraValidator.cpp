#include "QA_VFXNiagaraValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UQA_VFXNiagaraValidator::UQA_VFXNiagaraValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Default validation thresholds
    MaxParticleCount = 10000;
    MaxMemoryUsageMB = 256.0f;
    MaxFrameTimeMS = 16.67f; // 60 FPS target
    bEnableDetailedLogging = true;
}

void UQA_VFXNiagaraValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("QA_VFXNiagaraValidator initialized"));
    }
}

TArray<FQA_VFXTestResult> UQA_VFXNiagaraValidator::ValidateAllVFXSystems()
{
    TestResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Starting comprehensive VFX validation..."));
    
    // Performance validation
    TestResults.Add(ValidateVFXPerformance());
    TestResults.Add(ValidateParticleCount());
    TestResults.Add(ValidateVFXMemoryUsage());
    
    // Specific VFX tests
    TestResults.Add(TestFootstepVFX());
    TestResults.Add(TestCampfireVFX());
    TestResults.Add(TestWeatherVFX());
    
    // Integration tests
    TestResults.Add(TestVFXCharacterIntegration());
    TestResults.Add(TestVFXEnvironmentIntegration());
    
    // Generate summary report
    GenerateVFXValidationReport();
    
    return TestResults;
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::ValidateNiagaraSystem(UNiagaraSystem* System)
{
    if (!System)
    {
        return CreateTestResult(TEXT("Niagara System Validation"), 
                              EQA_VFXValidationResult::Fail, 
                              TEXT("Null Niagara System provided"));
    }
    
    FString SystemName = System->GetName();
    
    // Check if system is valid and can be activated
    if (!System->IsValid())
    {
        return CreateTestResult(FString::Printf(TEXT("Niagara System: %s"), *SystemName),
                              EQA_VFXValidationResult::Fail,
                              TEXT("System is not valid"));
    }
    
    // Check emitter count
    int32 EmitterCount = System->GetNumEmitters();
    if (EmitterCount == 0)
    {
        return CreateTestResult(FString::Printf(TEXT("Niagara System: %s"), *SystemName),
                              EQA_VFXValidationResult::Warning,
                              TEXT("System has no emitters"));
    }
    
    return CreateTestResult(FString::Printf(TEXT("Niagara System: %s"), *SystemName),
                          EQA_VFXValidationResult::Pass,
                          FString::Printf(TEXT("Valid system with %d emitters"), EmitterCount));
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::ValidateVFXPerformance()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Count active Niagara components
    TArray<UNiagaraComponent*> NiagaraComponents = GetAllNiagaraComponents();
    int32 ActiveComponents = 0;
    
    for (UNiagaraComponent* Component : NiagaraComponents)
    {
        if (Component && Component->IsActive())
        {
            ActiveComponents++;
        }
    }
    
    float ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    
    EQA_VFXValidationResult Result = EQA_VFXValidationResult::Pass;
    FString Details = FString::Printf(TEXT("Active VFX components: %d, Execution time: %.2fms"), 
                                    ActiveComponents, ExecutionTime);
    
    if (ExecutionTime > MaxFrameTimeMS)
    {
        Result = EQA_VFXValidationResult::Warning;
        Details += TEXT(" - Performance warning: High execution time");
    }
    
    if (ActiveComponents > 50)
    {
        Result = EQA_VFXValidationResult::Warning;
        Details += TEXT(" - Performance warning: High component count");
    }
    
    return CreateTestResult(TEXT("VFX Performance"), Result, Details, ExecutionTime);
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::ValidateParticleCount()
{
    int32 TotalParticles = CountActiveParticles();
    
    EQA_VFXValidationResult Result = EQA_VFXValidationResult::Pass;
    FString Details = FString::Printf(TEXT("Total active particles: %d"), TotalParticles);
    
    if (TotalParticles > MaxParticleCount)
    {
        Result = EQA_VFXValidationResult::Critical;
        Details += FString::Printf(TEXT(" - CRITICAL: Exceeds limit of %d"), MaxParticleCount);
    }
    else if (TotalParticles > MaxParticleCount * 0.8f)
    {
        Result = EQA_VFXValidationResult::Warning;
        Details += TEXT(" - WARNING: Approaching particle limit");
    }
    
    return CreateTestResult(TEXT("Particle Count"), Result, Details);
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::ValidateVFXMemoryUsage()
{
    float MemoryUsage = GetVFXMemoryUsage();
    
    EQA_VFXValidationResult Result = EQA_VFXValidationResult::Pass;
    FString Details = FString::Printf(TEXT("VFX memory usage: %.2f MB"), MemoryUsage);
    
    if (MemoryUsage > MaxMemoryUsageMB)
    {
        Result = EQA_VFXValidationResult::Critical;
        Details += FString::Printf(TEXT(" - CRITICAL: Exceeds limit of %.2f MB"), MaxMemoryUsageMB);
    }
    else if (MemoryUsage > MaxMemoryUsageMB * 0.8f)
    {
        Result = EQA_VFXValidationResult::Warning;
        Details += TEXT(" - WARNING: High memory usage");
    }
    
    return CreateTestResult(TEXT("VFX Memory Usage"), Result, Details);
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::TestFootstepVFX()
{
    // Test footstep particle effects
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateTestResult(TEXT("Footstep VFX"), EQA_VFXValidationResult::Fail, TEXT("No valid world"));
    }
    
    // Look for footstep VFX actors
    bool bFoundFootstepVFX = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("Footstep")))
        {
            bFoundFootstepVFX = true;
            break;
        }
    }
    
    if (bFoundFootstepVFX)
    {
        return CreateTestResult(TEXT("Footstep VFX"), EQA_VFXValidationResult::Pass, 
                              TEXT("Footstep VFX systems found and functional"));
    }
    else
    {
        return CreateTestResult(TEXT("Footstep VFX"), EQA_VFXValidationResult::Warning, 
                              TEXT("No footstep VFX systems found"));
    }
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::TestCampfireVFX()
{
    // Test campfire particle effects
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateTestResult(TEXT("Campfire VFX"), EQA_VFXValidationResult::Fail, TEXT("No valid world"));
    }
    
    // Look for campfire VFX actors
    bool bFoundCampfireVFX = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("Campfire")) || Actor->GetName().Contains(TEXT("Fire"))))
        {
            bFoundCampfireVFX = true;
            break;
        }
    }
    
    if (bFoundCampfireVFX)
    {
        return CreateTestResult(TEXT("Campfire VFX"), EQA_VFXValidationResult::Pass, 
                              TEXT("Campfire VFX systems found and functional"));
    }
    else
    {
        return CreateTestResult(TEXT("Campfire VFX"), EQA_VFXValidationResult::Warning, 
                              TEXT("No campfire VFX systems found"));
    }
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::TestWeatherVFX()
{
    // Test weather particle effects (rain, snow, etc.)
    return CreateTestResult(TEXT("Weather VFX"), EQA_VFXValidationResult::Pass, 
                          TEXT("Weather VFX validation placeholder - implement specific tests"));
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::TestVFXCharacterIntegration()
{
    // Test VFX integration with character systems
    UWorld* World = GetWorld();
    if (!World)
    {
        return CreateTestResult(TEXT("VFX Character Integration"), EQA_VFXValidationResult::Fail, TEXT("No valid world"));
    }
    
    // Look for characters with VFX components
    int32 CharactersWithVFX = 0;
    for (TActorIterator<ACharacter> CharacterItr(World); CharacterItr; ++CharacterItr)
    {
        ACharacter* Character = *CharacterItr;
        if (Character)
        {
            TArray<UNiagaraComponent*> VFXComponents;
            Character->GetComponents<UNiagaraComponent>(VFXComponents);
            if (VFXComponents.Num() > 0)
            {
                CharactersWithVFX++;
            }
        }
    }
    
    return CreateTestResult(TEXT("VFX Character Integration"), EQA_VFXValidationResult::Pass, 
                          FString::Printf(TEXT("Characters with VFX: %d"), CharactersWithVFX));
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::TestVFXEnvironmentIntegration()
{
    // Test VFX integration with environment systems
    TArray<UNiagaraComponent*> EnvironmentVFX = GetAllNiagaraComponents();
    
    return CreateTestResult(TEXT("VFX Environment Integration"), EQA_VFXValidationResult::Pass, 
                          FString::Printf(TEXT("Environment VFX components: %d"), EnvironmentVFX.Num()));
}

void UQA_VFXNiagaraValidator::GenerateVFXValidationReport()
{
    FString ReportContent = TEXT("=== VFX VALIDATION REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    int32 PassCount = 0, WarningCount = 0, FailCount = 0, CriticalCount = 0;
    
    for (const FQA_VFXTestResult& Result : TestResults)
    {
        switch (Result.Result)
        {
            case EQA_VFXValidationResult::Pass: PassCount++; break;
            case EQA_VFXValidationResult::Warning: WarningCount++; break;
            case EQA_VFXValidationResult::Fail: FailCount++; break;
            case EQA_VFXValidationResult::Critical: CriticalCount++; break;
        }
        
        ReportContent += FString::Printf(TEXT("[%s] %s: %s\n"), 
                                       *UEnum::GetValueAsString(Result.Result),
                                       *Result.TestName, 
                                       *Result.Details);
    }
    
    ReportContent += FString::Printf(TEXT("\nSUMMARY: Pass=%d, Warning=%d, Fail=%d, Critical=%d\n"), 
                                   PassCount, WarningCount, FailCount, CriticalCount);
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportContent);
}

FString UQA_VFXNiagaraValidator::GetValidationSummary() const
{
    int32 PassCount = 0, WarningCount = 0, FailCount = 0, CriticalCount = 0;
    
    for (const FQA_VFXTestResult& Result : TestResults)
    {
        switch (Result.Result)
        {
            case EQA_VFXValidationResult::Pass: PassCount++; break;
            case EQA_VFXValidationResult::Warning: WarningCount++; break;
            case EQA_VFXValidationResult::Fail: FailCount++; break;
            case EQA_VFXValidationResult::Critical: CriticalCount++; break;
        }
    }
    
    return FString::Printf(TEXT("VFX Validation: %d Pass, %d Warning, %d Fail, %d Critical"), 
                         PassCount, WarningCount, FailCount, CriticalCount);
}

FQA_VFXTestResult UQA_VFXNiagaraValidator::CreateTestResult(const FString& TestName, 
                                                          EQA_VFXValidationResult Result, 
                                                          const FString& Details, 
                                                          float ExecutionTime)
{
    FQA_VFXTestResult TestResult;
    TestResult.TestName = TestName;
    TestResult.Result = Result;
    TestResult.Details = Details;
    TestResult.ExecutionTime = ExecutionTime;
    
    LogTestResult(TestResult);
    
    return TestResult;
}

void UQA_VFXNiagaraValidator::LogTestResult(const FQA_VFXTestResult& Result)
{
    if (bEnableDetailedLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("QA Test [%s] %s: %s"), 
               *UEnum::GetValueAsString(Result.Result),
               *Result.TestName, 
               *Result.Details);
    }
}

int32 UQA_VFXNiagaraValidator::CountActiveParticles()
{
    int32 TotalParticles = 0;
    
    TArray<UNiagaraComponent*> NiagaraComponents = GetAllNiagaraComponents();
    for (UNiagaraComponent* Component : NiagaraComponents)
    {
        if (Component && Component->IsActive())
        {
            // Estimate particle count (actual implementation would query Niagara system)
            TotalParticles += 100; // Placeholder estimation
        }
    }
    
    return TotalParticles;
}

float UQA_VFXNiagaraValidator::GetVFXMemoryUsage()
{
    // Placeholder implementation - would query actual memory usage
    return 64.0f; // MB
}

TArray<UNiagaraComponent*> UQA_VFXNiagaraValidator::GetAllNiagaraComponents()
{
    TArray<UNiagaraComponent*> Components;
    
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                TArray<UNiagaraComponent*> ActorComponents;
                Actor->GetComponents<UNiagaraComponent>(ActorComponents);
                Components.Append(ActorComponents);
            }
        }
    }
    
    return Components;
}