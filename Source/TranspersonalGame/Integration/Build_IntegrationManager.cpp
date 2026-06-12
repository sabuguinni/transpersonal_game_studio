#include "Build_IntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

UBuild_IntegrationManager::UBuild_IntegrationManager()
{
    CurrentStatus = EBuild_IntegrationStatus::Unknown;
    TotalActorCount = 0;
    LastValidationTime = 0.0f;

    // Initialize core module names
    CoreModuleNames.Add(TEXT("TranspersonalGame"));
    CoreModuleNames.Add(TEXT("Core"));
    CoreModuleNames.Add(TEXT("Engine"));
    CoreModuleNames.Add(TEXT("UnrealEd"));
}

void UBuild_IntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentStatus = EBuild_IntegrationStatus::Initializing;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Initializing integration validation system"));
    
    // Start validation after a short delay
    FTimerHandle ValidationTimer;
    GetWorld()->GetTimerManager().SetTimer(ValidationTimer, this, &UBuild_IntegrationManager::StartIntegrationValidation, 2.0f, false);
}

void UBuild_IntegrationManager::Deinitialize()
{
    CurrentStatus = EBuild_IntegrationStatus::Unknown;
    ModuleStatuses.Empty();
    SystemValidations.Empty();
    IntegrationErrors.Empty();
    
    Super::Deinitialize();
}

void UBuild_IntegrationManager::StartIntegrationValidation()
{
    CurrentStatus = EBuild_IntegrationStatus::Testing;
    IntegrationErrors.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Starting comprehensive integration validation"));
    
    // Validate all modules
    ValidateAllModules();
    
    // Validate map systems
    ValidateMapSystems();
    
    // Enforce actor cap
    EnforceActorCap(8000);
    
    // Check compilation status
    bool bCompilationOK = CheckCompilationStatus();
    
    // Determine final status
    if (IntegrationErrors.Num() == 0 && bCompilationOK)
    {
        CurrentStatus = EBuild_IntegrationStatus::Complete;
        UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Integration validation PASSED"));
    }
    else
    {
        CurrentStatus = EBuild_IntegrationStatus::Failed;
        UE_LOG(LogTemp, Error, TEXT("Build_IntegrationManager: Integration validation FAILED with %d errors"), IntegrationErrors.Num());
    }
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
    LogIntegrationStatus();
}

void UBuild_IntegrationManager::ValidateAllModules()
{
    ModuleStatuses.Empty();
    
    // Validate core modules
    for (const FString& ModuleName : CoreModuleNames)
    {
        ValidateModule(ModuleName);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Validated %d modules"), ModuleStatuses.Num());
}

void UBuild_IntegrationManager::ValidateModule(const FString& ModuleName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    
    try
    {
        // Check if module is loaded
        if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            Status.bIsLoaded = true;
            
            // Count classes in module
            int32 ClassCount = 0;
            for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
            {
                UClass* Class = *ClassIt;
                if (Class && Class->GetOutermost()->GetName().Contains(ModuleName))
                {
                    ClassCount++;
                }
            }
            Status.ClassCount = ClassCount;
            
            UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Module %s loaded with %d classes"), *ModuleName, ClassCount);
        }
        else
        {
            Status.bIsLoaded = false;
            Status.LastError = TEXT("Module not loaded");
            ReportIntegrationError(FString::Printf(TEXT("Module %s is not loaded"), *ModuleName));
        }
    }
    catch (...)
    {
        Status.bHasErrors = true;
        Status.LastError = TEXT("Exception during module validation");
        ReportIntegrationError(FString::Printf(TEXT("Exception validating module %s"), *ModuleName));
    }
    
    ModuleStatuses.Add(Status);
}

void UBuild_IntegrationManager::ValidateMapSystems()
{
    SystemValidations.Empty();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        ReportIntegrationError(TEXT("No valid world found for system validation"));
        return;
    }
    
    // Get all actors in the level
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();
    
    // Validate essential systems
    ValidateSystem(TEXT("PlayerStart"), APlayerStart::StaticClass());
    ValidateSystem(TEXT("DirectionalLight"), ADirectionalLight::StaticClass());
    
    // Try to validate custom classes
    UClass* GameStateClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalGameState"));
    if (GameStateClass)
    {
        ValidateSystem(TEXT("TranspersonalGameState"), GameStateClass);
    }
    
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (CharacterClass)
    {
        ValidateSystem(TEXT("TranspersonalCharacter"), CharacterClass);
    }
    
    UClass* WorldGenClass = FindObject<UClass>(ANY_PACKAGE, TEXT("PCGWorldGenerator"));
    if (WorldGenClass)
    {
        ValidateSystem(TEXT("PCGWorldGenerator"), WorldGenClass);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Validated %d systems with %d total actors"), SystemValidations.Num(), TotalActorCount);
}

