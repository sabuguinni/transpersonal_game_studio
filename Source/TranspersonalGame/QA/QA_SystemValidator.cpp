#include "QA_SystemValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

UQA_SystemValidator::UQA_SystemValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    bAutoRunOnBeginPlay = false;
    ValidationInterval = 30.0f;
    MaxActorCountThreshold = 10000;
    MaxFrameTimeThreshold = 33.33f; // 30 FPS threshold
    LastValidationTime = 0.0f;
}

void UQA_SystemValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        RunFullSystemValidation();
    }
}

void UQA_SystemValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastValidationTime += DeltaTime;
    if (LastValidationTime >= ValidationInterval)
    {
        RunFullSystemValidation();
        LastValidationTime = 0.0f;
    }
}

void UQA_SystemValidator::RunFullSystemValidation()
{
    ClearValidationReports();
    
    float StartTime = FPlatformTime::Seconds();
    
    // Validate core systems
    bool VFXValid = ValidateVFXSystems();
    bool CharacterValid = ValidateCharacterSystems();
    bool WorldValid = ValidateWorldSystems();
    bool PerformanceValid = ValidatePerformanceMetrics();
    
    float TotalTime = FPlatformTime::Seconds() - StartTime;
    
    FString OverallResult = FString::Printf(TEXT("VFX:%s Character:%s World:%s Performance:%s"),
        VFXValid ? TEXT("PASS") : TEXT("FAIL"),
        CharacterValid ? TEXT("PASS") : TEXT("FAIL"),
        WorldValid ? TEXT("PASS") : TEXT("FAIL"),
        PerformanceValid ? TEXT("PASS") : TEXT("FAIL"));
    
    EQA_ValidationResult OverallStatus = (VFXValid && CharacterValid && WorldValid && PerformanceValid) 
        ? EQA_ValidationResult::Pass 
        : EQA_ValidationResult::Fail;
    
    AddValidationReport(TEXT("Full System Validation"), OverallStatus, OverallResult, TotalTime);
    
    UE_LOG(LogTemp, Warning, TEXT("QA Validation Complete: %s (%.2fs)"), *OverallResult, TotalTime);
}

bool UQA_SystemValidator::ValidateVFXSystems()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    
    // Test VFX class loading
    if (!ValidateClassLoading(TEXT("VFX_ImpactManager"), TEXT("/Script/TranspersonalGame.VFX_ImpactManager")))
    {
        bSuccess = false;
    }
    
    // Test Niagara system availability
    UWorld* World = GetWorld();
    if (World)
    {
        int32 NiagaraActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Niagara")))
            {
                NiagaraActorCount++;
            }
        }
        
        if (NiagaraActorCount == 0)
        {
            AddValidationReport(TEXT("VFX Niagara Systems"), EQA_ValidationResult::Warning, 
                TEXT("No Niagara actors found in level"), 0.0f);
        }
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddValidationReport(TEXT("VFX Systems"), bSuccess ? EQA_ValidationResult::Pass : EQA_ValidationResult::Fail,
        bSuccess ? TEXT("All VFX systems operational") : TEXT("VFX system failures detected"), ExecutionTime);
    
    return bSuccess;
}

bool UQA_SystemValidator::ValidateCharacterSystems()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    
    // Test character class loading
    if (!ValidateClassLoading(TEXT("TranspersonalCharacter"), TEXT("/Script/TranspersonalGame.TranspersonalCharacter")))
    {
        bSuccess = false;
    }
    
    // Test game mode class loading
    if (!ValidateClassLoading(TEXT("TranspersonalGameMode"), TEXT("/Script/TranspersonalGame.TranspersonalGameMode")))
    {
        bSuccess = false;
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddValidationReport(TEXT("Character Systems"), bSuccess ? EQA_ValidationResult::Pass : EQA_ValidationResult::Fail,
        bSuccess ? TEXT("All character systems operational") : TEXT("Character system failures detected"), ExecutionTime);
    
    return bSuccess;
}

