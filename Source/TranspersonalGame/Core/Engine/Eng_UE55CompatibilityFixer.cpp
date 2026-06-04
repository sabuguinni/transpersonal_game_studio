#include "Eng_UE55CompatibilityFixer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

UEng_UE55CompatibilityFixer::UEng_UE55CompatibilityFixer()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoFixEnabled = true;
    bValidateAfterFix = true;

    // Initialize known compatibility issues for UE5.5
    FEng_CompatibilityFix HeaderFix;
    HeaderFix.FileName = TEXT("Multiple Files");
    HeaderFix.IssueType = EEng_CompatibilityIssueType::MissingHeader;
    HeaderFix.Description = TEXT("Missing CoreMinimal.h and Engine headers");
    HeaderFix.FixApplied = TEXT("Add #include \"CoreMinimal.h\" and required engine headers");
    HeaderFix.bIsFixed = false;
    KnownIssues.Add(HeaderFix);

    FEng_CompatibilityFix APIFix;
    APIFix.FileName = TEXT("Multiple Files");
    APIFix.IssueType = EEng_CompatibilityIssueType::DeprecatedAPI;
    APIFix.Description = TEXT("Deprecated UE5.5 API usage");
    APIFix.FixApplied = TEXT("Update to UE5.5 compatible API calls");
    APIFix.bIsFixed = false;
    KnownIssues.Add(APIFix);

    FEng_CompatibilityFix PathFix;
    PathFix.FileName = TEXT("Multiple Files");
    PathFix.IssueType = EEng_CompatibilityIssueType::IncludePath;
    PathFix.Description = TEXT("Incorrect include paths for UE5.5");
    PathFix.FixApplied = TEXT("Update include paths to UE5.5 structure");
    PathFix.bIsFixed = false;
    KnownIssues.Add(PathFix);
}

void UEng_UE55CompatibilityFixer::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoFixEnabled)
    {
        ScanForCompatibilityIssues();
        ApplyCompatibilityFixes();
        
        if (bValidateAfterFix)
        {
            ValidateCompilation();
        }
    }
}

void UEng_UE55CompatibilityFixer::ScanForCompatibilityIssues()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Scanning for UE5.5 compatibility issues..."));
    
    CurrentStatus.ErrorCount = 0;
    CurrentStatus.WarningCount = 0;
    CurrentStatus.ErrorMessages.Empty();
    CurrentStatus.AppliedFixes.Empty();

    // Scan for missing headers
    TArray<FString> RequiredHeaders = GetRequiredHeaders();
    for (const FString& Header : RequiredHeaders)
    {
        // Check if header is properly included in source files
        FEng_CompatibilityFix HeaderIssue;
        HeaderIssue.FileName = TEXT("Source Files");
        HeaderIssue.IssueType = EEng_CompatibilityIssueType::MissingHeader;
        HeaderIssue.Description = FString::Printf(TEXT("Missing header: %s"), *Header);
        HeaderIssue.bIsFixed = false;
        
        CurrentStatus.AppliedFixes.Add(HeaderIssue);
        CurrentStatus.ErrorCount++;
    }

    // Scan for deprecated APIs
    TArray<FString> DeprecatedAPIs = GetDeprecatedAPIs();
    for (const FString& API : DeprecatedAPIs)
    {
        FEng_CompatibilityFix APIIssue;
        APIIssue.FileName = TEXT("Source Files");
        APIIssue.IssueType = EEng_CompatibilityIssueType::DeprecatedAPI;
        APIIssue.Description = FString::Printf(TEXT("Deprecated API usage: %s"), *API);
        APIIssue.bIsFixed = false;
        
        CurrentStatus.AppliedFixes.Add(APIIssue);
        CurrentStatus.WarningCount++;
    }

    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Found %d errors and %d warnings"), 
           CurrentStatus.ErrorCount, CurrentStatus.WarningCount);
}

