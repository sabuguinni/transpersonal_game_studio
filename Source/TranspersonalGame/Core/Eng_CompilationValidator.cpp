#include "Eng_CompilationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Modules/ModuleManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

AEng_CompilationValidator::AEng_CompilationValidator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create validator mesh component
    ValidatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ValidatorMesh"));
    ValidatorMesh->SetupAttachment(RootComponent);

    // Default settings
    ValidationInterval = 30.0f;
    bAutoValidateOnStart = true;
    bLogDetailedResults = true;
    bIsValidating = false;
    LastValidationTime = 0.0f;
    ValidationCount = 0;
    bValidationInProgress = false;
    ValidationStartTime = 0.0f;

    // Initialize critical class names
    CriticalClassNames.Add(TEXT("TranspersonalCharacter"));
    CriticalClassNames.Add(TEXT("TranspersonalGameState"));
    CriticalClassNames.Add(TEXT("DinosaurBase"));
    CriticalClassNames.Add(TEXT("BiomeManager"));
    CriticalClassNames.Add(TEXT("Eng_PhysicsCore"));
    CriticalClassNames.Add(TEXT("Eng_BiomeArchitecture"));

    // Initialize required modules
    RequiredModules.Add(TEXT("TranspersonalGame"));
    RequiredModules.Add(TEXT("Engine"));
    RequiredModules.Add(TEXT("Core"));
    RequiredModules.Add(TEXT("CoreUObject"));
}

void AEng_CompilationValidator::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoValidateOnStart)
    {
        // Delay initial validation to allow everything to load
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &AEng_CompilationValidator::RunFullValidation,
            2.0f,
            false
        );
    }

    StartAutoValidation();
}

void AEng_CompilationValidator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bValidationInProgress)
    {
        PerformValidationTick();
    }
}

void AEng_CompilationValidator::RunFullValidation()
{
    if (bIsValidating)
    {
        LogValidationWarning(TEXT("Validation already in progress"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Starting Full Compilation Validation"));

    bIsValidating = true;
    ValidationStartTime = GetWorld()->GetTimeSeconds();
    ValidationCount++;

    // Reset current report
    CurrentReport = FEng_CompilationReport();
    CurrentReport.LastCompilationTime = FDateTime::Now().ToString();

    // Run all validation phases
    ValidateModules();
    ValidateClasses();
    ValidateCriticalSystems();

    // Finalize report
    CurrentReport.ValidationDuration = GetWorld()->GetTimeSeconds() - ValidationStartTime;
    
    // Determine overall status
    if (CurrentReport.FailedClasses == 0 && CurrentReport.MissingCppFiles == 0)
    {
        CurrentReport.OverallStatus = EEng_CompilationStatus::Success;
    }
    else if (CurrentReport.LoadedClasses > CurrentReport.FailedClasses)
    {
        CurrentReport.OverallStatus = EEng_CompilationStatus::PartialSuccess;
    }
    else
    {
        CurrentReport.OverallStatus = EEng_CompilationStatus::Failed;
    }

    bIsValidating = false;
    LastValidationTime = GetWorld()->GetTimeSeconds();

    if (bLogDetailedResults)
    {
        LogValidationResults();
    }

    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Validation Complete. Status: %s"), 
           *UEnum::GetValueAsString(CurrentReport.OverallStatus));
}

void AEng_CompilationValidator::ValidateClasses()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Classes..."));

    CurrentReport.ClassResults.Empty();
    CurrentReport.TotalClasses = CriticalClassNames.Num();
    CurrentReport.LoadedClasses = 0;
    CurrentReport.FailedClasses = 0;

    for (const FString& ClassName : CriticalClassNames)
    {
        FEng_ClassValidationResult Result = ValidateClass(ClassName);
        CurrentReport.ClassResults.Add(Result);

        if (Result.bIsLoaded)
        {
            CurrentReport.LoadedClasses++;
        }
        else
        {
            CurrentReport.FailedClasses++;
        }

        if (!Result.bHasCppFile)
        {
            CurrentReport.MissingCppFiles++;
        }
    }
}

void AEng_CompilationValidator::ValidateModules()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Modules..."));

    CurrentReport.ModuleResults.Empty();

    for (const FString& ModuleName : RequiredModules)
    {
        FEng_ModuleValidationResult Result = ValidateModule(ModuleName);
        CurrentReport.ModuleResults.Add(Result);
    }
}

