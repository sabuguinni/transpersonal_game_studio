#include "Build_SystemIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UBuild_SystemIntegrator::UBuild_SystemIntegrator()
{
    bIntegrationValid = false;
    TotalInitializationTime = 0.0f;
    LastIntegrationReport = TEXT("No integration performed yet");
}

void UBuild_SystemIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Build_SystemIntegrator: Initializing integration subsystem"));
    
    // Register core systems automatically
    RegisterSystem(TEXT("CoreSystems"), TEXT("1.0.0"), TArray<FString>());
    RegisterSystem(TEXT("WorldGeneration"), TEXT("1.0.0"), {TEXT("CoreSystems")});
    RegisterSystem(TEXT("CharacterSystem"), TEXT("1.0.0"), {TEXT("CoreSystems")});
    RegisterSystem(TEXT("QAFramework"), TEXT("1.0.0"), {TEXT("CoreSystems")});
    RegisterSystem(TEXT("Integration"), TEXT("1.0.0"), {TEXT("CoreSystems"), TEXT("QAFramework")});
    
    // Initialize all registered systems
    InitializeAllSystems();
}

void UBuild_SystemIntegrator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Build_SystemIntegrator: Shutting down integration subsystem"));
    ShutdownAllSystems();
    Super::Deinitialize();
}

bool UBuild_SystemIntegrator::RegisterSystem(const FString& SystemName, const FString& Version, const TArray<FString>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s is already registered"), *SystemName);
        return false;
    }
    
    FBuild_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Status = EBuild_SystemStatus::Uninitialized;
    SystemInfo.Version = Version;
    SystemInfo.Dependencies = Dependencies;
    SystemInfo.InitializationTime = 0.0f;
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    SystemMessages.Add(SystemName, TArray<FString>());
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s v%s"), *SystemName, *Version);
    return true;
}

bool UBuild_SystemIntegrator::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    SystemMessages.Remove(SystemName);
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
    return true;
}

EBuild_SystemStatus UBuild_SystemIntegrator::GetSystemStatus(const FString& SystemName) const
{
    if (const FBuild_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->Status;
    }
    return EBuild_SystemStatus::Uninitialized;
}

TArray<FBuild_SystemInfo> UBuild_SystemIntegrator::GetAllSystemInfo() const
{
    TArray<FBuild_SystemInfo> SystemInfoArray;
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemInfoArray.Add(SystemPair.Value);
    }
    return SystemInfoArray;
}

bool UBuild_SystemIntegrator::InitializeAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing all registered systems..."));
    
    float StartTime = FPlatformTime::Seconds();
    bool bAllInitialized = true;
    
    // Initialize systems in dependency order
    TArray<FString> InitializedSystems;
    TArray<FString> PendingSystems;
    
    // Get all system names
    for (const auto& SystemPair : RegisteredSystems)
    {
        PendingSystems.Add(SystemPair.Key);
    }
    
    // Initialize systems with resolved dependencies first
    while (PendingSystems.Num() > 0)
    {
        bool bProgressMade = false;
        
        for (int32 i = PendingSystems.Num() - 1; i >= 0; i--)
        {
            const FString& SystemName = PendingSystems[i];
            
            if (CheckSystemDependencies(SystemName))
            {
                if (InitializeSystem(SystemName))
                {
                    InitializedSystems.Add(SystemName);
                    PendingSystems.RemoveAt(i);
                    bProgressMade = true;
                }
                else
                {
                    bAllInitialized = false;
                }
            }
        }
        
        // Prevent infinite loop if dependencies can't be resolved
        if (!bProgressMade)
        {
            UE_LOG(LogTemp, Error, TEXT("Cannot resolve dependencies for remaining systems"));
            for (const FString& PendingSystem : PendingSystems)
            {
                UpdateSystemStatus(PendingSystem, EBuild_SystemStatus::Error);
            }
            bAllInitialized = false;
            break;
        }
    }
    
    float EndTime = FPlatformTime::Seconds();
    TotalInitializationTime = EndTime - StartTime;
    
    UE_LOG(LogTemp, Log, TEXT("System initialization complete: %d/%d systems (%.2fs)"), 
        InitializedSystems.Num(), RegisteredSystems.Num(), TotalInitializationTime);
    
    return bAllInitialized;
}

void UBuild_SystemIntegrator::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Shutting down all systems..."));
    
    for (auto& SystemPair : RegisteredSystems)
    {
        UpdateSystemStatus(SystemPair.Key, EBuild_SystemStatus::Uninitialized);
    }
}

bool UBuild_SystemIntegrator::SendSystemMessage(const FString& FromSystem, const FString& ToSystem, const FString& Message)
{
    if (!RegisteredSystems.Contains(FromSystem) || !RegisteredSystems.Contains(ToSystem))
    {
        return false;
    }
    
    FString FormattedMessage = FString::Printf(TEXT("[%s]: %s"), *FromSystem, *Message);
    
    if (TArray<FString>* Messages = SystemMessages.Find(ToSystem))
    {
        Messages->Add(FormattedMessage);
        UE_LOG(LogTemp, Log, TEXT("Message sent from %s to %s: %s"), *FromSystem, *ToSystem, *Message);
        return true;
    }
    
    return false;
}

