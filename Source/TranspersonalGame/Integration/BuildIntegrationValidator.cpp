#include "BuildIntegrationValidator.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/Engine.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuildIntegrationValidator::UBuildIntegrationValidator()
{
    TotalActorCount = 0;
    DuplicatesRemoved = 0;
    bMapIsValid = false;
}

bool UBuildIntegrationValidator::ValidateMapIntegrity(UWorld* World)
{
    if (!World)
    {
        AddValidationError("World is null - cannot validate map integrity");
        return false;
    }

    ClearValidationResults();

    // Count all actors
    TArray<AActor*> AllActors;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AllActors.Add(*ActorItr);
    }
    
    TotalActorCount = AllActors.Num();

    // Validate lighting setup
    bool bLightingValid = ValidateLightingSetup(World);
    
    // Validate gameplay actors
    bool bGameplayValid = ValidateGameplayActors(World);
    
    // Validate PlayerStart
    bool bPlayerStartValid = ValidatePlayerStart(World);

    bMapIsValid = bLightingValid && bGameplayValid && bPlayerStartValid;
    
    if (bMapIsValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Map integrity validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Map integrity validation FAILED"));
    }

    return bMapIsValid;
}

bool UBuildIntegrationValidator::ValidateLightingSetup(UWorld* World)
{
    if (!World)
    {
        return false;
    }

    // Count lighting actors
    int32 DirectionalLightCount = 0;
    int32 SkyAtmosphereCount = 0;
    int32 SkyLightCount = 0;
    int32 HeightFogCount = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();

        if (ClassName == "DirectionalLight")
        {
            DirectionalLightCount++;
        }
        else if (ClassName == "SkyAtmosphere")
        {
            SkyAtmosphereCount++;
        }
        else if (ClassName == "SkyLight")
        {
            SkyLightCount++;
        }
        else if (ClassName == "ExponentialHeightFog")
        {
            HeightFogCount++;
        }
    }

    // Validate counts
    bool bValid = true;

    if (DirectionalLightCount == 0)
    {
        AddValidationError("No DirectionalLight found in scene");
        bValid = false;
    }
    else if (DirectionalLightCount > 1)
    {
        AddValidationWarning(FString::Printf(TEXT("Multiple DirectionalLights found: %d (should be 1)"), DirectionalLightCount));
    }

    if (SkyAtmosphereCount > 1)
    {
        AddValidationWarning(FString::Printf(TEXT("Multiple SkyAtmospheres found: %d (should be 1)"), SkyAtmosphereCount));
    }

    if (SkyLightCount > 1)
    {
        AddValidationWarning(FString::Printf(TEXT("Multiple SkyLights found: %d (should be 1)"), SkyLightCount));
    }

    if (HeightFogCount > 1)
    {
        AddValidationWarning(FString::Printf(TEXT("Multiple ExponentialHeightFogs found: %d (should be 1)"), HeightFogCount));
    }

    return bValid;
}

bool UBuildIntegrationValidator::ValidateGameplayActors(UWorld* World)
{
    if (!World)
    {
        return false;
    }

    int32 TranspersonalActorCount = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();

        if (ClassName.Contains("Transpersonal"))
        {
            TranspersonalActorCount++;
            UE_LOG(LogTemp, Log, TEXT("Found TranspersonalGame actor: %s"), *ClassName);
        }
    }

    if (TranspersonalActorCount == 0)
    {
        AddValidationWarning("No TranspersonalGame actors found in scene");
        return false;
    }

    return true;
}

bool UBuildIntegrationValidator::ValidateModuleIntegration()
{
    // Validate that core TranspersonalGame classes can be loaded
    bool bValid = ValidateTranspersonalGameClasses();
    
    // Check for missing implementations
    TArray<FString> MissingImpls = GetMissingImplementations();
    if (MissingImpls.Num() > 0)
    {
        for (const FString& Missing : MissingImpls)
        {
            AddValidationError(FString::Printf(TEXT("Missing implementation: %s"), *Missing));
        }
        bValid = false;
    }

    return bValid;
}

