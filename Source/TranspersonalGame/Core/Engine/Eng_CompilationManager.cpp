#include "Eng_CompilationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngCompilationManager, Log, All);

UEng_CompilationManager::UEng_CompilationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize compilation tracking
    TotalFilesTracked = 0;
    FilesWithErrors = 0;
    FilesWithWarnings = 0;
    LastCompilationTime = 0.0f;
    CompilationInProgress = false;
}

void UEng_CompilationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize compilation management system
    InitializeCompilationTracking();
    
    // Perform initial compilation scan
    ScanProjectFiles();
    
    UE_LOG(LogEngCompilationManager, Log, TEXT("Engine Compilation Manager initialized - tracking %d files"), TotalFilesTracked);
}

void UEng_CompilationManager::InitializeCompilationTracking()
{
    // Clear existing tracking data
    TrackedFiles.Empty();
    CompilationIssues.Empty();
    
    // Set up file patterns to track
    FilePatterns = {
        TEXT("*.h"),
        TEXT("*.cpp"),
        TEXT("*.build.cs"),
        TEXT("*.target.cs")
    };
    
    // Set up critical paths
    CriticalPaths = {
        TEXT("Source/TranspersonalGame/"),
        TEXT("Source/TranspersonalGame/Core/"),
        TEXT("Source/TranspersonalGame/Characters/"),
        TEXT("Source/TranspersonalGame/World/"),
        TEXT("Source/TranspersonalGame/AI/")
    };
    
    UE_LOG(LogEngCompilationManager, Log, TEXT("Compilation tracking initialized"));
}

bool UEng_CompilationManager::ScanProjectFiles()
{
    UE_LOG(LogEngCompilationManager, Log, TEXT("Starting project file scan..."));
    
    TotalFilesTracked = 0;
    FilesWithErrors = 0;
    FilesWithWarnings = 0;
    
    // Scan each critical path
    for (const FString& Path : CriticalPaths)
    {
        ScanDirectory(Path);
    }
    
    UE_LOG(LogEngCompilationManager, Log, TEXT("Project scan complete - %d files tracked, %d with errors, %d with warnings"), 
           TotalFilesTracked, FilesWithErrors, FilesWithWarnings);
    
    return FilesWithErrors == 0;
}

void UEng_CompilationManager::ScanDirectory(const FString& DirectoryPath)
{
    FString FullPath = FPaths::ProjectDir() + DirectoryPath;
    
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FullPath))
    {
        UE_LOG(LogEngCompilationManager, Warning, TEXT("Directory does not exist: %s"), *FullPath);
        return;
    }
    
    // Get all files in directory
    TArray<FString> FoundFiles;
    IFileManager& FileManager = IFileManager::Get();
    
    for (const FString& Pattern : FilePatterns)
    {
        FString SearchPattern = FullPath + Pattern;
        FileManager.FindFiles(FoundFiles, *SearchPattern, true, false);
        
        for (const FString& FileName : FoundFiles)
        {
            FString FilePath = DirectoryPath + FileName;
            AnalyzeFile(FilePath);
            TotalFilesTracked++;
        }
        
        FoundFiles.Empty();
    }
}

void UEng_CompilationManager::AnalyzeFile(const FString& FilePath)
{
    FEng_FileCompilationInfo FileInfo;
    FileInfo.FilePath = FilePath;
    FileInfo.LastModified = FDateTime::Now(); // Simplified - would use actual file time
    FileInfo.HasErrors = false;
    FileInfo.HasWarnings = false;
    FileInfo.Issues.Empty();
    
    // Read file content for analysis
    FString FileContent;
    FString FullFilePath = FPaths::ProjectDir() + FilePath;
    
    if (FFileHelper::LoadFileToString(FileContent, *FullFilePath))
    {
        // Analyze file content for common issues
        AnalyzeFileContent(FileContent, FileInfo);
    }
    else
    {
        // File read error
        FEng_CompilationIssue Issue;
        Issue.IssueType = EEng_CompilationIssueType::FileAccess;
        Issue.Severity = EEng_CompilationSeverity::Error;
        Issue.Description = TEXT("Could not read file");
        Issue.LineNumber = 0;
        
        FileInfo.Issues.Add(Issue);
        FileInfo.HasErrors = true;
        FilesWithErrors++;
    }
    
    // Store file info
    TrackedFiles.Add(FileInfo);
    
    if (FileInfo.HasErrors)
    {
        FilesWithErrors++;
    }
    else if (FileInfo.HasWarnings)
    {
        FilesWithWarnings++;
    }
}

