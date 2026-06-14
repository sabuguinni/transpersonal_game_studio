#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Validate every 5 seconds
    
    CurrentBuildStatus = EBuild_IntegrationStatus::Unknown;
    ValidationInterval = 30.0f; // Full validation every 30 seconds
    MaxActorCount = 8000;
    MaxDinosaurCount = 150;
    LastValidationTime = 0.0f;
    bValidationEnabled = true;
    
    // Initialize core module names
    CoreModuleNames.Add(TEXT("TranspersonalGameState"));
    CoreModuleNames.Add(TEXT("TranspersonalCharacter"));
    CoreModuleNames.Add(TEXT("PCGWorldGenerator"));
    CoreModuleNames.Add(TEXT("FoliageManager"));
    CoreModuleNames.Add(TEXT("CrowdSimulationManager"));
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Perform initial validation
    ValidateSystemIntegration();
    
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: System validation started"));
}

void UBuild_IntegrationValidator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bValidationEnabled)
        return;
    
    LastValidationTime += DeltaTime;
    
    // Perform full validation at intervals
    if (LastValidationTime >= ValidationInterval)
    {
        ValidateSystemIntegration();
        LastValidationTime = 0.0f;
    }
}

FBuild_SystemReport UBuild_IntegrationValidator::ValidateSystemIntegration()
{
    FBuild_SystemReport Report;
    Report.BuildTimestamp = FDateTime::Now().ToString();
    
    // Validate module loading
    bool bModulesHealthy = ValidateModuleLoading();
    
    // Validate actor counts
    bool bActorCountsHealthy = ValidateActorCounts();
    
    // Validate core gameplay
    bool bGameplayHealthy = ValidateCoreGameplay();
    
    // Count total actors
    UWorld* World = GetWorld();
    if (World)
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        Report.TotalActors = ActorCount;
        
        // Count dinosaurs
        TArray<FString> DinoKeywords = {TEXT("trex"), TEXT("raptor"), TEXT("brachi"), TEXT("tricera"), TEXT("ankylo")};
        Report.DinosaurCount = CountActorsByKeyword(DinoKeywords);
    }
    
    // Count loaded modules
    int32 LoadedCount = 0;
    for (const FString& ModuleName : CoreModuleNames)
    {
        FBuild_ModuleStatus ModuleStatus = ValidateModule(ModuleName, ModuleName);
        Report.ModuleStatuses.Add(ModuleStatus);
        if (ModuleStatus.bIsLoaded)
        {
            LoadedCount++;
        }
    }
    Report.LoadedModules = LoadedCount;
    
    // Determine overall status
    if (bModulesHealthy && bActorCountsHealthy && bGameplayHealthy)
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Healthy;
    }
    else if (LoadedCount >= 3 && Report.TotalActors <= MaxActorCount)
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Warning;
    }
    else
    {
        Report.OverallStatus = EBuild_IntegrationStatus::Critical;
    }
    
    CurrentBuildStatus = Report.OverallStatus;
    LastSystemReport = Report;
    
    // Log status
    FString StatusString = TEXT("Unknown");
    switch (Report.OverallStatus)
    {
        case EBuild_IntegrationStatus::Healthy: StatusString = TEXT("Healthy"); break;
        case EBuild_IntegrationStatus::Warning: StatusString = TEXT("Warning"); break;
        case EBuild_IntegrationStatus::Critical: StatusString = TEXT("Critical"); break;
        case EBuild_IntegrationStatus::Failed: StatusString = TEXT("Failed"); break;
    }
    
    LogIntegrationStatus(FString::Printf(TEXT("System Status: %s | Actors: %d | Modules: %d/%d | Dinosaurs: %d"),
        *StatusString, Report.TotalActors, Report.LoadedModules, CoreModuleNames.Num(), Report.DinosaurCount));
    
    return Report;
}

bool UBuild_IntegrationValidator::ValidateModuleLoading()
{
    int32 LoadedCount = 0;
    
    for (const FString& ModuleName : CoreModuleNames)
    {
        FString ClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ModuleName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        
        if (LoadedClass)
        {
            LoadedCount++;
        }
        else
        {
            LogIntegrationStatus(FString::Printf(TEXT("Failed to load module: %s"), *ModuleName), true);
        }
    }
    
    bool bHealthy = LoadedCount >= (CoreModuleNames.Num() * 0.6f); // At least 60% loaded
    return bHealthy;
}

