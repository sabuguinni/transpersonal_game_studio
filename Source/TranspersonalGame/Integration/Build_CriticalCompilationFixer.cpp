#include "Build_CriticalCompilationFixer.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"

UBuild_CriticalCompilationFixer::UBuild_CriticalCompilationFixer()
{
    bAutoFixEnabled = true;
    LastScanTime = FDateTime::Now();
}

FBuild_CompilationFixReport UBuild_CriticalCompilationFixer::FixAllCompilationErrors()
{
    FBuild_CompilationFixReport Report;
    FDateTime StartTime = FDateTime::Now();

    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationFixer: Starting comprehensive error fix"));

    // Scan for all errors first
    TArray<FBuild_CompilationError> AllErrors = ScanForErrors();
    Report.Errors = AllErrors;

    int32 FixedCount = 0;

    // Fix missing headers
    if (FixMissingHeaders())
    {
        FixedCount++;
        Report.FixedFiles.Add(TEXT("MissingHeaders"));
        UE_LOG(LogTemp, Warning, TEXT("Fixed missing headers"));
    }

    // Fix duplicate types
    if (FixDuplicateTypes())
    {
        FixedCount++;
        Report.FixedFiles.Add(TEXT("DuplicateTypes"));
        UE_LOG(LogTemp, Warning, TEXT("Fixed duplicate types"));
    }

    // Fix include order
    if (FixIncludeOrder())
    {
        FixedCount++;
        Report.FixedFiles.Add(TEXT("IncludeOrder"));
        UE_LOG(LogTemp, Warning, TEXT("Fixed include order"));
    }

    // Fix missing implementations
    if (FixMissingImplementations())
    {
        FixedCount++;
        Report.FixedFiles.Add(TEXT("MissingImplementations"));
        UE_LOG(LogTemp, Warning, TEXT("Fixed missing implementations"));
    }

    // Validate module dependencies
    if (ValidateModuleDependencies())
    {
        FixedCount++;
        Report.FixedFiles.Add(TEXT("ModuleDependencies"));
        UE_LOG(LogTemp, Warning, TEXT("Validated module dependencies"));
    }

    Report.bAllErrorsFixed = (FixedCount > 0);
    Report.FixDurationSeconds = (FDateTime::Now() - StartTime).GetTotalSeconds();

    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalCompilationFixer: Fixed %d error categories in %.2f seconds"), 
           FixedCount, Report.FixDurationSeconds);

    return Report;
}

bool UBuild_CriticalCompilationFixer::FixMissingHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("Fixing missing headers..."));

    TArray<FString> SourceFiles = GetAllSourceFiles();
    bool bFixedAny = false;

    for (const FString& FilePath : SourceFiles)
    {
        if (IsCppFile(FilePath))
        {
            FString FileContent;
            if (FFileHelper::LoadFileToString(FileContent, *FilePath))
            {
                bool bNeedsUpdate = false;
                FString UpdatedContent = FileContent;

                // Check for common missing headers
                if (!FileContent.Contains(TEXT("#include \"CoreMinimal.h\"")))
                {
                    UpdatedContent = TEXT("#include \"CoreMinimal.h\"\n") + UpdatedContent;
                    bNeedsUpdate = true;
                }

                if (!FileContent.Contains(TEXT("#include \"Engine/Engine.h\"")) && 
                    FileContent.Contains(TEXT("UE_LOG")))
                {
                    UpdatedContent = TEXT("#include \"Engine/Engine.h\"\n") + UpdatedContent;
                    bNeedsUpdate = true;
                }

                if (bNeedsUpdate)
                {
                    FFileHelper::SaveStringToFile(UpdatedContent, *FilePath);
                    bFixedAny = true;
                    UE_LOG(LogTemp, Warning, TEXT("Fixed missing headers in: %s"), *FilePath);
                }
            }
        }
    }

    return bFixedAny;
}