bool UQA_SystemValidator::ValidateWorldSystems()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        AddValidationReport(TEXT("World Systems"), EQA_ValidationResult::Critical, TEXT("No valid world found"), 0.0f);
        return false;
    }
    
    // Count actors and check for reasonable limits
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    if (ActorCount > MaxActorCountThreshold)
    {
        bSuccess = false;
        AddValidationReport(TEXT("Actor Count"), EQA_ValidationResult::Warning, 
            FString::Printf(TEXT("High actor count: %d (threshold: %d)"), ActorCount, MaxActorCountThreshold), 0.0f);
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddValidationReport(TEXT("World Systems"), bSuccess ? EQA_ValidationResult::Pass : EQA_ValidationResult::Warning,
        FString::Printf(TEXT("World valid with %d actors"), ActorCount), ExecutionTime);
    
    return bSuccess;
}

bool UQA_SystemValidator::ValidatePerformanceMetrics()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    
    // Check memory usage
    if (!CheckMemoryUsage())
    {
        bSuccess = false;
    }
    
    // Check frame rate
    if (!CheckFrameRate())
    {
        bSuccess = false;
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    AddValidationReport(TEXT("Performance Metrics"), bSuccess ? EQA_ValidationResult::Pass : EQA_ValidationResult::Warning,
        bSuccess ? TEXT("Performance within acceptable limits") : TEXT("Performance issues detected"), ExecutionTime);
    
    return bSuccess;
}

TArray<FQA_ValidationReport> UQA_SystemValidator::GetValidationReports() const
{
    return ValidationReports;
}

void UQA_SystemValidator::ClearValidationReports()
{
    ValidationReports.Empty();
}

void UQA_SystemValidator::AddValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime)
{
    FQA_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Details = Details;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
}

bool UQA_SystemValidator::ValidateClassLoading(const FString& ClassName, const FString& ClassPath)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    if (!LoadedClass)
    {
        AddValidationReport(FString::Printf(TEXT("Class Loading - %s"), *ClassName), 
            EQA_ValidationResult::Fail, 
            FString::Printf(TEXT("Failed to load class: %s"), *ClassPath), 0.0f);
        return false;
    }
    
    AddValidationReport(FString::Printf(TEXT("Class Loading - %s"), *ClassName), 
        EQA_ValidationResult::Pass, 
        FString::Printf(TEXT("Successfully loaded: %s"), *ClassPath), 0.0f);
    return true;
}

bool UQA_SystemValidator::ValidateActorSpawning(UClass* ActorClass)
{
    if (!ActorClass)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (SpawnedActor)
    {
        SpawnedActor->Destroy();
        return true;
    }
    
    return false;
}

bool UQA_SystemValidator::CheckMemoryUsage()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float MemoryUsageGB = MemStats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
    
    if (MemoryUsageGB > 8.0f) // 8GB threshold
    {
        AddValidationReport(TEXT("Memory Usage"), EQA_ValidationResult::Warning,
            FString::Printf(TEXT("High memory usage: %.2f GB"), MemoryUsageGB), 0.0f);
        return false;
    }
    
    return true;
}

bool UQA_SystemValidator::CheckFrameRate()
{
    if (GEngine && GEngine->GameViewport)
    {
        float CurrentFrameTime = GEngine->GameViewport->GetAverageFrameTime();
        float CurrentFrameTimeMs = CurrentFrameTime * 1000.0f;
        
        if (CurrentFrameTimeMs > MaxFrameTimeThreshold)
        {
            AddValidationReport(TEXT("Frame Rate"), EQA_ValidationResult::Warning,
                FString::Printf(TEXT("Low frame rate: %.2f ms (%.1f FPS)"), CurrentFrameTimeMs, 1000.0f / CurrentFrameTimeMs), 0.0f);
            return false;
        }
    }
    
    return true;
}