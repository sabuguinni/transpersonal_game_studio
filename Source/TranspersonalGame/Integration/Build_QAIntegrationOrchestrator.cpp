#include "Build_QAIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/UObjectIterator.h"

UBuild_QAIntegrationOrchestrator::UBuild_QAIntegrationOrchestrator()
{
    bIntegrationValid = false;
    LastValidationTime = 0.0f;
}

void UBuild_QAIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_QAIntegrationOrchestrator initialized"));
    
    // Initialize integration validation
    bIntegrationValid = false;
    LastValidationTime = FPlatformTime::Seconds();
    
    // Clear cached results
    CachedTestResults.Empty();
    LastReport = FBuild_IntegrationReport();
}

void UBuild_QAIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_QAIntegrationOrchestrator deinitialized"));
    Super::Deinitialize();
}

FBuild_IntegrationReport UBuild_QAIntegrationOrchestrator::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    
    if (UWorld* World = GetWorld())
    {
        // Count total actors
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        Report.TotalActors = AllActors.Num();
        
        // Count dinosaurs
        int32 DinoCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Dino")))
            {
                DinoCount++;
            }
        }
        Report.DinosaurCount = DinoCount;
        
        // Count VFX actors
        int32 VFXCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->FindComponentByClass<UParticleSystemComponent>())
            {
                VFXCount++;
            }
        }
        Report.VFXActors = VFXCount;
        
        // Count audio actors
        int32 AudioCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->FindComponentByClass<UAudioComponent>())
            {
                AudioCount++;
            }
        }
        Report.AudioActors = AudioCount;
        
        // Parse QA test results
        Report.TestResults = ParseQATestResults();
        
        // Validate build integrity
        Report.bBuildValid = ValidateBuildIntegrity();
        Report.BuildStatus = Report.bBuildValid ? TEXT("Valid") : TEXT("Invalid");
        
        UE_LOG(LogTemp, Warning, TEXT("Integration Report: %d actors, %d dinos, %d VFX, %d audio"), 
               Report.TotalActors, Report.DinosaurCount, Report.VFXActors, Report.AudioActors);
    }
    
    LastReport = Report;
    return Report;
}

bool UBuild_QAIntegrationOrchestrator::ValidateQATestResults()
{
    TArray<FBuild_QATestResult> Results = ParseQATestResults();
    
    bool bAllPassed = true;
    for (const FBuild_QATestResult& Result : Results)
    {
        if (!Result.bPassed)
        {
            bAllPassed = false;
            UE_LOG(LogTemp, Error, TEXT("QA Test Failed: %s - %s"), *Result.TestName, *Result.ErrorMessage);
        }
    }
    
    return bAllPassed;
}

bool UBuild_QAIntegrationOrchestrator::ValidateActorLimits()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        const int32 MaxActors = 8000;
        if (AllActors.Num() > MaxActors)
        {
            UE_LOG(LogTemp, Error, TEXT("Actor limit exceeded: %d/%d"), AllActors.Num(), MaxActors);
            return false;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Actor count within limits: %d/%d"), AllActors.Num(), MaxActors);
        return true;
    }
    
    return false;
}

bool UBuild_QAIntegrationOrchestrator::ValidateDinosaurLimits()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 DinoCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Dino")))
            {
                DinoCount++;
            }
        }
        
        const int32 MaxDinosaurs = 150;
        if (DinoCount > MaxDinosaurs)
        {
            UE_LOG(LogTemp, Error, TEXT("Dinosaur limit exceeded: %d/%d"), DinoCount, MaxDinosaurs);
            return false;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Dinosaur count within limits: %d/%d"), DinoCount, MaxDinosaurs);
        return true;
    }
    
    return false;
}

bool UBuild_QAIntegrationOrchestrator::ValidateVFXSystems()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 VFXCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->FindComponentByClass<UParticleSystemComponent>())
            {
                VFXCount++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("VFX systems validated: %d active"), VFXCount);
        return VFXCount > 0;
    }
    
    return false;
}

bool UBuild_QAIntegrationOrchestrator::ValidateAudioSystems()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        int32 AudioCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->FindComponentByClass<UAudioComponent>())
            {
                AudioCount++;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Audio systems validated: %d active"), AudioCount);
        return AudioCount > 0;
    }
    
    return false;
}

bool UBuild_QAIntegrationOrchestrator::ValidateBuildIntegrity()
{
    // Check if critical systems are loaded
    bool bCriticalSystemsLoaded = true;
    
    // Validate QA framework
    UClass* QAFrameworkClass = FindObject<UClass>(ANY_PACKAGE, TEXT("QA_TestFramework"));
    if (!QAFrameworkClass)
    {
        UE_LOG(LogTemp, Error, TEXT("QA_TestFramework class not found"));
        bCriticalSystemsLoaded = false;
    }
    
    // Validate build integration manager
    UClass* BuildManagerClass = FindObject<UClass>(ANY_PACKAGE, TEXT("Build_QAIntegrationManager"));
    if (!BuildManagerClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_QAIntegrationManager class not found - may be expected"));
    }
    
    // Check world state
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World not available for validation"));
        bCriticalSystemsLoaded = false;
    }
    
    return bCriticalSystemsLoaded;
}

