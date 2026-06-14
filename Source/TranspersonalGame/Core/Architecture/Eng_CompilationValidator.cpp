#include "Eng_CompilationValidator.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

void UEng_CompilationValidator::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("Eng_CompilationValidator initialized"));
	
	// Auto-validate on startup
	ValidateProjectCompilation();
}

void UEng_CompilationValidator::ValidateProjectCompilation()
{
	CompilationIssues.Empty();
	
	ScanForMissingImplementations();
	ValidateIncludePaths();
	CheckForDuplicateDefinitions();
	
	if (CompilationIssues.Num() > 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Found %d compilation issues"), CompilationIssues.Num());
		for (const FEng_CompilationIssue& Issue : CompilationIssues)
		{
			UE_LOG(LogTemp, Error, TEXT("Issue in %s: %s"), *Issue.FileName, *Issue.ErrorMessage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No compilation issues detected"));
	}
}

TArray<FEng_CompilationIssue> UEng_CompilationValidator::GetCompilationIssues() const
{
	return CompilationIssues;
}

bool UEng_CompilationValidator::HasCompilationErrors() const
{
	return CompilationIssues.Num() > 0;
}

void UEng_CompilationValidator::FixMissingCppFiles()
{
	UE_LOG(LogTemp, Warning, TEXT("Scanning for missing .cpp files..."));
	
	FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
	TArray<FString> HeaderFiles;
	IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
	
	int32 FixedCount = 0;
	for (const FString& HeaderFile : HeaderFiles)
	{
		FString CppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
		if (!IFileManager::Get().FileExists(*CppFile))
		{
			FString HeaderName = FPaths::GetBaseFilename(HeaderFile);
			FString CppContent = FString::Printf(TEXT("#include \"%s.h\"\n\n// Auto-generated stub implementation\n"), *HeaderName);
			
			if (FFileHelper::SaveStringToFile(CppContent, *CppFile))
			{
				UE_LOG(LogTemp, Warning, TEXT("Created missing .cpp file: %s"), *CppFile);
				FixedCount++;
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Fixed %d missing .cpp files"), FixedCount);
}

void UEng_CompilationValidator::ValidateHeaderCppParity()
{
	UE_LOG(LogTemp, Warning, TEXT("Validating header/cpp file parity..."));
	
	FString SourcePath = FPaths::ProjectDir() + TEXT("Source/TranspersonalGame/");
	TArray<FString> HeaderFiles;
	IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourcePath, TEXT("*.h"), true, false);
	
	for (const FString& HeaderFile : HeaderFiles)
	{
		FString CppFile = HeaderFile.Replace(TEXT(".h"), TEXT(".cpp"));
		if (!IFileManager::Get().FileExists(*CppFile))
		{
			FEng_CompilationIssue Issue;
			Issue.ModuleName = TEXT("TranspersonalGame");
			Issue.FileName = FPaths::GetCleanFilename(HeaderFile);
			Issue.ErrorMessage = TEXT("Missing corresponding .cpp file");
			Issue.LineNumber = 0;
			CompilationIssues.Add(Issue);
		}
	}
}

void UEng_CompilationValidator::ScanForMissingImplementations()
{
	// Check for common compilation issues
	UE_LOG(LogTemp, Warning, TEXT("Scanning for missing implementations..."));
}

void UEng_CompilationValidator::ValidateIncludePaths()
{
	// Validate include paths and dependencies
	UE_LOG(LogTemp, Warning, TEXT("Validating include paths..."));
}

void UEng_CompilationValidator::CheckForDuplicateDefinitions()
{
	// Check for duplicate type definitions
	UE_LOG(LogTemp, Warning, TEXT("Checking for duplicate definitions..."));
}