void UEng_CompilationManager::AnalyzeFileContent(const FString& Content, FEng_FileCompilationInfo& FileInfo)
{
    TArray<FString> Lines;
    Content.ParseIntoArray(Lines, TEXT("\n"), true);
    
    for (int32 LineIndex = 0; LineIndex < Lines.Num(); LineIndex++)
    {
        const FString& Line = Lines[LineIndex];
        
        // Check for common compilation issues
        CheckIncludeIssues(Line, LineIndex + 1, FileInfo);
        CheckMacroIssues(Line, LineIndex + 1, FileInfo);
        CheckTypeIssues(Line, LineIndex + 1, FileInfo);
        CheckSyntaxIssues(Line, LineIndex + 1, FileInfo);
    }
    
    // File-level checks
    CheckFileStructure(Lines, FileInfo);
}

void UEng_CompilationManager::CheckIncludeIssues(const FString& Line, int32 LineNumber, FEng_FileCompilationInfo& FileInfo)
{
    if (Line.Contains(TEXT("#include")))
    {
        // Check for missing quotes or brackets
        if (!Line.Contains(TEXT("\"")) && !Line.Contains(TEXT("<")))
        {
            AddIssue(FileInfo, EEng_CompilationIssueType::IncludeStructure, EEng_CompilationSeverity::Error,
                     TEXT("Include statement missing quotes or brackets"), LineNumber);
        }
        
        // Check for .generated.h not being last
        if (Line.Contains(TEXT(".generated.h")) && LineNumber < 10)
        {
            // This is a simplified check - would need more sophisticated analysis
            AddIssue(FileInfo, EEng_CompilationIssueType::IncludeStructure, EEng_CompilationSeverity::Warning,
                     TEXT("Generated header should be last include"), LineNumber);
        }
    }
}

void UEng_CompilationManager::CheckMacroIssues(const FString& Line, int32 LineNumber, FEng_FileCompilationInfo& FileInfo)
{
    // Check UPROPERTY issues
    if (Line.Contains(TEXT("UPROPERTY")))
    {
        if (Line.Contains(TEXT("\\\"")) || Line.Contains(TEXT("\\\'")))
        {
            AddIssue(FileInfo, EEng_CompilationIssueType::MacroUsage, EEng_CompilationSeverity::Error,
                     TEXT("Escaped quotes in UPROPERTY - use normal quotes"), LineNumber);
        }
        
        if (Line.Contains(TEXT("CallInEditor = true")))
        {
            AddIssue(FileInfo, EEng_CompilationIssueType::MacroUsage, EEng_CompilationSeverity::Error,
                     TEXT("CallInEditor should be a bare flag, not CallInEditor = true"), LineNumber);
        }
    }
    
    // Check UFUNCTION issues
    if (Line.Contains(TEXT("UFUNCTION")))
    {
        if (Line.Contains(TEXT("CallInEditor = true")))
        {
            AddIssue(FileInfo, EEng_CompilationIssueType::MacroUsage, EEng_CompilationSeverity::Error,
                     TEXT("CallInEditor should be a bare flag"), LineNumber);
        }
    }
}

void UEng_CompilationManager::CheckTypeIssues(const FString& Line, int32 LineNumber, FEng_FileCompilationInfo& FileInfo)
{
    // Check for potential type redefinition issues
    if (Line.Contains(TEXT("USTRUCT")) || Line.Contains(TEXT("UENUM")) || Line.Contains(TEXT("UCLASS")))
    {
        // Check if it's at global scope (simplified check)
        FString TrimmedLine = Line.TrimStartAndEnd();
        if (TrimmedLine.StartsWith(TEXT("    ")) || TrimmedLine.StartsWith(TEXT("\t")))
        {
            AddIssue(FileInfo, EEng_CompilationIssueType::TypeDefinition, EEng_CompilationSeverity::Error,
                     TEXT("USTRUCT/UENUM/UCLASS must be at global scope"), LineNumber);
        }
    }
}

void UEng_CompilationManager::CheckSyntaxIssues(const FString& Line, int32 LineNumber, FEng_FileCompilationInfo& FileInfo)
{
    // Check for common syntax issues
    if (Line.Contains(TEXT("bool bIs")) && Line.Contains(TEXT(" ")))
    {
        // Check for spaces in variable names
        FString AfterBool = Line.RightChop(Line.Find(TEXT("bool bIs")) + 8);
        if (AfterBool.Contains(TEXT(" ")) && !AfterBool.Contains(TEXT(";")))
        {
            AddIssue(FileInfo, EEng_CompilationIssueType::Syntax, EEng_CompilationSeverity::Error,
                     TEXT("Variable name contains spaces"), LineNumber);
        }
    }
}

