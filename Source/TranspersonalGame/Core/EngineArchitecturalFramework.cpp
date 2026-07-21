#include "EngineArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UEngineArchitecturalFramework::UEngineArchitecturalFramework()
{
    // Initialize default configuration
    bEnablePerformanceMonitoring = true;
    bEnableArchitecturalValidation = true;
    ValidationFrequency = 5.0f;
    PerformanceMonitoringFrequency = 1.0f;
}

void UEngineArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initializing..."));

    // Clear any existing data
    RegisteredSystems.Empty();
    PerformanceMetrics.Empty();
    ValidationResults.Empty();
    SystemHealthMap.Empty();

    // Register core engine systems by default
    FEng_SystemRegistration CorePhysics;
    CorePhysics.SystemName = TEXT("CorePhysicsSystem");
    CorePhysics.Priority = EEng_SystemPriority::Critical;
    CorePhysics.ModuleName = TEXT("TranspersonalGame");
    CorePhysics.AgentResponsible = TEXT("Agent #3 - Core Systems Programmer");
    CorePhysics.bIsActive = true;
    RegisterSystem(CorePhysics);

    FEng_SystemRegistration WorldGen;
    WorldGen.SystemName = TEXT("ProceduralWorldGenerator");
    WorldGen.Priority = EEng_SystemPriority::High;
    WorldGen.ModuleName = TEXT("TranspersonalGame");
    WorldGen.AgentResponsible = TEXT("Agent #5 - Procedural World Generator");
    WorldGen.bIsActive = true;
    RegisterSystem(WorldGen);

    FEng_SystemRegistration BiomeSystem;
    BiomeSystem.SystemName = TEXT("BiomeManager");
    BiomeSystem.Priority = EEng_SystemPriority::High;
    BiomeSystem.ModuleName = TEXT("TranspersonalGame");
    BiomeSystem.AgentResponsible = TEXT("Agent #6 - Environment Artist");
    BiomeSystem.bIsActive = true;
    RegisterSystem(BiomeSystem);

    // Initialize default performance metrics
    FEng_PerformanceMetrics PhysicsMetric;
    PhysicsMetric.Category = EEng_PerformanceCategory::Physics;
    PhysicsMetric.MetricName = TEXT("PhysicsFrameTime");
    PhysicsMetric.TargetValue = 16.67f; // 60 FPS target
    PhysicsMetric.MaxValue = 33.33f; // 30 FPS minimum
    PhysicsMetric.CurrentValue = 0.0f;
    PhysicsMetric.bIsWithinLimits = true;
    PerformanceMetrics.Add(PhysicsMetric);

    FEng_PerformanceMetrics RenderingMetric;
    RenderingMetric.Category = EEng_PerformanceCategory::Rendering;
    RenderingMetric.MetricName = TEXT("RenderFrameTime");
    RenderingMetric.TargetValue = 16.67f;
    RenderingMetric.MaxValue = 33.33f;
    RenderingMetric.CurrentValue = 0.0f;
    RenderingMetric.bIsWithinLimits = true;
    PerformanceMetrics.Add(RenderingMetric);

    // Start periodic monitoring if enabled
    if (UWorld* World = GetWorld())
    {
        if (bEnablePerformanceMonitoring && PerformanceMonitoringFrequency > 0.0f)
        {
            World->GetTimerManager().SetTimer(PerformanceTimerHandle, 
                FTimerDelegate::CreateUObject(this, &UEngineArchitecturalFramework::UpdateFrameworkMetrics),
                PerformanceMonitoringFrequency, true);
        }

        if (bEnableArchitecturalValidation && ValidationFrequency > 0.0f)
        {
            World->GetTimerManager().SetTimer(ValidationTimerHandle,
                FTimerDelegate::CreateUObject(this, &UEngineArchitecturalFramework::CheckArchitecturalCompliance),
                ValidationFrequency, true);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initialized with %d systems"), RegisteredSystems.Num());
}

void UEngineArchitecturalFramework::Deinitialize()
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }

    // Clear all data
    RegisteredSystems.Empty();
    PerformanceMetrics.Empty();
    ValidationResults.Empty();
    SystemHealthMap.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Deinitialized"));

    Super::Deinitialize();
}