void UBuild_IntegrationManager::ValidateSystem(const FString& SystemName, UClass* SystemClass)
{
    FBuild_SystemValidation Validation;
    Validation.SystemName = SystemName;
    
    if (!SystemClass)
    {
        Validation.bIsValid = false;
        Validation.ValidationMessage = TEXT("Class not found");
        SystemValidations.Add(Validation);
        return;
    }
    
    UWorld* World = GetWorld();
    TArray<AActor*> SystemActors;
    UGameplayStatics::GetAllActorsOfClass(World, SystemClass, SystemActors);
    
    Validation.ActorCount = SystemActors.Num();
    
    if (SystemActors.Num() > 0)
    {
        Validation.bIsValid = true;
        Validation.ValidationMessage = FString::Printf(TEXT("Found %d instances"), SystemActors.Num());
    }
    else
    {
        Validation.bIsValid = false;
        Validation.ValidationMessage = TEXT("No instances found in level");
    }
    
    SystemValidations.Add(Validation);
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: System %s validation: %s"), 
           *SystemName, 
           Validation.bIsValid ? TEXT("PASS") : TEXT("FAIL"));
}

void UBuild_IntegrationManager::EnforceActorCap(int32 MaxActors)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() <= MaxActors)
    {
        UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Actor count %d within limit %d"), AllActors.Num(), MaxActors);
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Actor count %d exceeds limit %d, cleaning up"), AllActors.Num(), MaxActors);
    
    CleanupExcessActors(MaxActors);
    
    // Recount after cleanup
    AllActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Build_IntegrationManager: Actor count after cleanup: %d"), TotalActorCount);
}

void UBuild_IntegrationManager::CleanupExcessActors(int32 MaxActors)
{
    UWorld* World = GetWorld();
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Define essential actor types to keep
    TArray<FString> EssentialLabels = {
        TEXT("playerstart"), TEXT("directionallight"), TEXT("skylight"), 
        TEXT("skyatmosphere"), TEXT("fog"), TEXT("trex"), TEXT("veloci"), 
        TEXT("tricera"), TEXT("brachi"), TEXT("ankylo")
    };
    
    TArray<AActor*> ActorsToKeep;
    TArray<AActor*> ActorsToRemove;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        bool bIsEssential = false;
        
        for (const FString& Essential : EssentialLabels)
        {
            if (ActorLabel.Contains(Essential))
            {
                bIsEssential = true;
                break;
            }
        }
        
        if (bIsEssential)
        {
            ActorsToKeep.Add(Actor);
        }
        else
        {
            ActorsToRemove.Add(Actor);
        }
    }
    
    // Remove excess actors
    int32 ActorsToDestroy = FMath::Max(0, AllActors.Num() - MaxActors);
    ActorsToDestroy = FMath::Min(ActorsToDestroy, ActorsToRemove.Num());
    
    for (int32 i = 0; i < ActorsToDestroy; i++)
    {
        if (ActorsToRemove[i] && IsValid(ActorsToRemove[i]))
        {
            ActorsToRemove[i]->Destroy();
        }
    }
}

bool UBuild_IntegrationManager::CheckCompilationStatus()
{
    // Check if binaries exist
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"));
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    if (!PlatformFile.DirectoryExists(*BinariesDir))
    {
        ReportIntegrationError(TEXT("Binaries directory not found"));
        return false;
    }
    
    // Look for game binaries
    TArray<FString> FoundFiles;
    PlatformFile.FindFilesRecursively(FoundFiles, *BinariesDir, TEXT("*.so"));
    PlatformFile.FindFilesRecursively(FoundFiles, *BinariesDir, TEXT("*.dll"));
    PlatformFile.FindFilesRecursively(FoundFiles, *BinariesDir, TEXT("*.dylib"));
    
    if (FoundFiles.Num() == 0)
    {
        ReportIntegrationError(TEXT("No compiled binaries found"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationManager: Found %d binary files"), FoundFiles.Num());
    return true;
}

void UBuild_IntegrationManager::ReportIntegrationError(const FString& ErrorMessage)
{
    IntegrationErrors.Add(ErrorMessage);
    UE_LOG(LogTemp, Error, TEXT("Build_IntegrationManager: %s"), *ErrorMessage);
}

void UBuild_IntegrationManager::LogIntegrationStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Status: %s"), CurrentStatus == EBuild_IntegrationStatus::Complete ? TEXT("COMPLETE") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Modules: %d validated"), ModuleStatuses.Num());
    UE_LOG(LogTemp, Warning, TEXT("Systems: %d validated"), SystemValidations.Num());
    UE_LOG(LogTemp, Warning, TEXT("Actors: %d total"), TotalActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Errors: %d"), IntegrationErrors.Num());
    
    for (const FString& Error : IntegrationErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END INTEGRATION STATUS ==="));
}