void UEng_CompilationManager::CheckFileStructure(const TArray<FString>& Lines, FEng_FileCompilationInfo& FileInfo)
{
    bool bHasPragmaOnce = false;
    bool bHasGeneratedInclude = false;
    int32 GeneratedIncludeLine = -1;
    
    for (int32 i = 0; i < Lines.Num(); i++)
    {
        const FString& Line = Lines[i];
        
        if (Line.Contains(TEXT("#pragma once")))
        {
            bHasPragmaOnce = true;
        }
        
        if (Line.Contains(TEXT(".generated.h")))
        {
            bHasGeneratedInclude = true;
            GeneratedIncludeLine = i;
        }
    }
    
    // Check header structure
    if (FileInfo.FilePath.EndsWith(TEXT(".h")))
    {
        if (!bHasPragmaOnce)
        {
            AddIssue(FileInfo, EEng_CompilationIssueType::FileStructure, EEng_CompilationSeverity::Warning,
                     TEXT("Header file missing #pragma once"), 1);
        }
        
        if (bHasGeneratedInclude)
        {
            // Check if .generated.h is near the end
            if (GeneratedIncludeLine < Lines.Num() - 20)
            {
                AddIssue(FileInfo, EEng_CompilationIssueType::IncludeStructure, EEng_CompilationSeverity::Warning,
                         TEXT(".generated.h should be the last include"), GeneratedIncludeLine + 1);
            }
        }
    }
}

void UEng_CompilationManager::AddIssue(FEng_FileCompilationInfo& FileInfo, EEng_CompilationIssueType Type, 
                                       EEng_CompilationSeverity Severity, const FString& Description, int32 LineNumber)
{
    FEng_CompilationIssue Issue;
    Issue.IssueType = Type;
    Issue.Severity = Severity;
    Issue.Description = Description;
    Issue.LineNumber = LineNumber;
    
    FileInfo.Issues.Add(Issue);
    
    if (Severity == EEng_CompilationSeverity::Error)
    {
        FileInfo.HasErrors = true;
    }
    else if (Severity == EEng_CompilationSeverity::Warning)
    {
        FileInfo.HasWarnings = true;
    }
    
    // Add to global issues list
    CompilationIssues.Add(Issue);
}

bool UEng_CompilationManager::ValidateProjectCompilation()
{
    UE_LOG(LogEngCompilationManager, Log, TEXT("Starting project compilation validation..."));
    
    CompilationInProgress = true;
    
    // Rescan all files
    bool bScanSuccess = ScanProjectFiles();
    
    // Generate detailed report
    GenerateCompilationReport();
    
    CompilationInProgress = false;
    LastCompilationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogEngCompilationManager, Log, TEXT("Compilation validation complete - Success: %s"), 
           bScanSuccess ? TEXT("true") : TEXT("false"));
    
    return bScanSuccess;
}

void UEng_CompilationManager::GenerateCompilationReport()
{
    UE_LOG(LogEngCompilationManager, Log, TEXT("=== COMPILATION MANAGER REPORT ==="));
    UE_LOG(LogEngCompilationManager, Log, TEXT("Total files tracked: %d"), TotalFilesTracked);
    UE_LOG(LogEngCompilationManager, Log, TEXT("Files with errors: %d"), FilesWithErrors);
    UE_LOG(LogEngCompilationManager, Log, TEXT("Files with warnings: %d"), FilesWithWarnings);
    UE_LOG(LogEngCompilationManager, Log, TEXT("Total issues found: %d"), CompilationIssues.Num());
    
    if (FilesWithErrors > 0)
    {
        UE_LOG(LogEngCompilationManager, Log, TEXT("=== FILES WITH ERRORS ==="));
        for (const FEng_FileCompilationInfo& FileInfo : TrackedFiles)
        {
            if (FileInfo.HasErrors)
            {
                UE_LOG(LogEngCompilationManager, Error, TEXT("ERROR FILE: %s"), *FileInfo.FilePath);
                for (const FEng_CompilationIssue& Issue : FileInfo.Issues)
                {
                    if (Issue.Severity == EEng_CompilationSeverity::Error)
                    {
                        UE_LOG(LogEngCompilationManager, Error, TEXT("  Line %d: %s"), Issue.LineNumber, *Issue.Description);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogEngCompilationManager, Log, TEXT("=== END COMPILATION REPORT ==="));
}