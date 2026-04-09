#include "BugReportSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Console.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformMemory.h"

DEFINE_LOG_CATEGORY(LogTranspersonalBugReport);

UTranspersonalBugReportSystem::UTranspersonalBugReportSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize bug report settings
    bAutoGenerateScreenshots = true;
    bIncludeSystemInfo = true;
    bIncludePerformanceData = true;
    bIncludePlayerState = true;
    bIncludeWorldState = true;
    
    MaxBugReportsToKeep = 100;
    
    // Initialize bug report directory
    BugReportDirectory = FPaths::ProjectSavedDir() / TEXT("BugReports");
}

void UTranspersonalBugReportSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure bug report directory exists
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*BugReportDirectory))
    {
        PlatformFile.CreateDirectoryTree(*BugReportDirectory);
    }
    
    UE_LOG(LogTranspersonalBugReport, Warning, TEXT("Bug Report System initialized. Reports will be saved to: %s"), *BugReportDirectory);
}

FString UTranspersonalBugReportSystem::SubmitBugReport(const FBugReportData& BugData)
{
    // Generate unique bug report ID
    FString BugReportID = GenerateBugReportID();
    
    // Create comprehensive bug report
    FComprehensiveBugReport Report;
    Report.BugReportID = BugReportID;
    Report.BasicData = BugData;
    Report.Timestamp = FDateTime::Now();
    
    // Collect additional data based on settings
    if (bIncludeSystemInfo)
    {
        CollectSystemInformation(Report);
    }
    
    if (bIncludePerformanceData)
    {
        CollectPerformanceData(Report);
    }
    
    if (bIncludePlayerState)
    {
        CollectPlayerState(Report);
    }
    
    if (bIncludeWorldState)
    {
        CollectWorldState(Report);
    }
    
    // Generate screenshot if enabled
    if (bAutoGenerateScreenshots)
    {
        GenerateScreenshot(Report);
    }
    
    // Save bug report to file
    SaveBugReportToFile(Report);
    
    // Add to in-memory storage
    BugReports.Add(Report);
    
    // Maintain maximum number of reports
    if (BugReports.Num() > MaxBugReportsToKeep)
    {
        BugReports.RemoveAt(0);
    }
    
    // Log the bug report
    UE_LOG(LogTranspersonalBugReport, Warning, TEXT("Bug report submitted: %s - %s"), *BugReportID, *BugData.Title);
    
    // Broadcast bug report event
    OnBugReportSubmitted.Broadcast(Report);
    
    return BugReportID;
}

FString UTranspersonalBugReportSystem::SubmitQuickBugReport(const FString& Title, const FString& Description, EBugSeverity Severity)
{
    FBugReportData QuickReport;
    QuickReport.Title = Title;
    QuickReport.Description = Description;
    QuickReport.Severity = Severity;
    QuickReport.Category = EBugCategory::Gameplay;
    QuickReport.ReporterName = TEXT("Auto-Generated");
    
    return SubmitBugReport(QuickReport);
}

FString UTranspersonalBugReportSystem::SubmitCrashReport(const FString& CrashDescription, const FString& CallStack)
{
    FBugReportData CrashReport;
    CrashReport.Title = TEXT("Application Crash");
    CrashReport.Description = CrashDescription;
    CrashReport.Severity = EBugSeverity::Critical;
    CrashReport.Category = EBugCategory::Crash;
    CrashReport.ReporterName = TEXT("Crash Handler");
    CrashReport.StepsToReproduce = TEXT("Crash occurred during normal gameplay");
    CrashReport.AdditionalNotes = FString::Printf(TEXT("Call Stack:\n%s"), *CallStack);
    
    return SubmitBugReport(CrashReport);
}

FString UTranspersonalBugReportSystem::SubmitPerformanceReport(float CurrentFPS, float MemoryUsageMB, const FString& PerformanceIssue)
{
    FBugReportData PerfReport;
    PerfReport.Title = FString::Printf(TEXT("Performance Issue: %s"), *PerformanceIssue);
    PerfReport.Description = FString::Printf(TEXT("Performance degradation detected. FPS: %.2f, Memory: %.2f MB"), CurrentFPS, MemoryUsageMB);
    PerfReport.Severity = CurrentFPS < 30.0f ? EBugSeverity::High : EBugSeverity::Medium;
    PerfReport.Category = EBugCategory::Performance;
    PerfReport.ReporterName = TEXT("Performance Monitor");
    
    return SubmitBugReport(PerfReport);
}