void AEng_CompilationValidator::ValidateCriticalSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Validating Critical Systems..."));

    // Test basic UE5 functionality
    UWorld* World = GetWorld();
    if (!World)
    {
        LogValidationError(TEXT("World not accessible"));
        return;
    }

    // Test actor spawning capability
    try
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        UE_LOG(LogTemp, Log, TEXT("World contains %d actors"), AllActors.Num());
    }
    catch (...)
    {
        LogValidationError(TEXT("Failed to enumerate world actors"));
    }
}

FEng_ClassValidationResult AEng_CompilationValidator::ValidateClass(const FString& ClassName)
{
    FEng_ClassValidationResult Result;
    Result.ClassName = ClassName;
    Result.LastValidationTime = GetWorld()->GetTimeSeconds();

    // Try to load the class
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    
    try
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
        Result.bIsLoaded = (LoadedClass != nullptr);
        
        if (Result.bIsLoaded)
        {
            UE_LOG(LogTemp, Log, TEXT("✓ Class %s loaded successfully"), *ClassName);
        }
        else
        {
            Result.ErrorMessage = TEXT("Class not found in module");
            UE_LOG(LogTemp, Warning, TEXT("✗ Class %s failed to load"), *ClassName);
        }
    }
    catch (...)
    {
        Result.bIsLoaded = false;
        Result.ErrorMessage = TEXT("Exception during class loading");
        UE_LOG(LogTemp, Error, TEXT("✗ Exception loading class %s"), *ClassName);
    }

    // Check for header file
    FString HeaderPath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/Core/") + ClassName + TEXT(".h");
    Result.bHasHeaderFile = FPlatformFileManager::Get().GetPlatformFile().FileExists(*HeaderPath);

    // Check for cpp file
    FString CppPath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/Core/") + ClassName + TEXT(".cpp");
    Result.bHasCppFile = FPlatformFileManager::Get().GetPlatformFile().FileExists(*CppPath);

    if (!Result.bHasCppFile && Result.bHasHeaderFile)
    {
        Result.ErrorMessage += TEXT(" Missing .cpp implementation");
    }

    return Result;
}

FEng_ModuleValidationResult AEng_CompilationValidator::ValidateModule(const FString& ModuleName)
{
    FEng_ModuleValidationResult Result;
    Result.ModuleName = ModuleName;

    // Check if module is loaded
    if (FModuleManager::Get().IsModuleLoaded(*ModuleName))
    {
        Result.Status = EEng_ModuleStatus::Loaded;
        UE_LOG(LogTemp, Log, TEXT("✓ Module %s is loaded"), *ModuleName);
    }
    else
    {
        Result.Status = EEng_ModuleStatus::NotLoaded;
        UE_LOG(LogTemp, Warning, TEXT("✗ Module %s is not loaded"), *ModuleName);
    }

    return Result;
}

bool AEng_CompilationValidator::IsClassLoaded(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
    return (LoadedClass != nullptr);
}

bool AEng_CompilationValidator::IsModuleLoaded(const FString& ModuleName)
{
    return FModuleManager::Get().IsModuleLoaded(*ModuleName);
}

EEng_CompilationStatus AEng_CompilationValidator::GetCompilationHealth()
{
    return CurrentReport.OverallStatus;
}

float AEng_CompilationValidator::GetCompilationScore()
{
    if (CurrentReport.TotalClasses == 0)
        return 0.0f;

    return (float)CurrentReport.LoadedClasses / (float)CurrentReport.TotalClasses;
}

bool AEng_CompilationValidator::HasCriticalErrors()
{
    return CurrentReport.FailedClasses > 0 || CurrentReport.MissingCppFiles > 0;
}

TArray<FString> AEng_CompilationValidator::GetCriticalErrors()
{
    TArray<FString> Errors;
    
    for (const FEng_ClassValidationResult& Result : CurrentReport.ClassResults)
    {
        if (!Result.bIsLoaded || !Result.ErrorMessage.IsEmpty())
        {
            Errors.Add(FString::Printf(TEXT("%s: %s"), *Result.ClassName, *Result.ErrorMessage));
        }
    }
    
    return Errors;
}

