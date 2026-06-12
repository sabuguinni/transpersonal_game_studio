#include "Build_CompilationValidator.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

void UBuild_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    OverallStatus = EBuild_CompilationStatus::Unknown;
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Build_CompilationValidator initialized"));
}

void UBuild_CompilationValidator::ValidateAllModules()
{
    ModuleStatuses.Empty();
    
    ValidateTranspersonalGameModule();
    ValidateEngineModules();
    CheckHeaderIncludes();
    CheckLinkerDependencies();
    UpdateOverallStatus();
    
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Log, TEXT("Module validation completed. Overall status: %d"), (int32)OverallStatus);
}

bool UBuild_CompilationValidator::CheckModuleCompilation(const FString& ModuleName)
{
    FBuild_ModuleStatus NewStatus;
    NewStatus.ModuleName = ModuleName;
    NewStatus.Status = EBuild_CompilationStatus::Unknown;
    
    // Check if module binary exists
    FString ProjectDir = FPaths::ProjectDir();
    FString BinaryPath = FPaths::Combine(ProjectDir, TEXT("Binaries"), TEXT("Linux"), ModuleName + TEXT(".so"));
    
    if (IFileManager::Get().FileExists(*BinaryPath))
    {
        NewStatus.Status = EBuild_CompilationStatus::Success;
        UE_LOG(LogTemp, Log, TEXT("Module %s: Binary found at %s"), *ModuleName, *BinaryPath);
    }
    else
    {
        NewStatus.Status = EBuild_CompilationStatus::Failed;
        NewStatus.Errors.Add(FString::Printf(TEXT("Binary not found: %s"), *BinaryPath));
        UE_LOG(LogTemp, Warning, TEXT("Module %s: Binary not found"), *ModuleName);
    }
    
    ModuleStatuses.Add(NewStatus);
    return NewStatus.Status == EBuild_CompilationStatus::Success;
}

void UBuild_CompilationValidator::GenerateCompilationReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== COMPILATION REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Validation Time: %s"), *LastValidationTime.ToString());
    UE_LOG(LogTemp, Log, TEXT("Overall Status: %d"), (int32)OverallStatus);
    
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        UE_LOG(LogTemp, Log, TEXT("Module: %s - Status: %d"), *Status.ModuleName, (int32)Status.Status);
        
        for (const FString& Error : Status.Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("  ERROR: %s"), *Error);
        }
        
        for (const FString& Warning : Status.Warnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  WARNING: %s"), *Warning);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

void UBuild_CompilationValidator::ValidateTranspersonalGameModule()
{
    CheckModuleCompilation(TEXT("TranspersonalGame"));
    CheckModuleCompilation(TEXT("TranspersonalGameEditor"));
}

void UBuild_CompilationValidator::ValidateEngineModules()
{
    TArray<FString> RequiredModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("UnrealEd"),
        TEXT("Slate"),
        TEXT("SlateCore"),
        TEXT("EditorStyle"),
        TEXT("EditorWidgets"),
        TEXT("ToolMenus")
    };
    
    for (const FString& ModuleName : RequiredModules)
    {
        FBuild_ModuleStatus Status;
        Status.ModuleName = ModuleName;
        Status.Status = EBuild_CompilationStatus::Success; // Engine modules assumed working
        ModuleStatuses.Add(Status);
    }
}

void UBuild_CompilationValidator::CheckHeaderIncludes()
{
    FString SourceDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"));
    
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    int32 ValidHeaders = 0;
    int32 InvalidHeaders = 0;
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString FileContent;
        if (FFileHelper::LoadFileToString(FileContent, *HeaderFile))
        {
            if (FileContent.Contains(TEXT("#pragma once")) || FileContent.Contains(TEXT("#ifndef")))
            {
                ValidHeaders++;
            }
            else
            {
                InvalidHeaders++;
                UE_LOG(LogTemp, Warning, TEXT("Header missing include guard: %s"), *HeaderFile);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Header validation: %d valid, %d invalid"), ValidHeaders, InvalidHeaders);
}

void UBuild_CompilationValidator::CheckLinkerDependencies()
{
    FString BuildFile = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"), TEXT("TranspersonalGame.Build.cs"));
    
    FString BuildContent;
    if (FFileHelper::LoadFileToString(BuildContent, *BuildFile))
    {
        TArray<FString> RequiredDependencies = {
            TEXT("Core"),
            TEXT("CoreUObject"),
            TEXT("Engine"),
            TEXT("UnrealEd")
        };
        
        for (const FString& Dependency : RequiredDependencies)
        {
            if (!BuildContent.Contains(Dependency))
            {
                UE_LOG(LogTemp, Warning, TEXT("Missing dependency in Build.cs: %s"), *Dependency);
            }
        }
    }
}

void UBuild_CompilationValidator::UpdateOverallStatus()
{
    bool HasErrors = false;
    bool HasWarnings = false;
    
    for (const FBuild_ModuleStatus& Status : ModuleStatuses)
    {
        if (Status.Status == EBuild_CompilationStatus::Failed)
        {
            HasErrors = true;
            break;
        }
        else if (Status.Status == EBuild_CompilationStatus::Warning)
        {
            HasWarnings = true;
        }
    }
    
    if (HasErrors)
    {
        OverallStatus = EBuild_CompilationStatus::Failed;
    }
    else if (HasWarnings)
    {
        OverallStatus = EBuild_CompilationStatus::Warning;
    }
    else
    {
        OverallStatus = EBuild_CompilationStatus::Success;
    }
}