bool UBuild_IntegrationValidator::ValidateActorCounts()
{
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    int32 TotalActors = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        TotalActors++;
    }
    
    TArray<FString> DinoKeywords = {TEXT("trex"), TEXT("raptor"), TEXT("brachi"), TEXT("tricera"), TEXT("ankylo")};
    int32 DinoCount = CountActorsByKeyword(DinoKeywords);
    
    bool bActorCountHealthy = TotalActors <= MaxActorCount;
    bool bDinoCountHealthy = DinoCount <= MaxDinosaurCount;
    
    if (!bActorCountHealthy)
    {
        LogIntegrationStatus(FString::Printf(TEXT("Actor count exceeded: %d/%d"), TotalActors, MaxActorCount), true);
    }
    
    if (!bDinoCountHealthy)
    {
        LogIntegrationStatus(FString::Printf(TEXT("Dinosaur count exceeded: %d/%d"), DinoCount, MaxDinosaurCount), true);
    }
    
    return bActorCountHealthy && bDinoCountHealthy;
}

bool UBuild_IntegrationValidator::ValidateCoreGameplay()
{
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    // Check for essential gameplay elements
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    bool bHasGameMode = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ClassName = Actor->GetClass()->GetName().ToLower();
        
        if (ClassName.Contains(TEXT("playerstart")))
        {
            bHasPlayerStart = true;
        }
        else if (ClassName.Contains(TEXT("light")))
        {
            bHasLighting = true;
        }
        else if (ClassName.Contains(TEXT("gamemode")))
        {
            bHasGameMode = true;
        }
    }
    
    bool bGameplayHealthy = bHasPlayerStart && bHasLighting;
    
    if (!bGameplayHealthy)
    {
        LogIntegrationStatus(TEXT("Core gameplay validation failed - missing essential elements"), true);
    }
    
    return bGameplayHealthy;
}

void UBuild_IntegrationValidator::GenerateBuildReport()
{
    FBuild_SystemReport Report = ValidateSystemIntegration();
    
    FString ReportText = FString::Printf(TEXT(
        "=== TRANSPERSONAL GAME STUDIO - BUILD INTEGRATION REPORT ===\n"
        "Timestamp: %s\n"
        "Overall Status: %s\n"
        "Total Actors: %d\n"
        "Dinosaur Count: %d\n"
        "Loaded Modules: %d/%d\n"
        "\nModule Details:\n"
    ), *Report.BuildTimestamp, 
       *UEnum::GetValueAsString(Report.OverallStatus),
       Report.TotalActors,
       Report.DinosaurCount,
       Report.LoadedModules,
       CoreModuleNames.Num());
    
    for (const FBuild_ModuleStatus& ModuleStatus : Report.ModuleStatuses)
    {
        ReportText += FString::Printf(TEXT("- %s: %s\n"), 
            *ModuleStatus.ModuleName, 
            ModuleStatus.bIsLoaded ? TEXT("LOADED") : TEXT("FAILED"));
    }
    
    ReportText += TEXT("=== END REPORT ===\n");
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportText);
    LogIntegrationStatus(TEXT("Build report generated successfully"));
}

FBuild_ModuleStatus UBuild_IntegrationValidator::ValidateModule(const FString& ModuleName, const FString& ClassName)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    
    FString FullClassName = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *FullClassName);
    
    Status.bIsLoaded = (LoadedClass != nullptr);
    Status.ClassCount = Status.bIsLoaded ? 1 : 0;
    
    if (!Status.bIsLoaded)
    {
        Status.ErrorMessage = FString::Printf(TEXT("Failed to load class: %s"), *FullClassName);
    }
    
    return Status;
}

int32 UBuild_IntegrationValidator::CountActorsByKeyword(const TArray<FString>& Keywords)
{
    UWorld* World = GetWorld();
    if (!World)
        return 0;
    
    int32 Count = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        
        for (const FString& Keyword : Keywords)
        {
            if (ActorLabel.Contains(Keyword.ToLower()))
            {
                Count++;
                break; // Don't double-count actors with multiple keywords
            }
        }
    }
    
    return Count;
}

void UBuild_IntegrationValidator::LogIntegrationStatus(const FString& Message, bool bIsError)
{
    if (bIsError)
    {
        UE_LOG(LogTemp, Error, TEXT("Build_IntegrationValidator: %s"), *Message);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: %s"), *Message);
    }
}