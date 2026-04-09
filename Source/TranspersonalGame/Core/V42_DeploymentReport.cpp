// Copyright Transpersonal Game Studio. All Rights Reserved.
// V42_DeploymentReport.cpp - Deployment status and health report for v4.2

#include "V42_DeploymentReport.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "StudioDirector.h"
#include "TranspersonalCoreSubsystem.h"

UV42DeploymentReport::UV42DeploymentReport()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    DeploymentInfo.Version = TEXT("4.2");
    DeploymentInfo.BuildDate = FDateTime::Now();
    DeploymentInfo.Status = EDeploymentStatus::Unknown;
}

void UV42DeploymentReport::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate deployment report on startup
    GenerateDeploymentReport();
}

void UV42DeploymentReport::GenerateDeploymentReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== GENERATING v4.2 DEPLOYMENT REPORT ==="));
    
    // Reset report data
    DeploymentInfo.SystemReports.Empty();
    DeploymentInfo.PerformanceMetrics.Empty();
    DeploymentInfo.Issues.Empty();
    
    // Collect system information
    CollectSystemInformation();
    
    // Analyze performance metrics
    CollectPerformanceMetrics();
    
    // Check for issues
    AnalyzeSystemHealth();
    
    // Determine overall deployment status
    DetermineDeploymentStatus();
    
    // Generate report output
    GenerateReportOutput();
    
    UE_LOG(LogTemp, Warning, TEXT("=== DEPLOYMENT REPORT GENERATION COMPLETE ==="));
}

void UV42DeploymentReport::CollectSystemInformation()
{
    // Studio Director System
    FSystemReport DirectorReport;
    DirectorReport.SystemName = TEXT("Studio Director");
    DirectorReport.ModuleName = TEXT("TranspersonalGameCore");
    DirectorReport.bIsOnline = false;
    DirectorReport.LastUpdateTime = 0.0f;
    
    UStudioDirectorSubsystem* DirectorSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStudioDirectorSubsystem>();
    if (DirectorSubsystem)
    {
        DirectorReport.bIsOnline = true;
        DirectorReport.LastUpdateTime = GetWorld()->GetTimeSeconds();
        DirectorReport.AdditionalInfo.Add(TEXT("Studio Director Subsystem operational"));
        
        TArray<FSystemInfo> RegisteredSystems = DirectorSubsystem->GetAllSystems();
        DirectorReport.AdditionalInfo.Add(FString::Printf(TEXT("Registered systems: %d"), RegisteredSystems.Num()));
        
        bool bCreativeVisionCompliant = DirectorSubsystem->IsCreativeVisionCompliant();
        DirectorReport.AdditionalInfo.Add(FString::Printf(TEXT("Creative vision compliance: %s"), 
            bCreativeVisionCompliant ? TEXT("YES") : TEXT("NO")));
    }
    else
    {
        DirectorReport.AdditionalInfo.Add(TEXT("Studio Director Subsystem not found"));
    }
    
    DeploymentInfo.SystemReports.Add(DirectorReport);
    
    // Core Subsystem
    FSystemReport CoreReport;
    CoreReport.SystemName = TEXT("Transpersonal Core");
    CoreReport.ModuleName = TEXT("TranspersonalGameCore");
    CoreReport.bIsOnline = false;
    CoreReport.LastUpdateTime = 0.0f;
    
    UTranspersonalCoreSubsystem* CoreSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UTranspersonalCoreSubsystem>();
    if (CoreSubsystem)
    {
        CoreReport.bIsOnline = true;
        CoreReport.LastUpdateTime = GetWorld()->GetTimeSeconds();
        CoreReport.AdditionalInfo.Add(TEXT("Core subsystem operational"));
        
        if (CoreSubsystem->IsInitialized())
        {
            CoreReport.AdditionalInfo.Add(TEXT("Core subsystem fully initialized"));
        }
        else
        {
            CoreReport.AdditionalInfo.Add(TEXT("Core subsystem initializing"));
        }
    }
    else
    {
        CoreReport.AdditionalInfo.Add(TEXT("Core subsystem not found"));
    }
    
    DeploymentInfo.SystemReports.Add(CoreReport);
    
    // Module Status
    FModuleManager& ModuleManager = FModuleManager::Get();
    TArray<FName> LoadedModules;
    ModuleManager.FindModules(TEXT("Transpersonal*"), LoadedModules);
    
    for (const FName& ModuleName : LoadedModules)
    {
        FSystemReport ModuleReport;
        ModuleReport.SystemName = FString::Printf(TEXT("Module: %s"), *ModuleName.ToString());
        ModuleReport.ModuleName = ModuleName.ToString();
        ModuleReport.bIsOnline = ModuleManager.IsModuleLoaded(ModuleName);
        ModuleReport.LastUpdateTime = GetWorld()->GetTimeSeconds();
        
        if (ModuleReport.bIsOnline)
        {
            ModuleReport.AdditionalInfo.Add(TEXT("Module loaded successfully"));
        }
        else
        {
            ModuleReport.AdditionalInfo.Add(TEXT("Module failed to load"));
        }
        
        DeploymentInfo.SystemReports.Add(ModuleReport);
    }
}

