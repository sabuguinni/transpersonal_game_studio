#include "EngineArchitecturalFramework.h"
#include "BiomeManager.h"
#include "AssetProductionManager.h"
#include "BuildSystemManager.h"
#include "ArchitectureValidationSuite.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UEngineArchitecturalFramework::UEngineArchitecturalFramework()
{
    // Initialize default values
    MaxAllowedFrameTime = 16.67f; // 60 FPS target
    MaxActorsPerSystem = 1000;
    bEnablePerformanceMonitoring = true;
    bEnforceModuleCompliance = true;
    bValidateSystemDependencies = true;
    bGenerateComplianceReports = true;

    // Initialize references to null
    BiomeManagerRef = nullptr;
    AssetProductionManagerRef = nullptr;
    BuildSystemManagerRef = nullptr;
    ValidationSuiteRef = nullptr;
}

void UEngineArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Initializing core architectural systems"));

    // Initialize core system references
    InitializeCoreSystemReferences();

    // Register this framework as the primary architectural system
    RegisterCoreSystem(TEXT("EngineArchitecturalFramework"), this, 0);

    // Validate system integrity
    ValidateSystemIntegrity();

    // Log architectural status
    LogArchitecturalStatus();

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Initialization complete"));
}

void UEngineArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Deinitializing"));

    // Generate final architectural report
    if (bGenerateComplianceReports)
    {
        GenerateArchitecturalReport();
    }

    // Clear all system registrations
    RegisteredSystems.Empty();
    SystemPriorities.Empty();
    SystemPerformanceMetrics.Empty();
    SystemDependencies.Empty();

    // Clear references
    BiomeManagerRef = nullptr;
    AssetProductionManagerRef = nullptr;
    BuildSystemManagerRef = nullptr;
    ValidationSuiteRef = nullptr;

    Super::Deinitialize();
}

void UEngineArchitecturalFramework::RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance, int32 Priority)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: Cannot register null system: %s"), *SystemName);
        return;
    }

    // Register the system
    RegisteredSystems.Add(SystemName, SystemInstance);
    SystemPriorities.Add(SystemName, Priority);
    SystemPerformanceMetrics.Add(SystemName, 0.0f);

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Registered system '%s' with priority %d"), *SystemName, Priority);

    // Validate system dependencies if enabled
    if (bValidateSystemDependencies)
    {
        AreSystemDependenciesMet(SystemName);
    }
}

void UEngineArchitecturalFramework::UnregisterCoreSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems.Remove(SystemName);
        SystemPriorities.Remove(SystemName);
        SystemPerformanceMetrics.Remove(SystemName);
        SystemDependencies.Remove(SystemName);

        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Unregistered system '%s'"), *SystemName);
    }
}

bool UEngineArchitecturalFramework::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

void UEngineArchitecturalFramework::UpdateSystemPerformance(const FString& SystemName, float DeltaTime, int32 ActorCount)
{
    if (!bEnablePerformanceMonitoring)
    {
        return;
    }

    if (SystemPerformanceMetrics.Contains(SystemName))
    {
        // Update performance metrics
        float CurrentMetric = SystemPerformanceMetrics[SystemName];
        float NewMetric = (CurrentMetric * 0.9f) + (DeltaTime * 0.1f); // Smoothed average
        SystemPerformanceMetrics[SystemName] = NewMetric;

        // Check performance limits
        if (DeltaTime > MaxAllowedFrameTime || ActorCount > MaxActorsPerSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Performance warning for system '%s' - DeltaTime: %f, ActorCount: %d"), 
                   *SystemName, DeltaTime, ActorCount);
        }
    }
}

float UEngineArchitecturalFramework::GetSystemPerformanceMetric(const FString& SystemName) const
{
    if (SystemPerformanceMetrics.Contains(SystemName))
    {
        return SystemPerformanceMetrics[SystemName];
    }
    return 0.0f;
}

bool UEngineArchitecturalFramework::ValidateSystemArchitecture(const FString& SystemName)
{
    if (!IsSystemRegistered(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: Cannot validate unregistered system: %s"), *SystemName);
        return false;
    }

    // Check system dependencies
    if (!AreSystemDependenciesMet(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: System dependencies not met for: %s"), *SystemName);
        return false;
    }

    // Check performance compliance
    if (!CheckSystemPerformance(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Performance issues detected for: %s"), *SystemName);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("EngineArchitecturalFramework: System validation passed for: %s"), *SystemName);
    return true;
}

void UEngineArchitecturalFramework::RunFullArchitecturalValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Running full architectural validation"));

    int32 PassedSystems = 0;
    int32 FailedSystems = 0;

    for (const auto& SystemPair : RegisteredSystems)
    {
        if (ValidateSystemArchitecture(SystemPair.Key))
        {
            PassedSystems++;
        }
        else
        {
            FailedSystems++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Validation complete - Passed: %d, Failed: %d"), 
           PassedSystems, FailedSystems);
}

void UEngineArchitecturalFramework::AddSystemDependency(const FString& SystemName, const FString& DependencyName)
{
    if (!SystemDependencies.Contains(SystemName))
    {
        SystemDependencies.Add(SystemName, TArray<FString>());
    }

    SystemDependencies[SystemName].AddUnique(DependencyName);
    UE_LOG(LogTemp, Log, TEXT("EngineArchitecturalFramework: Added dependency '%s' for system '%s'"), 
           *DependencyName, *SystemName);
}

bool UEngineArchitecturalFramework::AreSystemDependenciesMet(const FString& SystemName) const
{
    if (!SystemDependencies.Contains(SystemName))
    {
        return true; // No dependencies means they are met
    }

    const TArray<FString>& Dependencies = SystemDependencies[SystemName];
    for (const FString& Dependency : Dependencies)
    {
        if (!IsSystemRegistered(Dependency))
        {
            UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Dependency '%s' not met for system '%s'"), 
                   *Dependency, *SystemName);
            return false;
        }
    }

    return true;
}

UBiomeManager* UEngineArchitecturalFramework::GetBiomeManager() const
{
    return BiomeManagerRef;
}

UAssetProductionManager* UEngineArchitecturalFramework::GetAssetProductionManager() const
{
    return AssetProductionManagerRef;
}

UBuildSystemManager* UEngineArchitecturalFramework::GetBuildSystemManager() const
{
    return BuildSystemManagerRef;
}

UArchitectureValidationSuite* UEngineArchitecturalFramework::GetValidationSuite() const
{
    return ValidationSuiteRef;
}

void UEngineArchitecturalFramework::EnforcePerformanceStandards()
{
    if (!bEnablePerformanceMonitoring)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Enforcing performance standards"));

    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        if (!CheckSystemPerformance(SystemName))
        {
            UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Performance enforcement action for system: %s"), 
                   *SystemName);
            // Could implement automatic performance optimization here
        }
    }
}

void UEngineArchitecturalFramework::ValidateModuleCompliance()
{
    if (!bEnforceModuleCompliance)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Validating module compliance"));

    // Check that all critical systems are registered
    TArray<FString> RequiredSystems = {
        TEXT("BiomeManager"),
        TEXT("AssetProductionManager"),
        TEXT("BuildSystemManager"),
        TEXT("ArchitectureValidationSuite")
    };

    for (const FString& RequiredSystem : RequiredSystems)
    {
        if (!IsSystemRegistered(RequiredSystem))
        {
            UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: Required system not registered: %s"), 
                   *RequiredSystem);
        }
    }
}

