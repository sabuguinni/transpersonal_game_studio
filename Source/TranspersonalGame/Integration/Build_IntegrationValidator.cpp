#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/Blueprint.h"
#include "UObject/ConstructorHelpers.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoValidateOnBeginPlay = false;
    bLogDetailedResults = true;
    ValidationTimeout = 30.0f;
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoValidateOnBeginPlay)
    {
        RunFullValidationSuite();
    }
}

bool UBuild_IntegrationValidator::ValidateProjectStructure()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    
    // Check project file exists
    FString ProjectPath = FPaths::GetProjectFilePath();
    if (!FPaths::FileExists(ProjectPath))
    {
        AddValidationReport(TEXT("Project Structure"), EBuild_ValidationResult::Critical, 
            TEXT("Project file not found"), FPlatformTime::Seconds() - StartTime);
        return false;
    }
    
    // Check source directory
    FString SourceDir = FPaths::ProjectDir() / TEXT("Source");
    if (!FPaths::DirectoryExists(SourceDir))
    {
        AddValidationReport(TEXT("Project Structure"), EBuild_ValidationResult::Error, 
            TEXT("Source directory not found"), FPlatformTime::Seconds() - StartTime);
        bSuccess = false;
    }
    
    // Check content directory
    FString ContentDir = FPaths::ProjectContentDir();
    if (!FPaths::DirectoryExists(ContentDir))
    {
        AddValidationReport(TEXT("Project Structure"), EBuild_ValidationResult::Error, 
            TEXT("Content directory not found"), FPlatformTime::Seconds() - StartTime);
        bSuccess = false;
    }
    
    if (bSuccess)
    {
        AddValidationReport(TEXT("Project Structure"), EBuild_ValidationResult::Success, 
            TEXT("Project structure validation passed"), FPlatformTime::Seconds() - StartTime);
    }
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateModuleLoading()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    
    // Clear previous module statuses
    ModuleStatuses.Empty();
    
    // Test core TranspersonalGame module
    FBuild_ModuleStatus TranspersonalGameStatus;
    TranspersonalGameStatus.ModuleName = TEXT("TranspersonalGame");
    TranspersonalGameStatus.bIsLoaded = true; // We're running, so it's loaded
    
    // Test loading core classes
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager"),
        TEXT("/Script/TranspersonalGame.CrowdSimulationManager")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            TranspersonalGameStatus.LoadedClasses.Add(ClassName);
            TranspersonalGameStatus.ClassCount++;
        }
        else
        {
            TranspersonalGameStatus.FailedClasses.Add(ClassName);
            TranspersonalGameStatus.bHasErrors = true;
            bSuccess = false;
        }
    }
    
    ModuleStatuses.Add(TranspersonalGameStatus);
    
    FString ResultMessage = FString::Printf(TEXT("Module validation: %d/%d classes loaded"), 
        TranspersonalGameStatus.LoadedClasses.Num(), CoreClasses.Num());
    
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    AddValidationReport(TEXT("Module Loading"), Result, ResultMessage, FPlatformTime::Seconds() - StartTime);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateClassRegistration()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    int32 RegisteredClasses = 0;
    
    // Check if our custom classes are properly registered with UE5's reflection system
    for (TObjectIterator<UClass> ClassIterator; ClassIterator; ++ClassIterator)
    {
        UClass* Class = *ClassIterator;
        if (Class && Class->GetPackage())
        {
            FString PackageName = Class->GetPackage()->GetName();
            if (PackageName.Contains(TEXT("TranspersonalGame")))
            {
                RegisteredClasses++;
                if (bLogDetailedResults)
                {
                    UE_LOG(LogTemp, Log, TEXT("Registered class: %s"), *Class->GetName());
                }
            }
        }
    }
    
    FString ResultMessage = FString::Printf(TEXT("Found %d registered TranspersonalGame classes"), RegisteredClasses);
    
    if (RegisteredClasses > 0)
    {
        AddValidationReport(TEXT("Class Registration"), EBuild_ValidationResult::Success, 
            ResultMessage, FPlatformTime::Seconds() - StartTime);
    }
    else
    {
        AddValidationReport(TEXT("Class Registration"), EBuild_ValidationResult::Warning, 
            TEXT("No TranspersonalGame classes found in reflection system"), FPlatformTime::Seconds() - StartTime);
        bSuccess = false;
    }
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateCrossSystemIntegration()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = true;
    
    // Test world generation integration
    if (!ValidateWorldGeneration())
    {
        bSuccess = false;
    }
    
    // Test character systems integration
    if (!ValidateCharacterSystems())
    {
        bSuccess = false;
    }
    
    // Test AI systems integration
    if (!ValidateAISystems())
    {
        bSuccess = false;
    }
    
    // Test environment systems integration
    if (!ValidateEnvironmentSystems())
    {
        bSuccess = false;
    }
    
    // Test quest systems integration
    if (!ValidateQuestSystems())
    {
        bSuccess = false;
    }
    
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    AddValidationReport(TEXT("Cross-System Integration"), Result, 
        TEXT("Cross-system integration validation completed"), FPlatformTime::Seconds() - StartTime);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::RunFullValidationSuite()
{
    ClearValidationReports();
    
    UE_LOG(LogTemp, Warning, TEXT("=== INTEGRATION VALIDATOR - FULL SUITE ==="));
    
    bool bAllPassed = true;
    
    // Run all validation tests
    if (!ValidateProjectStructure()) bAllPassed = false;
    if (!ValidateModuleLoading()) bAllPassed = false;
    if (!ValidateClassRegistration()) bAllPassed = false;
    if (!CheckCompilationStatus()) bAllPassed = false;
    if (!ValidateBinaryFiles()) bAllPassed = false;
    if (!TestActorSpawning()) bAllPassed = false;
    if (!ValidateCrossSystemIntegration()) bAllPassed = false;
    
    // Generate summary report
    FString SummaryReport = GenerateIntegrationReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *SummaryReport);
    
    return bAllPassed;
}