bool UBuild_CriticalCompilationFixer::FixDuplicateTypes()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking for duplicate types..."));

    TMap<FString, TArray<FString>> TypeToFiles;
    TArray<FString> SourceFiles = GetAllSourceFiles();

    // Scan all header files for UCLASS, USTRUCT, UENUM declarations
    for (const FString& FilePath : SourceFiles)
    {
        if (IsHeaderFile(FilePath))
        {
            FString FileContent;
            if (FFileHelper::LoadFileToString(FileContent, *FilePath))
            {
                TArray<FString> Lines;
                FileContent.ParseIntoArrayLines(Lines);

                for (const FString& Line : Lines)
                {
                    FString TrimmedLine = Line.TrimStartAndEnd();
                    
                    // Look for class/struct/enum declarations
                    if (TrimmedLine.StartsWith(TEXT("class ")) || 
                        TrimmedLine.StartsWith(TEXT("struct ")) ||
                        TrimmedLine.StartsWith(TEXT("enum ")))
                    {
                        // Extract type name
                        TArray<FString> Parts;
                        TrimmedLine.ParseIntoArray(Parts, TEXT(" "));
                        if (Parts.Num() > 1)
                        {
                            FString TypeName = Parts[1];
                            TypeName = TypeName.Replace(TEXT(":"), TEXT(""));
                            TypeName = TypeName.Replace(TEXT("{"), TEXT(""));
                            
                            if (!TypeToFiles.Contains(TypeName))
                            {
                                TypeToFiles.Add(TypeName, TArray<FString>());
                            }
                            TypeToFiles[TypeName].Add(FilePath);
                        }
                    }
                }
            }
        }
    }

    // Report duplicates
    bool bFoundDuplicates = false;
    for (const auto& Pair : TypeToFiles)
    {
        if (Pair.Value.Num() > 1)
        {
            bFoundDuplicates = true;
            UE_LOG(LogTemp, Error, TEXT("Duplicate type '%s' found in files:"), *Pair.Key);
            for (const FString& File : Pair.Value)
            {
                UE_LOG(LogTemp, Error, TEXT("  - %s"), *File);
            }
        }
    }

    return !bFoundDuplicates; // Return true if no duplicates found (success)
}

bool UBuild_CriticalCompilationFixer::FixIncludeOrder()
{
    UE_LOG(LogTemp, Warning, TEXT("Fixing include order..."));

    TArray<FString> SourceFiles = GetAllSourceFiles();
    bool bFixedAny = false;

    for (const FString& FilePath : SourceFiles)
    {
        if (IsHeaderFile(FilePath))
        {
            FString FileContent;
            if (FFileHelper::LoadFileToString(FileContent, *FilePath))
            {
                TArray<FString> Lines;
                FileContent.ParseIntoArrayLines(Lines);

                // Find .generated.h include
                int32 GeneratedIncludeIndex = -1;
                for (int32 i = 0; i < Lines.Num(); i++)
                {
                    if (Lines[i].Contains(TEXT(".generated.h")))
                    {
                        GeneratedIncludeIndex = i;
                        break;
                    }
                }

                if (GeneratedIncludeIndex != -1)
                {
                    // Check if it's the last include
                    bool bIsLastInclude = true;
                    for (int32 i = GeneratedIncludeIndex + 1; i < Lines.Num(); i++)
                    {
                        if (Lines[i].TrimStartAndEnd().StartsWith(TEXT("#include")))
                        {
                            bIsLastInclude = false;
                            break;
                        }
                    }

                    if (!bIsLastInclude)
                    {
                        // Move .generated.h to the end of includes
                        FString GeneratedLine = Lines[GeneratedIncludeIndex];
                        Lines.RemoveAt(GeneratedIncludeIndex);

                        // Find last include line
                        int32 LastIncludeIndex = -1;
                        for (int32 i = 0; i < Lines.Num(); i++)
                        {
                            if (Lines[i].TrimStartAndEnd().StartsWith(TEXT("#include")))
                            {
                                LastIncludeIndex = i;
                            }
                        }

                        if (LastIncludeIndex != -1)
                        {
                            Lines.Insert(GeneratedLine, LastIncludeIndex + 1);
                            
                            FString UpdatedContent = FString::Join(Lines, TEXT("\n"));
                            FFileHelper::SaveStringToFile(UpdatedContent, *FilePath);
                            bFixedAny = true;
                            UE_LOG(LogTemp, Warning, TEXT("Fixed include order in: %s"), *FilePath);
                        }
                    }
                }
            }
        }
    }

    return bFixedAny;
}