void UV42DeploymentReport::CollectPerformanceMetrics()
{
    // Frame rate metrics
    FPerformanceMetric FrameRateMetric;
    FrameRateMetric.MetricName = TEXT("Frame Rate");
    FrameRateMetric.Value = 1.0f / GetWorld()->GetDeltaSeconds();
    FrameRateMetric.Unit = TEXT("FPS");
    FrameRateMetric.bIsWithinTarget = FrameRateMetric.Value >= 30.0f; // Minimum 30 FPS
    DeploymentInfo.PerformanceMetrics.Add(FrameRateMetric);
    
    // Memory metrics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    FPerformanceMetric MemoryUsageMetric;
    MemoryUsageMetric.MetricName = TEXT("Memory Usage");
    MemoryUsageMetric.Value = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    MemoryUsageMetric.Unit = TEXT("MB");
    MemoryUsageMetric.bIsWithinTarget = MemoryUsageMetric.Value < 2048.0f; // Under 2GB
    DeploymentInfo.PerformanceMetrics.Add(MemoryUsageMetric);
    
    FPerformanceMetric MemoryAvailableMetric;
    MemoryAvailableMetric.MetricName = TEXT("Available Memory");
    MemoryAvailableMetric.Value = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
    MemoryAvailableMetric.Unit = TEXT("MB");
    MemoryAvailableMetric.bIsWithinTarget = MemoryAvailableMetric.Value > 512.0f; // At least 512MB free
    DeploymentInfo.PerformanceMetrics.Add(MemoryAvailableMetric);
    
    // Asset count metrics
    IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
    TArray<FAssetData> AllAssets;
    AssetRegistry.GetAllAssets(AllAssets);
    
    FPerformanceMetric AssetCountMetric;
    AssetCountMetric.MetricName = TEXT("Total Assets");
    AssetCountMetric.Value = AllAssets.Num();
    AssetCountMetric.Unit = TEXT("Assets");
    AssetCountMetric.bIsWithinTarget = true; // No specific target for asset count
    DeploymentInfo.PerformanceMetrics.Add(AssetCountMetric);
}

