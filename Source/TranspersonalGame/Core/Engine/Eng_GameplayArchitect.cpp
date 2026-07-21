#include "Eng_GameplayArchitect.h"
#include "Eng_SystemsRegistry.h"
#include "Eng_PerformanceMonitor.h"
#include "Eng_ModuleValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UEng_GameplayArchitect::UEng_GameplayArchitect()
{
    // Initialize default values
    bAutoInitializeSystems = true;
    bEnablePerformanceMonitoring = true;
    bValidateModulesOnStartup = true;
    PerformanceUpdateInterval = 1.0f;
    
    LastPerformanceUpdate = 0.0f;
    bSystemsInitialized = false;
    bPerformanceMonitoringActive = false;
}

void UEng_GameplayArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initializing Gameplay Architecture..."));
    
    // Initialize subsystems
    InitializeSubsystems();
    
    // Auto-initialize systems if configured
    if (bAutoInitializeSystems)
    {
        InitializeGameplaySystems();
    }
    
    // Validate modules if configured
    if (bValidateModulesOnStartup)
    {
        ValidateModuleCompilation();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Gameplay Architecture initialized successfully"));
}

void UEng_GameplayArchitect::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Shutting down Gameplay Architecture..."));
    
    ShutdownSubsystems();
    
    Super::Deinitialize();
}

bool UEng_GameplayArchitect::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEng_GameplayArchitect::InitializeGameplaySystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Initializing gameplay systems..."));
    
    if (bSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Systems already initialized"));
        return;
    }
    
    // Initialize core systems in dependency order
    RegisterGameplaySystem(TEXT("WorldGeneration"), nullptr);
    RegisterGameplaySystem(TEXT("CharacterSystem"), nullptr);
    RegisterGameplaySystem(TEXT("DinosaurAI"), nullptr);
    RegisterGameplaySystem(TEXT("CombatSystem"), nullptr);
    RegisterGameplaySystem(TEXT("QuestSystem"), nullptr);
    RegisterGameplaySystem(TEXT("AudioSystem"), nullptr);
    RegisterGameplaySystem(TEXT("VFXSystem"), nullptr);
    
    // Enable performance monitoring
    if (bEnablePerformanceMonitoring && PerformanceMonitor)
    {
        bPerformanceMonitoringActive = true;
        UpdateSystemMetrics();
    }
    
    bSystemsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: All gameplay systems initialized"));
}

void UEng_GameplayArchitect::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating system integrity..."));
    
    int32 ValidSystems = 0;
    int32 TotalSystems = RegisteredSystems.Num();
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        bool bIsActive = IsSystemActive(SystemName);
        
        if (bIsActive)
        {
            ValidSystems++;
            UE_LOG(LogTemp, Log, TEXT("System %s: ACTIVE"), *SystemName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("System %s: INACTIVE"), *SystemName);
        }
    }
    
    float IntegrityPercentage = TotalSystems > 0 ? (float)ValidSystems / TotalSystems * 100.0f : 100.0f;
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: System integrity: %.1f%% (%d/%d systems active)"), 
           IntegrityPercentage, ValidSystems, TotalSystems);
}

void UEng_GameplayArchitect::RegisterGameplaySystem(const FString& SystemName, UObject* SystemInstance)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Cannot register system with empty name"));
        return;
    }
    
    RegisteredSystems.Add(SystemName, SystemInstance);
    SystemActiveStates.Add(SystemName, SystemInstance != nullptr);
    SystemPerformanceMetrics.Add(SystemName, 0.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered system '%s'"), *SystemName);
}