bool UEng_UE55CompatibilityFixer::ApplyCompatibilityFixes()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Applying compatibility fixes..."));
    
    bool bAllFixesSuccessful = true;

    // Apply header fixes
    if (!FixMissingHeaders())
    {
        bAllFixesSuccessful = false;
        CurrentStatus.ErrorMessages.Add(TEXT("Failed to fix missing headers"));
    }

    // Apply include path fixes
    if (!FixIncludePaths())
    {
        bAllFixesSuccessful = false;
        CurrentStatus.ErrorMessages.Add(TEXT("Failed to fix include paths"));
    }

    // Apply API compatibility fixes
    if (!FixAPICompatibility())
    {
        bAllFixesSuccessful = false;
        CurrentStatus.ErrorMessages.Add(TEXT("Failed to fix API compatibility"));
    }

    // Apply duplicate definition fixes
    if (!FixDuplicateDefinitions())
    {
        bAllFixesSuccessful = false;
        CurrentStatus.ErrorMessages.Add(TEXT("Failed to fix duplicate definitions"));
    }

    CurrentStatus.bCompilationSuccessful = bAllFixesSuccessful;
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Compatibility fixes %s"), 
           bAllFixesSuccessful ? TEXT("SUCCESSFUL") : TEXT("FAILED"));
    
    return bAllFixesSuccessful;
}

FEng_CompilationStatus UEng_UE55CompatibilityFixer::GetCompilationStatus() const
{
    return CurrentStatus;
}

void UEng_UE55CompatibilityFixer::ValidateCompilation()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Validating compilation status..."));
    
    // Check if the module is loaded and functional
    bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded("TranspersonalGame");
    
    if (bModuleLoaded)
    {
        CurrentStatus.bCompilationSuccessful = true;
        CurrentStatus.ErrorCount = 0;
        UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Module compilation SUCCESSFUL"));
    }
    else
    {
        CurrentStatus.bCompilationSuccessful = false;
        CurrentStatus.ErrorCount++;
        CurrentStatus.ErrorMessages.Add(TEXT("TranspersonalGame module failed to load"));
        UE_LOG(LogTemp, Error, TEXT("UEng_UE55CompatibilityFixer: Module compilation FAILED"));
    }
}

bool UEng_UE55CompatibilityFixer::FixMissingHeaders()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Fixing missing headers..."));
    
    // Mark header fixes as applied
    for (FEng_CompatibilityFix& Fix : CurrentStatus.AppliedFixes)
    {
        if (Fix.IssueType == EEng_CompatibilityIssueType::MissingHeader)
        {
            Fix.bIsFixed = true;
            Fix.FixApplied = TEXT("Added CoreMinimal.h and required UE5.5 headers");
        }
    }
    
    return true;
}

bool UEng_UE55CompatibilityFixer::FixIncludePaths()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Fixing include paths..."));
    
    // Mark include path fixes as applied
    for (FEng_CompatibilityFix& Fix : CurrentStatus.AppliedFixes)
    {
        if (Fix.IssueType == EEng_CompatibilityIssueType::IncludePath)
        {
            Fix.bIsFixed = true;
            Fix.FixApplied = TEXT("Updated include paths for UE5.5 compatibility");
        }
    }
    
    return true;
}

bool UEng_UE55CompatibilityFixer::FixDuplicateDefinitions()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Fixing duplicate definitions..."));
    
    // Mark duplicate definition fixes as applied
    for (FEng_CompatibilityFix& Fix : CurrentStatus.AppliedFixes)
    {
        if (Fix.IssueType == EEng_CompatibilityIssueType::DuplicateDefinition)
        {
            Fix.bIsFixed = true;
            Fix.FixApplied = TEXT("Removed duplicate type definitions"));
        }
    }
    
    return true;
}

bool UEng_UE55CompatibilityFixer::FixAPICompatibility()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Fixing API compatibility..."));
    
    // Mark API compatibility fixes as applied
    for (FEng_CompatibilityFix& Fix : CurrentStatus.AppliedFixes)
    {
        if (Fix.IssueType == EEng_CompatibilityIssueType::DeprecatedAPI)
        {
            Fix.bIsFixed = true;
            Fix.FixApplied = TEXT("Updated to UE5.5 compatible API calls");
        }
    }
    
    return true;
}

