#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Engine/AssetManager.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bValidationComplete = false;
    LastValidationTime = 0.0f;
}

bool UBuild_IntegrationValidator::ValidateAllModules()
{
    ValidationReports.Empty();
    bValidationComplete = false;

    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Starting comprehensive module validation"));

    // Validate core classes
    bool bCoreValid = ValidateCoreClasses();
    
    // Validate world state
    bool bWorldValid = ValidateWorldState();
    
    // Validate QA results
    bool bQAValid = ValidateQAResults();
    
    // Validate asset integrity
    bool bAssetsValid = ValidateAssetIntegrity();
    
    // Validate class dependencies
    bool bDepsValid = ValidateClassDependencies();

    bValidationComplete = true;
    LastValidationTime = GetWorld()->GetTimeSeconds();

    bool bOverallValid = bCoreValid && bWorldValid && bQAValid && bAssetsValid && bDepsValid;
    
    FBuild_ValidationReport OverallReport;
    OverallReport.ModuleName = TEXT("Overall");
    OverallReport.Result = bOverallValid ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    OverallReport.Message = bOverallValid ? TEXT("All modules validated successfully") : TEXT("Some modules failed validation");
    ValidationReports.Add(OverallReport);

    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validation complete - %s"), 
           bOverallValid ? TEXT("SUCCESS") : TEXT("FAILED"));

    return bOverallValid;
}

FBuild_ValidationReport UBuild_IntegrationValidator::ValidateModule(const FString& ModuleName)
{
    FBuild_ValidationReport Report;
    Report.ModuleName = ModuleName;

    if (ModuleName == TEXT("TranspersonalGame"))
    {
        Report.Result = ValidateCoreClasses() ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
        Report.Message = TEXT("Core TranspersonalGame module validation");
    }
    else if (ModuleName == TEXT("World"))
    {
        Report.Result = ValidateWorldState() ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
        Report.Message = TEXT("World state validation");
    }
    else if (ModuleName == TEXT("QA"))
    {
        Report.Result = ValidateQAResults() ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
        Report.Message = TEXT("QA results validation");
    }
    else
    {
        Report.Result = EBuild_ValidationResult::Warning;
        Report.Message = FString::Printf(TEXT("Unknown module: %s"), *ModuleName);
    }

    LogValidationResult(Report);
    return Report;
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::GetValidationReports() const
{
    return ValidationReports;
}

bool UBuild_IntegrationValidator::ValidateAssetIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating asset integrity"));

    // Check if AssetManager is available
    if (!UAssetManager::IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AssetManager not available for validation"));
        return false;
    }

    // Basic asset validation - check if we can access the asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> AllAssets;
    AssetRegistry.GetAllAssets(AllAssets);

    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Found %d total assets"), AllAssets.Num());

    FBuild_ValidationReport AssetReport;
    AssetReport.ModuleName = TEXT("Assets");
    AssetReport.Result = AllAssets.Num() > 0 ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    AssetReport.Message = FString::Printf(TEXT("Asset count: %d"), AllAssets.Num());
    ValidationReports.Add(AssetReport);

    return AllAssets.Num() > 0;
}

bool UBuild_IntegrationValidator::ValidateClassDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating class dependencies"));

    TArray<FString> RequiredClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };

    int32 LoadedClasses = 0;
    for (const FString& ClassName : RequiredClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            LoadedClasses++;
            UE_LOG(LogTemp, Log, TEXT("Successfully loaded class: %s"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s"), *ClassName);
        }
    }

    FBuild_ValidationReport DepsReport;
    DepsReport.ModuleName = TEXT("Dependencies");
    DepsReport.Result = (LoadedClasses == RequiredClasses.Num()) ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    DepsReport.Message = FString::Printf(TEXT("Loaded %d/%d required classes"), LoadedClasses, RequiredClasses.Num());
    ValidationReports.Add(DepsReport);

    return LoadedClasses > 0;
}

int32 UBuild_IntegrationValidator::GetTotalActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        return ActorCount;
    }
    return 0;
}

bool UBuild_IntegrationValidator::ValidateCoreClasses()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating core classes"));

    // Try to load core game classes
    UClass* GameStateClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalGameState"));
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));

    bool bCoreValid = (GameStateClass != nullptr) && (CharacterClass != nullptr);

    FBuild_ValidationReport CoreReport;
    CoreReport.ModuleName = TEXT("Core");
    CoreReport.Result = bCoreValid ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    CoreReport.Message = bCoreValid ? TEXT("Core classes loaded successfully") : TEXT("Failed to load core classes");
    ValidationReports.Add(CoreReport);

    return bCoreValid;
}

bool UBuild_IntegrationValidator::ValidateWorldState()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating world state"));

    UWorld* World = GetWorld();
    if (!World)
    {
        FBuild_ValidationReport WorldReport;
        WorldReport.ModuleName = TEXT("World");
        WorldReport.Result = EBuild_ValidationResult::Error;
        WorldReport.Message = TEXT("No valid world found");
        ValidationReports.Add(WorldReport);
        return false;
    }

    int32 ActorCount = GetTotalActorCount();
    bool bWorldValid = ActorCount > 0;

    FBuild_ValidationReport WorldReport;
    WorldReport.ModuleName = TEXT("World");
    WorldReport.Result = bWorldValid ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    WorldReport.Message = FString::Printf(TEXT("World has %d actors"), ActorCount);
    ValidationReports.Add(WorldReport);

    return bWorldValid;
}

bool UBuild_IntegrationValidator::ValidateQAResults()
{
    UE_LOG(LogTemp, Log, TEXT("Build_IntegrationValidator: Validating QA results"));

    // Check for QA results file
    FString ProjectDir = FPaths::ProjectDir();
    FString QAResultsPath = FPaths::Combine(ProjectDir, TEXT("Saved"), TEXT("QA_Results.json"));

    bool bQAResultsExist = IFileManager::Get().FileExists(*QAResultsPath);

    FBuild_ValidationReport QAReport;
    QAReport.ModuleName = TEXT("QA");
    QAReport.Result = bQAResultsExist ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    QAReport.Message = bQAResultsExist ? TEXT("QA results file found") : TEXT("No QA results file found");
    ValidationReports.Add(QAReport);

    return true; // QA results are optional, so always return true
}

void UBuild_IntegrationValidator::LogValidationResult(const FBuild_ValidationReport& Report)
{
    FString ResultString;
    switch (Report.Result)
    {
        case EBuild_ValidationResult::Success:
            ResultString = TEXT("SUCCESS");
            break;
        case EBuild_ValidationResult::Warning:
            ResultString = TEXT("WARNING");
            break;
        case EBuild_ValidationResult::Error:
            ResultString = TEXT("ERROR");
            break;
        case EBuild_ValidationResult::Critical:
            ResultString = TEXT("CRITICAL");
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Validation [%s] %s: %s"), 
           *Report.ModuleName, *ResultString, *Report.Message);
}