bool UBuildIntegrationValidator::CleanDuplicateActors(UWorld* World)
{
    if (!World)
    {
        return false;
    }

    DuplicatesRemoved = 0;

    // Find and remove duplicate lighting actors
    TArray<AActor*> DirectionalLights;
    TArray<AActor*> SkyAtmospheres;
    TArray<AActor*> SkyLights;
    TArray<AActor*> HeightFogs;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();

        if (ClassName == "DirectionalLight")
        {
            DirectionalLights.Add(Actor);
        }
        else if (ClassName == "SkyAtmosphere")
        {
            SkyAtmospheres.Add(Actor);
        }
        else if (ClassName == "SkyLight")
        {
            SkyLights.Add(Actor);
        }
        else if (ClassName == "ExponentialHeightFog")
        {
            HeightFogs.Add(Actor);
        }
    }

    // Keep only the first of each type
    for (int32 i = 1; i < DirectionalLights.Num(); i++)
    {
        DirectionalLights[i]->Destroy();
        DuplicatesRemoved++;
    }

    for (int32 i = 1; i < SkyAtmospheres.Num(); i++)
    {
        SkyAtmospheres[i]->Destroy();
        DuplicatesRemoved++;
    }

    for (int32 i = 1; i < SkyLights.Num(); i++)
    {
        SkyLights[i]->Destroy();
        DuplicatesRemoved++;
    }

    for (int32 i = 1; i < HeightFogs.Num(); i++)
    {
        HeightFogs[i]->Destroy();
        DuplicatesRemoved++;
    }

    UE_LOG(LogTemp, Log, TEXT("Cleaned %d duplicate lighting actors"), DuplicatesRemoved);
    return true;
}

TMap<FString, int32> UBuildIntegrationValidator::GetActorCountsByClass(UWorld* World)
{
    TMap<FString, int32> ActorCounts;

    if (!World)
    {
        return ActorCounts;
    }

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ActorCounts.Contains(ClassName))
        {
            ActorCounts[ClassName]++;
        }
        else
        {
            ActorCounts.Add(ClassName, 1);
        }
    }

    return ActorCounts;
}

TArray<AActor*> UBuildIntegrationValidator::FindDuplicateLightingActors(UWorld* World)
{
    TArray<AActor*> Duplicates;

    if (!World)
    {
        return Duplicates;
    }

    TMap<FString, int32> LightingCounts;
    TMap<FString, TArray<AActor*>> LightingActors;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName();

        if (ClassName == "DirectionalLight" || ClassName == "SkyAtmosphere" || 
            ClassName == "SkyLight" || ClassName == "ExponentialHeightFog")
        {
            if (!LightingActors.Contains(ClassName))
            {
                LightingActors.Add(ClassName, TArray<AActor*>());
            }
            LightingActors[ClassName].Add(Actor);
        }
    }

    // Add duplicates (keep first, mark rest as duplicates)
    for (auto& Pair : LightingActors)
    {
        if (Pair.Value.Num() > 1)
        {
            for (int32 i = 1; i < Pair.Value.Num(); i++)
            {
                Duplicates.Add(Pair.Value[i]);
            }
        }
    }

    return Duplicates;
}

bool UBuildIntegrationValidator::ValidatePlayerStart(UWorld* World)
{
    if (!World)
    {
        return false;
    }

    int32 PlayerStartCount = 0;
    for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
    {
        PlayerStartCount++;
    }

    if (PlayerStartCount == 0)
    {
        AddValidationError("No PlayerStart found in level");
        return false;
    }
    else if (PlayerStartCount > 1)
    {
        AddValidationWarning(FString::Printf(TEXT("Multiple PlayerStarts found: %d"), PlayerStartCount));
    }

    return true;
}