void UEngineArchitecturalFramework::GenerateArchitecturalReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL FRAMEWORK REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitoring: %s"), bEnablePerformanceMonitoring ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogTemp, Warning, TEXT("Module Compliance: %s"), bEnforceModuleCompliance ? TEXT("ENABLED") : TEXT("DISABLED"));

    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        int32 Priority = SystemPriorities.Contains(SystemName) ? SystemPriorities[SystemName] : -1;
        float Performance = GetSystemPerformanceMetric(SystemName);
        bool HasDependencies = SystemDependencies.Contains(SystemName);

        UE_LOG(LogTemp, Warning, TEXT("System: %s | Priority: %d | Performance: %f | Dependencies: %s"), 
               *SystemName, Priority, Performance, HasDependencies ? TEXT("YES") : TEXT("NO"));
    }

    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURAL REPORT ==="));
}

void UEngineArchitecturalFramework::InitializeCoreSystemReferences()
{
    // Get references to core systems from the game instance
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: GameInstance not available"));
        return;
    }

    // Initialize BiomeManager reference
    BiomeManagerRef = GameInstance->GetSubsystem<UBiomeManager>();
    if (BiomeManagerRef)
    {
        RegisterCoreSystem(TEXT("BiomeManager"), BiomeManagerRef, 10);
    }

    // Initialize AssetProductionManager reference
    AssetProductionManagerRef = GameInstance->GetSubsystem<UAssetProductionManager>();
    if (AssetProductionManagerRef)
    {
        RegisterCoreSystem(TEXT("AssetProductionManager"), AssetProductionManagerRef, 15);
    }

    // Initialize BuildSystemManager reference
    BuildSystemManagerRef = GameInstance->GetSubsystem<UBuildSystemManager>();
    if (BuildSystemManagerRef)
    {
        RegisterCoreSystem(TEXT("BuildSystemManager"), BuildSystemManagerRef, 20);
    }

    // Initialize ValidationSuite reference
    ValidationSuiteRef = GameInstance->GetSubsystem<UArchitectureValidationSuite>();
    if (ValidationSuiteRef)
    {
        RegisterCoreSystem(TEXT("ArchitectureValidationSuite"), ValidationSuiteRef, 5);
    }
}

void UEngineArchitecturalFramework::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Validating system integrity"));

    // Check that critical systems are available
    if (!BiomeManagerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: BiomeManager not available"));
    }

    if (!AssetProductionManagerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: AssetProductionManager not available"));
    }

    if (!BuildSystemManagerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: BuildSystemManager not available"));
    }

    if (!ValidationSuiteRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: ArchitectureValidationSuite not available"));
    }
}

void UEngineArchitecturalFramework::LogArchitecturalStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Current Status"));
    UE_LOG(LogTemp, Warning, TEXT("- Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Performance Monitoring: %s"), bEnablePerformanceMonitoring ? TEXT("ON") : TEXT("OFF"));
    UE_LOG(LogTemp, Warning, TEXT("- Module Compliance: %s"), bEnforceModuleCompliance ? TEXT("ON") : TEXT("OFF"));
    UE_LOG(LogTemp, Warning, TEXT("- Dependency Validation: %s"), bValidateSystemDependencies ? TEXT("ON") : TEXT("OFF"));
}

bool UEngineArchitecturalFramework::CheckSystemPerformance(const FString& SystemName) const
{
    if (!SystemPerformanceMetrics.Contains(SystemName))
    {
        return true; // No metrics means no problems
    }

    float PerformanceMetric = SystemPerformanceMetrics[SystemName];
    return PerformanceMetric <= MaxAllowedFrameTime;
}

void UEngineArchitecturalFramework::EnforceSystemLimits()
{
    // This could implement automatic system throttling or optimization
    // For now, just log warnings when limits are exceeded
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        if (!CheckSystemPerformance(SystemName))
        {
            UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: System '%s' exceeding performance limits"), 
                   *SystemName);
        }
    }
}