void UV42DeploymentReport::AnalyzeSystemHealth()
{
    // Check for critical issues
    
    // Missing subsystems
    UStudioDirectorSubsystem* DirectorSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStudioDirectorSubsystem>();
    if (!DirectorSubsystem)
    {
        FDeploymentIssue Issue;
        Issue.Severity = EIssueSeverity::Critical;
        Issue.Category = TEXT("Subsystem");
        Issue.Description = TEXT("Studio Director Subsystem not found");
        Issue.Recommendation = TEXT("Verify TranspersonalGameCore module is properly loaded");
        DeploymentInfo.Issues.Add(Issue);
    }
    
    UTranspersonalCoreSubsystem* CoreSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UTranspersonalCoreSubsystem>();
    if (!CoreSubsystem)
    {
        FDeploymentIssue Issue;
        Issue.Severity = EIssueSeverity::Critical;
        Issue.Category = TEXT("Subsystem");
        Issue.Description = TEXT("Transpersonal Core Subsystem not found");
        Issue.Recommendation = TEXT("Verify TranspersonalGameCore module is properly loaded");
        DeploymentInfo.Issues.Add(Issue);
    }
    
    // Performance issues
    for (const FPerformanceMetric& Metric : DeploymentInfo.PerformanceMetrics)
    {
        if (!Metric.bIsWithinTarget)
        {
            FDeploymentIssue Issue;
            Issue.Severity = EIssueSeverity::Warning;
            Issue.Category = TEXT("Performance");
            Issue.Description = FString::Printf(TEXT("%s is outside target range: %.2f %s"), 
                *Metric.MetricName, Metric.Value, *Metric.Unit);
            Issue.Recommendation = TEXT("Monitor performance and optimize if necessary");
            DeploymentInfo.Issues.Add(Issue);
        }
    }
    
    // Module loading issues
    FModuleManager& ModuleManager = FModuleManager::Get();
    
    TArray<FString> ExpectedModules = {
        TEXT("TranspersonalGame"),
        TEXT("TranspersonalGameCore")
    };
    
    for (const FString& ModuleName : ExpectedModules)
    {
        if (!ModuleManager.IsModuleLoaded(*ModuleName))
        {
            FDeploymentIssue Issue;
            Issue.Severity = EIssueSeverity::Critical;
            Issue.Category = TEXT("Module");
            Issue.Description = FString::Printf(TEXT("Required module '%s' is not loaded"), *ModuleName);
            Issue.Recommendation = TEXT("Check module dependencies and build configuration");
            DeploymentInfo.Issues.Add(Issue);
        }
    }
}

void UV42DeploymentReport::DetermineDeploymentStatus()
{
    int32 CriticalIssues = 0;
    int32 WarningIssues = 0;
    int32 InfoIssues = 0;
    
    for (const FDeploymentIssue& Issue : DeploymentInfo.Issues)
    {
        switch (Issue.Severity)
        {
        case EIssueSeverity::Critical:
            CriticalIssues++;
            break;
        case EIssueSeverity::Warning:
            WarningIssues++;
            break;
        case EIssueSeverity::Info:
            InfoIssues++;
            break;
        }
    }
    
    if (CriticalIssues > 0)
    {
        DeploymentInfo.Status = EDeploymentStatus::Failed;
        DeploymentInfo.StatusMessage = FString::Printf(TEXT("Deployment failed with %d critical issues"), CriticalIssues);
    }
    else if (WarningIssues > 3)
    {
        DeploymentInfo.Status = EDeploymentStatus::Warning;
        DeploymentInfo.StatusMessage = FString::Printf(TEXT("Deployment successful with %d warnings"), WarningIssues);
    }
    else
    {
        DeploymentInfo.Status = EDeploymentStatus::Success;
        DeploymentInfo.StatusMessage = TEXT("Deployment successful - all systems operational");
    }
}

