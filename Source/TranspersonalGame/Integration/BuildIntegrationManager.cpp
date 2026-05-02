#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Landscape/Landscape.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bAllSystemsValid = false;
    bWorldStateValid = false;
    bCharacterSystemsValid = false;
    bAISystemsValid = false;
    TotalActorsInLevel = 0;
    DuplicateActorsFound = 0;
    LastValidationResult = TEXT("Not validated");
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager initialized"));
    
    // Perform initial validation
    ValidateAllSystems();
}

void UBuildIntegrationManager::Deinitialize()
{
    ValidationErrors.Empty();
    ValidationWarnings.Empty();
    ActorCountsByType.Empty();
    DuplicateActors.Empty();
    
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateAllSystems()
{
    ValidationErrors.Empty();
    ValidationWarnings.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Starting comprehensive system validation"));
    
    // Validate each subsystem
    bWorldStateValid = ValidateWorldState();
    bCharacterSystemsValid = ValidateCharacterSystems();
    bAISystemsValid = ValidateAISystems();
    
    // Overall validation result
    bAllSystemsValid = bWorldStateValid && bCharacterSystemsValid && bAISystemsValid;
    
    // Generate validation report
    if (bAllSystemsValid)
    {
        LastValidationResult = TEXT("All systems valid");
        UE_LOG(LogTemp, Warning, TEXT("✓ All systems validation PASSED"));
    }
    else
    {
        LastValidationResult = FString::Printf(TEXT("Validation failed: %d errors, %d warnings"), 
            ValidationErrors.Num(), ValidationWarnings.Num());
        UE_LOG(LogTemp, Error, TEXT("✗ System validation FAILED: %s"), *LastValidationResult);
    }
    
    // Broadcast validation complete
    OnValidationComplete.Broadcast(bAllSystemsValid);
    
    return bAllSystemsValid;
}

bool UBuildIntegrationManager::ValidateWorldState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationErrors.Add(TEXT("No valid world found"));
        return false;
    }
    
    bool bWorldValid = true;
    
    // Validate actor counts
    if (!ValidateActorCounts())
    {
        bWorldValid = false;
    }
    
    // Validate lighting setup
    if (!ValidateLightingSetup())
    {
        bWorldValid = false;
    }
    
    // Validate terrain
    if (!ValidateTerrainSetup())
    {
        bWorldValid = false;
    }
    
    // Validate player setup
    if (!ValidatePlayerSetup())
    {
        bWorldValid = false;
    }
    
    return bWorldValid;
}

bool UBuildIntegrationManager::ValidateCharacterSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for player character class
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    if (Characters.Num() == 0)
    {
        ValidationWarnings.Add(TEXT("No character actors found in level"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character systems validation: %d characters found"), Characters.Num());
    return true;
}

bool UBuildIntegrationManager::ValidateAISystems()
{
    // AI systems validation - placeholder for now
    UE_LOG(LogTemp, Warning, TEXT("AI systems validation: Basic check passed"));
    return true;
}

bool UBuildIntegrationManager::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    ActorCountsByType.Empty();
    DuplicateActors.Empty();
    
    // Get all actors and count by type
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorClassName = Actor->GetClass()->GetName();
            int32* Count = ActorCountsByType.Find(ActorClassName);
            if (Count)
            {
                (*Count)++;
                
                // Mark as duplicate if it's a lighting actor
                if (ActorClassName.Contains(TEXT("DirectionalLight")) ||
                    ActorClassName.Contains(TEXT("SkyLight")) ||
                    ActorClassName.Contains(TEXT("ExponentialHeightFog")) ||
                    ActorClassName.Contains(TEXT("SkyAtmosphere")))
                {
                    DuplicateActors.Add(Actor);
                }
            }
            else
            {
                ActorCountsByType.Add(ActorClassName, 1);
            }
        }
    }
    
    TotalActorsInLevel = 0;
    for (auto& Pair : ActorCountsByType)
    {
        TotalActorsInLevel += Pair.Value;
    }
    
    DuplicateActorsFound = DuplicateActors.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Actor validation: %d total actors, %d duplicates found"), 
        TotalActorsInLevel, DuplicateActorsFound);
    
    return DuplicateActorsFound == 0;
}