bool UEngineArchitecturalFramework::RegisterSystem(const FEng_SystemRegistration& SystemInfo)
{
    // Check if system is already registered
    for (const FEng_SystemRegistration& ExistingSystem : RegisteredSystems)
    {
        if (ExistingSystem.SystemName == SystemInfo.SystemName)
        {
            UE_LOG(LogTemp, Warning, TEXT("System %s is already registered"), *SystemInfo.SystemName);
            return false;
        }
    }

    // Add the new system
    FEng_SystemRegistration NewSystem = SystemInfo;
    NewSystem.LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    RegisteredSystems.Add(NewSystem);

    // Initialize health tracking
    SystemHealthMap.Add(SystemInfo.SystemName, 100.0f);

    UE_LOG(LogTemp, Log, TEXT("Registered system: %s (Agent: %s, Priority: %s)"), 
        *SystemInfo.SystemName, 
        *SystemInfo.AgentResponsible,
        *UEnum::GetValueAsString(SystemInfo.Priority));

    return true;
}

bool UEngineArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    for (int32 i = RegisteredSystems.Num() - 1; i >= 0; --i)
    {
        if (RegisteredSystems[i].SystemName == SystemName)
        {
            RegisteredSystems.RemoveAt(i);
            SystemHealthMap.Remove(SystemName);
            UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Failed to unregister system: %s (not found)"), *SystemName);
    return false;
}

TArray<FEng_SystemRegistration> UEngineArchitecturalFramework::GetRegisteredSystems() const
{
    return RegisteredSystems;
}

bool UEngineArchitecturalFramework::IsSystemRegistered(const FString& SystemName) const
{
    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            return true;
        }
    }
    return false;
}

void UEngineArchitecturalFramework::UpdatePerformanceMetric(const FEng_PerformanceMetrics& Metric)
{
    // Find existing metric or add new one
    for (FEng_PerformanceMetrics& ExistingMetric : PerformanceMetrics)
    {
        if (ExistingMetric.MetricName == Metric.MetricName)
        {
            ExistingMetric.CurrentValue = Metric.CurrentValue;
            ExistingMetric.bIsWithinLimits = (Metric.CurrentValue <= ExistingMetric.MaxValue);
            return;
        }
    }

    // Add new metric
    PerformanceMetrics.Add(Metric);
}

FEng_PerformanceMetrics UEngineArchitecturalFramework::GetPerformanceMetric(const FString& MetricName) const
{
    for (const FEng_PerformanceMetrics& Metric : PerformanceMetrics)
    {
        if (Metric.MetricName == MetricName)
        {
            return Metric;
        }
    }

    // Return default metric if not found
    FEng_PerformanceMetrics DefaultMetric;
    DefaultMetric.MetricName = MetricName;
    return DefaultMetric;
}

TArray<FEng_PerformanceMetrics> UEngineArchitecturalFramework::GetAllPerformanceMetrics() const
{
    return PerformanceMetrics;
}

bool UEngineArchitecturalFramework::IsPerformanceWithinLimits() const
{
    for (const FEng_PerformanceMetrics& Metric : PerformanceMetrics)
    {
        if (!Metric.bIsWithinLimits)
        {
            return false;
        }
    }
    return true;
}

FEng_ValidationResult UEngineArchitecturalFramework::ValidateSystem(const FString& SystemName)
{
    FEng_ValidationResult Result;
    ValidateSystemInternal(SystemName, Result);
    return Result;
}

TArray<FEng_ValidationResult> UEngineArchitecturalFramework::ValidateAllSystems()
{
    TArray<FEng_ValidationResult> Results;

    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        FEng_ValidationResult Result;
        if (ValidateSystemInternal(System.SystemName, Result))
        {
            Results.Add(Result);
        }
    }

    ValidationResults = Results;
    return Results;
}

