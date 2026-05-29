#include "Build_CriticalFinalValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Components/LightComponent.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UBuild_CriticalFinalValidator::UBuild_CriticalFinalValidator()
{
    bValidationInProgress = false;
    LastValidationTime = 0.0f;
    MinValidationScore = 85.0f;
    MinRequiredActors = 20;
    MinRequiredSystems = 6;
}

void UBuild_CriticalFinalValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Initializing final validation system"));
    
    InitializeCriticalSystemPaths();
    
    // Initialize validation metrics
    LastValidationResults = FBuild_FinalValidationMetrics();
    LastValidationResults.ValidationTimestamp = FDateTime::Now().ToString();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Initialization complete"));
}

void UBuild_CriticalFinalValidator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Deinitializing"));
    
    // Clear validation data
    SystemStatuses.Empty();
    CriticalSystemPaths.Empty();
    
    Super::Deinitialize();
}

void UBuild_CriticalFinalValidator::InitializeCriticalSystemPaths()
{
    CriticalSystemPaths.Empty();
    
    // Core systems
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.FoliageManager"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.BuildIntegrationManager"));
    
    // Additional critical systems
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.BuildValidationManager"));
    CriticalSystemPaths.Add(TEXT("/Script/TranspersonalGame.IntegrationManager"));
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Initialized %d critical system paths"), CriticalSystemPaths.Num());
}

FBuild_FinalValidationMetrics UBuild_CriticalFinalValidator::RunCompleteValidation()
{
    if (bValidationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Validation already in progress"));
        return LastValidationResults;
    }
    
    bValidationInProgress = true;
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Starting complete validation"));
    
    // Reset validation metrics
    LastValidationResults = FBuild_FinalValidationMetrics();
    LastValidationResults.ValidationTimestamp = FDateTime::Now().ToString();
    
    // Validate all critical systems
    bool bSystemsValid = ValidateAllCriticalSystems();
    LastValidationResults.bAllCriticalSystemsLoaded = bSystemsValid;
    
    // Validate MinPlayableMap
    bool bMapValid = ValidateMinPlayableMap();
    LastValidationResults.bMinPlayableMapValid = bMapValid;
    
    // Calculate overall score
    LastValidationResults.ValidationScore = CalculateOverallValidationScore();
    
    LastValidationTime = FPlatformTime::Seconds() - StartTime;
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Validation complete - Score: %.2f"), LastValidationResults.ValidationScore);
    
    return LastValidationResults;
}

bool UBuild_CriticalFinalValidator::ValidateAllCriticalSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Validating all critical systems"));
    
    int32 LoadedCount = 0;
    int32 FailedCount = 0;
    
    LastValidationResults.LoadedSystems.Empty();
    LastValidationResults.FailedSystems.Empty();
    
    for (const FString& SystemPath : CriticalSystemPaths)
    {
        if (ValidateSystemClass(SystemPath))
        {
            LoadedCount++;
            LastValidationResults.LoadedSystems.Add(SystemPath);
            UpdateSystemStatus(SystemPath, true);
        }
        else
        {
            FailedCount++;
            LastValidationResults.FailedSystems.Add(SystemPath);
            UpdateSystemStatus(SystemPath, false, TEXT("Failed to load class"));
        }
    }
    
    LastValidationResults.TotalClassesLoaded = LoadedCount;
    LastValidationResults.FailedClassesCount = FailedCount;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Systems validation - Loaded: %d, Failed: %d"), LoadedCount, FailedCount);
    
    return FailedCount == 0;
}

bool UBuild_CriticalFinalValidator::ValidateSystemClass(const FString& ClassPath)
{
    try
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        if (LoadedClass)
        {
            UE_LOG(LogTemp, Log, TEXT("Build_CriticalFinalValidator: Successfully loaded class: %s"), *ClassPath);
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Build_CriticalFinalValidator: Failed to load class: %s"), *ClassPath);
            return false;
        }
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalFinalValidator: Exception loading class: %s"), *ClassPath);
        return false;
    }
}

