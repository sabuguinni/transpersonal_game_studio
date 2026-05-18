#include "Build_FinalIntegrationOrchestrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "AudioDevice.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"

UBuild_FinalIntegrationOrchestrator::UBuild_FinalIntegrationOrchestrator()
{
    CurrentStatus = EBuild_IntegrationStatus::Unknown;
    LastValidationTime = FDateTime::Now();
}

void UBuild_FinalIntegrationOrchestrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Initializing integration system"));
    
    CurrentStatus = EBuild_IntegrationStatus::Initializing;
    
    // Schedule initial validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick([this]()
        {
            ValidateAllSystems();
        });
    }
}

void UBuild_FinalIntegrationOrchestrator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Shutting down integration system"));
    
    // Save final build snapshot
    SaveBuildSnapshot();
    
    Super::Deinitialize();
}

void UBuild_FinalIntegrationOrchestrator::ValidateAllSystems()
{
    if (bIsValidating)
    {
        return;
    }
    
    bIsValidating = true;
    CurrentStatus = EBuild_IntegrationStatus::Validating;
    ValidationResults.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Starting comprehensive system validation"));
    
    // Validate core systems
    ValidationResults.Add(ValidateSystem(TEXT("Character System"), [this]() { return ValidateCharacterSystem(); }));
    ValidationResults.Add(ValidateSystem(TEXT("Dinosaur Assets"), [this]() { return ValidateDinosaurAssets(); }));
    ValidationResults.Add(ValidateSystem(TEXT("Biome Population"), [this]() { return ValidateBiomePopulation(); }));
    ValidationResults.Add(ValidateSystem(TEXT("Physics System"), [this]() { return ValidatePhysicsSystem(); }));
    ValidationResults.Add(ValidateSystem(TEXT("Audio System"), [this]() { return ValidateAudioSystem(); }));
    
    // Update metrics
    UpdateIntegrationStatus();
    LastValidationTime = FDateTime::Now();
    bIsValidating = false;
    
    // Generate report
    GenerateIntegrationReport();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: System validation complete - Status: %s"), 
           *UEnum::GetValueAsString(CurrentStatus));
}

FBuild_IntegrationMetrics UBuild_FinalIntegrationOrchestrator::GetIntegrationMetrics() const
{
    return CurrentMetrics;
}

EBuild_IntegrationStatus UBuild_FinalIntegrationOrchestrator::GetIntegrationStatus() const
{
    return CurrentStatus;
}

TArray<FBuild_SystemValidationResult> UBuild_FinalIntegrationOrchestrator::GetSystemValidationResults() const
{
    return ValidationResults;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateCharacterSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for TranspersonalCharacter class
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TranspersonalCharacter class not found"));
        return false;
    }
    
    // Check for character instances in world
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (Character && Character->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            UE_LOG(LogTemp, Log, TEXT("Found TranspersonalCharacter instance: %s"), *Character->GetName());
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("No TranspersonalCharacter instances found in world"));
    return false;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateDinosaurAssets()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    int32 DinosaurCount = 0;
    
    // Count actors with dinosaur-related names
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("raptor")) || 
                ActorName.Contains(TEXT("tricera")) || ActorName.Contains(TEXT("brachio")) ||
                ActorName.Contains(TEXT("ankylo")) || ActorName.Contains(TEXT("dinosaur")))
            {
                DinosaurCount++;
            }
        }
    }
    
    CurrentMetrics.DinosaurActors = DinosaurCount;
    UE_LOG(LogTemp, Log, TEXT("Found %d dinosaur actors"), DinosaurCount);
    
    return DinosaurCount > 0;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateBiomePopulation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Define biome centers (from memory ID 709)
    TArray<FVector> BiomeCenters = {
        FVector(0, 0, 0),           // Savana
        FVector(-50000, -45000, 0), // Pantano
        FVector(-45000, 40000, 0),  // Floresta
        FVector(55000, 0, 0),       // Deserto
        FVector(40000, 50000, 0)    // Montanha
    };
    
    int32 TotalEnvironmentActors = 0;
    
    for (const FVector& BiomeCenter : BiomeCenters)
    {
        int32 ActorsInBiome = 0;
        
        for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AStaticMeshActor* Actor = *ActorItr;
            if (Actor)
            {
                FVector ActorLocation = Actor->GetActorLocation();
                float Distance = FVector::Dist2D(ActorLocation, BiomeCenter);
                
                if (Distance < 10000.0f) // Within 10km of biome center
                {
                    ActorsInBiome++;
                }
            }
        }
        
        TotalEnvironmentActors += ActorsInBiome;
    }
    
    CurrentMetrics.EnvironmentActors = TotalEnvironmentActors;
    UE_LOG(LogTemp, Log, TEXT("Found %d environment actors across all biomes"), TotalEnvironmentActors);
    
    return TotalEnvironmentActors > 50; // Minimum threshold
}

