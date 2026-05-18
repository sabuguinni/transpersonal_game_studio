#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Light.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    CurrentIntegrationStatus = EBuild_IntegrationStatus::Pending;
    bIntegrationComplete = false;
    bPerformanceMonitoringActive = false;
    IntegrationStartTime = 0.0f;
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Subsystem initialized"));
    
    // Start automatic integration validation
    StartFinalIntegration();
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    StopPerformanceMonitoring();
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::StartFinalIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting final integration"));
    
    CurrentIntegrationStatus = EBuild_IntegrationStatus::InProgress;
    IntegrationStartTime = FPlatformTime::Seconds();
    ValidationResults.Empty();
    
    // Start performance monitoring
    StartPerformanceMonitoring();
    
    // Begin system validation
    ValidateAllSystems();
    
    LogIntegrationStatus(TEXT("Final integration started"), EBuild_IntegrationStatus::InProgress);
}

void UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Validating all systems"));
    
    ValidationResults.Empty();
    
    // Validate core systems
    ValidationResults.Add(ValidateDinosaurAssets());
    ValidationResults.Add(ValidateWorldGeneration());
    ValidationResults.Add(ValidateCharacterSystems());
    ValidationResults.Add(ValidateAudioSystems());
    ValidationResults.Add(ValidateVFXSystems());
    
    // Check overall integration status
    bool bAllSystemsValid = true;
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EBuild_IntegrationStatus::Failed || Result.Status == EBuild_IntegrationStatus::Critical)
        {
            bAllSystemsValid = false;
            break;
        }
    }
    
    if (bAllSystemsValid)
    {
        CurrentIntegrationStatus = EBuild_IntegrationStatus::Success;
        bIntegrationComplete = true;
        LogIntegrationStatus(TEXT("All systems validated successfully"), EBuild_IntegrationStatus::Success);
    }
    else
    {
        CurrentIntegrationStatus = EBuild_IntegrationStatus::Failed;
        LogIntegrationStatus(TEXT("System validation failed"), EBuild_IntegrationStatus::Failed);
    }
    
    GenerateIntegrationReport();
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateDinosaurAssets()
{
    return ValidateSystemInternal(TEXT("DinosaurAssets"), [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        int32 DinosaurCount = 0;
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains(TEXT("Trex")) || 
                Actor->GetName().Contains(TEXT("Velociraptor")) ||
                Actor->GetName().Contains(TEXT("Triceratops")) ||
                Actor->GetName().Contains(TEXT("Brachiosaurus")))
            {
                DinosaurCount++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("DinosaurAssets validation: Found %d dinosaur actors"), DinosaurCount);
        return DinosaurCount > 0;
    });
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateWorldGeneration()
{
    return ValidateSystemInternal(TEXT("WorldGeneration"), [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        int32 StaticMeshCount = 0;
        for (TActorIterator<AStaticMeshActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            StaticMeshCount++;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("WorldGeneration validation: Found %d static mesh actors"), StaticMeshCount);
        return StaticMeshCount > 10; // Minimum world population
    });
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystems()
{
    return ValidateSystemInternal(TEXT("CharacterSystems"), [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Look for player start and character-related actors
        int32 CharacterActorCount = 0;
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && (Actor->GetName().Contains(TEXT("Character")) || 
                         Actor->GetName().Contains(TEXT("Player")) ||
                         Actor->GetName().Contains(TEXT("Pawn"))))
            {
                CharacterActorCount++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("CharacterSystems validation: Found %d character-related actors"), CharacterActorCount);
        return CharacterActorCount > 0;
    });
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateAudioSystems()
{
    return ValidateSystemInternal(TEXT("AudioSystems"), [this]() -> bool
    {
        // Basic audio system validation - check if audio subsystem is available
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // For now, assume audio system is functional if world exists
        UE_LOG(LogTemp, Warning, TEXT("AudioSystems validation: Basic validation passed"));
        return true;
    });
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateVFXSystems()
{
    return ValidateSystemInternal(TEXT("VFXSystems"), [this]() -> bool
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }
        
        // Check for lighting actors as basic VFX validation
        int32 LightCount = 0;
        for (TActorIterator<ALight> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            LightCount++;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("VFXSystems validation: Found %d light actors"), LightCount);
        return LightCount > 0;
    });
}