bool UBuild_CriticalFinalValidator::ValidateMinPlayableMap()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Validating MinPlayableMap"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalFinalValidator: No world available"));
        return false;
    }
    
    // Count all actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    LastValidationResults.LevelActorCount = AllActors.Num();
    
    // Count specific actor types
    int32 CharacterCount = 0;
    int32 DinosaurCount = 0;
    int32 LightCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        
        FString ActorName = Actor->GetName();
        
        if (Actor->IsA<ACharacter>() || ActorName.Contains(TEXT("Character")) || ActorName.Contains(TEXT("Player")))
        {
            CharacterCount++;
        }
        else if (ActorName.Contains(TEXT("Dinosaur")) || ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")))
        {
            DinosaurCount++;
        }
        else if (Actor->FindComponentByClass<ULightComponent>() || ActorName.Contains(TEXT("Light")) || ActorName.Contains(TEXT("Sun")))
        {
            LightCount++;
        }
    }
    
    LastValidationResults.CharacterActorCount = CharacterCount;
    LastValidationResults.DinosaurActorCount = DinosaurCount;
    LastValidationResults.LightActorCount = LightCount;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Map validation - Total: %d, Characters: %d, Dinosaurs: %d, Lights: %d"), 
           AllActors.Num(), CharacterCount, DinosaurCount, LightCount);
    
    // Map is valid if it has minimum required actors
    return AllActors.Num() >= MinRequiredActors;
}

float UBuild_CriticalFinalValidator::CalculateOverallValidationScore()
{
    float Score = 0.0f;
    
    // System loading score (40% weight)
    if (CriticalSystemPaths.Num() > 0)
    {
        float SystemScore = (float)LastValidationResults.TotalClassesLoaded / (float)CriticalSystemPaths.Num() * 40.0f;
        Score += SystemScore;
    }
    
    // Map validation score (30% weight)
    if (LastValidationResults.bMinPlayableMapValid)
    {
        Score += 30.0f;
    }
    
    // Actor count score (20% weight)
    if (LastValidationResults.LevelActorCount >= MinRequiredActors)
    {
        float ActorScore = FMath::Min(1.0f, (float)LastValidationResults.LevelActorCount / (float)(MinRequiredActors * 2)) * 20.0f;
        Score += ActorScore;
    }
    
    // Diversity score (10% weight)
    int32 ActorTypes = 0;
    if (LastValidationResults.CharacterActorCount > 0) ActorTypes++;
    if (LastValidationResults.DinosaurActorCount > 0) ActorTypes++;
    if (LastValidationResults.LightActorCount > 0) ActorTypes++;
    
    float DiversityScore = (float)ActorTypes / 3.0f * 10.0f;
    Score += DiversityScore;
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

void UBuild_CriticalFinalValidator::UpdateSystemStatus(const FString& SystemName, bool bLoaded, const FString& ErrorMsg)
{
    FBuild_SystemIntegrationStatus* ExistingStatus = SystemStatuses.FindByPredicate([&SystemName](const FBuild_SystemIntegrationStatus& Status)
    {
        return Status.SystemName == SystemName;
    });
    
    if (ExistingStatus)
    {
        ExistingStatus->bIsLoaded = bLoaded;
        ExistingStatus->bIsFunctional = bLoaded;
        ExistingStatus->ErrorMessage = ErrorMsg;
    }
    else
    {
        FBuild_SystemIntegrationStatus NewStatus;
        NewStatus.SystemName = SystemName;
        NewStatus.bIsLoaded = bLoaded;
        NewStatus.bIsFunctional = bLoaded;
        NewStatus.ErrorMessage = ErrorMsg;
        NewStatus.LoadTime = FPlatformTime::Seconds();
        
        SystemStatuses.Add(NewStatus);
    }
}

TArray<FBuild_SystemIntegrationStatus> UBuild_CriticalFinalValidator::GetSystemIntegrationStatus()
{
    return SystemStatuses;
}

bool UBuild_CriticalFinalValidator::IsProjectReadyForDeployment()
{
    FBuild_FinalValidationMetrics CurrentMetrics = RunCompleteValidation();
    
    bool bReady = CurrentMetrics.ValidationScore >= MinValidationScore &&
                  CurrentMetrics.bAllCriticalSystemsLoaded &&
                  CurrentMetrics.bMinPlayableMapValid &&
                  CurrentMetrics.LevelActorCount >= MinRequiredActors;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Project ready for deployment: %s (Score: %.2f)"), 
           bReady ? TEXT("YES") : TEXT("NO"), CurrentMetrics.ValidationScore);
    
    return bReady;
}

