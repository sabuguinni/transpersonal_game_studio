#include "Build_IntegrationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuild_IntegrationValidator::UBuild_IntegrationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bAutoRunOnBeginPlay = false;
    bLogDetailedResults = true;
    ValidationTimeout = 30.0f;
    
    // Initialize critical classes to validate
    CriticalClasses.Add(TEXT("TranspersonalCharacter"));
    CriticalClasses.Add(TEXT("TranspersonalGameState"));
    CriticalClasses.Add(TEXT("PCGWorldGenerator"));
    CriticalClasses.Add(TEXT("FoliageManager"));
    CriticalClasses.Add(TEXT("CrowdSimulationManager"));
    
    // Initialize critical modules
    CriticalModules.Add(TEXT("Core"));
    CriticalModules.Add(TEXT("Character"));
    CriticalModules.Add(TEXT("World"));
    CriticalModules.Add(TEXT("QA"));
    CriticalModules.Add(TEXT("Integration"));
}

void UBuild_IntegrationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        RunFullValidationSuite();
    }
}

bool UBuild_IntegrationValidator::ValidateProjectStructure()
{
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATING PROJECT STRUCTURE ==="));
    
    bool bSuccess = true;
    FString ProjectDir = FPaths::ProjectDir();
    
    // Check critical directories
    TArray<FString> CriticalDirs = {
        TEXT("Source/TranspersonalGame"),
        TEXT("Content"),
        TEXT("Config")
    };
    
    for (const FString& Dir : CriticalDirs)
    {
        FString FullPath = FPaths::Combine(ProjectDir, Dir);
        if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FullPath))
        {
            UE_LOG(LogTemp, Log, TEXT("Directory found: %s"), *Dir);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Critical directory missing: %s"), *Dir);
            bSuccess = false;
        }
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    AddValidationReport(TEXT("Project Structure"), Result, 
        bSuccess ? TEXT("All critical directories found") : TEXT("Missing critical directories"), 
        ExecutionTime);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateModuleLoading()
{
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATING MODULE LOADING ==="));
    
    bool bSuccess = true;
    TArray<FString> LoadedClasses;
    TArray<FString> FailedClasses;
    
    for (const FString& ClassName : CriticalClasses)
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (LoadedClass)
        {
            LoadedClasses.Add(ClassName);
            UE_LOG(LogTemp, Log, TEXT("Class loaded successfully: %s"), *ClassName);
        }
        else
        {
            FailedClasses.Add(ClassName);
            UE_LOG(LogTemp, Warning, TEXT("Failed to load class: %s"), *ClassName);
            bSuccess = false;
        }
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    
    FString Message = FString::Printf(TEXT("Loaded %d/%d classes"), LoadedClasses.Num(), CriticalClasses.Num());
    AddValidationReport(TEXT("Module Loading"), Result, Message, ExecutionTime);
    AddModuleStatus(TEXT("TranspersonalGame"), bSuccess, LoadedClasses, FailedClasses);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateLevelIntegrity()
{
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATING LEVEL INTEGRITY ==="));
    
    bool bSuccess = true;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found"));
        AddValidationReport(TEXT("Level Integrity"), EBuild_ValidationResult::Error, 
            TEXT("No valid world found"), FPlatformTime::Seconds() - StartTime);
        return false;
    }
    
    // Count actors in the level
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Found %d actors in current level"), ActorCount);
    
    // Check for essential actors
    bool bHasPlayerStart = false;
    bool bHasLighting = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->GetClass()->GetName().Contains(TEXT("PlayerStart")))
        {
            bHasPlayerStart = true;
        }
        if (Actor->GetClass()->GetName().Contains(TEXT("Light")))
        {
            bHasLighting = true;
        }
    }
    
    if (!bHasPlayerStart)
    {
        UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found in level"));
        bSuccess = false;
    }
    
    if (!bHasLighting)
    {
        UE_LOG(LogTemp, Warning, TEXT("No lighting actors found in level"));
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Warning;
    
    FString Message = FString::Printf(TEXT("Level has %d actors, PlayerStart: %s, Lighting: %s"), 
        ActorCount, 
        bHasPlayerStart ? TEXT("Yes") : TEXT("No"),
        bHasLighting ? TEXT("Yes") : TEXT("No"));
    
    AddValidationReport(TEXT("Level Integrity"), Result, Message, ExecutionTime);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateActorSpawning()
{
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATING ACTOR SPAWNING ==="));
    
    bool bSuccess = true;
    UWorld* World = GetWorld();
    
    if (!World)
    {
        AddValidationReport(TEXT("Actor Spawning"), EBuild_ValidationResult::Error, 
            TEXT("No valid world for spawning test"), FPlatformTime::Seconds() - StartTime);
        return false;
    }
    
    // Test spawning a basic actor
    FVector SpawnLocation(0.0f, 0.0f, 100.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    AActor* TestActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation);
    
    if (TestActor)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully spawned test actor"));
        TestActor->SetActorLabel(TEXT("ValidationTest_Actor"));
        
        // Clean up test actor
        TestActor->Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn test actor"));
        bSuccess = false;
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    
    AddValidationReport(TEXT("Actor Spawning"), Result, 
        bSuccess ? TEXT("Actor spawning functional") : TEXT("Actor spawning failed"), 
        ExecutionTime);
    
    return bSuccess;
}

bool UBuild_IntegrationValidator::ValidateComponentSystems()
{
    float StartTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATING COMPONENT SYSTEMS ==="));
    
    bool bSuccess = true;
    
    // Test component creation
    UStaticMeshComponent* TestComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValidationTestComponent"));
    
    if (TestComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("Component creation successful"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Component creation failed"));
        bSuccess = false;
    }
    
    float ExecutionTime = FPlatformTime::Seconds() - StartTime;
    EBuild_ValidationResult Result = bSuccess ? EBuild_ValidationResult::Success : EBuild_ValidationResult::Error;
    
    AddValidationReport(TEXT("Component Systems"), Result, 
        bSuccess ? TEXT("Component systems functional") : TEXT("Component systems failed"), 
        ExecutionTime);
    
    return bSuccess;
}

void UBuild_IntegrationValidator::RunFullValidationSuite()
{
    UE_LOG(LogTemp, Log, TEXT("=== STARTING FULL VALIDATION SUITE ==="));
    
    ClearValidationData();
    
    float SuiteStartTime = FPlatformTime::Seconds();
    
    // Run all validation tests
    ValidateProjectStructure();
    ValidateModuleLoading();
    ValidateLevelIntegrity();
    ValidateActorSpawning();
    ValidateComponentSystems();
    
    float SuiteExecutionTime = FPlatformTime::Seconds() - SuiteStartTime;
    
    UE_LOG(LogTemp, Log, TEXT("=== VALIDATION SUITE COMPLETE (%.2fs) ==="), SuiteExecutionTime);
    
    LogValidationSummary();
    GenerateIntegrationReport();
}

void UBuild_IntegrationValidator::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== INTEGRATION REPORT ==="));
    
    int32 SuccessCount = 0;
    int32 WarningCount = 0;
    int32 ErrorCount = 0;
    
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        switch (Report.Result)
        {
            case EBuild_ValidationResult::Success:
                SuccessCount++;
                break;
            case EBuild_ValidationResult::Warning:
                WarningCount++;
                break;
            case EBuild_ValidationResult::Error:
            case EBuild_ValidationResult::Critical:
                ErrorCount++;
                break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Validation Summary: %d Success, %d Warnings, %d Errors"), 
        SuccessCount, WarningCount, ErrorCount);
    
    for (const FBuild_ModuleStatus& ModuleStatus : ModuleStatuses)
    {
        UE_LOG(LogTemp, Log, TEXT("Module %s: %s (%d classes loaded)"), 
            *ModuleStatus.ModuleName, 
            ModuleStatus.bIsLoaded ? TEXT("LOADED") : TEXT("FAILED"),
            ModuleStatus.ClassCount);
    }
}

void UBuild_IntegrationValidator::LogValidationSummary()
{
    if (bLogDetailedResults)
    {
        for (const FBuild_ValidationReport& Report : ValidationReports)
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
            
            UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s (%.3fs)"), 
                *ResultString, *Report.TestName, *Report.Message, Report.ExecutionTime);
        }
    }
}

