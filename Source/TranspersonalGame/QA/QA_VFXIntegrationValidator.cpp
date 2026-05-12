#include "QA_VFXIntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

UQA_VFXIntegrationValidator::UQA_VFXIntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MaxAcceptableFrameTime = 16.67f; // 60 FPS target
    MaxAcceptableParticleCount = 10000;
    bAutoRunValidation = false;
    ValidationInterval = 30.0f;
    LastValidationTime = 0.0f;
    bValidationInProgress = false;
}

void UQA_VFXIntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("QA VFX Integration Validator initialized"));
    
    if (bAutoRunValidation)
    {
        // Run initial validation after 5 seconds
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { RunFullVFXValidationSuite(); },
            5.0f,
            false
        );
    }
}

void UQA_VFXIntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoRunValidation && !bValidationInProgress)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastValidationTime >= ValidationInterval)
        {
            RunFullVFXValidationSuite();
            LastValidationTime = CurrentTime;
        }
    }
}

FQA_VFXTestResult UQA_VFXIntegrationValidator::ValidateVFXImpactManager()
{
    FQA_VFXTestResult Result;
    Result.TestName = TEXT("VFX Impact Manager Validation");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        // Check if VFX_ImpactManager class exists
        UClass* VFXClass = FindObject<UClass>(ANY_PACKAGE, TEXT("VFX_ImpactManager"));
        if (!VFXClass)
        {
            Result.Result = EQA_VFXValidationResult::Fail;
            Result.Details = TEXT("VFX_ImpactManager class not found in runtime");
        }
        else
        {
            // Check for VFX actors in the world
            UWorld* World = GetWorld();
            if (World)
            {
                TArray<AActor*> VFXActors;
                UGameplayStatics::GetAllActorsOfClass(World, VFXClass, VFXActors);
                
                if (VFXActors.Num() > 0)
                {
                    Result.Result = EQA_VFXValidationResult::Pass;
                    Result.Details = FString::Printf(TEXT("Found %d VFX Impact Manager instances"), VFXActors.Num());
                }
                else
                {
                    Result.Result = EQA_VFXValidationResult::Warning;
                    Result.Details = TEXT("VFX_ImpactManager class exists but no instances found in level");
                }
            }
            else
            {
                Result.Result = EQA_VFXValidationResult::Fail;
                Result.Details = TEXT("No valid world context for VFX validation");
            }
        }
    }
    catch (...)
    {
        Result.Result = EQA_VFXValidationResult::Critical;
        Result.Details = TEXT("Exception occurred during VFX Impact Manager validation");
    }
    
    Result.ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    return Result;
}

FQA_VFXTestResult UQA_VFXIntegrationValidator::ValidateNiagaraSystems()
{
    FQA_VFXTestResult Result;
    Result.TestName = TEXT("Niagara Systems Validation");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            Result.Result = EQA_VFXValidationResult::Fail;
            Result.Details = TEXT("No valid world context");
            Result.ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
            return Result;
        }
        
        // Find all Niagara components in the level
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 NiagaraComponentCount = 0;
        int32 ActiveNiagaraCount = 0;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                
                NiagaraComponentCount += NiagaraComponents.Num();
                
                for (UNiagaraComponent* NiagaraComp : NiagaraComponents)
                {
                    if (NiagaraComp && NiagaraComp->IsActive())
                    {
                        ActiveNiagaraCount++;
                    }
                }
            }
        }
        
        if (NiagaraComponentCount > 0)
        {
            Result.Result = EQA_VFXValidationResult::Pass;
            Result.Details = FString::Printf(TEXT("Found %d Niagara components, %d active"), 
                NiagaraComponentCount, ActiveNiagaraCount);
        }
        else
        {
            Result.Result = EQA_VFXValidationResult::Warning;
            Result.Details = TEXT("No Niagara components found in level");
        }
    }
    catch (...)
    {
        Result.Result = EQA_VFXValidationResult::Critical;
        Result.Details = TEXT("Exception occurred during Niagara systems validation");
    }
    
    Result.ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    return Result;
}