TArray<FComprehensiveBugReport> UTranspersonalBugReportSystem::GetBugReportsByCategory(EBugCategory Category) const
{
    TArray<FComprehensiveBugReport> FilteredReports;
    
    for (const FComprehensiveBugReport& Report : BugReports)
    {
        if (Report.BasicData.Category == Category)
        {
            FilteredReports.Add(Report);
        }
    }
    
    return FilteredReports;
}

TArray<FComprehensiveBugReport> UTranspersonalBugReportSystem::GetBugReportsBySeverity(EBugSeverity Severity) const
{
    TArray<FComprehensiveBugReport> FilteredReports;
    
    for (const FComprehensiveBugReport& Report : BugReports)
    {
        if (Report.BasicData.Severity == Severity)
        {
            FilteredReports.Add(Report);
        }
    }
    
    return FilteredReports;
}

FComprehensiveBugReport UTranspersonalBugReportSystem::GetBugReportByID(const FString& BugReportID) const
{
    for (const FComprehensiveBugReport& Report : BugReports)
    {
        if (Report.BugReportID == BugReportID)
        {
            return Report;
        }
    }
    
    return FComprehensiveBugReport();
}

int32 UTranspersonalBugReportSystem::GetBugReportCount() const
{
    return BugReports.Num();
}

void UTranspersonalBugReportSystem::ClearAllBugReports()
{
    BugReports.Empty();
    UE_LOG(LogTranspersonalBugReport, Warning, TEXT("All bug reports cleared from memory"));
}

FString UTranspersonalBugReportSystem::ExportBugReportsToCSV() const
{
    FString CSVContent;
    
    // CSV Header
    CSVContent += TEXT("BugReportID,Timestamp,Title,Description,Severity,Category,Reporter,Status\n");
    
    // CSV Data
    for (const FComprehensiveBugReport& Report : BugReports)
    {
        FString SeverityStr = GetSeverityString(Report.BasicData.Severity);
        FString CategoryStr = GetCategoryString(Report.BasicData.Category);
        FString StatusStr = GetStatusString(Report.BasicData.Status);
        
        // Escape commas and quotes in text fields
        FString EscapedTitle = Report.BasicData.Title.Replace(TEXT(","), TEXT(";")).Replace(TEXT("\""), TEXT("\"\""));
        FString EscapedDescription = Report.BasicData.Description.Replace(TEXT(","), TEXT(";")).Replace(TEXT("\""), TEXT("\"\""));
        FString EscapedReporter = Report.BasicData.ReporterName.Replace(TEXT(","), TEXT(";")).Replace(TEXT("\""), TEXT("\"\""));
        
        CSVContent += FString::Printf(TEXT("%s,%s,\"%s\",\"%s\",%s,%s,\"%s\",%s\n"),
            *Report.BugReportID,
            *Report.Timestamp.ToString(),
            *EscapedTitle,
            *EscapedDescription,
            *SeverityStr,
            *CategoryStr,
            *EscapedReporter,
            *StatusStr);
    }
    
    // Save CSV file
    FString FileName = FString::Printf(TEXT("BugReports_Export_%s.csv"), 
        *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FString FilePath = BugReportDirectory / FileName;
    
    if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
    {
        UE_LOG(LogTranspersonalBugReport, Warning, TEXT("Bug reports exported to CSV: %s"), *FilePath);
        return FilePath;
    }
    else
    {
        UE_LOG(LogTranspersonalBugReport, Error, TEXT("Failed to export bug reports to CSV"));
        return FString();
    }
}

FString UTranspersonalBugReportSystem::GenerateBugReportID()
{
    FDateTime Now = FDateTime::Now();
    static int32 SequenceNumber = 0;
    SequenceNumber++;
    
    return FString::Printf(TEXT("BUG_%s_%04d"), 
        *Now.ToString(TEXT("%Y%m%d_%H%M%S")), SequenceNumber);
}

void UTranspersonalBugReportSystem::CollectSystemInformation(FComprehensiveBugReport& Report)
{
    Report.SystemInfo.PlatformName = FPlatformProperties::PlatformName();
    Report.SystemInfo.CPUCores = FPlatformMisc::NumberOfCores();
    Report.SystemInfo.LogicalCores = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
    
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Report.SystemInfo.TotalMemoryGB = MemStats.TotalPhysical / (1024.0f * 1024.0f * 1024.0f);
    Report.SystemInfo.AvailableMemoryGB = MemStats.AvailablePhysical / (1024.0f * 1024.0f * 1024.0f);
    Report.SystemInfo.UsedMemoryGB = MemStats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
    
    Report.SystemInfo.EngineVersion = FEngineVersion::Current().ToString();
    Report.SystemInfo.BuildConfiguration = FApp::GetBuildConfiguration();
    Report.SystemInfo.ProjectName = FApp::GetProjectName();
}

void UTranspersonalBugReportSystem::CollectPerformanceData(FComprehensiveBugReport& Report)
{
    Report.PerformanceData.CurrentFPS = 1.0f / FApp::GetDeltaTime();
    Report.PerformanceData.AverageFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Report.PerformanceData.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Get render thread time if available
    Report.PerformanceData.RenderThreadTime = 16.67f; // Placeholder
    Report.PerformanceData.GameThreadTime = 16.67f; // Placeholder
    Report.PerformanceData.GPUTime = 16.67f; // Placeholder
    
    Report.PerformanceData.DrawCalls = 0; // Would need access to render stats
    Report.PerformanceData.Triangles = 0; // Would need access to render stats
}

void UTranspersonalBugReportSystem::CollectPlayerState(FComprehensiveBugReport& Report)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC)
    {
        Report.PlayerState.PlayerControllerClass = PC->GetClass()->GetName();
        
        if (PC->GetPawn())
        {
            Report.PlayerState.PawnClass = PC->GetPawn()->GetClass()->GetName();
            Report.PlayerState.PlayerLocation = PC->GetPawn()->GetActorLocation();
            Report.PlayerState.PlayerRotation = PC->GetPawn()->GetActorRotation();
            Report.PlayerState.PlayerVelocity = PC->GetPawn()->GetVelocity();
        }
        
        // Get input state
        Report.PlayerState.bIsInputEnabled = PC->IsInputKeyDown(EKeys::AnyKey);
    }
}