FString AEng_CompilationValidator::GenerateValidationReport()
{
    FString Report = TEXT("=== ENGINE ARCHITECT COMPILATION VALIDATION REPORT ===\n");
    Report += FString::Printf(TEXT("Validation Time: %s\n"), *CurrentReport.LastCompilationTime);
    Report += FString::Printf(TEXT("Overall Status: %s\n"), *UEnum::GetValueAsString(CurrentReport.OverallStatus));
    Report += FString::Printf(TEXT("Duration: %.2f seconds\n"), CurrentReport.ValidationDuration);
    Report += FString::Printf(TEXT("Classes: %d loaded, %d failed, %d total\n"), 
                             CurrentReport.LoadedClasses, CurrentReport.FailedClasses, CurrentReport.TotalClasses);
    Report += FString::Printf(TEXT("Missing .cpp files: %d\n\n"), CurrentReport.MissingCppFiles);

    Report += TEXT("Class Details:\n");
    for (const FEng_ClassValidationResult& Result : CurrentReport.ClassResults)
    {
        FString Status = Result.bIsLoaded ? TEXT("LOADED") : TEXT("FAILED");
        Report += FString::Printf(TEXT("  %s: %s\n"), *Result.ClassName, *Status);
        if (!Result.ErrorMessage.IsEmpty())
        {
            Report += FString::Printf(TEXT("    Error: %s\n"), *Result.ErrorMessage);
        }
    }

    return Report;
}

void AEng_CompilationValidator::LogValidationResults()
{
    FString Report = GenerateValidationReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void AEng_CompilationValidator::StartAutoValidation()
{
    if (ValidationInterval > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &AEng_CompilationValidator::RunFullValidation,
            ValidationInterval,
            true
        );
    }
}

void AEng_CompilationValidator::StopAutoValidation()
{
    GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
}

void AEng_CompilationValidator::CleanupOrphanedHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Cleaning up orphaned headers"));
    
    // This would scan for .h files without corresponding .cpp files
    // Implementation would check filesystem and report findings
}

void AEng_CompilationValidator::IdentifyMissingCppFiles()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Identifying missing .cpp files"));
    
    TArray<FString> MissingFiles;
    for (const FEng_ClassValidationResult& Result : CurrentReport.ClassResults)
    {
        if (Result.bHasHeaderFile && !Result.bHasCppFile)
        {
            MissingFiles.Add(Result.ClassName + TEXT(".cpp"));
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d missing .cpp files"), MissingFiles.Num());
    for (const FString& MissingFile : MissingFiles)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Missing: %s"), *MissingFile);
    }
}

TArray<FString> AEng_CompilationValidator::GetLoadedClasses()
{
    TArray<FString> LoadedClasses;
    for (const FEng_ClassValidationResult& Result : CurrentReport.ClassResults)
    {
        if (Result.bIsLoaded)
        {
            LoadedClasses.Add(Result.ClassName);
        }
    }
    return LoadedClasses;
}

TArray<FString> AEng_CompilationValidator::GetFailedClasses()
{
    TArray<FString> FailedClasses;
    for (const FEng_ClassValidationResult& Result : CurrentReport.ClassResults)
    {
        if (!Result.bIsLoaded)
        {
            FailedClasses.Add(Result.ClassName);
        }
    }
    return FailedClasses;
}

void AEng_CompilationValidator::SetValidationInterval(float NewInterval)
{
    ValidationInterval = NewInterval;
    if (ValidationInterval > 0.0f)
    {
        StartAutoValidation();
    }
    else
    {
        StopAutoValidation();
    }
}

void AEng_CompilationValidator::RefreshModule(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("Refreshing module: %s"), *ModuleName);
    // Module refresh logic would go here
}

void AEng_CompilationValidator::RepairBrokenDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect - Repairing broken dependencies"));
    // Dependency repair logic would go here
}

void AEng_CompilationValidator::SaveValidationReport(const FString& FilePath)
{
    FString Report = GenerateValidationReport();
    FFileHelper::SaveStringToFile(Report, *FilePath);
}

// Private helper functions
void AEng_CompilationValidator::PerformValidationTick()
{
    // Update validation progress
    UpdateValidationStatus();
}

void AEng_CompilationValidator::UpdateValidationStatus()
{
    // Update internal validation state
}

void AEng_CompilationValidator::ProcessValidationResults()
{
    // Process and analyze validation results
}

bool AEng_CompilationValidator::CheckFileExists(const FString& FilePath)
{
    return FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath);
}

FString AEng_CompilationValidator::GetClassPath(const FString& ClassName)
{
    return FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
}

void AEng_CompilationValidator::LogValidationError(const FString& Message)
{
    UE_LOG(LogTemp, Error, TEXT("Engine Architect Validation Error: %s"), *Message);
}

void AEng_CompilationValidator::LogValidationWarning(const FString& Message)
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Validation Warning: %s"), *Message);
}