EEng_ComplianceStatus UEngineArchitecturalFramework::GetOverallComplianceStatus() const
{
    if (ValidationResults.Num() == 0)
    {
        return EEng_ComplianceStatus::Unknown;
    }

    bool bHasCritical = false;
    bool bHasViolation = false;
    bool bHasWarning = false;

    for (const FEng_ValidationResult& Result : ValidationResults)
    {
        switch (Result.Status)
        {
        case EEng_ComplianceStatus::Critical:
            bHasCritical = true;
            break;
        case EEng_ComplianceStatus::Violation:
            bHasViolation = true;
            break;
        case EEng_ComplianceStatus::Warning:
            bHasWarning = true;
            break;
        }
    }

    if (bHasCritical) return EEng_ComplianceStatus::Critical;
    if (bHasViolation) return EEng_ComplianceStatus::Violation;
    if (bHasWarning) return EEng_ComplianceStatus::Warning;

    return EEng_ComplianceStatus::Compliant;
}

void UEngineArchitecturalFramework::UpdateSystemHealth(const FString& SystemName, float HealthValue)
{
    SystemHealthMap.Add(SystemName, FMath::Clamp(HealthValue, 0.0f, 100.0f));
}

float UEngineArchitecturalFramework::GetSystemHealth(const FString& SystemName) const
{
    if (const float* Health = SystemHealthMap.Find(SystemName))
    {
        return *Health;
    }
    return 0.0f;
}

bool UEngineArchitecturalFramework::AreAllSystemsHealthy() const
{
    for (const auto& HealthPair : SystemHealthMap)
    {
        if (HealthPair.Value < 50.0f) // Below 50% is considered unhealthy
        {
            return false;
        }
    }
    return true;
}

bool UEngineArchitecturalFramework::EnforceArchitecturalStandards()
{
    // Run validation on all systems
    ValidateAllSystems();

    // Check performance metrics
    bool bPerformanceOK = IsPerformanceWithinLimits();
    bool bSystemsHealthy = AreAllSystemsHealthy();
    EEng_ComplianceStatus ComplianceStatus = GetOverallComplianceStatus();

    bool bStandardsEnforced = (bPerformanceOK && bSystemsHealthy && 
        (ComplianceStatus == EEng_ComplianceStatus::Compliant || 
         ComplianceStatus == EEng_ComplianceStatus::Warning));

    UE_LOG(LogTemp, Log, TEXT("Architectural Standards Enforcement: %s"), 
        bStandardsEnforced ? TEXT("PASS") : TEXT("FAIL"));

    return bStandardsEnforced;
}

TArray<FString> UEngineArchitecturalFramework::GetArchitecturalViolations() const
{
    TArray<FString> Violations;

    for (const FEng_ValidationResult& Result : ValidationResults)
    {
        if (Result.Status == EEng_ComplianceStatus::Violation || 
            Result.Status == EEng_ComplianceStatus::Critical)
        {
            Violations.Add(FString::Printf(TEXT("%s: %s"), *Result.SystemName, *Result.ValidationMessage));
        }
    }

    return Violations;
}

void UEngineArchitecturalFramework::GenerateArchitecturalReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURAL FRAMEWORK REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Metrics: %d"), PerformanceMetrics.Num());
    UE_LOG(LogTemp, Warning, TEXT("Validation Results: %d"), ValidationResults.Num());
    UE_LOG(LogTemp, Warning, TEXT("Overall Compliance: %s"), *UEnum::GetValueAsString(GetOverallComplianceStatus()));
    UE_LOG(LogTemp, Warning, TEXT("Performance Within Limits: %s"), IsPerformanceWithinLimits() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("All Systems Healthy: %s"), AreAllSystemsHealthy() ? TEXT("YES") : TEXT("NO"));

    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        float Health = GetSystemHealth(System.SystemName);
        UE_LOG(LogTemp, Warning, TEXT("  - %s: %s (Health: %.1f%%)"), 
            *System.SystemName, 
            *UEnum::GetValueAsString(System.Priority),
            Health);
    }

    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

FString UEngineArchitecturalFramework::GetFrameworkStatus() const
{
    return FString::Printf(TEXT("Systems: %d | Metrics: %d | Compliance: %s | Performance: %s"),
        RegisteredSystems.Num(),
        PerformanceMetrics.Num(),
        *UEnum::GetValueAsString(GetOverallComplianceStatus()),
        IsPerformanceWithinLimits() ? TEXT("OK") : TEXT("FAIL"));
}