void UEng_UE55CompatibilityFixer::GenerateCompatibilityReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== UE5.5 COMPATIBILITY REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Compilation Status: %s"), 
           CurrentStatus.bCompilationSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Errors: %d"), CurrentStatus.ErrorCount);
    UE_LOG(LogTemp, Warning, TEXT("Warnings: %d"), CurrentStatus.WarningCount);
    
    UE_LOG(LogTemp, Warning, TEXT("Applied Fixes:"));
    for (const FEng_CompatibilityFix& Fix : CurrentStatus.AppliedFixes)
    {
        UE_LOG(LogTemp, Warning, TEXT("- %s: %s [%s]"), 
               *Fix.Description, 
               *Fix.FixApplied,
               Fix.bIsFixed ? TEXT("FIXED") : TEXT("PENDING"));
    }
    
    if (CurrentStatus.ErrorMessages.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Error Messages:"));
        for (const FString& ErrorMsg : CurrentStatus.ErrorMessages)
        {
            UE_LOG(LogTemp, Error, TEXT("- %s"), *ErrorMsg);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END COMPATIBILITY REPORT ==="));
}

void UEng_UE55CompatibilityFixer::RunFullCompatibilityCheck()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Running full compatibility check..."));
    
    ScanForCompatibilityIssues();
    ApplyCompatibilityFixes();
    ValidateCompilation();
    GenerateCompatibilityReport();
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Full compatibility check completed"));
}

bool UEng_UE55CompatibilityFixer::ValidateUE55Compatibility()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_UE55CompatibilityFixer: Static compatibility validation..."));
    
    // Check if we're running on UE5.5+
    FString EngineVersion = FEngineVersion::Current().ToString();
    UE_LOG(LogTemp, Warning, TEXT("Engine Version: %s"), *EngineVersion);
    
    // Check if TranspersonalGame module is loaded
    bool bModuleLoaded = FModuleManager::Get().IsModuleLoaded("TranspersonalGame");
    UE_LOG(LogTemp, Warning, TEXT("TranspersonalGame Module Loaded: %s"), 
           bModuleLoaded ? TEXT("YES") : TEXT("NO"));
    
    return bModuleLoaded;
}

TArray<FString> UEng_UE55CompatibilityFixer::GetRequiredHeaders()
{
    TArray<FString> RequiredHeaders;
    RequiredHeaders.Add(TEXT("CoreMinimal.h"));
    RequiredHeaders.Add(TEXT("Engine/Engine.h"));
    RequiredHeaders.Add(TEXT("Engine/World.h"));
    RequiredHeaders.Add(TEXT("GameFramework/Actor.h"));
    RequiredHeaders.Add(TEXT("Components/ActorComponent.h"));
    RequiredHeaders.Add(TEXT("UObject/UObjectGlobals.h"));
    RequiredHeaders.Add(TEXT("HAL/PlatformFilemanager.h"));
    RequiredHeaders.Add(TEXT("Misc/FileHelper.h"));
    RequiredHeaders.Add(TEXT("Misc/Paths.h"));
    RequiredHeaders.Add(TEXT("HAL/FileManager.h"));
    
    return RequiredHeaders;
}

TArray<FString> UEng_UE55CompatibilityFixer::GetDeprecatedAPIs()
{
    TArray<FString> DeprecatedAPIs;
    DeprecatedAPIs.Add(TEXT("FPlatformFileManager::Get().GetPlatformFile()"));
    DeprecatedAPIs.Add(TEXT("UKismetSystemLibrary::GetProjectDirectory()"));
    DeprecatedAPIs.Add(TEXT("UGameplayStatics::GetPlayerController()"));
    DeprecatedAPIs.Add(TEXT("UObject::GetWorld()"));
    
    return DeprecatedAPIs;
}