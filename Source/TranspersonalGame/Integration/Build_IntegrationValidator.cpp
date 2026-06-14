#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Character.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/DateTime.h"

void UBuild_IntegrationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeCriticalModules();
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator initialized"));
}

void UBuild_IntegrationValidator::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator deinitialized"));
}

void UBuild_IntegrationValidator::InitializeCriticalModules()
{
    CriticalModules.Empty();
    CriticalModules.Add(TEXT("TranspersonalGameState"));
    CriticalModules.Add(TEXT("TranspersonalCharacter"));
    CriticalModules.Add(TEXT("PCGWorldGenerator"));
    CriticalModules.Add(TEXT("FoliageManager"));
    CriticalModules.Add(TEXT("CrowdSimulationManager"));
    CriticalModules.Add(TEXT("ProceduralWorldManager"));
    CriticalModules.Add(TEXT("BuildIntegrationManager"));
}

FBuild_IntegrationReport UBuild_IntegrationValidator::ValidateFullSystem()
{
    FBuild_IntegrationReport Report;
    Report.BuildTimestamp = FDateTime::Now().ToString();

    // Validate all critical modules
    for (const FString& ModuleName : CriticalModules)
    {
        FBuild_ModuleStatus ModuleStatus = ValidateModuleClasses(ModuleName);
        Report.ModuleStatuses.Add(ModuleStatus);
    }

    // Validate actor counts
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        Report.TotalActors = AllActors.Num();

        // Count dinosaur actors (simplified check)
        int32 DinoCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("Dino")))
            {
                DinoCount++;
            }
        }
        Report.DinosaurActors = DinoCount;
    }

    // Validate game state
    Report.bGameStateValid = ValidateGameState();

    // Validate character system
    Report.bCharacterSystemValid = ValidateCharacterSystem();

    LastReport = Report;
    LogIntegrationReport(Report);

    return Report;
}

EBuild_ValidationStatus UBuild_IntegrationValidator::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleStatus Status = ValidateModuleClasses(ModuleName);
    return Status.Status;
}

FBuild_ModuleStatus UBuild_IntegrationValidator::ValidateModuleClasses(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    Status.TotalClasses = 1; // Simplified for single class validation

    // Try to load the class
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);

    if (LoadedClass)
    {
        Status.ClassesLoaded = 1;
        Status.Status = EBuild_ValidationStatus::Passed;
        Status.ErrorMessage = TEXT("Class loaded successfully");
    }
    else
    {
        Status.ClassesLoaded = 0;
        Status.Status = EBuild_ValidationStatus::Failed;
        Status.ErrorMessage = FString::Printf(TEXT("Failed to load class: %s"), *ClassPath);
    }

    return Status;
}

bool UBuild_IntegrationValidator::ValidateCrossSystemDependencies()
{
    // Check if critical systems can interact
    bool bDependenciesValid = true;

    // Validate World Generator -> Foliage Manager dependency
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
    
    if (!WorldGenClass || !FoliageClass)
    {
        bDependenciesValid = false;
        UE_LOG(LogTemp, Warning, TEXT("World Generation <-> Foliage dependency validation failed"));
    }

    // Validate Character -> Game State dependency
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    UClass* GameStateClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    
    if (!CharacterClass || !GameStateClass)
    {
        bDependenciesValid = false;
        UE_LOG(LogTemp, Warning, TEXT("Character <-> GameState dependency validation failed"));
    }

    return bDependenciesValid;
}

bool UBuild_IntegrationValidator::ValidateActorCounts()
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        // Check if actor count is within reasonable limits
        if (AllActors.Num() > 10000)
        {
            UE_LOG(LogTemp, Warning, TEXT("Actor count exceeds recommended limit: %d"), AllActors.Num());
            return false;
        }
        
        return true;
    }
    
    return false;
}

bool UBuild_IntegrationValidator::ValidateGameState()
{
    if (UWorld* World = GetWorld())
    {
        AGameStateBase* GameState = World->GetGameState();
        if (GameState)
        {
            // Check if it's our custom game state
            return GameState->GetClass()->GetName().Contains(TEXT("TranspersonalGameState"));
        }
    }
    
    return false;
}

bool UBuild_IntegrationValidator::ValidateCharacterSystem()
{
    // Check if TranspersonalCharacter class is available
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    return CharacterClass != nullptr;
}

void UBuild_IntegrationValidator::LogIntegrationReport(const FBuild_IntegrationReport& Report)
{
    UE_LOG(LogTemp, Log, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Timestamp: %s"), *Report.BuildTimestamp);
    UE_LOG(LogTemp, Log, TEXT("Total Actors: %d"), Report.TotalActors);
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Actors: %d"), Report.DinosaurActors);
    UE_LOG(LogTemp, Log, TEXT("Game State Valid: %s"), Report.bGameStateValid ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Character System Valid: %s"), Report.bCharacterSystemValid ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Log, TEXT("Module Validation Results:"));
    for (const FBuild_ModuleStatus& ModuleStatus : Report.ModuleStatuses)
    {
        FString StatusText;
        switch (ModuleStatus.Status)
        {
            case EBuild_ValidationStatus::Passed: StatusText = TEXT("PASSED"); break;
            case EBuild_ValidationStatus::Failed: StatusText = TEXT("FAILED"); break;
            case EBuild_ValidationStatus::Warning: StatusText = TEXT("WARNING"); break;
            default: StatusText = TEXT("UNKNOWN"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("  %s: %s (%d/%d classes) - %s"), 
               *ModuleStatus.ModuleName, 
               *StatusText, 
               ModuleStatus.ClassesLoaded, 
               ModuleStatus.TotalClasses,
               *ModuleStatus.ErrorMessage);
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END INTEGRATION REPORT ==="));
}

void UBuild_IntegrationValidator::RunEditorValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Running editor validation..."));
    FBuild_IntegrationReport Report = ValidateFullSystem();
    
    // Additional editor-specific validations
    bool bCrossSystemValid = ValidateCrossSystemDependencies();
    UE_LOG(LogTemp, Log, TEXT("Cross-system dependencies: %s"), bCrossSystemValid ? TEXT("VALID") : TEXT("INVALID"));
}