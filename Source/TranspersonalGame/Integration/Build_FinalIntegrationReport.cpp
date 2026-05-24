#include "Build_FinalIntegrationReport.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuild_FinalIntegrationReport::UBuild_FinalIntegrationReport()
{
    CurrentMetrics = FBuild_IntegrationMetrics();
    ValidationResults.Empty();
    CriticalSystemPaths.Empty();
}

void UBuild_FinalIntegrationReport::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationReport: Initializing integration validation system"));
    
    InitializeCriticalSystems();
    CurrentMetrics.BuildStatus = EBuild_IntegrationStatus::Initializing;
}

void UBuild_FinalIntegrationReport::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationReport: Shutting down integration system"));
    Super::Deinitialize();
}

void UBuild_FinalIntegrationReport::InitializeCriticalSystems()
{
    CriticalSystemPaths.Empty();
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.FoliageManager"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.ProceduralWorldManager"));
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationReport: Initialized %d critical systems"), CriticalSystemPaths.Num());
}

void UBuild_FinalIntegrationReport::RunFullIntegrationValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationReport: Starting full integration validation"));
    
    CurrentMetrics.BuildStatus = EBuild_IntegrationStatus::Testing;
    ValidationResults.Empty();
    
    // Validate system loading
    ValidateSystemLoading();
    
    // Validate actor counts
    ValidateActorCounts();
    
    // Update build status
    UpdateBuildStatus();
    
    // Generate final report
    GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationReport: Integration validation complete"));
}

void UBuild_FinalIntegrationReport::ValidateSystemLoading()
{
    CurrentMetrics.LoadedSystemCount = 0;
    CurrentMetrics.FailedSystemCount = 0;
    
    for (const FString& SystemPath : CriticalSystemPaths)
    {
        FBuild_SystemValidationResult Result;
        Result.SystemName = SystemPath;
        
        if (ValidateSystemClass(SystemPath))
        {
            Result.bIsLoaded = true;
            Result.bIsCompiled = true;
            Result.ValidationScore = 100.0f;
            CurrentMetrics.LoadedSystemCount++;
            
            UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationReport: System OK - %s"), *SystemPath);
        }
        else
        {
            Result.bIsLoaded = false;
            Result.bIsCompiled = false;
            Result.ErrorMessage = TEXT("Failed to load class");
            Result.ValidationScore = 0.0f;
            CurrentMetrics.FailedSystemCount++;
            
            UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationReport: System FAILED - %s"), *SystemPath);
        }
        
        ValidationResults.Add(Result);
    }
}

void UBuild_FinalIntegrationReport::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_FinalIntegrationReport: No world available for actor validation"));
        return;
    }
    
    CurrentMetrics.TotalActorCount = 0;
    CurrentMetrics.DinosaurActorCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            CurrentMetrics.TotalActorCount++;
            
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Trex")) || 
                ActorName.Contains(TEXT("Velociraptor")) || 
                ActorName.Contains(TEXT("Triceratops")) || 
                ActorName.Contains(TEXT("Brachiosaurus")))
            {
                CurrentMetrics.DinosaurActorCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationReport: Found %d total actors, %d dinosaurs"), 
           CurrentMetrics.TotalActorCount, CurrentMetrics.DinosaurActorCount);
}

bool UBuild_FinalIntegrationReport::ValidateSystemClass(const FString& ClassPath)
{
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return LoadedClass != nullptr;
}

void UBuild_FinalIntegrationReport::UpdateBuildStatus()
{
    CurrentMetrics.OverallBuildScore = CalculateBuildScore();
    
    if (CurrentMetrics.OverallBuildScore >= 100.0f)
    {
        CurrentMetrics.BuildStatus = EBuild_IntegrationStatus::Complete;
    }
    else if (CurrentMetrics.OverallBuildScore >= 75.0f)
    {
        CurrentMetrics.BuildStatus = EBuild_IntegrationStatus::Validating;
    }
    else
    {
        CurrentMetrics.BuildStatus = EBuild_IntegrationStatus::Failed;
    }
}

float UBuild_FinalIntegrationReport::CalculateBuildScore() const
{
    float SystemScore = (CurrentMetrics.LoadedSystemCount * 25.0f);
    float ActorScore = FMath::Min(CurrentMetrics.DinosaurActorCount * 5.0f, 50.0f);
    float TotalScore = SystemScore + ActorScore;
    
    return FMath::Clamp(TotalScore, 0.0f, 150.0f);
}

void UBuild_FinalIntegrationReport::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Build Score: %.1f/150"), CurrentMetrics.OverallBuildScore);
    UE_LOG(LogTemp, Warning, TEXT("Systems Loaded: %d/%d"), CurrentMetrics.LoadedSystemCount, CriticalSystemPaths.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActorCount);
    
    FString StatusString;
    switch (CurrentMetrics.BuildStatus)
    {
        case EBuild_IntegrationStatus::Complete:
            StatusString = TEXT("EXCELLENT - Ready for production");
            break;
        case EBuild_IntegrationStatus::Validating:
            StatusString = TEXT("GOOD - Minor issues to resolve");
            break;
        case EBuild_IntegrationStatus::Failed:
            StatusString = TEXT("NEEDS_WORK - Critical systems missing");
            break;
        default:
            StatusString = TEXT("UNKNOWN");
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build Status: %s"), *StatusString);
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION REPORT ==="));
}

FBuild_IntegrationMetrics UBuild_FinalIntegrationReport::GetCurrentBuildMetrics() const
{
    return CurrentMetrics;
}

TArray<FBuild_SystemValidationResult> UBuild_FinalIntegrationReport::GetSystemValidationResults() const
{
    return ValidationResults;
}