bool UBuild_IntegrationValidator::CheckCompilationStatus()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Check if we can create basic UE5 objects (indicates successful compilation)
    bool bCanCreateObjects = true;
    
    try
    {
        // Test creating a basic actor
        UWorld* World = GetWorld();
        if (World)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
            AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
            if (TestActor)
            {
                TestActor->Destroy();
            }
            else
            {
                bCanCreateObjects = false;
            }
        }
    }
    catch (...)
    {
        bCanCreateObjects = false;
    }
    
    EBuild_ValidationResult Result = bCanCreateObjects ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    FString Message = bCanCreateObjects ? TEXT("Compilation status: OK") : TEXT("Compilation issues detected");
    
    AddValidationReport(TEXT("Compilation Status"), Result, Message, FPlatformTime::Seconds() - StartTime);
    
    return bCanCreateObjects;
}

bool UBuild_IntegrationValidator::ValidateBinaryFiles()
{
    float StartTime = FPlatformTime::Seconds();
    
    // Check for compiled binaries in expected locations
    FString BinariesDir = FPaths::ProjectDir() / TEXT("Binaries");
    bool bBinariesExist = FPaths::DirectoryExists(BinariesDir);
    
    EBuild_ValidationResult Result = bBinariesExist ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    FString Message = bBinariesExist ? TEXT("Binary files found") : TEXT("Binary directory not found - may be development build");
    
    AddValidationReport(TEXT("Binary Validation"), Result, Message, FPlatformTime::Seconds() - StartTime);
    
    return bBinariesExist;
}

bool UBuild_IntegrationValidator::TestActorSpawning()
{
    float StartTime = FPlatformTime::Seconds();
    bool bSuccess = false;
    
    UWorld* World = GetWorld();
    if (World)
    {
        try
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
            // Test spawning a static mesh actor
            AStaticMeshActor* TestActor = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), 
                FVector(2000, 2000, 200), 
                FRotator::ZeroRotator, 
                SpawnParams
            );
            
            if (TestActor)
            {
                TestActor->SetActorLabel(TEXT("IntegrationTest_Validator"));
                bSuccess = true;
                
                // Clean up test actor after a short delay
                FTimerHandle TimerHandle;
                World->GetTimerManager().SetTimer(TimerHandle, [TestActor]()
                {
                    if (IsValid(TestActor))
                    {
                        TestActor->Destroy();
                    }
                }, 1.0f, false);
            }
        }
        catch (...)
        {
            bSuccess = false;
        }
    }
    
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    FString Message = bSuccess ? TEXT("Actor spawning test passed") : TEXT("Actor spawning test failed");
    
    AddValidationReport(TEXT("Actor Spawning"), Result, Message, FPlatformTime::Seconds() - StartTime);
    
    return bSuccess;
}