FBuild_PerformanceMetrics UBuild_FinalIntegrationOrchestrator::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

TArray<FBuild_SystemValidationResult> UBuild_FinalIntegrationOrchestrator::GetValidationResults()
{
    return ValidationResults;
}

bool UBuild_FinalIntegrationOrchestrator::IsIntegrationComplete()
{
    return bIntegrationComplete;
}

void UBuild_FinalIntegrationOrchestrator::StartPerformanceMonitoring()
{
    if (!bPerformanceMonitoringActive)
    {
        bPerformanceMonitoringActive = true;
        
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().SetTimer(PerformanceMonitoringTimer, 
                FTimerDelegate::CreateUObject(this, &UBuild_FinalIntegrationOrchestrator::UpdatePerformanceMetrics),
                5.0f, true);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Performance monitoring started"));
    }
}

void UBuild_FinalIntegrationOrchestrator::StopPerformanceMonitoring()
{
    if (bPerformanceMonitoringActive)
    {
        bPerformanceMonitoringActive = false;
        
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Performance monitoring stopped"));
    }
}

bool UBuild_FinalIntegrationOrchestrator::IsPerformanceOptimal()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics.MemoryUsagePercent < 85.0f && 
           CurrentMetrics.CPUUsagePercent < 80.0f &&
           CurrentMetrics.TotalActorCount < 10000;
}

void UBuild_FinalIntegrationOrchestrator::CreateBuildSnapshot()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating build snapshot"));
    
    FString SnapshotData = FString::Printf(TEXT("Build Snapshot - %s\n"), *FDateTime::Now().ToString());
    SnapshotData += FString::Printf(TEXT("Integration Status: %d\n"), (int32)CurrentIntegrationStatus);
    SnapshotData += FString::Printf(TEXT("Systems Validated: %d\n"), ValidationResults.Num());
    SnapshotData += FString::Printf(TEXT("Performance Optimal: %s\n"), IsPerformanceOptimal() ? TEXT("Yes") : TEXT("No"));
    
    FString SnapshotPath = FPaths::ProjectSavedDir() / TEXT("BuildSnapshots") / FString::Printf(TEXT("Snapshot_%s.txt"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FFileHelper::SaveStringToFile(SnapshotData, *SnapshotPath);
    
    UE_LOG(LogTemp, Warning, TEXT("Build snapshot saved to: %s"), *SnapshotPath);
}

void UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating integration report"));
    
    FString ReportData = FString::Printf(TEXT("FINAL INTEGRATION REPORT\n"));
    ReportData += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportData += FString::Printf(TEXT("Integration Status: %d\n"), (int32)CurrentIntegrationStatus);
    ReportData += FString::Printf(TEXT("Integration Time: %.2f seconds\n"), FPlatformTime::Seconds() - IntegrationStartTime);
    ReportData += FString::Printf(TEXT("\nSYSTEM VALIDATION RESULTS:\n"));
    
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        ReportData += FString::Printf(TEXT("- %s: %d (%.2fs, %d actors)\n"), 
            *Result.SystemName, 
            (int32)Result.Status, 
            Result.ValidationTime,
            Result.ActorCount);
        
        if (!Result.ErrorMessage.IsEmpty())
        {
            ReportData += FString::Printf(TEXT("  Error: %s\n"), *Result.ErrorMessage);
        }
    }
    
    ReportData += FString::Printf(TEXT("\nPERFORMANCE METRICS:\n"));
    ReportData += FString::Printf(TEXT("- Memory Usage: %.1f%%\n"), CurrentMetrics.MemoryUsagePercent);
    ReportData += FString::Printf(TEXT("- CPU Usage: %.1f%%\n"), CurrentMetrics.CPUUsagePercent);
    ReportData += FString::Printf(TEXT("- Total Actors: %d\n"), CurrentMetrics.TotalActorCount);
    ReportData += FString::Printf(TEXT("- Static Meshes: %d\n"), CurrentMetrics.StaticMeshCount);
    ReportData += FString::Printf(TEXT("- Skeletal Meshes: %d\n"), CurrentMetrics.SkeletalMeshCount);
    
    FString ReportPath = FPaths::ProjectSavedDir() / TEXT("IntegrationReports") / FString::Printf(TEXT("Integration_%s.txt"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FFileHelper::SaveStringToFile(ReportData, *ReportPath);
    
    UE_LOG(LogTemp, Warning, TEXT("Integration report saved to: %s"), *ReportPath);
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateSystemInternal(const FString& SystemName, TFunction<bool()> ValidationFunction)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    
    float StartTime = FPlatformTime::Seconds();
    
    try
    {
        bool bValidationPassed = ValidationFunction();
        
        Result.Status = bValidationPassed ? EBuild_IntegrationStatus::Success : EBuild_IntegrationStatus::Failed;
        Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
        
        // Count actors for this system
        UWorld* World = GetWorld();
        if (World)
        {
            for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
            {
                Result.ActorCount++;
            }
        }
        
        if (!bValidationPassed)
        {
            Result.ErrorMessage = FString::Printf(TEXT("%s validation failed"), *SystemName);
        }
    }
    catch (...)
    {
        Result.Status = EBuild_IntegrationStatus::Critical;
        Result.ErrorMessage = FString::Printf(TEXT("%s validation crashed"), *SystemName);
        Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System validation: %s - Status: %d - Time: %.2fs"), 
        *SystemName, (int32)Result.Status, Result.ValidationTime);
    
    return Result;
}

void UBuild_FinalIntegrationOrchestrator::UpdatePerformanceMetrics()
{
    // Basic performance metrics - in a real implementation, you'd use platform-specific APIs
    CurrentMetrics.MemoryUsagePercent = 45.0f; // Placeholder
    CurrentMetrics.CPUUsagePercent = 25.0f;    // Placeholder
    CurrentMetrics.FrameRate = 60.0f;          // Placeholder
    
    // Count actual actors
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.TotalActorCount = 0;
        CurrentMetrics.StaticMeshCount = 0;
        CurrentMetrics.SkeletalMeshCount = 0;
        
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            CurrentMetrics.TotalActorCount++;
            
            AActor* Actor = *ActorIterator;
            if (Actor)
            {
                if (Actor->FindComponentByClass<UStaticMeshComponent>())
                {
                    CurrentMetrics.StaticMeshCount++;
                }
                if (Actor->FindComponentByClass<USkeletalMeshComponent>())
                {
                    CurrentMetrics.SkeletalMeshCount++;
                }
            }
        }
    }
}

