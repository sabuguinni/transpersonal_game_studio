#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Atmosphere/AtmosphericFog.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    bAutoCleanDuplicates = true;
    bValidateSystemIntegration = true;
    bLogDetailedReports = true;
    
    MaxAllowedDuplicates = 1;
    ValidationCheckInterval = 10.0f;
    LastValidationTime = 0.0f;
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager started"));
    
    if (bAutoCleanDuplicates)
    {
        // Initial cleanup on level start
        CleanDuplicateActors();
    }
    
    if (bValidateSystemIntegration)
    {
        // Initial system validation
        ValidateSystemIntegration();
    }
}

void UBuildIntegrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastValidationTime += DeltaTime;
    
    if (LastValidationTime >= ValidationCheckInterval)
    {
        if (bAutoCleanDuplicates)
        {
            CleanDuplicateActors();
        }
        
        if (bValidateSystemIntegration)
        {
            ValidateSystemIntegration();
        }
        
        LastValidationTime = 0.0f;
    }
}

void UBuildIntegrationManager::CleanDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("No valid world for duplicate cleanup"));
        return;
    }
    
    // Critical lighting types that should have only one instance
    TArray<UClass*> CriticalTypes = {
        ADirectionalLight::StaticClass(),
        ASkyLight::StaticClass(),
        AExponentialHeightFog::StaticClass()
    };
    
    int32 TotalDestroyed = 0;
    
    for (UClass* ActorClass : CriticalTypes)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ActorClass, FoundActors);
        
        if (FoundActors.Num() > MaxAllowedDuplicates)
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("Found %d %s actors, keeping %d, destroying %d"), 
                   FoundActors.Num(), *ActorClass->GetName(), MaxAllowedDuplicates, FoundActors.Num() - MaxAllowedDuplicates);
            
            // Keep the first N actors, destroy the rest
            for (int32 i = MaxAllowedDuplicates; i < FoundActors.Num(); i++)
            {
                if (FoundActors[i] && IsValid(FoundActors[i]))
                {
                    FoundActors[i]->Destroy();
                    TotalDestroyed++;
                    
                    UE_LOG(LogBuildIntegration, Log, TEXT("Destroyed duplicate %s #%d"), 
                           *ActorClass->GetName(), i + 1);
                }
            }
        }
    }
    
    if (TotalDestroyed > 0)
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("Cleanup complete: %d duplicate actors destroyed"), TotalDestroyed);
        OnDuplicatesCleanedDelegate.Broadcast(TotalDestroyed);
    }
}

void UBuildIntegrationManager::ValidateSystemIntegration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FBuildValidationReport Report;
    Report.ValidationTime = FDateTime::Now();
    Report.bIsValid = true;
    
    // Count actors by category
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    Report.TotalActorCount = AllActors.Num();
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValid(Actor))
        {
            continue;
        }
        
        FString ActorClassName = Actor->GetClass()->GetName();
        FString ActorLabel = Actor->GetActorLabel();
        
        // Categorize actors
        if (ActorClassName.Contains(TEXT("Light")) || ActorClassName.Contains(TEXT("Sky")) || ActorClassName.Contains(TEXT("Fog")))
        {
            Report.LightingActorCount++;
        }
        else if (ActorClassName.Contains(TEXT("Landscape")) || ActorClassName.Contains(TEXT("StaticMesh")) || 
                 ActorLabel.Contains(TEXT("Tree")) || ActorLabel.Contains(TEXT("Rock")))
        {
            Report.EnvironmentActorCount++;
        }
        else if (ActorClassName.Contains(TEXT("Character")) || ActorClassName.Contains(TEXT("Pawn")))
        {
            Report.CharacterActorCount++;
        }
        else if (ActorLabel.Contains(TEXT("Dinosaur")) || ActorLabel.Contains(TEXT("TRex")) || 
                 ActorLabel.Contains(TEXT("Raptor")) || ActorClassName.Contains(TEXT("AI")))
        {
            Report.AIActorCount++;
        }
        else if (ActorClassName.Contains(TEXT("Audio")) || ActorClassName.Contains(TEXT("Sound")) || 
                 ActorClassName.Contains(TEXT("VFX")) || ActorClassName.Contains(TEXT("Niagara")))
        {
            Report.AudioVFXActorCount++;
        }
        else
        {
            Report.OtherActorCount++;
        }
    }
    
    // Validate critical systems
    ValidateLightingSystem(Report);
    ValidateCharacterSystem(Report);
    ValidateWorldGeneration(Report);
    ValidateAISystems(Report);
    
    // Store the report
    ValidationReports.Add(Report);
    
    // Keep only the last 10 reports
    if (ValidationReports.Num() > 10)
    {
        ValidationReports.RemoveAt(0);
    }
    
    if (bLogDetailedReports)
    {
        LogValidationReport(Report);
    }
    
    OnValidationCompleteDelegate.Broadcast(Report);
}

