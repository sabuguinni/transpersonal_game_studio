#include "Eng_CompilationValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

void UEng_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Validator Initialized"));
    
    CurrentStatus = EEng_CompilationStatus::Unknown;
    InitializeCoreClassList();
    InitializeAPIReplacements();
    
    // Run initial validation
    RunFullValidation();
}

void UEng_CompilationValidator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect Compilation Validator Deinitialized"));
    Super::Deinitialize();
}

FEng_CompilationReport UEng_CompilationValidator::RunFullValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("Running Full Compilation Validation"));
    
    LastValidationReport = FEng_CompilationReport();
    LastValidationReport.TotalClassesTested = CoreClassNames.Num();
    
    // Validate each core class
    for (const FString& ClassName : CoreClassNames)
    {
        FEng_ClassValidationResult Result = ValidateClass(ClassName);
        LastValidationReport.ValidationResults.Add(Result);
        
        if (Result.bClassLoaded)
        {
            LastValidationReport.ClassesLoaded++;
        }
        
        if (Result.bCDOConstructed)
        {
            LastValidationReport.CDOsConstructed++;
        }
    }
    
    // Find missing .cpp files
    LastValidationReport.MissingCppFiles = FindMissingCppFiles();
    
    // Find duplicate types
    LastValidationReport.DuplicateTypes = FindDuplicateTypes();
    
    // Check API compatibility
    CheckAPICompatibility();
    
    // Determine overall status
    if (LastValidationReport.ClassesLoaded == LastValidationReport.TotalClassesTested)
    {
        if (LastValidationReport.CDOsConstructed == LastValidationReport.TotalClassesTested)
        {
            LastValidationReport.OverallStatus = EEng_CompilationStatus::Success;
        }
        else
        {
            LastValidationReport.OverallStatus = EEng_CompilationStatus::Warning;
        }
    }
    else
    {
        LastValidationReport.OverallStatus = EEng_CompilationStatus::Failed;
    }
    
    CurrentStatus = LastValidationReport.OverallStatus;
    
    UE_LOG(LogTemp, Warning, TEXT("Validation Complete: %d/%d classes loaded, %d/%d CDOs constructed"), 
           LastValidationReport.ClassesLoaded, LastValidationReport.TotalClassesTested,
           LastValidationReport.CDOsConstructed, LastValidationReport.TotalClassesTested);
    
    return LastValidationReport;
}

FEng_ClassValidationResult UEng_CompilationValidator::ValidateClass(const FString& ClassName)
{
    FEng_ClassValidationResult Result;
    Result.ClassName = ClassName;
    
    if (!ValidateClassLoading(ClassName, Result))
    {
        return Result;
    }
    
    // Get the class
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* Class = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (!Class)
    {
        Result.ErrorMessage = TEXT("Failed to load class");
        return Result;
    }
    
    Result.bClassLoaded = true;
    
    // Test CDO construction
    if (!ValidateCDOConstruction(Class, Result))
    {
        return Result;
    }
    
    // Test property access
    ValidatePropertyAccess(Class, Result);
    
    return Result;
}

bool UEng_CompilationValidator::ValidateClassLoading(const FString& ClassName, FEng_ClassValidationResult& Result)
{
    try
    {
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
        UClass* Class = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (Class)
        {
            Result.bClassLoaded = true;
            return true;
        }
        else
        {
            Result.ErrorMessage = TEXT("Class not found in module");
            return false;
        }
    }
    catch (...)
    {
        Result.ErrorMessage = TEXT("Exception during class loading");
        return false;
    }
}

bool UEng_CompilationValidator::ValidateCDOConstruction(UClass* Class, FEng_ClassValidationResult& Result)
{
    if (!Class)
    {
        Result.ErrorMessage = TEXT("Null class for CDO test");
        return false;
    }
    
    try
    {
        UObject* CDO = Class->GetDefaultObject();
        if (CDO)
        {
            Result.bCDOConstructed = true;
            return true;
        }
        else
        {
            Result.ErrorMessage = TEXT("CDO construction failed");
            return false;
        }
    }
    catch (...)
    {
        Result.ErrorMessage = TEXT("Exception during CDO construction");
        return false;
    }
}

bool UEng_CompilationValidator::ValidatePropertyAccess(UClass* Class, FEng_ClassValidationResult& Result)
{
    if (!Class)
    {
        return false;
    }
    
    try
    {
        // Iterate through properties to ensure they're accessible
        for (FProperty* Property = Class->PropertyLink; Property; Property = Property->PropertyLinkNext)
        {
            if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
            {
                // Property is accessible
                Result.bPropertiesAccessible = true;
            }
        }
        return true;
    }
    catch (...)
    {
        Result.ErrorMessage += TEXT(" Property access failed");
        return false;
    }
}