void UTranspersonalBugReportSystem::CollectWorldState(FComprehensiveBugReport& Report)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    Report.WorldState.WorldName = World->GetName();
    Report.WorldState.GameMode = World->GetAuthGameMode() ? World->GetAuthGameMode()->GetClass()->GetName() : TEXT("None");
    Report.WorldState.GameState = World->GetGameState() ? World->GetGameState()->GetClass()->GetName() : TEXT("None");
    Report.WorldState.LevelName = World->GetCurrentLevel() ? World->GetCurrentLevel()->GetName() : TEXT("Unknown");
    
    // Count actors by type
    Report.WorldState.TotalActors = 0;
    Report.WorldState.StaticMeshActors = 0;
    Report.WorldState.SkeletalMeshActors = 0;
    Report.WorldState.LightActors = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            Report.WorldState.TotalActors++;
            
            if (Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                Report.WorldState.StaticMeshActors++;
            }
            if (Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                Report.WorldState.SkeletalMeshActors++;
            }
            if (Actor->FindComponentByClass<ULightComponent>())
            {
                Report.WorldState.LightActors++;
            }
        }
    }
    
    Report.WorldState.WorldTimeSeconds = World->GetTimeSeconds();
    Report.WorldState.RealTimeSeconds = World->GetRealTimeSeconds();
}

void UTranspersonalBugReportSystem::GenerateScreenshot(FComprehensiveBugReport& Report)
{
    // Generate screenshot filename
    FString ScreenshotFileName = FString::Printf(TEXT("Screenshot_%s.png"), *Report.BugReportID);
    FString ScreenshotPath = BugReportDirectory / ScreenshotFileName;
    
    // Request screenshot
    if (GEngine && GEngine->GameViewport)
    {
        FScreenshotRequest::RequestScreenshot(ScreenshotPath, false, false);
        Report.ScreenshotPath = ScreenshotPath;
        
        UE_LOG(LogTranspersonalBugReport, Log, TEXT("Screenshot requested: %s"), *ScreenshotPath);
    }
}