void UV42DeploymentReport::GenerateReportOutput()
{
    FString ReportText;
    
    // Header
    ReportText += TEXT("=== TRANSPERSONAL GAME STUDIO v4.2 DEPLOYMENT REPORT ===\n");
    ReportText += FString::Printf(TEXT("Generated: %s\n"), *DeploymentInfo.BuildDate.ToString());
    ReportText += FString::Printf(TEXT("Version: %s\n"), *DeploymentInfo.Version);
    ReportText += FString::Printf(TEXT("Status: %s\n"), *GetDeploymentStatusString(DeploymentInfo.Status));
    ReportText += FString::Printf(TEXT("Message: %s\n"), *DeploymentInfo.StatusMessage);
    ReportText += TEXT("\n");
    
    // System Reports
    ReportText += TEXT("=== SYSTEM STATUS ===\n");
    for (const FSystemReport& Report : DeploymentInfo.SystemReports)
    {
        ReportText += FString::Printf(TEXT("%s: %s\n"), 
            *Report.SystemName, 
            Report.bIsOnline ? TEXT("ONLINE") : TEXT("OFFLINE"));
        
        for (const FString& Info : Report.AdditionalInfo)
        {
            ReportText += FString::Printf(TEXT("  - %s\n"), *Info);
        }
    }
    ReportText += TEXT("\n");
    
    // Performance Metrics
    ReportText += TEXT("=== PERFORMANCE METRICS ===\n");
    for (const FPerformanceMetric& Metric : DeploymentInfo.PerformanceMetrics)
    {
        FString Status = Metric.bIsWithinTarget ? TEXT("✓") : TEXT("⚠");
        ReportText += FString::Printf(TEXT("%s %s: %.2f %s\n"), 
            *Status, *Metric.MetricName, Metric.Value, *Metric.Unit);
    }
    ReportText += TEXT("\n");
    
    // Issues
    if (DeploymentInfo.Issues.Num() > 0)
    {
        ReportText += TEXT("=== ISSUES DETECTED ===\n");
        for (const FDeploymentIssue& Issue : DeploymentInfo.Issues)
        {
            FString SeverityIcon;
            switch (Issue.Severity)
            {
            case EIssueSeverity::Critical: SeverityIcon = TEXT("✗"); break;
            case EIssueSeverity::Warning: SeverityIcon = TEXT("⚠"); break;
            case EIssueSeverity::Info: SeverityIcon = TEXT("ℹ"); break;
            }
            
            ReportText += FString::Printf(TEXT("%s [%s] %s: %s\n"), 
                *SeverityIcon, *Issue.Category, *GetIssueSeverityString(Issue.Severity), *Issue.Description);
            ReportText += FString::Printf(TEXT("    Recommendation: %s\n"), *Issue.Recommendation);
        }
    }
    else
    {
        ReportText += TEXT("=== NO ISSUES DETECTED ===\n");
        ReportText += TEXT("All systems are operating within normal parameters.\n");
    }
    
    ReportText += TEXT("\n=== END REPORT ===\n");
    
    // Log to console
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ReportText);
    
    // Save to file if possible
    SaveReportToFile(ReportText);
}

void UV42DeploymentReport::SaveReportToFile(const FString& ReportContent)
{
    FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    FString FileName = FString::Printf(TEXT("TranspersonalGame_v42_DeploymentReport_%s.txt"), *Timestamp);
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("Reports") / FileName;
    
    // Ensure directory exists
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FilePath));
    
    // Write file
    if (FFileHelper::SaveStringToFile(ReportContent, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Deployment report saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save deployment report to: %s"), *FilePath);
    }
}

FString UV42DeploymentReport::GetDeploymentStatusString(EDeploymentStatus Status) const
{
    switch (Status)
    {
    case EDeploymentStatus::Success: return TEXT("SUCCESS");
    case EDeploymentStatus::Warning: return TEXT("WARNING");
    case EDeploymentStatus::Failed: return TEXT("FAILED");
    case EDeploymentStatus::Unknown: return TEXT("UNKNOWN");
    default: return TEXT("INVALID");
    }
}

FString UV42DeploymentReport::GetIssueSeverityString(EIssueSeverity Severity) const
{
    switch (Severity)
    {
    case EIssueSeverity::Critical: return TEXT("CRITICAL");
    case EIssueSeverity::Warning: return TEXT("WARNING");
    case EIssueSeverity::Info: return TEXT("INFO");
    default: return TEXT("UNKNOWN");
    }
}

FDeploymentInformation UV42DeploymentReport::GetDeploymentInfo() const
{
    return DeploymentInfo;
}

bool UV42DeploymentReport::IsDeploymentHealthy() const
{
    return DeploymentInfo.Status == EDeploymentStatus::Success || 
           DeploymentInfo.Status == EDeploymentStatus::Warning;
}

TArray<FDeploymentIssue> UV42DeploymentReport::GetCriticalIssues() const
{
    TArray<FDeploymentIssue> CriticalIssues;
    
    for (const FDeploymentIssue& Issue : DeploymentInfo.Issues)
    {
        if (Issue.Severity == EIssueSeverity::Critical)
        {
            CriticalIssues.Add(Issue);
        }
    }
    
    return CriticalIssues;
}