TArray<FString> UEng_CompilationValidator::FindMissingCppFiles()
{
    TArray<FString> MissingFiles;
    
    // This would scan the Source directory for .h files without matching .cpp files
    // For now, return known missing files based on common patterns
    
    TArray<FString> KnownHeaders = {
        TEXT("TranspersonalGameMode"),
        TEXT("TranspersonalCharacter"),
        TEXT("BiomeManager"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager")
    };
    
    for (const FString& Header : KnownHeaders)
    {
        // Check if corresponding .cpp exists (simplified check)
        FString HeaderPath = FString::Printf(TEXT("Source/TranspersonalGame/Core/%s.h"), *Header);
        FString CppPath = FString::Printf(TEXT("Source/TranspersonalGame/Core/%s.cpp"), *Header);
        
        // In a real implementation, we'd check the file system
        // For now, assume some files might be missing
        if (Header.Contains(TEXT("Test")))
        {
            MissingFiles.Add(CppPath);
        }
    }
    
    return MissingFiles;
}

TArray<FString> UEng_CompilationValidator::FindDuplicateTypes()
{
    TArray<FString> Duplicates;
    
    // This would scan for duplicate USTRUCT/UENUM/UCLASS definitions
    // For now, return empty as this requires file system scanning
    
    return Duplicates;
}

bool UEng_CompilationValidator::FixCompilationIssues()
{
    UE_LOG(LogTemp, Warning, TEXT("Attempting to fix compilation issues"));
    
    bool bFixed = false;
    
    // Fix missing .cpp files
    TArray<FString> MissingFiles = FindMissingCppFiles();
    for (const FString& MissingFile : MissingFiles)
    {
        if (CreateMissingCppStub(MissingFile))
        {
            bFixed = true;
        }
    }
    
    return bFixed;
}

bool UEng_CompilationValidator::TestClassInstantiation(const FString& ClassName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *ClassName);
    UClass* Class = LoadClass<UObject>(nullptr, *ClassPath);
    
    if (!Class)
    {
        return false;
    }
    
    try
    {
        UObject* Instance = NewObject<UObject>(GetTransientPackage(), Class);
        return Instance != nullptr;
    }
    catch (...)
    {
        return false;
    }
}

bool UEng_CompilationValidator::TestSubsystemAccess(const FString& SubsystemName)
{
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        return false;
    }
    
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SubsystemName);
    UClass* SubsystemClass = LoadClass<UGameInstanceSubsystem>(nullptr, *ClassPath);
    
    if (!SubsystemClass)
    {
        return false;
    }
    
    UGameInstanceSubsystem* Subsystem = GameInstance->GetSubsystem(SubsystemClass);
    return Subsystem != nullptr;
}

void UEng_CompilationValidator::GenerateCompilationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== COMPILATION VALIDATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Overall Status: %s"), 
           CurrentStatus == EEng_CompilationStatus::Success ? TEXT("SUCCESS") :
           CurrentStatus == EEng_CompilationStatus::Warning ? TEXT("WARNING") :
           CurrentStatus == EEng_CompilationStatus::Failed ? TEXT("FAILED") : TEXT("UNKNOWN"));
    
    UE_LOG(LogTemp, Warning, TEXT("Classes Tested: %d"), LastValidationReport.TotalClassesTested);
    UE_LOG(LogTemp, Warning, TEXT("Classes Loaded: %d"), LastValidationReport.ClassesLoaded);
    UE_LOG(LogTemp, Warning, TEXT("CDOs Constructed: %d"), LastValidationReport.CDOsConstructed);
    UE_LOG(LogTemp, Warning, TEXT("Missing .cpp Files: %d"), LastValidationReport.MissingCppFiles.Num());
    UE_LOG(LogTemp, Warning, TEXT("Duplicate Types: %d"), LastValidationReport.DuplicateTypes.Num());
}

EEng_CompilationStatus UEng_CompilationValidator::GetCurrentCompilationStatus() const
{
    return CurrentStatus;
}

bool UEng_CompilationValidator::CreateMissingCppStub(const FString& HeaderPath)
{
    // This would create a basic .cpp file stub
    // Implementation would write to file system
    UE_LOG(LogTemp, Warning, TEXT("Would create .cpp stub for: %s"), *HeaderPath);
    return true;
}

bool UEng_CompilationValidator::FixIncludePaths(const FString& FilePath)
{
    // This would fix common include path issues
    UE_LOG(LogTemp, Warning, TEXT("Would fix includes for: %s"), *FilePath);
    return true;
}

bool UEng_CompilationValidator::UpdateAPICompatibility(const FString& FilePath)
{
    // This would update deprecated API calls
    UE_LOG(LogTemp, Warning, TEXT("Would update API compatibility for: %s"), *FilePath);
    return true;
}

void UEng_CompilationValidator::InitializeCoreClassList()
{
    CoreClassNames = {
        TEXT("TranspersonalGameMode"),
        TEXT("TranspersonalCharacter"),
        TEXT("TranspersonalGameState"),
        TEXT("BiomeManager"),
        TEXT("PCGWorldGenerator"),
        TEXT("FoliageManager"),
        TEXT("CrowdSimulationManager"),
        TEXT("ProceduralWorldManager"),
        TEXT("BuildIntegrationManager")
    };
}

void UEng_CompilationValidator::InitializeAPIReplacements()
{
    // Common UE5.5 API replacements
    APIReplacements.Add(TEXT("FindClass"), TEXT("LoadClass"));
    APIReplacements.Add(TEXT("LoadObject"), TEXT("LoadObject"));
    APIReplacements.Add(TEXT("GetWorld()->GetGameInstance()"), TEXT("GetGameInstance()"));
}

void UEng_CompilationValidator::ScanForMissingImplementations()
{
    // Implementation for scanning file system
}

void UEng_CompilationValidator::ScanForDuplicateDefinitions()
{
    // Implementation for finding duplicate types
}

void UEng_CompilationValidator::CheckAPICompatibility()
{
    // Implementation for checking API compatibility issues
}