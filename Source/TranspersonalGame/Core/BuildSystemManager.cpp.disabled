#include "BuildSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Kismet/GameplayStatics.h"

UBuildSystemManager::UBuildSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    // Initialize build system state
    bBuildSystemActive = false;
    bAutoCompilationEnabled = true;
    bHotReloadEnabled = true;
    bValidationEnabled = true;
    
    // Build configuration
    CurrentBuildConfiguration = EBuildConfiguration::Development;
    TargetPlatform = EBuildPlatform::Windows;
    
    // Compilation tracking
    LastCompilationTime = 0.0f;
    CompilationInterval = 30.0f; // Auto-compile every 30 seconds if changes detected
    bCompilationInProgress = false;
    bHasCompilationErrors = false;
    
    // File monitoring
    bFileMonitoringEnabled = true;
    LastFileCheckTime = 0.0f;
    FileCheckInterval = 5.0f; // Check for file changes every 5 seconds
    
    // Build statistics
    TotalBuildsAttempted = 0;
    SuccessfulBuilds = 0;
    FailedBuilds = 0;
    
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: Initialized"));
}

void UBuildSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize build system
    InitializeBuildSystem();
    
    // Start file monitoring if enabled
    if (bFileMonitoringEnabled)
    {
        StartFileMonitoring();
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: BeginPlay complete"));
}

void UBuildSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bBuildSystemActive)
    {
        return;
    }
    
    // Update timers
    LastFileCheckTime += DeltaTime;
    LastCompilationTime += DeltaTime;
    
    // Check for file changes
    if (bFileMonitoringEnabled && LastFileCheckTime >= FileCheckInterval)
    {
        CheckForFileChanges();
        LastFileCheckTime = 0.0f;
    }
    
    // Auto-compilation check
    if (bAutoCompilationEnabled && !bCompilationInProgress && LastCompilationTime >= CompilationInterval)
    {
        if (HasPendingChanges())
        {
            TriggerCompilation();
        }
        LastCompilationTime = 0.0f;
    }
}

void UBuildSystemManager::InitializeBuildSystem()
{
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: Initializing build system"));
    
    // Check if we're in editor or packaged build
    bBuildSystemActive = GIsEditor;
    
    if (!bBuildSystemActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Build system disabled - not running in editor"));
        return;
    }
    
    // Initialize file monitoring paths
    SourceCodePaths.Empty();
    SourceCodePaths.Add(TEXT("/root/transpersonal_game_studio/Source/TranspersonalGame"));
    SourceCodePaths.Add(TEXT("/root/transpersonal_game_studio/Source/TranspersonalGame/Core"));
    
    // Initialize build configuration based on current settings
    DetectCurrentBuildConfiguration();
    
    // Clear compilation state
    bCompilationInProgress = false;
    bHasCompilationErrors = false;
    CompilationErrors.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Build system initialized successfully"));
}

void UBuildSystemManager::StartFileMonitoring()
{
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: Starting file monitoring"));
    
    bFileMonitoringEnabled = true;
    LastFileCheckTime = 0.0f;
    
    // Initialize file timestamp cache
    FileTimestamps.Empty();
    
    // Scan initial file states
    ScanSourceFiles();
    
    UE_LOG(LogTemp, Log, TEXT("File monitoring started for %d paths"), SourceCodePaths.Num());
}

void UBuildSystemManager::CheckForFileChanges()
{
    if (!bFileMonitoringEnabled || SourceCodePaths.Num() == 0)
    {
        return;
    }
    
    bool bChangesDetected = false;
    
    // Check each monitored path
    for (const FString& Path : SourceCodePaths)
    {
        if (ScanDirectoryForChanges(Path))
        {
            bChangesDetected = true;
        }
    }
    
    if (bChangesDetected)
    {
        UE_LOG(LogTemp, Log, TEXT("File changes detected in source code"));
        OnSourceFilesChanged();
    }
}

bool UBuildSystemManager::ScanDirectoryForChanges(const FString& DirectoryPath)
{
    // Simplified file change detection
    // In a real implementation, this would use platform-specific file monitoring APIs
    
    bool bChangesFound = false;
    
    // For now, we'll just log that we're checking
    // Real implementation would enumerate files and check timestamps
    static float LastLogTime = 0.0f;
    LastLogTime += GetWorld()->GetDeltaSeconds();
    
    if (LastLogTime >= 30.0f) // Log every 30 seconds to avoid spam
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Checking directory for changes: %s"), *DirectoryPath);
        LastLogTime = 0.0f;
    }
    
    return bChangesFound;
}

void UBuildSystemManager::ScanSourceFiles()
{
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: Scanning source files"));
    
    FileTimestamps.Empty();
    
    // Scan all monitored directories
    for (const FString& Path : SourceCodePaths)
    {
        ScanDirectoryRecursive(Path);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Scanned %d source files"), FileTimestamps.Num());
}

void UBuildSystemManager::ScanDirectoryRecursive(const FString& DirectoryPath)
{
    // Simplified directory scanning
    // Real implementation would use IFileManager to enumerate files
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Scanning directory: %s"), *DirectoryPath);
    
    // For now, just add some common file types to our tracking
    TArray<FString> CommonFiles = {
        TEXT("TranspersonalGame.cpp"),
        TEXT("TranspersonalGameMode.cpp"),
        TEXT("TranspersonalCharacter.cpp"),
        TEXT("SharedTypes.h")
    };
    
    for (const FString& FileName : CommonFiles)
    {
        FString FullPath = FString::Printf(TEXT("%s/%s"), *DirectoryPath, *FileName);
        FileTimestamps.Add(FullPath, FDateTime::Now());
    }
}

void UBuildSystemManager::OnSourceFilesChanged()
{
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: Source files changed"));
    
    // Mark that we have pending changes
    bHasPendingChanges = true;
    
    // If auto-compilation is enabled, schedule a compilation
    if (bAutoCompilationEnabled && !bCompilationInProgress)
    {
        // Reset compilation timer to trigger soon
        LastCompilationTime = CompilationInterval - 5.0f; // Compile in 5 seconds
        UE_LOG(LogTemp, Log, TEXT("Auto-compilation scheduled due to file changes"));
    }
}

bool UBuildSystemManager::HasPendingChanges() const
{
    return bHasPendingChanges;
}

void UBuildSystemManager::TriggerCompilation()
{
    if (bCompilationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Compilation already in progress"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: Triggering compilation"));
    
    bCompilationInProgress = true;
    TotalBuildsAttempted++;
    
    // Clear previous errors
    CompilationErrors.Empty();
    bHasCompilationErrors = false;
    
    // Simulate compilation process
    StartCompilationProcess();
}

