#include "Build_CompilationManager.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

void UBuild_CompilationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeTrackedModules();
    UE_LOG(LogTemp, Log, TEXT("Build_CompilationManager initialized"));
}

void UBuild_CompilationManager::Deinitialize()
{
    CompilationHistory.Empty();
    Super::Deinitialize();
}

void UBuild_CompilationManager::InitializeTrackedModules()
{
    TrackedModules.Empty();
    TrackedModules.Add(TEXT("TranspersonalGame"));
    TrackedModules.Add(TEXT("UnrealEd"));
    TrackedModules.Add(TEXT("Engine"));
    TrackedModules.Add(TEXT("Core"));
    TrackedModules.Add(TEXT("CoreUObject"));
}

bool UBuild_CompilationManager::IsModuleCompiled(const FString& ModuleName)
{
    FString BinaryPath = GetModuleBinaryPath(ModuleName);
    return CheckBinaryExists(BinaryPath);
}

void UBuild_CompilationManager::ValidateModuleCompilation(const FString& ModuleName)
{
    FBuild_CompilationInfo Info;
    Info.ModuleName = ModuleName;
    Info.Timestamp = FDateTime::Now().ToString();

    double StartTime = FPlatformTime::Seconds();
    
    if (IsModuleCompiled(ModuleName))
    {
        Info.Result = EBuild_CompilationResult::Success;
        UE_LOG(LogTemp, Log, TEXT("✓ Module %s: Compilation SUCCESS"), *ModuleName);
    }
    else
    {
        Info.Result = EBuild_CompilationResult::Error;
        Info.ErrorCount = 1;
        Info.ErrorMessages.Add(FString::Printf(TEXT("Module %s binary not found"), *ModuleName));
        UE_LOG(LogTemp, Error, TEXT("✗ Module %s: Compilation FAILED"), *ModuleName);
    }

    Info.CompilationTime = FPlatformTime::Seconds() - StartTime;
    AddCompilationResult(Info);
    LogCompilationStatus(Info);
}

void UBuild_CompilationManager::CheckAllModulesCompilation()
{
    UE_LOG(LogTemp, Log, TEXT("=== CHECKING ALL MODULES COMPILATION ==="));
    
    int32 SuccessCount = 0;
    int32 FailureCount = 0;

    for (const FString& ModuleName : TrackedModules)
    {
        ValidateModuleCompilation(ModuleName);
        
        if (LastCompilationResult.Result == EBuild_CompilationResult::Success)
        {
            SuccessCount++;
        }
        else
        {
            FailureCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Compilation Summary: %d Success, %d Failed"), SuccessCount, FailureCount);
    
    if (FailureCount == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("🟢 ALL MODULES COMPILED SUCCESSFULLY"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("🟡 %d MODULES FAILED TO COMPILE"), FailureCount);
    }
}

void UBuild_CompilationManager::GenerateCompilationReport()
{
    FString ReportContent = TEXT("COMPILATION MANAGER REPORT\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());

    ReportContent += TEXT("TRACKED MODULES:\n");
    for (const FString& ModuleName : TrackedModules)
    {
        bool IsCompiled = IsModuleCompiled(ModuleName);
        FString Status = IsCompiled ? TEXT("✓ COMPILED") : TEXT("✗ MISSING");
        ReportContent += FString::Printf(TEXT("  %s: %s\n"), *ModuleName, *Status);
    }

    ReportContent += TEXT("\nRECENT COMPILATION HISTORY:\n");
    int32 RecentCount = FMath::Min(10, CompilationHistory.Num());
    for (int32 i = CompilationHistory.Num() - RecentCount; i < CompilationHistory.Num(); i++)
    {
        const FBuild_CompilationInfo& Info = CompilationHistory[i];
        FString ResultStr = UEnum::GetValueAsString(Info.Result);
        ReportContent += FString::Printf(TEXT("  [%s] %s: %s (%.2fs)\n"), 
            *Info.Timestamp, *Info.ModuleName, *ResultStr, Info.CompilationTime);
    }

    FString ReportPath = FPaths::Combine(FPaths::ProjectLogDir(), TEXT("CompilationReport.txt"));
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogTemp, Log, TEXT("Compilation report saved to: %s"), *ReportPath);
}

void UBuild_CompilationManager::RefreshCompilationStatus()
{
    UE_LOG(LogTemp, Log, TEXT("Refreshing compilation status for all modules..."));
    CheckAllModulesCompilation();
    GenerateCompilationReport();
}

void UBuild_CompilationManager::AddCompilationResult(const FBuild_CompilationInfo& Result)
{
    CompilationHistory.Add(Result);
    LastCompilationResult = Result;

    // Maintain history size limit
    while (CompilationHistory.Num() > MaxHistoryEntries)
    {
        CompilationHistory.RemoveAt(0);
    }
}

FString UBuild_CompilationManager::GetModuleBinaryPath(const FString& ModuleName)
{
    FString ProjectDir = FPaths::ProjectDir();
    FString BinariesDir = FPaths::Combine(ProjectDir, TEXT("Binaries"));
    
    // Check different platforms
    TArray<FString> PlatformDirs = {TEXT("Linux"), TEXT("Win64"), TEXT("Mac")};
    TArray<FString> Extensions = {TEXT(".so"), TEXT(".dll"), TEXT(".dylib")};
    
    for (const FString& Platform : PlatformDirs)
    {
        for (const FString& Extension : Extensions)
        {
            FString BinaryPath = FPaths::Combine(BinariesDir, Platform, ModuleName + Extension);
            if (CheckBinaryExists(BinaryPath))
            {
                return BinaryPath;
            }
        }
    }
    
    // Return default path even if not found
    return FPaths::Combine(BinariesDir, TEXT("Linux"), ModuleName + TEXT(".so"));
}

bool UBuild_CompilationManager::CheckBinaryExists(const FString& BinaryPath)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    return PlatformFile.FileExists(*BinaryPath);
}

void UBuild_CompilationManager::ParseCompilationLog(const FString& LogPath, FBuild_CompilationInfo& OutInfo)
{
    FString LogContent;
    if (FFileHelper::LoadFileToString(LogContent, *LogPath))
    {
        TArray<FString> Lines;
        LogContent.ParseIntoArrayLines(Lines);
        
        for (const FString& Line : Lines)
        {
            if (Line.Contains(TEXT("error")))
            {
                OutInfo.ErrorCount++;
                OutInfo.ErrorMessages.Add(Line);
            }
            else if (Line.Contains(TEXT("warning")))
            {
                OutInfo.WarningCount++;
                OutInfo.WarningMessages.Add(Line);
            }
        }
        
        if (OutInfo.ErrorCount > 0)
        {
            OutInfo.Result = EBuild_CompilationResult::Error;
        }
        else if (OutInfo.WarningCount > 0)
        {
            OutInfo.Result = EBuild_CompilationResult::Warning;
        }
        else
        {
            OutInfo.Result = EBuild_CompilationResult::Success;
        }
    }
}

void UBuild_CompilationManager::LogCompilationStatus(const FBuild_CompilationInfo& Info)
{
    FString ResultStr = UEnum::GetValueAsString(Info.Result);
    UE_LOG(LogTemp, Log, TEXT("Module %s compilation: %s (%.2fs)"), 
        *Info.ModuleName, *ResultStr, Info.CompilationTime);
    
    if (Info.ErrorCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("  Errors: %d"), Info.ErrorCount);
    }
    
    if (Info.WarningCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Warnings: %d"), Info.WarningCount);
    }
}