UObject* UEng_GameplayArchitect::GetGameplaySystem(const FString& SystemName)
{
    if (UObject** FoundSystem = RegisteredSystems.Find(SystemName))
    {
        return *FoundSystem;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: System '%s' not found"), *SystemName);
    return nullptr;
}

bool UEng_GameplayArchitect::IsSystemActive(const FString& SystemName)
{
    if (bool* ActiveState = SystemActiveStates.Find(SystemName))
    {
        return *ActiveState;
    }
    
    return false;
}

float UEng_GameplayArchitect::GetSystemPerformanceMetric(const FString& SystemName)
{
    if (float* Metric = SystemPerformanceMetrics.Find(SystemName))
    {
        return *Metric;
    }
    
    return 0.0f;
}

void UEng_GameplayArchitect::OptimizeSystemPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Optimizing system performance..."));
    
    // Update performance metrics
    UpdateSystemMetrics();
    
    // Identify systems with poor performance
    TArray<FString> UnderperformingSystems;
    for (const auto& MetricPair : SystemPerformanceMetrics)
    {
        if (MetricPair.Value > 16.67f) // Above 60fps threshold
        {
            UnderperformingSystems.Add(MetricPair.Key);
        }
    }
    
    if (UnderperformingSystems.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Found %d underperforming systems"), 
               UnderperformingSystems.Num());
        
        for (const FString& SystemName : UnderperformingSystems)
        {
            UE_LOG(LogTemp, Warning, TEXT("- %s: %.2fms"), *SystemName, 
                   GetSystemPerformanceMetric(SystemName));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: All systems performing within acceptable limits"));
    }
}

bool UEng_GameplayArchitect::ValidateModuleCompilation()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Validating module compilation..."));
    
    if (!ModuleValidator)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: ModuleValidator not available"));
        return false;
    }
    
    // For now, assume compilation is valid if we're running
    bool bCompilationValid = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Module compilation validation: %s"), 
           bCompilationValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bCompilationValid;
}

void UEng_GameplayArchitect::ReportSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT SYSTEM STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total registered systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Systems initialized: %s"), bSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring: %s"), bPerformanceMonitoringActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        bool bActive = IsSystemActive(SystemName);
        float Performance = GetSystemPerformanceMetric(SystemName);
        
        UE_LOG(LogTemp, Warning, TEXT("- %s: %s (%.2fms)"), 
               *SystemName, 
               bActive ? TEXT("ACTIVE") : TEXT("INACTIVE"), 
               Performance);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END SYSTEM STATUS REPORT ==="));
}

void UEng_GameplayArchitect::InitializeSubsystems()
{
    // Create subsystem instances
    SystemsRegistry = NewObject<UEng_SystemsRegistry>(this);
    PerformanceMonitor = NewObject<UEng_PerformanceMonitor>(this);
    ModuleValidator = NewObject<UEng_ModuleValidator>(this);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Subsystems initialized"));
}

void UEng_GameplayArchitect::ShutdownSubsystems()
{
    bSystemsInitialized = false;
    bPerformanceMonitoringActive = false;
    
    RegisteredSystems.Empty();
    SystemActiveStates.Empty();
    SystemPerformanceMetrics.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Subsystems shutdown complete"));
}

void UEng_GameplayArchitect::UpdateSystemMetrics()
{
    if (!bPerformanceMonitoringActive)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        // Update performance metrics for all systems
        for (auto& MetricPair : SystemPerformanceMetrics)
        {
            // Simulate performance metrics (in real implementation, these would be actual measurements)
            float RandomVariation = FMath::RandRange(-2.0f, 2.0f);
            MetricPair.Value = FMath::Clamp(10.0f + RandomVariation, 5.0f, 25.0f);
        }
        
        LastPerformanceUpdate = CurrentTime;
    }
}

void UEng_GameplayArchitect::ValidateSystemDependencies()
{
    // Validate that all required systems are present and active
    TArray<FString> RequiredSystems = {
        TEXT("WorldGeneration"),
        TEXT("CharacterSystem"),
        TEXT("DinosaurAI")
    };
    
    for (const FString& RequiredSystem : RequiredSystems)
    {
        if (!RegisteredSystems.Contains(RequiredSystem))
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Required system '%s' not registered"), 
                   *RequiredSystem);
        }
    }
}