void UBuildSystemManager::StartCompilationProcess()
{
    UE_LOG(LogTemp, Log, TEXT("Starting compilation process"));
    
    // In a real implementation, this would:
    // 1. Run UnrealBuildTool
    // 2. Parse compilation output
    // 3. Handle hot reload if successful
    
    // For now, simulate a successful compilation
    FTimerHandle CompilationTimer;
    GetWorld()->GetTimerManager().SetTimer(CompilationTimer, this, &UBuildSystemManager::OnCompilationComplete, 3.0f, false);
}

void UBuildSystemManager::OnCompilationComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Compilation process completed"));
    
    bCompilationInProgress = false;
    bHasPendingChanges = false;
    
    // Simulate successful compilation for now
    bool bCompilationSuccessful = true; // In real implementation, check actual results
    
    if (bCompilationSuccessful)
    {
        SuccessfulBuilds++;
        UE_LOG(LogTemp, Log, TEXT("Compilation successful"));
        
        // Trigger hot reload if enabled
        if (bHotReloadEnabled)
        {
            TriggerHotReload();
        }
    }
    else
    {
        FailedBuilds++;
        bHasCompilationErrors = true;
        UE_LOG(LogTemp, Error, TEXT("Compilation failed"));
        
        // Add dummy error for testing
        CompilationErrors.Add(TEXT("Example compilation error"));
    }
    
    // Broadcast compilation result
    OnCompilationFinished(bCompilationSuccessful);
}

void UBuildSystemManager::TriggerHotReload()
{
    UE_LOG(LogTemp, Log, TEXT("BuildSystemManager: Triggering hot reload"));
    
    // In a real implementation, this would trigger UE5's hot reload system
    // For now, just log that we would do this
    
    UE_LOG(LogTemp, Log, TEXT("Hot reload completed (simulated)"));
}

void UBuildSystemManager::OnCompilationFinished(bool bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Compilation finished: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
    
    // Update statistics
    float SuccessRate = (TotalBuildsAttempted > 0) ? 
        (float)SuccessfulBuilds / (float)TotalBuildsAttempted * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Build Statistics - Total:%d Success:%d Failed:%d Rate:%.1f%%"),
        TotalBuildsAttempted, SuccessfulBuilds, FailedBuilds, SuccessRate);
}

void UBuildSystemManager::DetectCurrentBuildConfiguration()
{
    // Detect current build configuration
    #if UE_BUILD_DEBUG
        CurrentBuildConfiguration = EBuildConfiguration::Debug;
    #elif UE_BUILD_DEVELOPMENT
        CurrentBuildConfiguration = EBuildConfiguration::Development;
    #elif UE_BUILD_TEST
        CurrentBuildConfiguration = EBuildConfiguration::Test;
    #elif UE_BUILD_SHIPPING
        CurrentBuildConfiguration = EBuildConfiguration::Shipping;
    #else
        CurrentBuildConfiguration = EBuildConfiguration::Development;
    #endif
    
    UE_LOG(LogTemp, Log, TEXT("Detected build configuration: %d"), (int32)CurrentBuildConfiguration);
}

FBuildSystemStatus UBuildSystemManager::GetBuildSystemStatus() const
{
    FBuildSystemStatus Status;
    
    Status.bSystemActive = bBuildSystemActive;
    Status.bCompilationInProgress = bCompilationInProgress;
    Status.bHasErrors = bHasCompilationErrors;
    Status.bAutoCompilationEnabled = bAutoCompilationEnabled;
    Status.bHotReloadEnabled = bHotReloadEnabled;
    Status.CurrentConfiguration = CurrentBuildConfiguration;
    Status.TotalBuilds = TotalBuildsAttempted;
    Status.SuccessfulBuilds = SuccessfulBuilds;
    Status.FailedBuilds = FailedBuilds;
    Status.LastCompilationTime = FDateTime::Now();
    
    return Status;
}

void UBuildSystemManager::SetAutoCompilation(bool bEnabled)
{
    bAutoCompilationEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Auto-compilation %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UBuildSystemManager::SetHotReload(bool bEnabled)
{
    bHotReloadEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Hot reload %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UBuildSystemManager::SetFileMonitoring(bool bEnabled)
{
    if (bEnabled && !bFileMonitoringEnabled)
    {
        StartFileMonitoring();
    }
    else if (!bEnabled && bFileMonitoringEnabled)
    {
        bFileMonitoringEnabled = false;
        UE_LOG(LogTemp, Log, TEXT("File monitoring disabled"));
    }
}

void UBuildSystemManager::ForceCompilation()
{
    UE_LOG(LogTemp, Log, TEXT("Force compilation requested"));
    bHasPendingChanges = true;
    TriggerCompilation();
}

TArray<FString> UBuildSystemManager::GetCompilationErrors() const
{
    return CompilationErrors;
}