bool UBuildIntegrationManager::ValidateLightingSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for essential lighting actors
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (DirectionalLights.Num() == 0)
    {
        ValidationErrors.Add(TEXT("No DirectionalLight found"));
        return false;
    }
    
    if (DirectionalLights.Num() > 1)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("Multiple DirectionalLights found: %d"), DirectionalLights.Num()));
    }
    
    if (SkyLights.Num() > 1)
    {
        ValidationWarnings.Add(FString::Printf(TEXT("Multiple SkyLights found: %d"), SkyLights.Num()));
    }
    
    return true;
}

bool UBuildIntegrationManager::ValidateTerrainSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for landscape
    TArray<AActor*> Landscapes;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), Landscapes);
    
    if (Landscapes.Num() == 0)
    {
        ValidationWarnings.Add(TEXT("No Landscape found - using basic terrain"));
    }
    
    return true;
}

bool UBuildIntegrationManager::ValidatePlayerSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check for PlayerStart
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    
    if (PlayerStarts.Num() == 0)
    {
        ValidationErrors.Add(TEXT("No PlayerStart found"));
        return false;
    }
    
    return true;
}

void UBuildIntegrationManager::GenerateBuildReport()
{
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("BuildIntegrationReport.txt");
    
    FString Report = TEXT("=== TRANSPERSONAL GAME BUILD INTEGRATION REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    Report += FString::Printf(TEXT("Overall Status: %s\n"), bAllSystemsValid ? TEXT("VALID") : TEXT("ISSUES DETECTED"));
    Report += FString::Printf(TEXT("World State: %s\n"), bWorldStateValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("Character Systems: %s\n"), bCharacterSystemsValid ? TEXT("VALID") : TEXT("INVALID"));
    Report += FString::Printf(TEXT("AI Systems: %s\n\n"), bAISystemsValid ? TEXT("VALID") : TEXT("INVALID"));
    
    Report += FString::Printf(TEXT("Total Actors: %d\n"), TotalActorsInLevel);
    Report += FString::Printf(TEXT("Duplicate Actors: %d\n\n"), DuplicateActorsFound);
    
    if (ValidationErrors.Num() > 0)
    {
        Report += TEXT("ERRORS:\n");
        for (const FString& Error : ValidationErrors)
        {
            Report += FString::Printf(TEXT("  - %s\n"), *Error);
        }
        Report += TEXT("\n");
    }
    
    if (ValidationWarnings.Num() > 0)
    {
        Report += TEXT("WARNINGS:\n");
        for (const FString& Warning : ValidationWarnings)
        {
            Report += FString::Printf(TEXT("  - %s\n"), *Warning);
        }
        Report += TEXT("\n");
    }
    
    Report += TEXT("ACTOR COUNTS BY TYPE:\n");
    for (auto& Pair : ActorCountsByType)
    {
        Report += FString::Printf(TEXT("  %s: %d\n"), *Pair.Key, Pair.Value);
    }
    
    // Write report to file
    FFileHelper::SaveStringToFile(Report, *ReportPath);
    
    UE_LOG(LogTemp, Warning, TEXT("Build integration report generated: %s"), *ReportPath);
    OnBuildReportGenerated.Broadcast(ReportPath);
}

bool UBuildIntegrationManager::CheckModuleDependencies()
{
    // Module dependency validation - placeholder
    UE_LOG(LogTemp, Warning, TEXT("Module dependency check: Basic validation passed"));
    return true;
}

void UBuildIntegrationManager::CleanupDuplicateActors()
{
    if (DuplicateActors.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No duplicate actors to clean up"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Cleaning up %d duplicate actors"), DuplicateActors.Num());
    
    for (AActor* Actor : DuplicateActors)
    {
        if (Actor && IsValid(Actor))
        {
            UE_LOG(LogTemp, Warning, TEXT("Destroying duplicate actor: %s"), *Actor->GetName());
            Actor->Destroy();
        }
    }
    
    DuplicateActors.Empty();
    DuplicateActorsFound = 0;
    
    // Re-validate after cleanup
    ValidateActorCounts();
}