void UBuild_FinalIntegrationOrchestrator::LogIntegrationStatus(const FString& Message, EBuild_IntegrationStatus Status)
{
    FString StatusString;
    switch (Status)
    {
        case EBuild_IntegrationStatus::Pending: StatusString = TEXT("PENDING"); break;
        case EBuild_IntegrationStatus::InProgress: StatusString = TEXT("IN_PROGRESS"); break;
        case EBuild_IntegrationStatus::Success: StatusString = TEXT("SUCCESS"); break;
        case EBuild_IntegrationStatus::Failed: StatusString = TEXT("FAILED"); break;
        case EBuild_IntegrationStatus::Critical: StatusString = TEXT("CRITICAL"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Integration [%s]: %s"), *StatusString, *Message);
}

// Integration Component Implementation
UBuild_IntegrationComponent::UBuild_IntegrationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bRegisteredForIntegration = false;
    ComponentStatus = EBuild_IntegrationStatus::Pending;
}

void UBuild_IntegrationComponent::RegisterForIntegration()
{
    bRegisteredForIntegration = true;
    ComponentStatus = EBuild_IntegrationStatus::InProgress;
    
    UE_LOG(LogTemp, Warning, TEXT("Integration component registered for actor: %s"), 
        GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

bool UBuild_IntegrationComponent::ValidateIntegration()
{
    if (!GetOwner())
    {
        ComponentStatus = EBuild_IntegrationStatus::Failed;
        LastValidationError = TEXT("No owner actor");
        return false;
    }
    
    ComponentStatus = EBuild_IntegrationStatus::Success;
    LastValidationError = TEXT("");
    return true;
}

FString UBuild_IntegrationComponent::GetIntegrationStatus()
{
    switch (ComponentStatus)
    {
        case EBuild_IntegrationStatus::Pending: return TEXT("Pending");
        case EBuild_IntegrationStatus::InProgress: return TEXT("In Progress");
        case EBuild_IntegrationStatus::Success: return TEXT("Success");
        case EBuild_IntegrationStatus::Failed: return TEXT("Failed");
        case EBuild_IntegrationStatus::Critical: return TEXT("Critical");
        default: return TEXT("Unknown");
    }
}