#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    ValidationInterval = 30.0f;
    bAutoValidateOnStartup = true;
    InitializeCoreSystemsList();
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing..."));
    
    InitializeCoreSystemsList();
    
    if (bAutoValidateOnStartup)
    {
        // Delay initial validation to allow other systems to initialize
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ValidationTimerHandle,
                this,
                &UBuildIntegrationManager::PeriodicValidation,
                5.0f,
                false
            );
        }
    }
}

void UBuildIntegrationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::InitializeCoreSystemsList()
{
    CoreSystemNames.Empty();
    CoreSystemNames.Add(TEXT("TranspersonalGameState"));
    CoreSystemNames.Add(TEXT("TranspersonalCharacter"));
    CoreSystemNames.Add(TEXT("PCGWorldGenerator"));
    CoreSystemNames.Add(TEXT("FoliageManager"));
    CoreSystemNames.Add(TEXT("CrowdSimulationManager"));
    CoreSystemNames.Add(TEXT("ProceduralWorldManager"));
    CoreSystemNames.Add(TEXT("QA_ValidationManager"));
}

FBuild_IntegrationReport UBuildIntegrationManager::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Running full system validation..."));
    
    FBuild_IntegrationReport Report;
    Report.TotalSystems = CoreSystemNames.Num();
    Report.LoadedSystems = 0;
    Report.InitializedSystems = 0;
    Report.LastValidationTime = FDateTime::Now();
    
    for (const FString& SystemName : CoreSystemNames)
    {
        FBuild_SystemStatus Status;
        Status.SystemName = SystemName;
        
        // Test loading
        Status.bIsLoaded = TestSystemLoading(SystemName);
        if (Status.bIsLoaded)
        {
            Report.LoadedSystems++;
        }
        
        // Test initialization
        Status.bIsInitialized = TestSystemInitialization(SystemName);
        if (Status.bIsInitialized)
        {
            Report.InitializedSystems++;
        }
        
        // Calculate health percentage
        Status.HealthPercentage = (Status.bIsLoaded ? 50.0f : 0.0f) + (Status.bIsInitialized ? 50.0f : 0.0f);
        
        Report.SystemStatuses.Add(Status);
        SystemStatusMap.Add(SystemName, Status);
    }
    
    // Calculate overall health
    Report.OverallHealth = Report.TotalSystems > 0 ? 
        ((float)(Report.LoadedSystems + Report.InitializedSystems) / (float)(Report.TotalSystems * 2)) * 100.0f : 0.0f;
    
    LastReport = Report;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete. Overall health: %.1f%%"), Report.OverallHealth);
    
    return Report;
}

bool UBuildIntegrationManager::ValidateSystem(const FString& SystemName)
{
    bool bLoaded = TestSystemLoading(SystemName);
    bool bInitialized = TestSystemInitialization(SystemName);
    
    UpdateSystemStatus(SystemName, bLoaded, bInitialized);
    
    return bLoaded && bInitialized;
}

FBuild_SystemStatus UBuildIntegrationManager::GetSystemStatus(const FString& SystemName)
{
    if (SystemStatusMap.Contains(SystemName))
    {
        return SystemStatusMap[SystemName];
    }
    
    // Return default status if not found
    FBuild_SystemStatus DefaultStatus;
    DefaultStatus.SystemName = SystemName;
    DefaultStatus.LastError = TEXT("System not found in status map");
    return DefaultStatus;
}

TArray<FString> UBuildIntegrationManager::GetFailedSystems()
{
    TArray<FString> FailedSystems;
    
    for (const auto& StatusPair : SystemStatusMap)
    {
        const FBuild_SystemStatus& Status = StatusPair.Value;
        if (!Status.bIsLoaded || !Status.bIsInitialized)
        {
            FailedSystems.Add(Status.SystemName);
        }
    }
    
    return FailedSystems;
}

float UBuildIntegrationManager::GetOverallSystemHealth()
{
    return LastReport.OverallHealth;
}

void UBuildIntegrationManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Running integration tests..."));
    
    FBuild_IntegrationReport Report = ValidateAllSystems();
    
    // Log detailed results
    for (const FBuild_SystemStatus& Status : Report.SystemStatuses)
    {
        FString StatusText = Status.bIsLoaded && Status.bIsInitialized ? TEXT("PASS") : TEXT("FAIL");
        UE_LOG(LogTemp, Warning, TEXT("Integration Test - %s: %s (%.1f%%)"), 
            *Status.SystemName, *StatusText, Status.HealthPercentage);
    }
}

void UBuildIntegrationManager::GenerateIntegrationReport()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Generating integration report..."));
    
    FBuild_IntegrationReport Report = ValidateAllSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("=== BUILD INTEGRATION REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), Report.TotalSystems);
    UE_LOG(LogTemp, Warning, TEXT("Loaded Systems: %d"), Report.LoadedSystems);
    UE_LOG(LogTemp, Warning, TEXT("Initialized Systems: %d"), Report.InitializedSystems);
    UE_LOG(LogTemp, Warning, TEXT("Overall Health: %.1f%%"), Report.OverallHealth);
    UE_LOG(LogTemp, Warning, TEXT("Validation Time: %s"), *Report.LastValidationTime.ToString());
    UE_LOG(LogTemp, Warning, TEXT("================================"));
}

bool UBuildIntegrationManager::TestSystemLoading(const FString& SystemName)
{
    FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
    
    try
    {
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        return SystemClass != nullptr;
    }
    catch (...)
    {
        return false;
    }
}

bool UBuildIntegrationManager::TestSystemInitialization(const FString& SystemName)
{
    // For now, assume initialization is successful if loading succeeded
    // In a more complex system, we would test actual initialization
    return TestSystemLoading(SystemName);
}

void UBuildIntegrationManager::UpdateSystemStatus(const FString& SystemName, bool bLoaded, bool bInitialized, const FString& Error)
{
    FBuild_SystemStatus Status;
    Status.SystemName = SystemName;
    Status.bIsLoaded = bLoaded;
    Status.bIsInitialized = bInitialized;
    Status.HealthPercentage = (bLoaded ? 50.0f : 0.0f) + (bInitialized ? 50.0f : 0.0f);
    Status.LastError = Error;
    
    SystemStatusMap.Add(SystemName, Status);
}

void UBuildIntegrationManager::CalculateOverallHealth()
{
    if (SystemStatusMap.Num() == 0)
    {
        LastReport.OverallHealth = 0.0f;
        return;
    }
    
    float TotalHealth = 0.0f;
    for (const auto& StatusPair : SystemStatusMap)
    {
        TotalHealth += StatusPair.Value.HealthPercentage;
    }
    
    LastReport.OverallHealth = TotalHealth / (float)SystemStatusMap.Num();
}

void UBuildIntegrationManager::PeriodicValidation()
{
    ValidateAllSystems();
    
    // Schedule next validation
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PeriodicValidation,
            ValidationInterval,
            false
        );
    }
}