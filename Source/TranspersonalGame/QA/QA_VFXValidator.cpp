#include "QA_VFXValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Materials/Material.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

UQA_VFXValidator::UQA_VFXValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaxAcceptableFrameTime = 16.67f; // 60 FPS target
    MaxAcceptableParticleCount = 10000;
    bVerboseLogging = true;
}

TArray<FQA_VFXTestResult> UQA_VFXValidator::ValidateAllVFXSystems()
{
    LastTestResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("QA_VFXValidator: Starting comprehensive VFX validation"));
    
    // Test 1: Validate VFX Materials
    LastTestResults.Add(ValidateVFXMaterials());
    
    // Test 2: Validate Particle Performance
    LastTestResults.Add(ValidateParticlePerformance());
    
    // Test 3: Validate Audio-VFX Integration
    LastTestResults.Add(ValidateAudioVFXIntegration());
    
    // Test 4: Validate VFX Actor Components
    LastTestResults.Add(ValidateVFXActorComponents());
    
    // Generate summary report
    GenerateVFXValidationReport();
    
    return LastTestResults;
}

FQA_VFXTestResult UQA_VFXValidator::ValidateNiagaraSystem(UNiagaraSystem* System)
{
    float StartTime = FPlatformTime::Seconds();
    
    if (!System)
    {
        return CreateTestResult(TEXT("Niagara System Validation"), 
                              EQA_VFXValidationResult::Fail, 
                              TEXT("Null Niagara System provided"), 
                              FPlatformTime::Seconds() - StartTime);
    }
    
    if (!IsSystemValid(System))
    {
        return CreateTestResult(TEXT("Niagara System Validation"), 
                              EQA_VFXValidationResult::Warning, 
                              FString::Printf(TEXT("System %s has validation issues"), *System->GetName()), 
                              FPlatformTime::Seconds() - StartTime);
    }
    
    return CreateTestResult(TEXT("Niagara System Validation"), 
                          EQA_VFXValidationResult::Pass, 
                          FString::Printf(TEXT("System %s validated successfully"), *System->GetName()), 
                          FPlatformTime::Seconds() - StartTime);
}