void UBuild_QAIntegrationOrchestrator::CleanupExcessActors()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        const int32 MaxActors = 8000;
        if (AllActors.Num() > MaxActors)
        {
            int32 ActorsToRemove = AllActors.Num() - MaxActors;
            UE_LOG(LogTemp, Warning, TEXT("Cleaning up %d excess actors"), ActorsToRemove);
            
            // Remove oldest non-critical actors
            for (int32 i = 0; i < ActorsToRemove && i < AllActors.Num(); i++)
            {
                AActor* Actor = AllActors[i];
                if (Actor && !Actor->GetName().Contains(TEXT("PlayerStart")) && 
                    !Actor->GetName().Contains(TEXT("GameMode")))
                {
                    Actor->Destroy();
                }
            }
        }
    }
}

TArray<FBuild_QATestResult> UBuild_QAIntegrationOrchestrator::ParseQATestResults()
{
    TArray<FBuild_QATestResult> Results;
    
    // Create sample test results based on current world state
    if (UWorld* World = GetWorld())
    {
        FBuild_QATestResult ActorCountTest;
        ActorCountTest.TestName = TEXT("Actor Count Validation");
        
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        ActorCountTest.ActorsAffected = AllActors.Num();
        ActorCountTest.bPassed = AllActors.Num() <= 8000;
        ActorCountTest.ErrorMessage = ActorCountTest.bPassed ? TEXT("") : TEXT("Actor count exceeds limit");
        ActorCountTest.ExecutionTime = 0.1f;
        Results.Add(ActorCountTest);
        
        FBuild_QATestResult DinosaurTest;
        DinosaurTest.TestName = TEXT("Dinosaur Limit Validation");
        
        int32 DinoCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Dino")))
            {
                DinoCount++;
            }
        }
        DinosaurTest.ActorsAffected = DinoCount;
        DinosaurTest.bPassed = DinoCount <= 150;
        DinosaurTest.ErrorMessage = DinosaurTest.bPassed ? TEXT("") : TEXT("Dinosaur count exceeds limit");
        DinosaurTest.ExecutionTime = 0.05f;
        Results.Add(DinosaurTest);
    }
    
    CachedTestResults = Results;
    return Results;
}

bool UBuild_QAIntegrationOrchestrator::ExecuteIntegrationValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Executing integration validation"));
    
    bool bValidationPassed = true;
    
    // Validate actor limits
    if (!ValidateActorLimits())
    {
        bValidationPassed = false;
        CleanupExcessActors();
    }
    
    // Validate dinosaur limits
    if (!ValidateDinosaurLimits())
    {
        bValidationPassed = false;
    }
    
    // Validate QA test results
    if (!ValidateQATestResults())
    {
        bValidationPassed = false;
    }
    
    // Validate build integrity
    if (!ValidateBuildIntegrity())
    {
        bValidationPassed = false;
    }
    
    bIntegrationValid = bValidationPassed;
    LastValidationTime = FPlatformTime::Seconds();
    
    LogIntegrationStatus(bValidationPassed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bValidationPassed;
}

void UBuild_QAIntegrationOrchestrator::LogIntegrationStatus(const FString& Status)
{
    UE_LOG(LogTemp, Warning, TEXT("Integration Status: %s"), *Status);
    
    FBuild_IntegrationReport Report = GenerateIntegrationReport();
    UE_LOG(LogTemp, Warning, TEXT("Report Summary: %d actors, %d tests, build %s"), 
           Report.TotalActors, Report.TestResults.Num(), *Report.BuildStatus);
}

void UBuild_QAIntegrationOrchestrator::ValidateSystemIntegration()
{
    // Validate cross-system compatibility
    CheckCrossSystemCompatibility();
    ValidatePerformanceMetrics();
    CheckModuleCompilation();
    GenerateIntegrationMetrics();
}

void UBuild_QAIntegrationOrchestrator::CheckCrossSystemCompatibility()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking cross-system compatibility"));
    
    // Check VFX-Audio integration
    ValidateVFXSystems();
    ValidateAudioSystems();
}

void UBuild_QAIntegrationOrchestrator::ValidatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        float PerformanceScore = FMath::Clamp(1.0f - (AllActors.Num() / 8000.0f), 0.0f, 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("Performance score: %.2f"), PerformanceScore);
    }
}

bool UBuild_QAIntegrationOrchestrator::CheckModuleCompilation()
{
    // Check if critical modules are compiled and loaded
    bool bModulesValid = true;
    
    UClass* TranspersonalGameClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (!TranspersonalGameClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TranspersonalCharacter class not found"));
        bModulesValid = false;
    }
    
    return bModulesValid;
}

void UBuild_QAIntegrationOrchestrator::GenerateIntegrationMetrics()
{
    FBuild_IntegrationReport Report = GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Metrics Generated:"));
    UE_LOG(LogTemp, Warning, TEXT("- Total Actors: %d"), Report.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("- Dinosaurs: %d"), Report.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("- VFX Actors: %d"), Report.VFXActors);
    UE_LOG(LogTemp, Warning, TEXT("- Audio Actors: %d"), Report.AudioActors);
    UE_LOG(LogTemp, Warning, TEXT("- Build Valid: %s"), Report.bBuildValid ? TEXT("Yes") : TEXT("No"));
}