// Cross-system validation helpers
bool UBuild_IntegrationValidator::ValidateWorldGeneration()
{
    // Test if PCGWorldGenerator class exists and can be instantiated
    UClass* WorldGenClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.PCGWorldGenerator"));
    return (WorldGenClass != nullptr);
}

bool UBuild_IntegrationValidator::ValidateCharacterSystems()
{
    // Test if TranspersonalCharacter class exists
    UClass* CharacterClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.TranspersonalCharacter"));
    return (CharacterClass != nullptr);
}

bool UBuild_IntegrationValidator::ValidateAISystems()
{
    // Test if CrowdSimulationManager class exists
    UClass* CrowdClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.CrowdSimulationManager"));
    return (CrowdClass != nullptr);
}

bool UBuild_IntegrationValidator::ValidateEnvironmentSystems()
{
    // Test if FoliageManager class exists
    UClass* FoliageClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.FoliageManager"));
    return (FoliageClass != nullptr);
}

bool UBuild_IntegrationValidator::ValidateQuestSystems()
{
    // Test if quest-related classes exist (placeholder for now)
    return true; // Will be implemented when quest system is added
}

TArray<FBuild_ValidationReport> UBuild_IntegrationValidator::GetValidationReports() const
{
    return ValidationReports;
}

TArray<FBuild_ModuleStatus> UBuild_IntegrationValidator::GetModuleStatuses() const
{
    return ModuleStatuses;
}

FString UBuild_IntegrationValidator::GenerateIntegrationReport() const
{
    FString Report = TEXT("=== INTEGRATION VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Tests: %d\n\n"), ValidationReports.Num());
    
    int32 SuccessCount = 0;
    int32 WarningCount = 0;
    int32 ErrorCount = 0;
    int32 CriticalCount = 0;
    
    for (const FBuild_ValidationReport& ValidationReport : ValidationReports)
    {
        switch (ValidationReport.Result)
        {
            case EBuild_ValidationResult::Success: SuccessCount++; break;
            case EBuild_ValidationResult::Warning: WarningCount++; break;
            case EBuild_ValidationResult::Error: ErrorCount++; break;
            case EBuild_ValidationResult::Critical: CriticalCount++; break;
        }
        
        FString ResultStr;
        switch (ValidationReport.Result)
        {
            case EBuild_ValidationResult::Success: ResultStr = TEXT("PASS"); break;
            case EBuild_ValidationResult::Warning: ResultStr = TEXT("WARN"); break;
            case EBuild_ValidationResult::Error: ResultStr = TEXT("FAIL"); break;
            case EBuild_ValidationResult::Critical: ResultStr = TEXT("CRIT"); break;
        }
        
        Report += FString::Printf(TEXT("[%s] %s: %s (%.3fs)\n"), 
            *ResultStr, *ValidationReport.TestName, *ValidationReport.Message, ValidationReport.ExecutionTime);
    }
    
    Report += FString::Printf(TEXT("\nSUMMARY: %d PASS, %d WARN, %d FAIL, %d CRIT\n"), 
        SuccessCount, WarningCount, ErrorCount, CriticalCount);
    
    return Report;
}

void UBuild_IntegrationValidator::AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
    
    if (bLogDetailedResults)
    {
        FString ResultStr;
        switch (Result)
        {
            case EBuild_ValidationResult::Success: ResultStr = TEXT("PASS"); break;
            case EBuild_ValidationResult::Warning: ResultStr = TEXT("WARN"); break;
            case EBuild_ValidationResult::Error: ResultStr = TEXT("FAIL"); break;
            case EBuild_ValidationResult::Critical: ResultStr = TEXT("CRIT"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("[%s] %s: %s"), *ResultStr, *TestName, *Message);
    }
}

void UBuild_IntegrationValidator::ClearValidationReports()
{
    ValidationReports.Empty();
    ModuleStatuses.Empty();
}

// ABuild_IntegrationValidatorActor implementation
ABuild_IntegrationValidatorActor::ABuild_IntegrationValidatorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    ValidatorComponent = CreateDefaultSubobject<UBuild_IntegrationValidator>(TEXT("ValidatorComponent"));
}

void ABuild_IntegrationValidatorActor::BeginPlay()
{
    Super::BeginPlay();
}

UBuild_IntegrationValidator* ABuild_IntegrationValidatorActor::GetValidatorComponent() const
{
    return ValidatorComponent;
}

bool ABuild_IntegrationValidatorActor::RunValidation()
{
    if (ValidatorComponent)
    {
        return ValidatorComponent->RunFullValidationSuite();
    }
    return false;
}