void UBuildIntegrationManager::ValidateLightingSystem(FBuildValidationReport& Report)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        Report.bIsValid = false;
        Report.ValidationErrors.Add(TEXT("No valid world for lighting validation"));
        return;
    }
    
    // Check for directional light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() == 0)
    {
        Report.ValidationErrors.Add(TEXT("No DirectionalLight found in level"));
        Report.bIsValid = false;
    }
    else if (DirectionalLights.Num() > 1)
    {
        Report.ValidationWarnings.Add(FString::Printf(TEXT("Multiple DirectionalLights found: %d"), DirectionalLights.Num()));
    }
    
    // Check for sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() == 0)
    {
        Report.ValidationWarnings.Add(TEXT("No SkyLight found in level"));
    }
    else if (SkyLights.Num() > 1)
    {
        Report.ValidationWarnings.Add(FString::Printf(TEXT("Multiple SkyLights found: %d"), SkyLights.Num()));
    }
}

void UBuildIntegrationManager::ValidateCharacterSystem(FBuildValidationReport& Report)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Look for player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        Report.ValidationWarnings.Add(TEXT("No player pawn found"));
    }
    else
    {
        FString PawnClassName = PlayerPawn->GetClass()->GetName();
        if (PawnClassName.Contains(TEXT("TranspersonalCharacter")))
        {
            Report.ValidationMessages.Add(TEXT("TranspersonalCharacter system active"));
        }
        else
        {
            Report.ValidationWarnings.Add(FString::Printf(TEXT("Player pawn is not TranspersonalCharacter: %s"), *PawnClassName));
        }
    }
}

void UBuildIntegrationManager::ValidateWorldGeneration(FBuildValidationReport& Report)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Check for landscape
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
    
    if (Landscapes.Num() == 0)
    {
        Report.ValidationWarnings.Add(TEXT("No Landscape found in level"));
    }
    else
    {
        Report.ValidationMessages.Add(FString::Printf(TEXT("World generation: %d landscapes active"), Landscapes.Num()));
    }
}

void UBuildIntegrationManager::ValidateAISystems(FBuildValidationReport& Report)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count AI-related actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 DinosaurCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && IsValid(Actor))
        {
            FString ActorLabel = Actor->GetActorLabel();
            if (ActorLabel.Contains(TEXT("Dinosaur")) || ActorLabel.Contains(TEXT("TRex")) || 
                ActorLabel.Contains(TEXT("Raptor")) || ActorLabel.Contains(TEXT("Brachiosaurus")))
            {
                DinosaurCount++;
            }
        }
    }
    
    if (DinosaurCount == 0)
    {
        Report.ValidationWarnings.Add(TEXT("No dinosaur actors found in level"));
    }
    else
    {
        Report.ValidationMessages.Add(FString::Printf(TEXT("AI systems: %d dinosaur actors active"), DinosaurCount));
    }
}

void UBuildIntegrationManager::LogValidationReport(const FBuildValidationReport& Report)
{
    UE_LOG(LogBuildIntegration, Log, TEXT("=== BUILD VALIDATION REPORT ==="));
    UE_LOG(LogBuildIntegration, Log, TEXT("Validation Time: %s"), *Report.ValidationTime.ToString());
    UE_LOG(LogBuildIntegration, Log, TEXT("Overall Status: %s"), Report.bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    UE_LOG(LogBuildIntegration, Log, TEXT("Total Actors: %d"), Report.TotalActorCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("  Lighting: %d"), Report.LightingActorCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("  Environment: %d"), Report.EnvironmentActorCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("  Characters: %d"), Report.CharacterActorCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("  AI/Dinosaurs: %d"), Report.AIActorCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("  Audio/VFX: %d"), Report.AudioVFXActorCount);
    UE_LOG(LogBuildIntegration, Log, TEXT("  Other: %d"), Report.OtherActorCount);
    
    if (Report.ValidationErrors.Num() > 0)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("Validation Errors:"));
        for (const FString& Error : Report.ValidationErrors)
        {
            UE_LOG(LogBuildIntegration, Error, TEXT("  - %s"), *Error);
        }
    }
    
    if (Report.ValidationWarnings.Num() > 0)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("Validation Warnings:"));
        for (const FString& Warning : Report.ValidationWarnings)
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("  - %s"), *Warning);
        }
    }
    
    if (Report.ValidationMessages.Num() > 0)
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("Validation Messages:"));
        for (const FString& Message : Report.ValidationMessages)
        {
            UE_LOG(LogBuildIntegration, Log, TEXT("  - %s"), *Message);
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("=== END VALIDATION REPORT ==="));
}

FBuildValidationReport UBuildIntegrationManager::GetLatestValidationReport() const
{
    if (ValidationReports.Num() > 0)
    {
        return ValidationReports.Last();
    }
    
    return FBuildValidationReport();
}

TArray<FBuildValidationReport> UBuildIntegrationManager::GetValidationHistory() const
{
    return ValidationReports;
}

void UBuildIntegrationManager::ForceValidation()
{
    ValidateSystemIntegration();
}

void UBuildIntegrationManager::ForceDuplicateCleanup()
{
    CleanDuplicateActors();
}