void UEngineArchitecturalFramework::RunArchitecturalDiagnostics()
{
    UE_LOG(LogTemp, Warning, TEXT("Running Architectural Diagnostics..."));

    // Validate all systems
    ValidateAllSystems();

    // Update performance metrics
    UpdateFrameworkMetrics();

    // Check compliance
    CheckArchitecturalCompliance();

    // Generate report
    GenerateArchitecturalReport();

    UE_LOG(LogTemp, Warning, TEXT("Architectural Diagnostics Complete"));
}

bool UEngineArchitecturalFramework::ValidateSystemInternal(const FString& SystemName, FEng_ValidationResult& OutResult)
{
    OutResult.SystemName = SystemName;
    OutResult.ValidationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Check if system is registered
    bool bIsRegistered = IsSystemRegistered(SystemName);
    if (!bIsRegistered)
    {
        OutResult.Status = EEng_ComplianceStatus::Violation;
        OutResult.ValidationMessage = TEXT("System is not registered with the framework");
        OutResult.Recommendations.Add(TEXT("Register the system using RegisterSystem()"));
        return true;
    }

    // Check system health
    float SystemHealth = GetSystemHealth(SystemName);
    if (SystemHealth < 25.0f)
    {
        OutResult.Status = EEng_ComplianceStatus::Critical;
        OutResult.ValidationMessage = FString::Printf(TEXT("System health is critically low: %.1f%%"), SystemHealth);
        OutResult.Recommendations.Add(TEXT("Investigate system performance issues"));
    }
    else if (SystemHealth < 50.0f)
    {
        OutResult.Status = EEng_ComplianceStatus::Warning;
        OutResult.ValidationMessage = FString::Printf(TEXT("System health is below optimal: %.1f%%"), SystemHealth);
        OutResult.Recommendations.Add(TEXT("Monitor system performance"));
    }
    else
    {
        OutResult.Status = EEng_ComplianceStatus::Compliant;
        OutResult.ValidationMessage = TEXT("System is operating within normal parameters");
    }

    return true;
}

void UEngineArchitecturalFramework::UpdateFrameworkMetrics()
{
    // Update performance metrics with current engine stats
    if (GEngine)
    {
        // Update physics frame time
        FEng_PerformanceMetrics PhysicsMetric = GetPerformanceMetric(TEXT("PhysicsFrameTime"));
        PhysicsMetric.CurrentValue = GEngine->GetMaxTickRate() > 0 ? (1000.0f / GEngine->GetMaxTickRate()) : 16.67f;
        PhysicsMetric.bIsWithinLimits = (PhysicsMetric.CurrentValue <= PhysicsMetric.MaxValue);
        UpdatePerformanceMetric(PhysicsMetric);

        // Update rendering frame time
        FEng_PerformanceMetrics RenderMetric = GetPerformanceMetric(TEXT("RenderFrameTime"));
        RenderMetric.CurrentValue = GEngine->GetMaxFPS() > 0 ? (1000.0f / GEngine->GetMaxFPS()) : 16.67f;
        RenderMetric.bIsWithinLimits = (RenderMetric.CurrentValue <= RenderMetric.MaxValue);
        UpdatePerformanceMetric(RenderMetric);
    }

    // Update system health based on performance
    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        float CurrentHealth = GetSystemHealth(System.SystemName);
        
        // Simulate health based on system activity and performance
        if (System.bIsActive)
        {
            // Active systems maintain good health if performance is good
            float HealthModifier = IsPerformanceWithinLimits() ? 1.0f : 0.95f;
            UpdateSystemHealth(System.SystemName, FMath::Min(100.0f, CurrentHealth * HealthModifier));
        }
        else
        {
            // Inactive systems gradually lose health
            UpdateSystemHealth(System.SystemName, FMath::Max(0.0f, CurrentHealth - 5.0f));
        }
    }
}

void UEngineArchitecturalFramework::CheckArchitecturalCompliance()
{
    // Run periodic validation
    ValidateAllSystems();

    // Log compliance status
    EEng_ComplianceStatus Status = GetOverallComplianceStatus();
    if (Status == EEng_ComplianceStatus::Critical || Status == EEng_ComplianceStatus::Violation)
    {
        UE_LOG(LogTemp, Error, TEXT("Architectural Compliance Issue Detected: %s"), *UEnum::GetValueAsString(Status));
        
        TArray<FString> Violations = GetArchitecturalViolations();
        for (const FString& Violation : Violations)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Violation);
        }
    }
}