void UBuild_IntegrationValidator::ClearValidationData()
{
    ValidationReports.Empty();
    ModuleStatuses.Empty();
}

bool UBuild_IntegrationValidator::IsValidationPassing() const
{
    return GetErrorCount() == 0;
}

int32 UBuild_IntegrationValidator::GetErrorCount() const
{
    int32 ErrorCount = 0;
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        if (Report.Result == EBuild_ValidationResult::Error || Report.Result == EBuild_ValidationResult::Critical)
        {
            ErrorCount++;
        }
    }
    return ErrorCount;
}

int32 UBuild_IntegrationValidator::GetWarningCount() const
{
    int32 WarningCount = 0;
    for (const FBuild_ValidationReport& Report : ValidationReports)
    {
        if (Report.Result == EBuild_ValidationResult::Warning)
        {
            WarningCount++;
        }
    }
    return WarningCount;
}

void UBuild_IntegrationValidator::AddValidationReport(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime)
{
    FBuild_ValidationReport Report;
    Report.TestName = TestName;
    Report.Result = Result;
    Report.Message = Message;
    Report.ExecutionTime = ExecutionTime;
    
    ValidationReports.Add(Report);
}

void UBuild_IntegrationValidator::AddModuleStatus(const FString& ModuleName, bool bLoaded, const TArray<FString>& LoadedClasses, const TArray<FString>& FailedClasses)
{
    FBuild_ModuleStatus Status;
    Status.ModuleName = ModuleName;
    Status.bIsLoaded = bLoaded;
    Status.ClassCount = LoadedClasses.Num();
    Status.LoadedClasses = LoadedClasses;
    Status.FailedClasses = FailedClasses;
    
    ModuleStatuses.Add(Status);
}

// === ACTOR IMPLEMENTATION ===

ABuild_IntegrationValidatorActor::ABuild_IntegrationValidatorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    ValidationComponent = CreateDefaultSubobject<UBuild_IntegrationValidator>(TEXT("ValidationComponent"));
}

void ABuild_IntegrationValidatorActor::BeginPlay()
{
    Super::BeginPlay();
}

void ABuild_IntegrationValidatorActor::RunValidation()
{
    if (ValidationComponent)
    {
        ValidationComponent->RunFullValidationSuite();
    }
}