TArray<FString> UBuild_SystemIntegrator::GetSystemMessages(const FString& SystemName)
{
    if (TArray<FString>* Messages = SystemMessages.Find(SystemName))
    {
        TArray<FString> Result = *Messages;
        Messages->Empty(); // Clear messages after reading
        return Result;
    }
    return TArray<FString>();
}

bool UBuild_SystemIntegrator::ValidateSystemIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Validating system integration..."));
    
    int32 ReadySystems = 0;
    int32 ErrorSystems = 0;
    TArray<FString> ValidationIssues;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        const FBuild_SystemInfo& SystemInfo = SystemPair.Value;
        
        switch (SystemInfo.Status)
        {
            case EBuild_SystemStatus::Ready:
                ReadySystems++;
                break;
            case EBuild_SystemStatus::Error:
                ErrorSystems++;
                ValidationIssues.Add(FString::Printf(TEXT("System %s is in error state"), *SystemName));
                break;
            case EBuild_SystemStatus::Uninitialized:
                ValidationIssues.Add(FString::Printf(TEXT("System %s is not initialized"), *SystemName));
                break;
            case EBuild_SystemStatus::Initializing:
                ValidationIssues.Add(FString::Printf(TEXT("System %s is still initializing"), *SystemName));
                break;
        }
    }
    
    bIntegrationValid = (ErrorSystems == 0) && (ReadySystems == RegisteredSystems.Num());
    
    // Generate integration report
    FString ReportBuilder = FString::Printf(TEXT("Integration Validation Report\n"));
    ReportBuilder += FString::Printf(TEXT("Timestamp: %s\n"), *FDateTime::Now().ToString());
    ReportBuilder += FString::Printf(TEXT("Total Systems: %d\n"), RegisteredSystems.Num());
    ReportBuilder += FString::Printf(TEXT("Ready Systems: %d\n"), ReadySystems);
    ReportBuilder += FString::Printf(TEXT("Error Systems: %d\n"), ErrorSystems);
    ReportBuilder += FString::Printf(TEXT("Integration Valid: %s\n"), bIntegrationValid ? TEXT("YES") : TEXT("NO"));
    ReportBuilder += FString::Printf(TEXT("Total Init Time: %.2fs\n"), TotalInitializationTime);
    
    if (ValidationIssues.Num() > 0)
    {
        ReportBuilder += TEXT("\nIssues Found:\n");
        for (const FString& Issue : ValidationIssues)
        {
            ReportBuilder += FString::Printf(TEXT("- %s\n"), *Issue);
        }
    }
    
    LastIntegrationReport = ReportBuilder;
    
    UE_LOG(LogTemp, Log, TEXT("Integration validation complete: %s"), bIntegrationValid ? TEXT("VALID") : TEXT("INVALID"));
    return bIntegrationValid;
}

FString UBuild_SystemIntegrator::GetIntegrationReport() const
{
    return LastIntegrationReport;
}

bool UBuild_SystemIntegrator::InitializeSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    UpdateSystemStatus(SystemName, EBuild_SystemStatus::Initializing);
    
    float StartTime = FPlatformTime::Seconds();
    
    // Simulate system initialization
    // In a real implementation, this would call specific system initialization functions
    bool bInitSuccess = true;
    
    // Basic validation that the system can be initialized
    if (SystemName == TEXT("CoreSystems"))
    {
        bInitSuccess = GEngine != nullptr;
    }
    else if (SystemName == TEXT("QAFramework"))
    {
        // Check if QA classes are available
        UClass* QAClass = LoadClass<UObject>(nullptr, TEXT("/Script/TranspersonalGame.QA_TestFramework"));
        bInitSuccess = QAClass != nullptr;
    }
    
    float EndTime = FPlatformTime::Seconds();
    float InitTime = EndTime - StartTime;
    
    // Update system info
    if (FBuild_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->InitializationTime = InitTime;
    }
    
    EBuild_SystemStatus NewStatus = bInitSuccess ? EBuild_SystemStatus::Ready : EBuild_SystemStatus::Error;
    UpdateSystemStatus(SystemName, NewStatus);
    
    UE_LOG(LogTemp, Log, TEXT("System %s initialization: %s (%.3fs)"), 
        *SystemName, bInitSuccess ? TEXT("SUCCESS") : TEXT("FAILED"), InitTime);
    
    return bInitSuccess;
}

bool UBuild_SystemIntegrator::CheckSystemDependencies(const FString& SystemName) const
{
    const FBuild_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    // Check if all dependencies are ready
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        EBuild_SystemStatus DepStatus = GetSystemStatus(Dependency);
        if (DepStatus != EBuild_SystemStatus::Ready)
        {
            return false;
        }
    }
    
    return true;
}

void UBuild_SystemIntegrator::UpdateSystemStatus(const FString& SystemName, EBuild_SystemStatus NewStatus)
{
    if (FBuild_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->Status = NewStatus;
    }
}