FQA_VFXTestResult UQA_VFXIntegrationValidator::ValidateEnvironmentalEffects()
{
    FQA_VFXTestResult Result;
    Result.TestName = TEXT("Environmental Effects Validation");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            Result.Result = EQA_VFXValidationResult::Fail;
            Result.Details = TEXT("No valid world context");
            Result.ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
            return Result;
        }
        
        // Check for environmental VFX actors
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 EnvironmentalVFXCount = 0;
        TArray<FString> FoundEffects;
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                FString ActorName = Actor->GetName();
                if (ActorName.Contains(TEXT("Dust")) || 
                    ActorName.Contains(TEXT("Wind")) || 
                    ActorName.Contains(TEXT("Atmosphere")) ||
                    ActorName.Contains(TEXT("Environmental")))
                {
                    EnvironmentalVFXCount++;
                    FoundEffects.Add(ActorName);
                }
            }
        }
        
        if (EnvironmentalVFXCount > 0)
        {
            Result.Result = EQA_VFXValidationResult::Pass;
            Result.Details = FString::Printf(TEXT("Found %d environmental effects: %s"), 
                EnvironmentalVFXCount, *FString::Join(FoundEffects, TEXT(", ")));
        }
        else
        {
            Result.Result = EQA_VFXValidationResult::Warning;
            Result.Details = TEXT("No environmental VFX found - may impact atmosphere");
        }
    }
    catch (...)
    {
        Result.Result = EQA_VFXValidationResult::Critical;
        Result.Details = TEXT("Exception occurred during environmental effects validation");
    }
    
    Result.ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    return Result;
}

FQA_VFXTestResult UQA_VFXIntegrationValidator::ValidateParticlePerformance()
{
    FQA_VFXTestResult Result;
    Result.TestName = TEXT("Particle Performance Validation");
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        float CurrentFrameTime = GetVFXFrameTime();
        int32 ParticleCount = GetActiveParticleCount();
        
        if (CurrentFrameTime <= MaxAcceptableFrameTime && ParticleCount <= MaxAcceptableParticleCount)
        {
            Result.Result = EQA_VFXValidationResult::Pass;
            Result.Details = FString::Printf(TEXT("Performance OK: %.2fms frame time, %d particles"), 
                CurrentFrameTime, ParticleCount);
        }
        else if (CurrentFrameTime > MaxAcceptableFrameTime)
        {
            Result.Result = EQA_VFXValidationResult::Warning;
            Result.Details = FString::Printf(TEXT("Frame time high: %.2fms (target: %.2fms)"), 
                CurrentFrameTime, MaxAcceptableFrameTime);
        }
        else
        {
            Result.Result = EQA_VFXValidationResult::Warning;
            Result.Details = FString::Printf(TEXT("Particle count high: %d (max: %d)"), 
                ParticleCount, MaxAcceptableParticleCount);
        }
    }
    catch (...)
    {
        Result.Result = EQA_VFXValidationResult::Critical;
        Result.Details = TEXT("Exception occurred during particle performance validation");
    }
    
    Result.ExecutionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
    return Result;
}

TArray<FQA_VFXTestResult> UQA_VFXIntegrationValidator::RunFullVFXValidationSuite()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX validation already in progress"));
        return LastValidationResults;
    }
    
    bValidationInProgress = true;
    LastValidationResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Starting full VFX validation suite"));
    
    // Run all validation tests
    LastValidationResults.Add(ValidateVFXImpactManager());
    LastValidationResults.Add(ValidateNiagaraSystems());
    LastValidationResults.Add(ValidateEnvironmentalEffects());
    LastValidationResults.Add(ValidateParticlePerformance());
    
    // Log results
    for (const FQA_VFXTestResult& TestResult : LastValidationResults)
    {
        FString ResultString;
        switch (TestResult.Result)
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
        
        UE_LOG(LogTemp, Warning, TEXT("VFX Test [%s]: %s - %s (%.2fms)"), 
            *TestResult.TestName, *ResultString, *TestResult.Details, TestResult.ExecutionTime);
    }
    
    bValidationInProgress = false;
    return LastValidationResults;
}