void UBuild_CriticalFinalValidator::GenerateFinalIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Generating final integration report"));
    
    FBuild_FinalValidationMetrics CurrentMetrics = RunCompleteValidation();
    
    UE_LOG(LogTemp, Warning, TEXT("=== FINAL INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Validation Score: %.2f/100"), CurrentMetrics.ValidationScore);
    UE_LOG(LogTemp, Warning, TEXT("Systems Loaded: %d/%d"), CurrentMetrics.TotalClassesLoaded, CriticalSystemPaths.Num());
    UE_LOG(LogTemp, Warning, TEXT("Map Valid: %s"), CurrentMetrics.bMinPlayableMapValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.LevelActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Characters: %d"), CurrentMetrics.CharacterActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaurs: %d"), CurrentMetrics.DinosaurActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Lights: %d"), CurrentMetrics.LightActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Ready for Deployment: %s"), IsProjectReadyForDeployment() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

bool UBuild_CriticalFinalValidator::CheckCoreSystemsHealth()
{
    // Implementation for core systems health check
    return ValidateAllCriticalSystems();
}

bool UBuild_CriticalFinalValidator::CheckWorldGenerationHealth()
{
    return ValidateSystemClass(TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
}

bool UBuild_CriticalFinalValidator::CheckCharacterSystemHealth()
{
    return ValidateSystemClass(TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
}

bool UBuild_CriticalFinalValidator::CheckAISystemHealth()
{
    return ValidateSystemClass(TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
}

bool UBuild_CriticalFinalValidator::CheckVFXSystemHealth()
{
    // VFX system health check - placeholder
    return true;
}

bool UBuild_CriticalFinalValidator::CheckAudioSystemHealth()
{
    // Audio system health check - placeholder
    return true;
}

bool UBuild_CriticalFinalValidator::VerifySystemIntegration()
{
    return ValidateAllCriticalSystems() && ValidateMinPlayableMap();
}

bool UBuild_CriticalFinalValidator::VerifyAssetIntegration()
{
    // Asset integration verification - placeholder
    return true;
}

bool UBuild_CriticalFinalValidator::VerifyLevelIntegration()
{
    return ValidateMinPlayableMap();
}

void UBuild_CriticalFinalValidator::TriggerEmergencyRecovery()
{
    UE_LOG(LogTemp, Error, TEXT("Build_CriticalFinalValidator: Emergency recovery triggered"));
    
    // Emergency recovery implementation
    // Reset validation state
    bValidationInProgress = false;
    SystemStatuses.Empty();
    
    // Reinitialize critical systems
    InitializeCriticalSystemPaths();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Emergency recovery complete"));
}

bool UBuild_CriticalFinalValidator::AttemptSystemRecovery(const FString& SystemName)
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: Attempting recovery for system: %s"), *SystemName);
    
    // Attempt to reload the system
    bool bRecovered = ValidateSystemClass(SystemName);
    
    if (bRecovered)
    {
        UpdateSystemStatus(SystemName, true, TEXT("Recovered"));
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalFinalValidator: System recovery successful: %s"), *SystemName);
    }
    else
    {
        UpdateSystemStatus(SystemName, false, TEXT("Recovery failed"));
        UE_LOG(LogTemp, Error, TEXT("Build_CriticalFinalValidator: System recovery failed: %s"), *SystemName);
    }
    
    return bRecovered;
}