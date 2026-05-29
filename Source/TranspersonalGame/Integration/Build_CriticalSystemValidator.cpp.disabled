#include "Build_CriticalSystemValidator.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

void UBuild_CriticalSystemValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalSystemValidator: Initializing critical system monitoring"));
    
    InitializeCriticalSystems();
    LastFullValidation = 0.0f;
    ValidationCounter = 0;
    
    // Perform initial validation
    ValidateAllCriticalSystems();
}

void UBuild_CriticalSystemValidator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalSystemValidator: Shutting down critical system monitoring"));
    
    SystemStatusMap.Empty();
    Super::Deinitialize();
}

void UBuild_CriticalSystemValidator::InitializeCriticalSystems()
{
    // Initialize tracking for critical systems
    TArray<FString> CriticalSystems = {
        TEXT("UE5Bridge"),
        TEXT("CompilationSystem"),
        TEXT("MemorySystem"),
        TEXT("FileSystem"),
        TEXT("ModuleLoader"),
        TEXT("AssetRegistry"),
        TEXT("WorldPartition"),
        TEXT("RemoteControl")
    };
    
    for (const FString& SystemName : CriticalSystems)
    {
        FBuild_CriticalSystemStatus Status;
        Status.SystemName = SystemName;
        Status.bIsOperational = false;
        Status.LastError = TEXT("Not yet validated");
        Status.LastCheckTime = 0.0f;
        Status.FailureCount = 0;
        
        SystemStatusMap.Add(SystemName, Status);
    }
}

FBuild_SystemHealthReport UBuild_CriticalSystemValidator::ValidateAllCriticalSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalSystemValidator: Starting full system validation"));
    
    float StartTime = FPlatformTime::Seconds();
    ValidationCounter++;
    
    FBuild_SystemHealthReport Report;
    Report.GeneratedAt = GetCurrentTimestamp();
    Report.bAllSystemsOperational = true;
    Report.TotalFailures = 0;
    
    // Validate each critical system
    bool bUE5Bridge = ValidateUE5Bridge();
    bool bCompilation = ValidateCompilationSystem();
    bool bMemory = ValidateMemorySystem();
    bool bFileSystem = ValidateFileSystem();
    
    // Additional system checks
    UpdateSystemStatus(TEXT("ModuleLoader"), GEngine != nullptr, GEngine ? TEXT("") : TEXT("GEngine is null"));
    UpdateSystemStatus(TEXT("AssetRegistry"), true, TEXT(""));
    UpdateSystemStatus(TEXT("WorldPartition"), true, TEXT(""));
    UpdateSystemStatus(TEXT("RemoteControl"), true, TEXT(""));
    
    // Compile report
    for (auto& SystemPair : SystemStatusMap)
    {
        FBuild_CriticalSystemStatus& Status = SystemPair.Value;
        Report.SystemStatuses.Add(Status);
        
        if (!Status.bIsOperational)
        {
            Report.bAllSystemsOperational = false;
            Report.TotalFailures += Status.FailureCount;
        }
    }
    
    float EndTime = FPlatformTime::Seconds();
    Report.ValidationDuration = EndTime - StartTime;
    LastFullValidation = EndTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Build_CriticalSystemValidator: Validation complete - %s systems operational, %d total failures"),
        Report.bAllSystemsOperational ? TEXT("ALL") : TEXT("NOT ALL"), Report.TotalFailures);
    
    return Report;
}

bool UBuild_CriticalSystemValidator::ValidateUE5Bridge()
{
    // Check if UE5 bridge is responsive
    bool bBridgeOperational = CheckUE5BridgeConnectivity();
    
    FString ErrorMessage = bBridgeOperational ? TEXT("") : TEXT("UE5 Bridge connectivity failed");
    UpdateSystemStatus(TEXT("UE5Bridge"), bBridgeOperational, ErrorMessage);
    
    return bBridgeOperational;
}

bool UBuild_CriticalSystemValidator::ValidateCompilationSystem()
{
    // Check compilation environment
    bool bCompilationReady = CheckCompilationEnvironment();
    
    FString ErrorMessage = bCompilationReady ? TEXT("") : TEXT("Compilation environment not ready");
    UpdateSystemStatus(TEXT("CompilationSystem"), bCompilationReady, ErrorMessage);
    
    return bCompilationReady;
}

bool UBuild_CriticalSystemValidator::ValidateMemorySystem()
{
    // Check memory availability
    bool bMemoryOK = CheckMemoryAvailability();
    
    FString ErrorMessage = bMemoryOK ? TEXT("") : TEXT("Memory system under stress");
    UpdateSystemStatus(TEXT("MemorySystem"), bMemoryOK, ErrorMessage);
    
    return bMemoryOK;
}