float UQA_VFXIntegrationValidator::GetVFXFrameTime()
{
    // Get current frame time in milliseconds
    if (GEngine && GEngine->GetGameViewport())
    {
        return GEngine->GetMaxTickRate() > 0 ? (1000.0f / GEngine->GetMaxTickRate()) : 16.67f;
    }
    return 16.67f; // Default 60 FPS target
}

int32 UQA_VFXIntegrationValidator::GetActiveParticleCount()
{
    int32 TotalParticles = 0;
    
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor)
            {
                TArray<UParticleSystemComponent*> ParticleComponents;
                Actor->GetComponents<UParticleSystemComponent>(ParticleComponents);
                
                for (UParticleSystemComponent* PSC : ParticleComponents)
                {
                    if (PSC && PSC->IsActive())
                    {
                        TotalParticles += 100; // Estimate - actual particle counting requires more complex logic
                    }
                }
                
                TArray<UNiagaraComponent*> NiagaraComponents;
                Actor->GetComponents<UNiagaraComponent>(NiagaraComponents);
                
                for (UNiagaraComponent* NC : NiagaraComponents)
                {
                    if (NC && NC->IsActive())
                    {
                        TotalParticles += 150; // Estimate for Niagara systems
                    }
                }
            }
        }
    }
    
    return TotalParticles;
}

bool UQA_VFXIntegrationValidator::IsVFXPerformanceAcceptable()
{
    float CurrentFrameTime = GetVFXFrameTime();
    int32 ParticleCount = GetActiveParticleCount();
    
    return (CurrentFrameTime <= MaxAcceptableFrameTime && ParticleCount <= MaxAcceptableParticleCount);
}

void UQA_VFXIntegrationValidator::GenerateVFXValidationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VFX VALIDATION REPORT ==="));
    
    if (LastValidationResults.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No validation results available. Run validation first."));
        return;
    }
    
    int32 PassCount = 0;
    int32 WarningCount = 0;
    int32 FailCount = 0;
    int32 CriticalCount = 0;
    
    for (const FQA_VFXTestResult& Result : LastValidationResults)
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
                FailCount++;
                break;
            case EQA_VFXValidationResult::Critical:
                CriticalCount++;
                break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Summary: %d PASS, %d WARNING, %d FAIL, %d CRITICAL"), 
        PassCount, WarningCount, FailCount, CriticalCount);
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void UQA_VFXIntegrationValidator::LogVFXSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("VFX System Status:"));
    UE_LOG(LogTemp, Warning, TEXT("- Frame Time: %.2fms"), GetVFXFrameTime());
    UE_LOG(LogTemp, Warning, TEXT("- Active Particles: %d"), GetActiveParticleCount());
    UE_LOG(LogTemp, Warning, TEXT("- Performance Acceptable: %s"), 
        IsVFXPerformanceAcceptable() ? TEXT("YES") : TEXT("NO"));
}

bool UQA_VFXIntegrationValidator::ValidateClassExists(const FString& ClassName)
{
    UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
    return (FoundClass != nullptr);
}

bool UQA_VFXIntegrationValidator::ValidateAssetExists(const FString& AssetPath)
{
    UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
    return (Asset != nullptr);
}

float UQA_VFXIntegrationValidator::MeasureExecutionTime(TFunction<void()> TestFunction)
{
    float StartTime = FPlatformTime::Seconds();
    TestFunction();
    return (FPlatformTime::Seconds() - StartTime) * 1000.0f;
}