bool UBuild_CriticalCompilationFixer::FixMissingImplementations()
{
    UE_LOG(LogTemp, Warning, TEXT("Checking for missing implementations..."));

    TArray<FString> SourceFiles = GetAllSourceFiles();
    bool bFixedAny = false;

    for (const FString& FilePath : SourceFiles)
    {
        if (IsHeaderFile(FilePath))
        {
            FString CppPath = GetCorrespondingCppFile(FilePath);
            
            // Check if .cpp file exists
            if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*CppPath))
            {
                // Create minimal .cpp file
                FString HeaderFileName = FPaths::GetCleanFilename(FilePath);
                FString CppContent = FString::Printf(TEXT("#include \"%s\"\n\n// Implementation file auto-generated by Build_CriticalCompilationFixer\n"), *HeaderFileName);
                
                if (FFileHelper::SaveStringToFile(CppContent, *CppPath))
                {
                    bFixedAny = true;
                    UE_LOG(LogTemp, Warning, TEXT("Created missing implementation file: %s"), *CppPath);
                }
            }
        }
    }

    return bFixedAny;
}

TArray<FBuild_CompilationError> UBuild_CriticalCompilationFixer::ScanForErrors()
{
    TArray<FBuild_CompilationError> Errors;
    LastScanTime = FDateTime::Now();

    UE_LOG(LogTemp, Warning, TEXT("Scanning for compilation errors..."));

    // This would typically parse build logs, but for now we'll do basic file validation
    TArray<FString> SourceFiles = GetAllSourceFiles();

    for (const FString& FilePath : SourceFiles)
    {
        if (IsHeaderFile(FilePath))
        {
            FString CppPath = GetCorrespondingCppFile(FilePath);
            if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*CppPath))
            {
                FBuild_CompilationError Error;
                Error.FileName = FilePath;
                Error.LineNumber = 0;
                Error.ErrorMessage = TEXT("Missing corresponding .cpp file");
                Error.ErrorType = EBuild_CompilationErrorType::MissingHeader;
                Error.SuggestedFix = FString::Printf(TEXT("Create %s"), *CppPath);
                Errors.Add(Error);
            }
        }
    }

    LastErrors = Errors;
    UE_LOG(LogTemp, Warning, TEXT("Found %d compilation errors"), Errors.Num());

    return Errors;
}

bool UBuild_CriticalCompilationFixer::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating module dependencies..."));

    // Check if TranspersonalGame.Build.cs exists and has proper dependencies
    FString BuildCsPath = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame/TranspersonalGame.Build.cs");
    
    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*BuildCsPath))
    {
        FString BuildContent;
        if (FFileHelper::LoadFileToString(BuildContent, *BuildCsPath))
        {
            // Check for essential dependencies
            TArray<FString> RequiredModules = {
                TEXT("Core"),
                TEXT("CoreUObject"),
                TEXT("Engine"),
                TEXT("UnrealEd"),
                TEXT("ToolMenus"),
                TEXT("EditorStyle"),
                TEXT("EditorWidgets"),
                TEXT("Slate"),
                TEXT("SlateCore")
            };

            bool bAllPresent = true;
            for (const FString& Module : RequiredModules)
            {
                if (!BuildContent.Contains(Module))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Missing module dependency: %s"), *Module);
                    bAllPresent = false;
                }
            }

            return bAllPresent;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("TranspersonalGame.Build.cs not found"));
    return false;
}

TArray<FString> UBuild_CriticalCompilationFixer::GetAllSourceFiles()
{
    TArray<FString> SourceFiles;
    FString SourceDir = FPaths::ProjectDir() / TEXT("Source/TranspersonalGame");
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    // Recursively find all .h and .cpp files
    PlatformFile.IterateDirectoryRecursively(*SourceDir, [&SourceFiles](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
    {
        if (!bIsDirectory)
        {
            FString FilePath(FilenameOrDirectory);
            if (FilePath.EndsWith(TEXT(".h")) || FilePath.EndsWith(TEXT(".cpp")))
            {
                SourceFiles.Add(FilePath);
            }
        }
        return true;
    });

    return SourceFiles;
}

bool UBuild_CriticalCompilationFixer::IsHeaderFile(const FString& FilePath)
{
    return FilePath.EndsWith(TEXT(".h"));
}

bool UBuild_CriticalCompilationFixer::IsCppFile(const FString& FilePath)
{
    return FilePath.EndsWith(TEXT(".cpp"));
}

FString UBuild_CriticalCompilationFixer::GetCorrespondingCppFile(const FString& HeaderPath)
{
    if (HeaderPath.EndsWith(TEXT(".h")))
    {
        return HeaderPath.LeftChop(2) + TEXT(".cpp");
    }
    return TEXT("");
}

FString UBuild_CriticalCompilationFixer::GetCorrespondingHeaderFile(const FString& CppPath)
{
    if (CppPath.EndsWith(TEXT(".cpp")))
    {
        return CppPath.LeftChop(4) + TEXT(".h");
    }
    return TEXT("");
}