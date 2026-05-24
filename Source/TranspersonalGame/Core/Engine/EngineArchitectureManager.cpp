#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogEngineArchitecture);

UEngineArchitectureManager::UEngineArchitectureManager()
    : bPerformanceMonitoringEnabled(true)
    , LastValidationTime(0.0)
    , ValidationInterval(30.0f) // Validate every 30 seconds
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("EngineArchitectureManager constructed"));
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitecture, Warning, TEXT("EngineArchitectureManager initializing..."));
    
    // Initialize performance monitoring
    bPerformanceMonitoringEnabled = true;
    LastValidationTime = FPlatformTime::Seconds();
    
    // Set up periodic validation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            FTimerDelegate::CreateUObject(this, &UEngineArchitectureManager::ValidateAllSystems),
            ValidationInterval,
            true // Loop
        );
    }
    
    // Perform initial validation
    ValidateAllSystems();
    
    UE_LOG(LogEngineArchitecture, Warning, TEXT("EngineArchitectureManager initialized successfully"));
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("EngineArchitectureManager deinitializing..."));
    
    // Clear validation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    // Clear registered systems
    RegisteredSystems.Empty();
    SystemValidations.Empty();
    
    Super::Deinitialize();
}

bool UEngineArchitectureManager::ValidateAllSystems()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Starting comprehensive system validation..."));
    
    double ValidationStartTime = FPlatformTime::Seconds();
    SystemValidations.Empty();
    
    bool bAllSystemsValid = true;
    
    // Validate core engine systems
    if (!ValidateCoreEngineSystems())
    {
        bAllSystemsValid = false;
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Core engine systems validation failed"));
    }
    
    // Validate game-specific systems
    if (!ValidateGameSystems())
    {
        bAllSystemsValid = false;
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Game systems validation failed"));
    }
    
    // Validate all registered systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.IsValid())
        {
            FEng_SystemValidation Validation = ValidateSystem(SystemPair.Key, SystemPair.Value.Get());
            SystemValidations.Add(Validation);
            
            if (!Validation.bIsValid)
            {
                bAllSystemsValid = false;
            }
        }
        else
        {
            // System object is no longer valid - remove it
            UE_LOG(LogEngineArchitecture, Warning, TEXT("System '%s' object is no longer valid"), *SystemPair.Key);
            FEng_SystemValidation Validation;
            Validation.SystemName = SystemPair.Key;
            Validation.bIsValid = false;
            Validation.ValidationMessage = TEXT("System object is no longer valid");
            Validation.ValidationTime = FPlatformTime::Seconds() - ValidationStartTime;
            SystemValidations.Add(Validation);
            bAllSystemsValid = false;
        }
    }
    
    // Update performance metrics if monitoring is enabled
    if (bPerformanceMonitoringEnabled)
    {
        UpdatePerformanceMetrics();
    }
    
    LastValidationTime = FPlatformTime::Seconds();
    
    float TotalValidationTime = LastValidationTime - ValidationStartTime;
    UE_LOG(LogEngineArchitecture, Log, TEXT("System validation completed in %.3f seconds. All systems valid: %s"), 
           TotalValidationTime, bAllSystemsValid ? TEXT("YES") : TEXT("NO"));
    
    return bAllSystemsValid;
}

FEng_PerformanceMetrics UEngineArchitectureManager::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

bool UEngineArchitectureManager::IsSystemValid(const FString& SystemName) const
{
    for (const FEng_SystemValidation& Validation : SystemValidations)
    {
        if (Validation.SystemName == SystemName)
        {
            return Validation.bIsValid;
        }
    }
    return false;
}

TArray<FEng_SystemValidation> UEngineArchitectureManager::GetSystemValidations() const
{
    return SystemValidations;
}

bool UEngineArchitectureManager::RevalidateSystem(const FString& SystemName)
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Revalidating system: %s"), *SystemName);
    
    if (TWeakObjectPtr<UObject>* SystemPtr = RegisteredSystems.Find(SystemName))
    {
        if (SystemPtr->IsValid())
        {
            FEng_SystemValidation Validation = ValidateSystem(SystemName, SystemPtr->Get());
            
            // Update or add validation result
            bool bFound = false;
            for (FEng_SystemValidation& ExistingValidation : SystemValidations)
            {
                if (ExistingValidation.SystemName == SystemName)
                {
                    ExistingValidation = Validation;
                    bFound = true;
                    break;
                }
            }
            
            if (!bFound)
            {
                SystemValidations.Add(Validation);
            }
            
            return Validation.bIsValid;
        }
    }
    
    UE_LOG(LogEngineArchitecture, Warning, TEXT("Cannot revalidate system '%s' - not found or invalid"), *SystemName);
    return false;
}