bool UBuild_FinalIntegrationOrchestrator::ValidatePhysicsSystem()
{
    // Check physics settings
    const UPhysicsSettings* PhysicsSettings = GetDefault<UPhysicsSettings>();
    if (!PhysicsSettings)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World || !World->GetPhysicsScene())
    {
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics system validation: PASS"));
    return true;
}

bool UBuild_FinalIntegrationOrchestrator::ValidateAudioSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FAudioDevice* AudioDevice = World->GetAudioDevice().GetAudioDevice();
    if (!AudioDevice)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio device not available"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Audio system validation: PASS"));
    return true;
}

void UBuild_FinalIntegrationOrchestrator::SaveBuildSnapshot()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_FinalIntegrationOrchestrator: Saving build snapshot"));
    
    // Update final metrics
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.TotalActors = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            CurrentMetrics.TotalActors++;
        }
    }
    
    CurrentMetrics.LastValidationTime = (FDateTime::Now() - LastValidationTime).GetTotalSeconds();
    CurrentMetrics.Status = CurrentStatus;
}

void UBuild_FinalIntegrationOrchestrator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), *UEnum::GetValueAsString(CurrentStatus));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentMetrics.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Actors: %d"), CurrentMetrics.DinosaurActors);
    UE_LOG(LogTemp, Warning, TEXT("Environment Actors: %d"), CurrentMetrics.EnvironmentActors);
    
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %s - %s"), 
               *Result.SystemName, 
               Result.bIsValid ? TEXT("PASS") : TEXT("FAIL"),
               *Result.ValidationMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

FBuild_SystemValidationResult UBuild_FinalIntegrationOrchestrator::ValidateSystem(const FString& SystemName, TFunction<bool()> ValidationFunction)
{
    FBuild_SystemValidationResult Result;
    Result.SystemName = SystemName;
    
    double StartTime = FPlatformTime::Seconds();
    
    try
    {
        Result.bIsValid = ValidationFunction();
        Result.ValidationMessage = Result.bIsValid ? TEXT("System operational") : TEXT("System validation failed");
    }
    catch (...)
    {
        Result.bIsValid = false;
        Result.ValidationMessage = TEXT("System validation exception");
    }
    
    Result.ValidationTime = FPlatformTime::Seconds() - StartTime;
    
    LogValidationResult(Result);
    return Result;
}

void UBuild_FinalIntegrationOrchestrator::UpdateIntegrationStatus()
{
    int32 PassedSystems = 0;
    int32 TotalSystems = ValidationResults.Num();
    
    for (const FBuild_SystemValidationResult& Result : ValidationResults)
    {
        if (Result.bIsValid)
        {
            PassedSystems++;
        }
    }
    
    if (TotalSystems == 0)
    {
        CurrentStatus = EBuild_IntegrationStatus::Unknown;
    }
    else if (PassedSystems == TotalSystems)
    {
        CurrentStatus = EBuild_IntegrationStatus::Stable;
    }
    else if (PassedSystems >= TotalSystems * 0.8f)
    {
        CurrentStatus = EBuild_IntegrationStatus::Degraded;
    }
    else
    {
        CurrentStatus = EBuild_IntegrationStatus::Critical;
    }
}

void UBuild_FinalIntegrationOrchestrator::LogValidationResult(const FBuild_SystemValidationResult& Result)
{
    if (Result.bIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("System validation PASS: %s (%.3fs)"), *Result.SystemName, Result.ValidationTime);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("System validation FAIL: %s - %s (%.3fs)"), 
               *Result.SystemName, *Result.ValidationMessage, Result.ValidationTime);
    }
}