void UTranspersonalBugReportSystem::SaveBugReportToFile(const FComprehensiveBugReport& Report)
{
    FString ReportContent = GenerateReportContent(Report);
    
    FString FileName = FString::Printf(TEXT("BugReport_%s.txt"), *Report.BugReportID);
    FString FilePath = BugReportDirectory / FileName;
    
    if (FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTranspersonalBugReport, Log, TEXT("Bug report saved: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTranspersonalBugReport, Error, TEXT("Failed to save bug report: %s"), *FilePath);
    }
}

FString UTranspersonalBugReportSystem::GenerateReportContent(const FComprehensiveBugReport& Report)
{
    FString Content;
    
    // Header
    Content += TEXT("=== TRANSPERSONAL GAME BUG REPORT ===\n");
    Content += FString::Printf(TEXT("Bug Report ID: %s\n"), *Report.BugReportID);
    Content += FString::Printf(TEXT("Timestamp: %s\n"), *Report.Timestamp.ToString());
    Content += TEXT("\n");
    
    // Basic Bug Information
    Content += TEXT("=== BUG INFORMATION ===\n");
    Content += FString::Printf(TEXT("Title: %s\n"), *Report.BasicData.Title);
    Content += FString::Printf(TEXT("Description: %s\n"), *Report.BasicData.Description);
    Content += FString::Printf(TEXT("Severity: %s\n"), *GetSeverityString(Report.BasicData.Severity));
    Content += FString::Printf(TEXT("Category: %s\n"), *GetCategoryString(Report.BasicData.Category));
    Content += FString::Printf(TEXT("Status: %s\n"), *GetStatusString(Report.BasicData.Status));
    Content += FString::Printf(TEXT("Reporter: %s\n"), *Report.BasicData.ReporterName);
    Content += FString::Printf(TEXT("Steps to Reproduce: %s\n"), *Report.BasicData.StepsToReproduce);
    Content += FString::Printf(TEXT("Expected Behavior: %s\n"), *Report.BasicData.ExpectedBehavior);
    Content += FString::Printf(TEXT("Actual Behavior: %s\n"), *Report.BasicData.ActualBehavior);
    Content += FString::Printf(TEXT("Additional Notes: %s\n"), *Report.BasicData.AdditionalNotes);
    Content += TEXT("\n");
    
    // System Information
    if (bIncludeSystemInfo)
    {
        Content += TEXT("=== SYSTEM INFORMATION ===\n");
        Content += FString::Printf(TEXT("Platform: %s\n"), *Report.SystemInfo.PlatformName);
        Content += FString::Printf(TEXT("CPU Cores: %d\n"), Report.SystemInfo.CPUCores);
        Content += FString::Printf(TEXT("Logical Cores: %d\n"), Report.SystemInfo.LogicalCores);
        Content += FString::Printf(TEXT("Total Memory: %.2f GB\n"), Report.SystemInfo.TotalMemoryGB);
        Content += FString::Printf(TEXT("Available Memory: %.2f GB\n"), Report.SystemInfo.AvailableMemoryGB);
        Content += FString::Printf(TEXT("Used Memory: %.2f GB\n"), Report.SystemInfo.UsedMemoryGB);
        Content += FString::Printf(TEXT("Engine Version: %s\n"), *Report.SystemInfo.EngineVersion);
        Content += FString::Printf(TEXT("Build Configuration: %s\n"), *Report.SystemInfo.BuildConfiguration);
        Content += FString::Printf(TEXT("Project Name: %s\n"), *Report.SystemInfo.ProjectName);
        Content += TEXT("\n");
    }
    
    // Performance Data
    if (bIncludePerformanceData)
    {
        Content += TEXT("=== PERFORMANCE DATA ===\n");
        Content += FString::Printf(TEXT("Current FPS: %.2f\n"), Report.PerformanceData.CurrentFPS);
        Content += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), Report.PerformanceData.AverageFrameTime);
        Content += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), Report.PerformanceData.MemoryUsageMB);
        Content += FString::Printf(TEXT("Render Thread Time: %.2f ms\n"), Report.PerformanceData.RenderThreadTime);
        Content += FString::Printf(TEXT("Game Thread Time: %.2f ms\n"), Report.PerformanceData.GameThreadTime);
        Content += FString::Printf(TEXT("GPU Time: %.2f ms\n"), Report.PerformanceData.GPUTime);
        Content += FString::Printf(TEXT("Draw Calls: %d\n"), Report.PerformanceData.DrawCalls);
        Content += FString::Printf(TEXT("Triangles: %d\n"), Report.PerformanceData.Triangles);
        Content += TEXT("\n");
    }
    
    // Player State
    if (bIncludePlayerState)
    {
        Content += TEXT("=== PLAYER STATE ===\n");
        Content += FString::Printf(TEXT("Player Controller: %s\n"), *Report.PlayerState.PlayerControllerClass);
        Content += FString::Printf(TEXT("Pawn Class: %s\n"), *Report.PlayerState.PawnClass);
        Content += FString::Printf(TEXT("Player Location: %s\n"), *Report.PlayerState.PlayerLocation.ToString());
        Content += FString::Printf(TEXT("Player Rotation: %s\n"), *Report.PlayerState.PlayerRotation.ToString());
        Content += FString::Printf(TEXT("Player Velocity: %s\n"), *Report.PlayerState.PlayerVelocity.ToString());
        Content += FString::Printf(TEXT("Input Enabled: %s\n"), Report.PlayerState.bIsInputEnabled ? TEXT("Yes") : TEXT("No"));
        Content += TEXT("\n");
    }
    
    // World State
    if (bIncludeWorldState)
    {
        Content += TEXT("=== WORLD STATE ===\n");
        Content += FString::Printf(TEXT("World Name: %s\n"), *Report.WorldState.WorldName);
        Content += FString::Printf(TEXT("Game Mode: %s\n"), *Report.WorldState.GameMode);
        Content += FString::Printf(TEXT("Game State: %s\n"), *Report.WorldState.GameState);
        Content += FString::Printf(TEXT("Level Name: %s\n"), *Report.WorldState.LevelName);
        Content += FString::Printf(TEXT("Total Actors: %d\n"), Report.WorldState.TotalActors);
        Content += FString::Printf(TEXT("Static Mesh Actors: %d\n"), Report.WorldState.StaticMeshActors);
        Content += FString::Printf(TEXT("Skeletal Mesh Actors: %d\n"), Report.WorldState.SkeletalMeshActors);
        Content += FString::Printf(TEXT("Light Actors: %d\n"), Report.WorldState.LightActors);
        Content += FString::Printf(TEXT("World Time: %.2f seconds\n"), Report.WorldState.WorldTimeSeconds);
        Content += FString::Printf(TEXT("Real Time: %.2f seconds\n"), Report.WorldState.RealTimeSeconds);
        Content += TEXT("\n");
    }
    
    // Screenshot Information
    if (!Report.ScreenshotPath.IsEmpty())
    {
        Content += TEXT("=== SCREENSHOT ===\n");
        Content += FString::Printf(TEXT("Screenshot Path: %s\n"), *Report.ScreenshotPath);
        Content += TEXT("\n");
    }
    
    return Content;
}