void UEngineArchitectureManager::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (!SystemObject)
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Cannot register null system object for '%s'"), *SystemName);
        return;
    }
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Registering system: %s"), *SystemName);
    RegisteredSystems.Add(SystemName, SystemObject);
    
    // Immediately validate the new system
    FEng_SystemValidation Validation = ValidateSystem(SystemName, SystemObject);
    SystemValidations.Add(Validation);
}

void UEngineArchitectureManager::UnregisterSystem(const FString& SystemName)
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Unregistering system: %s"), *SystemName);
    
    RegisteredSystems.Remove(SystemName);
    
    // Remove validation result
    SystemValidations.RemoveAll([&SystemName](const FEng_SystemValidation& Validation)
    {
        return Validation.SystemName == SystemName;
    });
}

float UEngineArchitectureManager::GetArchitecturalComplianceScore() const
{
    if (SystemValidations.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 ValidSystems = 0;
    for (const FEng_SystemValidation& Validation : SystemValidations)
    {
        if (Validation.bIsValid)
        {
            ValidSystems++;
        }
    }
    
    return (float(ValidSystems) / float(SystemValidations.Num())) * 100.0f;
}

void UEngineArchitectureManager::SetPerformanceMonitoringEnabled(bool bEnabled)
{
    bPerformanceMonitoringEnabled = bEnabled;
    UE_LOG(LogEngineArchitecture, Log, TEXT("Performance monitoring %s"), 
           bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

bool UEngineArchitectureManager::ValidateCoreEngineSystems()
{
    bool bValid = true;
    
    // Validate Engine instance
    if (!GEngine)
    {
        UE_LOG(LogEngineArchitecture, Error, TEXT("GEngine is null"));
        bValid = false;
    }
    
    // Validate World
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogEngineArchitecture, Error, TEXT("World is null"));
        bValid = false;
    }
    
    // Validate GameInstance
    if (World && !World->GetGameInstance())
    {
        UE_LOG(LogEngineArchitecture, Error, TEXT("GameInstance is null"));
        bValid = false;
    }
    
    return bValid;
}

bool UEngineArchitectureManager::ValidateGameSystems()
{
    bool bValid = true;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Validate GameMode
    if (!World->GetAuthGameMode())
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("No GameMode found"));
        // This might be valid in some contexts (like editor), so don't fail
    }
    
    // Validate GameState
    if (!World->GetGameState())
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("No GameState found"));
        // This might be valid in some contexts, so don't fail
    }
    
    return bValid;
}

void UEngineArchitectureManager::UpdatePerformanceMetrics()
{
    // Get frame time
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get basic stats
    CurrentMetrics.GameThreadTime = 0.0f; // Would need more complex stat gathering
    CurrentMetrics.RenderThreadTime = 0.0f; // Would need more complex stat gathering
    CurrentMetrics.DrawCalls = 0; // Would need render stats
    CurrentMetrics.TriangleCount = 0; // Would need render stats
    
    // Get memory usage (basic approximation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = float(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

FEng_SystemValidation UEngineArchitectureManager::ValidateSystem(const FString& SystemName, UObject* SystemObject)
{
    double ValidationStartTime = FPlatformTime::Seconds();
    
    FEng_SystemValidation Validation;
    Validation.SystemName = SystemName;
    Validation.bIsValid = true;
    Validation.ValidationMessage = TEXT("System validation passed");
    
    if (!SystemObject)
    {
        Validation.bIsValid = false;
        Validation.ValidationMessage = TEXT("System object is null");
    }
    else if (!IsValid(SystemObject))
    {
        Validation.bIsValid = false;
        Validation.ValidationMessage = TEXT("System object is not valid (pending kill or garbage)");
    }
    else
    {
        // Basic validation - object exists and is valid
        UE_LOG(LogEngineArchitecture, VeryVerbose, TEXT("System '%s' validation passed"), *SystemName);
    }
    
    Validation.ValidationTime = FPlatformTime::Seconds() - ValidationStartTime;
    return Validation;
}