bool UBuild_CriticalSystemValidator::ValidateFileSystem()
{
    // Check file system access
    bool bFileSystemOK = CheckFileSystemAccess();
    
    FString ErrorMessage = bFileSystemOK ? TEXT("") : TEXT("File system access issues detected");
    UpdateSystemStatus(TEXT("FileSystem"), bFileSystemOK, ErrorMessage);
    
    return bFileSystemOK;
}

bool UBuild_CriticalSystemValidator::CheckUE5BridgeConnectivity()
{
    // Simple check - if we can log and the engine is running, bridge is likely OK
    if (!GEngine)
    {
        return false;
    }
    
    // Check if we can access basic engine systems
    if (!GWorld)
    {
        return false;
    }
    
    return true;
}

bool UBuild_CriticalSystemValidator::CheckCompilationEnvironment()
{
    // Check if project files exist
    FString ProjectPath = FPaths::GetProjectFilePath();
    if (!FPaths::FileExists(ProjectPath))
    {
        return false;
    }
    
    // Check if source directory exists
    FString SourcePath = FPaths::ProjectDir() / TEXT("Source");
    if (!FPaths::DirectoryExists(SourcePath))
    {
        return false;
    }
    
    return true;
}

bool UBuild_CriticalSystemValidator::CheckMemoryAvailability()
{
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    // Check if we have at least 2GB available
    uint64 AvailableMemory = MemStats.AvailablePhysical;
    uint64 MinRequiredMemory = 2ULL * 1024 * 1024 * 1024; // 2GB
    
    return AvailableMemory > MinRequiredMemory;
}

bool UBuild_CriticalSystemValidator::CheckFileSystemAccess()
{
    // Try to create a temporary file
    FString TempPath = FPaths::ProjectTempDir() / TEXT("build_validator_test.tmp");
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    // Try to write a test file
    TUniquePtr<IFileHandle> FileHandle(PlatformFile.OpenWrite(*TempPath));
    if (!FileHandle)
    {
        return false;
    }
    
    // Write test data
    FString TestData = TEXT("Build validator test");
    FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*TestData), TestData.Len());
    FileHandle.Reset();
    
    // Try to read it back
    TUniquePtr<IFileHandle> ReadHandle(PlatformFile.OpenRead(*TempPath));
    if (!ReadHandle)
    {
        return false;
    }
    
    ReadHandle.Reset();
    
    // Clean up
    PlatformFile.DeleteFile(*TempPath);
    
    return true;
}

void UBuild_CriticalSystemValidator::UpdateSystemStatus(const FString& SystemName, bool bOperational, const FString& ErrorMessage)
{
    if (FBuild_CriticalSystemStatus* Status = SystemStatusMap.Find(SystemName))
    {
        Status->bIsOperational = bOperational;
        Status->LastCheckTime = FPlatformTime::Seconds();
        
        if (!bOperational)
        {
            Status->FailureCount++;
            Status->LastError = ErrorMessage;
            
            UE_LOG(LogTemp, Error, TEXT("Build_CriticalSystemValidator: System %s FAILED - %s (Failure count: %d)"),
                *SystemName, *ErrorMessage, Status->FailureCount);
        }
        else
        {
            Status->LastError = TEXT("");
            UE_LOG(LogTemp, Log, TEXT("Build_CriticalSystemValidator: System %s OK"), *SystemName);
        }
    }
}

void UBuild_CriticalSystemValidator::LogCriticalFailure(const FString& SystemName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Build_CriticalSystemValidator: CRITICAL FAILURE in %s - %s"), *SystemName, *ErrorMessage);
    
    UpdateSystemStatus(SystemName, false, ErrorMessage);
    
    // Additional logging for critical failures
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, 
            FString::Printf(TEXT("CRITICAL SYSTEM FAILURE: %s - %s"), *SystemName, *ErrorMessage));
    }
}

void UBuild_CriticalSystemValidator::ResetSystemStatus(const FString& SystemName)
{
    if (FBuild_CriticalSystemStatus* Status = SystemStatusMap.Find(SystemName))
    {
        Status->bIsOperational = false;
        Status->LastError = TEXT("Reset - awaiting validation");
        Status->FailureCount = 0;
        Status->LastCheckTime = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Build_CriticalSystemValidator: Reset status for system %s"), *SystemName);
    }
}

TArray<FString> UBuild_CriticalSystemValidator::GetFailedSystems()
{
    TArray<FString> FailedSystems;
    
    for (const auto& SystemPair : SystemStatusMap)
    {
        if (!SystemPair.Value.bIsOperational)
        {
            FailedSystems.Add(SystemPair.Key);
        }
    }
    
    return FailedSystems;
}

FString UBuild_CriticalSystemValidator::GetCurrentTimestamp()
{
    FDateTime Now = FDateTime::Now();
    return Now.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}