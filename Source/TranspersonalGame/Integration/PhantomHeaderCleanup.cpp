#include "PhantomHeaderCleanup.h"
#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogPhantomCleanup, Log, All);

UPhantomHeaderCleanup::UPhantomHeaderCleanup()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPhantomHeaderCleanup::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoCleanupOnBeginPlay)
    {
        PerformPhantomHeaderAnalysis();
    }
}

void UPhantomHeaderCleanup::PerformPhantomHeaderAnalysis()
{
    UE_LOG(LogPhantomCleanup, Warning, TEXT("=== PHANTOM HEADER ANALYSIS STARTED ==="));
    
    // Get project source directory
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    // Find all .h files
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    // Find all .cpp files  
    TArray<FString> CppFiles;
    IFileManager::Get().FindFilesRecursive(CppFiles, *SourceDir, TEXT("*.cpp"), true, false);
    
    UE_LOG(LogPhantomCleanup, Warning, TEXT("Found %d header files"), HeaderFiles.Num());
    UE_LOG(LogPhantomCleanup, Warning, TEXT("Found %d cpp files"), CppFiles.Num());
    
    // Create set of cpp file basenames for quick lookup
    TSet<FString> CppBasenames;
    for (const FString& CppFile : CppFiles)
    {
        FString Basename = FPaths::GetBaseFilename(CppFile);
        CppBasenames.Add(Basename);
    }
    
    // Find phantom headers
    PhantomHeaders.Empty();
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString Basename = FPaths::GetBaseFilename(HeaderFile);
        
        // Skip certain system files
        if (Basename.Contains(TEXT("generated")) || 
            Basename.EndsWith(TEXT(".Build")) ||
            Basename.EndsWith(TEXT(".Target")))
        {
            continue;
        }
        
        if (!CppBasenames.Contains(Basename))
        {
            PhantomHeaders.Add(HeaderFile);
        }
    }
    
    UE_LOG(LogPhantomCleanup, Error, TEXT("PHANTOM HEADERS DETECTED: %d"), PhantomHeaders.Num());
    
    // Log first 20 phantom headers
    for (int32 i = 0; i < FMath::Min(20, PhantomHeaders.Num()); i++)
    {
        FString RelativePath = PhantomHeaders[i];
        RelativePath.RemoveFromStart(SourceDir);
        UE_LOG(LogPhantomCleanup, Error, TEXT("PHANTOM %d: %s"), i + 1, *RelativePath);
    }
    
    // Identify critical duplicates
    IdentifyCriticalDuplicates();
    
    UE_LOG(LogPhantomCleanup, Warning, TEXT("=== PHANTOM HEADER ANALYSIS COMPLETE ==="));
}

void UPhantomHeaderCleanup::IdentifyCriticalDuplicates()
{
    TArray<FString> CriticalDuplicates = {
        TEXT("DinosaurCrowdSystem_Crowd.h"),
        TEXT("MassDinosaurSystem.h"),
        TEXT("CrowdDensityManager.h")
    };
    
    UE_LOG(LogPhantomCleanup, Warning, TEXT("=== CRITICAL DUPLICATE ANALYSIS ==="));
    
    for (const FString& DuplicateName : CriticalDuplicates)
    {
        TArray<FString> Matches;
        for (const FString& PhantomHeader : PhantomHeaders)
        {
            if (PhantomHeader.Contains(DuplicateName))
            {
                Matches.Add(PhantomHeader);
            }
        }
        
        if (Matches.Num() > 0)
        {
            UE_LOG(LogPhantomCleanup, Error, TEXT("DUPLICATE FOUND: %s - %d instances"), *DuplicateName, Matches.Num());
            for (const FString& Match : Matches)
            {
                FString RelativePath = Match;
                FString SourceDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"));
                RelativePath.RemoveFromStart(SourceDir);
                UE_LOG(LogPhantomCleanup, Error, TEXT("  -> %s"), *RelativePath);
            }
        }
    }
}

TArray<FString> UPhantomHeaderCleanup::GetPhantomHeaders() const
{
    return PhantomHeaders;
}

int32 UPhantomHeaderCleanup::GetPhantomHeaderCount() const
{
    return PhantomHeaders.Num();
}

bool UPhantomHeaderCleanup::IsHeaderPhantom(const FString& HeaderPath) const
{
    return PhantomHeaders.Contains(HeaderPath);
}

void UPhantomHeaderCleanup::GenerateCleanupReport()
{
    FString ReportContent = TEXT("# PHANTOM HEADER CLEANUP REPORT\n\n");
    ReportContent += FString::Printf(TEXT("## Summary\n"));
    ReportContent += FString::Printf(TEXT("- Total phantom headers: %d\n"), PhantomHeaders.Num());
    ReportContent += FString::Printf(TEXT("- Analysis date: %s\n\n"), *FDateTime::Now().ToString());
    
    ReportContent += TEXT("## Phantom Headers List\n");
    for (int32 i = 0; i < PhantomHeaders.Num(); i++)
    {
        FString RelativePath = PhantomHeaders[i];
        FString SourceDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"));
        RelativePath.RemoveFromStart(SourceDir);
        ReportContent += FString::Printf(TEXT("%d. %s\n"), i + 1, *RelativePath);
    }
    
    // Save report to file
    FString ReportPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("PhantomHeaderReport.txt"));
    FFileHelper::SaveStringToFile(ReportContent, *ReportPath);
    
    UE_LOG(LogPhantomCleanup, Warning, TEXT("Cleanup report saved to: %s"), *ReportPath);
}