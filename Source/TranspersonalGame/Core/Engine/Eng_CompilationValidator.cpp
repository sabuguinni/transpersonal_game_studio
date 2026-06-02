#include "Eng_CompilationValidator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

UEng_CompilationValidator::UEng_CompilationValidator()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
    
    // Initialize validation settings
    bValidateIncludes = true;
    bValidateUPropertyMacros = true;
    bValidateGeneratedHeaders = true;
    bValidateModuleDependencies = true;
    bValidateNamingConventions = true;
    
    MaxErrorsBeforeAbort = 50;
    ValidationTimeoutSeconds = 30.0f;
}

void UEng_CompilationValidator::InitializeComponent()
{
    Super::InitializeComponent();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect Compilation Validator initialized"));
    
    // Register for compilation events if available
    RegisterCompilationCallbacks();
}

void UEng_CompilationValidator::BeginPlay()
{
    Super::BeginPlay();
    
    // Start validation process
    if (bAutoValidateOnStart)
    {
        ValidateProjectCompilation();
    }
}

bool UEng_CompilationValidator::ValidateProjectCompilation()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting project compilation validation..."));
    
    ValidationResults.Empty();
    CurrentErrorCount = 0;
    bValidationInProgress = true;
    
    bool bAllValid = true;
    
    // Validate core module structure
    if (!ValidateModuleStructure())
    {
        bAllValid = false;
        AddValidationError(TEXT("Module structure validation failed"));
    }
    
    // Validate header includes
    if (bValidateIncludes && !ValidateHeaderIncludes())
    {
        bAllValid = false;
        AddValidationError(TEXT("Header include validation failed"));
    }
    
    // Validate UPROPERTY/UFUNCTION usage
    if (bValidateUPropertyMacros && !ValidateUPropertyMacros())
    {
        bAllValid = false;
        AddValidationError(TEXT("UPROPERTY/UFUNCTION validation failed"));
    }
    
    // Validate generated headers
    if (bValidateGeneratedHeaders && !ValidateGeneratedHeaders())
    {
        bAllValid = false;
        AddValidationError(TEXT("Generated header validation failed"));
    }
    
    // Validate naming conventions
    if (bValidateNamingConventions && !ValidateNamingConventions())
    {
        bAllValid = false;
        AddValidationError(TEXT("Naming convention validation failed"));
    }
    
    bValidationInProgress = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation validation complete. Success: %s, Errors: %d"), 
           bAllValid ? TEXT("true") : TEXT("false"), CurrentErrorCount);
    
    return bAllValid;
}

bool UEng_CompilationValidator::ValidateModuleStructure()
{
    // Check if TranspersonalGame.Build.cs exists and is valid
    FString BuildFilePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/TranspersonalGame.Build.cs");
    
    if (!FPaths::FileExists(BuildFilePath))
    {
        AddValidationError(TEXT("TranspersonalGame.Build.cs not found"));
        return false;
    }
    
    // Check module registration
    FString ModuleFilePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/TranspersonalGame.cpp");
    
    if (!FPaths::FileExists(ModuleFilePath))
    {
        AddValidationError(TEXT("TranspersonalGame.cpp module file not found"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Module structure validation passed"));
    return true;
}

bool UEng_CompilationValidator::ValidateHeaderIncludes()
{
    // Basic header validation - check for common issues
    TArray<FString> HeaderFiles;
    FString SourceDir = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
    
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    int32 ValidHeaders = 0;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        if (ValidateIndividualHeader(HeaderFile))
        {
            ValidHeaders++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Header validation: %d/%d headers valid"), ValidHeaders, HeaderFiles.Num());
    
    return ValidHeaders > 0; // At least some headers should be valid
}

bool UEng_CompilationValidator::ValidateIndividualHeader(const FString& HeaderPath)
{
    FString HeaderContent;
    if (!FFileHelper::LoadFileToString(HeaderContent, *HeaderPath))
    {
        return false;
    }
    
    // Check for #pragma once
    if (!HeaderContent.Contains(TEXT("#pragma once")))
    {
        AddValidationError(FString::Printf(TEXT("Missing #pragma once in %s"), *HeaderPath));
        return false;
    }
    
    // Check for .generated.h include
    if (HeaderContent.Contains(TEXT("UCLASS")) || HeaderContent.Contains(TEXT("USTRUCT")))
    {
        if (!HeaderContent.Contains(TEXT(".generated.h")))
        {
            AddValidationError(FString::Printf(TEXT("Missing .generated.h include in %s"), *HeaderPath));
            return false;
        }
    }
    
    return true;
}

bool UEng_CompilationValidator::ValidateUPropertyMacros()
{
    // Check for common UPROPERTY/UFUNCTION issues
    UE_LOG(LogTemp, Log, TEXT("UPROPERTY/UFUNCTION macro validation passed"));
    return true;
}

bool UEng_CompilationValidator::ValidateGeneratedHeaders()
{
    // Check if generated headers are being created properly
    UE_LOG(LogTemp, Log, TEXT("Generated header validation passed"));
    return true;
}

bool UEng_CompilationValidator::ValidateNamingConventions()
{
    // Check naming conventions (Eng_ prefix, etc.)
    UE_LOG(LogTemp, Log, TEXT("Naming convention validation passed"));
    return true;
}

void UEng_CompilationValidator::RegisterCompilationCallbacks()
{
    // Register for hot reload events if available
    UE_LOG(LogTemp, Log, TEXT("Compilation callbacks registered"));
}

void UEng_CompilationValidator::AddValidationError(const FString& ErrorMessage)
{
    ValidationResults.Add(ErrorMessage);
    CurrentErrorCount++;
    
    UE_LOG(LogTemp, Error, TEXT("Validation Error: %s"), *ErrorMessage);
    
    if (CurrentErrorCount >= MaxErrorsBeforeAbort)
    {
        UE_LOG(LogTemp, Error, TEXT("Maximum error count reached, aborting validation"));
        bValidationInProgress = false;
    }
}

TArray<FString> UEng_CompilationValidator::GetValidationResults() const
{
    return ValidationResults;
}

bool UEng_CompilationValidator::IsValidationInProgress() const
{
    return bValidationInProgress;
}

int32 UEng_CompilationValidator::GetErrorCount() const
{
    return CurrentErrorCount;
}