FString UTranspersonalBugReportSystem::GetSeverityString(EBugSeverity Severity)
{
    switch (Severity)
    {
        case EBugSeverity::Low: return TEXT("Low");
        case EBugSeverity::Medium: return TEXT("Medium");
        case EBugSeverity::High: return TEXT("High");
        case EBugSeverity::Critical: return TEXT("Critical");
        default: return TEXT("Unknown");
    }
}

FString UTranspersonalBugReportSystem::GetCategoryString(EBugCategory Category)
{
    switch (Category)
    {
        case EBugCategory::Gameplay: return TEXT("Gameplay");
        case EBugCategory::UI: return TEXT("UI");
        case EBugCategory::Audio: return TEXT("Audio");
        case EBugCategory::Graphics: return TEXT("Graphics");
        case EBugCategory::Performance: return TEXT("Performance");
        case EBugCategory::Network: return TEXT("Network");
        case EBugCategory::AI: return TEXT("AI");
        case EBugCategory::Physics: return TEXT("Physics");
        case EBugCategory::Animation: return TEXT("Animation");
        case EBugCategory::Crash: return TEXT("Crash");
        default: return TEXT("Unknown");
    }
}

FString UTranspersonalBugReportSystem::GetStatusString(EBugStatus Status)
{
    switch (Status)
    {
        case EBugStatus::Open: return TEXT("Open");
        case EBugStatus::InProgress: return TEXT("In Progress");
        case EBugStatus::Fixed: return TEXT("Fixed");
        case EBugStatus::Closed: return TEXT("Closed");
        case EBugStatus::Duplicate: return TEXT("Duplicate");
        case EBugStatus::WontFix: return TEXT("Won't Fix");
        default: return TEXT("Unknown");
    }
}