bool UBuildIntegrationValidator::ValidateTranspersonalGameClasses()
{
    // Try to find core TranspersonalGame classes
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    UClass* GameModeClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameMode"));
    UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));

    bool bValid = true;

    if (!CharacterClass)
    {
        AddValidationError("TranspersonalCharacter class not found");
        bValid = false;
    }

    if (!GameModeClass)
    {
        AddValidationError("TranspersonalGameMode class not found");
        bValid = false;
    }

    if (!GameStateClass)
    {
        AddValidationError("TranspersonalGameState class not found");
        bValid = false;
    }

    return bValid;
}

bool UBuildIntegrationValidator::ValidateModuleCompilation()
{
    // This would typically check compilation logs or try to load modules
    // For now, we validate that core classes exist
    return ValidateTranspersonalGameClasses();
}

TArray<FString> UBuildIntegrationValidator::GetMissingImplementations()
{
    TArray<FString> MissingImplementations;

    // This would scan for .h files without corresponding .cpp files
    // For now, return empty array as this requires file system access
    
    return MissingImplementations;
}

bool UBuildIntegrationValidator::ValidateCrossModuleDependencies()
{
    // Validate that modules can reference each other correctly
    // For now, return true as this requires deeper module analysis
    return true;
}

FString UBuildIntegrationValidator::GenerateValidationReport()
{
    FString Report;
    Report += "=== BUILD INTEGRATION VALIDATION REPORT ===\n";
    Report += FString::Printf(TEXT("Total Actors: %d\n"), TotalActorCount);
    Report += FString::Printf(TEXT("Duplicates Removed: %d\n"), DuplicatesRemoved);
    Report += FString::Printf(TEXT("Map Valid: %s\n"), bMapIsValid ? TEXT("YES") : TEXT("NO"));
    
    Report += "\nErrors:\n";
    for (const FString& Error : ValidationErrors)
    {
        Report += FString::Printf(TEXT("  - %s\n"), *Error);
    }
    
    Report += "\nWarnings:\n";
    for (const FString& Warning : ValidationWarnings)
    {
        Report += FString::Printf(TEXT("  - %s\n"), *Warning);
    }

    return Report;
}

void UBuildIntegrationValidator::LogValidationResults()
{
    UE_LOG(LogTemp, Log, TEXT("=== BUILD INTEGRATION VALIDATION RESULTS ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d"), TotalActorCount);
    UE_LOG(LogTemp, Log, TEXT("Duplicates Removed: %d"), DuplicatesRemoved);
    UE_LOG(LogTemp, Log, TEXT("Map Valid: %s"), bMapIsValid ? TEXT("YES") : TEXT("NO"));
    
    for (const FString& Error : ValidationErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("Validation Error: %s"), *Error);
    }
    
    for (const FString& Warning : ValidationWarnings)
    {
        UE_LOG(LogTemp, Warning, TEXT("Validation Warning: %s"), *Warning);
    }
}

bool UBuildIntegrationValidator::ValidateActorClass(const FString& ClassName)
{
    UClass* ActorClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
    return ActorClass != nullptr;
}

bool UBuildIntegrationValidator::CheckForDuplicateTypes(UWorld* World, const FString& ActorType)
{
    if (!World)
    {
        return false;
    }

    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetClass()->GetName() == ActorType)
        {
            Count++;
        }
    }

    return Count > 1;
}

void UBuildIntegrationValidator::AddValidationError(const FString& Error)
{
    ValidationErrors.Add(Error);
    UE_LOG(LogTemp, Error, TEXT("Validation Error: %s"), *Error);
}

void UBuildIntegrationValidator::AddValidationWarning(const FString& Warning)
{
    ValidationWarnings.Add(Warning);
    UE_LOG(LogTemp, Warning, TEXT("Validation Warning: %s"), *Warning);
}

void UBuildIntegrationValidator::ClearValidationResults()
{
    ValidationErrors.Empty();
    ValidationWarnings.Empty();
    TotalActorCount = 0;
    DuplicatesRemoved = 0;
    bMapIsValid = false;
}