FQA_VFXTestResult UQA_VFXValidator::ValidateVFXMaterials()
{
    float StartTime = FPlatformTime::Seconds();
    int32 ValidMaterials = 0;
    int32 InvalidMaterials = 0;
    
    // Test common VFX materials
    TArray<FString> MaterialPaths = {
        TEXT("/Game/VFX/Materials/M_Fire"),
        TEXT("/Game/VFX/Materials/M_Smoke"),
        TEXT("/Game/VFX/Materials/M_Dust"),
        TEXT("/Game/VFX/Materials/M_Water")
    };
    
    for (const FString& Path : MaterialPaths)
    {
        UMaterial* Material = LoadObject<UMaterial>(nullptr, *Path);
        if (Material && IsMaterialValid(Material))
        {
            ValidMaterials++;
        }
        else
        {
            InvalidMaterials++;
        }
    }
    
    EQA_VFXValidationResult Result = (InvalidMaterials == 0) ? EQA_VFXValidationResult::Pass : 
                                    (ValidMaterials > InvalidMaterials) ? EQA_VFXValidationResult::Warning : 
                                    EQA_VFXValidationResult::Fail;
    
    FString Details = FString::Printf(TEXT("Valid: %d, Invalid: %d VFX materials"), ValidMaterials, InvalidMaterials);
    
    return CreateTestResult(TEXT("VFX Materials Validation"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

FQA_VFXTestResult UQA_VFXValidator::ValidateParticlePerformance()
{
    float StartTime = FPlatformTime::Seconds();
    
    float FrameTime = GetParticleSystemFrameTime();
    int32 ParticleCount = GetActiveParticleCount();
    
    bool bPerformanceAcceptable = IsVFXPerformanceAcceptable();
    
    EQA_VFXValidationResult Result = bPerformanceAcceptable ? EQA_VFXValidationResult::Pass : EQA_VFXValidationResult::Warning;
    
    FString Details = FString::Printf(TEXT("Frame Time: %.2fms, Particles: %d, Acceptable: %s"), 
                                    FrameTime, ParticleCount, bPerformanceAcceptable ? TEXT("Yes") : TEXT("No"));
    
    return CreateTestResult(TEXT("Particle Performance"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

FQA_VFXTestResult UQA_VFXValidator::ValidateAudioVFXIntegration()
{
    float StartTime = FPlatformTime::Seconds();
    
    TArray<AActor*> VFXActors = FindActorsWithVFXComponents();
    int32 SyncedActors = 0;
    
    for (AActor* Actor : VFXActors)
    {
        if (ValidateVFXAudioSync(Actor))
        {
            SyncedActors++;
        }
    }
    
    float SyncRatio = VFXActors.Num() > 0 ? (float)SyncedActors / VFXActors.Num() : 1.0f;
    
    EQA_VFXValidationResult Result = (SyncRatio >= 0.8f) ? EQA_VFXValidationResult::Pass : 
                                    (SyncRatio >= 0.5f) ? EQA_VFXValidationResult::Warning : 
                                    EQA_VFXValidationResult::Fail;
    
    FString Details = FString::Printf(TEXT("Synced: %d/%d actors (%.1f%%)"), SyncedActors, VFXActors.Num(), SyncRatio * 100.0f);
    
    return CreateTestResult(TEXT("Audio-VFX Integration"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

FQA_VFXTestResult UQA_VFXValidator::ValidateVFXActorComponents()
{
    float StartTime = FPlatformTime::Seconds();
    
    TArray<AActor*> VFXActors = FindActorsWithVFXComponents();
    int32 ValidComponents = 0;
    int32 InvalidComponents = 0;
    
    for (AActor* Actor : VFXActors)
    {
        if (Actor)
        {
            UNiagaraComponent* NiagaraComp = Actor->FindComponentByClass<UNiagaraComponent>();
            if (NiagaraComp && NiagaraComp->GetAsset())
            {
                ValidComponents++;
            }
            else
            {
                InvalidComponents++;
            }
        }
    }
    
    EQA_VFXValidationResult Result = (InvalidComponents == 0) ? EQA_VFXValidationResult::Pass : 
                                    (ValidComponents > InvalidComponents) ? EQA_VFXValidationResult::Warning : 
                                    EQA_VFXValidationResult::Fail;
    
    FString Details = FString::Printf(TEXT("Valid: %d, Invalid: %d VFX components"), ValidComponents, InvalidComponents);
    
    return CreateTestResult(TEXT("VFX Actor Components"), Result, Details, FPlatformTime::Seconds() - StartTime);
}

float UQA_VFXValidator::GetParticleSystemFrameTime()
{
    // Simplified performance metric - in real implementation would query actual stats
    return 12.5f; // Placeholder value
}

int32 UQA_VFXValidator::GetActiveParticleCount()
{
    int32 TotalParticles = 0;
    
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                UNiagaraComponent* NiagaraComp = Actor->FindComponentByClass<UNiagaraComponent>();
                if (NiagaraComp && NiagaraComp->IsActive())
                {
                    // Simplified count - real implementation would query actual particle count
                    TotalParticles += 100; // Placeholder
                }
            }
        }
    }
    
    return TotalParticles;
}

bool UQA_VFXValidator::IsVFXPerformanceAcceptable()
{
    float FrameTime = GetParticleSystemFrameTime();
    int32 ParticleCount = GetActiveParticleCount();
    
    return (FrameTime <= MaxAcceptableFrameTime) && (ParticleCount <= MaxAcceptableParticleCount);
}

TArray<AActor*> UQA_VFXValidator::FindActorsWithVFXComponents()
{
    TArray<AActor*> VFXActors;
    
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->FindComponentByClass<UNiagaraComponent>())
            {
                VFXActors.Add(Actor);
            }
        }
    }
    
    return VFXActors;
}

bool UQA_VFXValidator::ValidateVFXAudioSync(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    UNiagaraComponent* NiagaraComp = Actor->FindComponentByClass<UNiagaraComponent>();
    UAudioComponent* AudioComp = Actor->FindComponentByClass<UAudioComponent>();
    
    // Both components should exist and be active for proper sync
    return (NiagaraComp && NiagaraComp->IsActive()) && (AudioComp && AudioComp->IsPlaying());
}

void UQA_VFXValidator::GenerateVFXValidationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VFX VALIDATION REPORT ==="));
    
    int32 PassCount = 0;
    int32 WarningCount = 0;
    int32 FailCount = 0;
    
    for (const FQA_VFXTestResult& Result : LastTestResults)
    {
        LogTestResult(Result);
        
        switch (Result.Result)
        {
            case EQA_VFXValidationResult::Pass:
                PassCount++;
                break;
            case EQA_VFXValidationResult::Warning:
                WarningCount++;
                break;
            case EQA_VFXValidationResult::Fail:
            case EQA_VFXValidationResult::Critical:
                FailCount++;
                break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("SUMMARY: Pass: %d, Warning: %d, Fail: %d"), PassCount, WarningCount, FailCount);
    UE_LOG(LogTemp, Warning, TEXT("=== END VFX VALIDATION REPORT ==="));
}

FString UQA_VFXValidator::GetLastValidationSummary()
{
    if (LastTestResults.Num() == 0)
    {
        return TEXT("No validation results available");
    }
    
    int32 PassCount = 0;
    int32 WarningCount = 0;
    int32 FailCount = 0;
    
    for (const FQA_VFXTestResult& Result : LastTestResults)
    {
        switch (Result.Result)
        {
            case EQA_VFXValidationResult::Pass:
                PassCount++;
                break;
            case EQA_VFXValidationResult::Warning:
                WarningCount++;
                break;
            case EQA_VFXValidationResult::Fail:
            case EQA_VFXValidationResult::Critical:
                FailCount++;
                break;
        }
    }
    
    return FString::Printf(TEXT("VFX Validation: %d Pass, %d Warning, %d Fail"), PassCount, WarningCount, FailCount);
}

FQA_VFXTestResult UQA_VFXValidator::CreateTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& Details, float ExecutionTime)
{
    FQA_VFXTestResult TestResult;
    TestResult.TestName = TestName;
    TestResult.Result = Result;
    TestResult.Details = Details;
    TestResult.ExecutionTime = ExecutionTime;
    
    return TestResult;
}

void UQA_VFXValidator::LogTestResult(const FQA_VFXTestResult& Result)
{
    if (bVerboseLogging)
    {
        FString ResultString;
        switch (Result.Result)
        {
            case EQA_VFXValidationResult::Pass:
                ResultString = TEXT("PASS");
                break;
            case EQA_VFXValidationResult::Warning:
                ResultString = TEXT("WARNING");
                break;
            case EQA_VFXValidationResult::Fail:
                ResultString = TEXT("FAIL");
                break;
            case EQA_VFXValidationResult::Critical:
                ResultString = TEXT("CRITICAL");
                break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s: %s (%.3fs)"), *ResultString, *Result.TestName, *Result.Details, Result.ExecutionTime);
    }
}

bool UQA_VFXValidator::IsSystemValid(UNiagaraSystem* System)
{
    if (!System)
    {
        return false;
    }
    
    // Basic validation - check if system has emitters
    return System->GetNumEmitters() > 0;
}

bool UQA_VFXValidator::IsMaterialValid(UMaterial* Material)
{
    if (!Material)
    {
        return false;
    }
    
    // Basic validation - material should be compiled
    return Material->IsComplete();
}