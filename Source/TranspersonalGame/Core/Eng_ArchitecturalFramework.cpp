#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"

// Constructor for UEng_ArchitecturalFramework
UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    bFrameworkInitialized = false;
    CurrentProfile = EEng_PerformanceProfile::Development;
    MetricsUpdateInterval = 1.0f; // Update metrics every second
    LastMetricsUpdate = 0.0f;
    
    // Initialize default metrics
    LastMetrics = FEng_SystemMetrics();
}

void UEng_ArchitecturalFramework::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: BeginPlay - Initializing framework"));
    InitializeFramework();
}

void UEng_ArchitecturalFramework::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bFrameworkInitialized)
    {
        return;
    }
    
    LastMetricsUpdate += DeltaTime;
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdateSystemMetrics();
        CheckPerformanceLimits();
        LastMetricsUpdate = 0.0f;
    }
    
    // Enforce architectural rules periodically
    static float RuleEnforcementTimer = 0.0f;
    RuleEnforcementTimer += DeltaTime;
    if (RuleEnforcementTimer >= 5.0f) // Check rules every 5 seconds
    {
        EnforceArchitecturalRules();
        RuleEnforcementTimer = 0.0f;
    }
}

void UEng_ArchitecturalFramework::InitializeFramework()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Initializing architectural framework"));
    
    // Clear existing data
    RegisteredModules.Empty();
    ArchitecturalRules.Empty();
    
    // Initialize default architectural rules
    InitializeDefaultRules();
    
    // Register core modules
    FEng_ModuleInfo CoreModule;
    CoreModule.ModuleName = TEXT("TranspersonalGameCore");
    CoreModule.Priority = EEng_SystemPriority::Critical;
    CoreModule.State = EEng_ModuleState::Active;
    CoreModule.InitializationTime = 0.1f;
    RegisterModule(CoreModule);
    
    FEng_ModuleInfo ArchitectureModule;
    ArchitectureModule.ModuleName = TEXT("ArchitecturalFramework");
    ArchitectureModule.Priority = EEng_SystemPriority::Critical;
    ArchitectureModule.State = EEng_ModuleState::Active;
    ArchitectureModule.InitializationTime = 0.05f;
    RegisterModule(ArchitectureModule);
    
    bFrameworkInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Framework initialization complete"));
}

bool UEng_ArchitecturalFramework::RegisterModule(const FEng_ModuleInfo& ModuleInfo)
{
    if (IsModuleRegistered(ModuleInfo.ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Module %s already registered"), *ModuleInfo.ModuleName);
        return false;
    }
    
    RegisteredModules.Add(ModuleInfo);
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Registered module %s with priority %d"), 
           *ModuleInfo.ModuleName, (int32)ModuleInfo.Priority);
    
    return true;
}

bool UEng_ArchitecturalFramework::UnregisterModule(const FString& ModuleName)
{
    for (int32 i = 0; i < RegisteredModules.Num(); i++)
    {
        if (RegisteredModules[i].ModuleName == ModuleName)
        {
            RegisteredModules.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Unregistered module %s"), *ModuleName);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Module %s not found for unregistration"), *ModuleName);
    return false;
}

void UEng_ArchitecturalFramework::EnforceArchitecturalRules()
{
    if (!bFrameworkInitialized)
    {
        return;
    }
    
    // Check each architectural rule
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        // Performance rule enforcement
        if (Rule.RuleName == TEXT("MaxFrameTime"))
        {
            if (LastMetrics.FrameTime > 33.33f) // 30 FPS limit
            {
                UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Frame time violation - %f ms"), LastMetrics.FrameTime);
            }
        }
        else if (Rule.RuleName == TEXT("MaxActiveActors"))
        {
            if (LastMetrics.ActiveActors > 10000) // 10k actor limit
            {
                UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Active actor limit violation - %d actors"), LastMetrics.ActiveActors);
            }
        }
    }
    
    // Validate module dependencies
    ValidateModuleDependencies();
}

FEng_SystemMetrics UEng_ArchitecturalFramework::GetCurrentMetrics() const
{
    return LastMetrics;
}

void UEng_ArchitecturalFramework::SetPerformanceProfile(EEng_PerformanceProfile Profile)
{
    CurrentProfile = Profile;
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Performance profile set to %d"), (int32)Profile);
    
    // Adjust monitoring based on profile
    switch (Profile)
    {
        case EEng_PerformanceProfile::Development:
            MetricsUpdateInterval = 1.0f;
            break;
        case EEng_PerformanceProfile::Testing:
            MetricsUpdateInterval = 0.5f;
            break;
        case EEng_PerformanceProfile::Shipping:
            MetricsUpdateInterval = 0.1f;
            break;
    }
}

bool UEng_ArchitecturalFramework::IsPerformanceWithinLimits() const
{
    // Check frame time limit
    if (LastMetrics.FrameTime > 33.33f) // 30 FPS minimum
    {
        return false;
    }
    
    // Check memory usage
    if (LastMetrics.MemoryUsage > 85.0f) // 85% memory limit
    {
        return false;
    }
    
    // Check actor count
    if (LastMetrics.ActiveActors > 10000)
    {
        return false;
    }
    
    return true;
}

TArray<FEng_ModuleInfo> UEng_ArchitecturalFramework::GetRegisteredModules() const
{
    return RegisteredModules;
}

EEng_ModuleState UEng_ArchitecturalFramework::GetModuleState(const FString& ModuleName) const
{
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            return Module.State;
        }
    }
    
    return EEng_ModuleState::Uninitialized;
}

bool UEng_ArchitecturalFramework::SetModuleState(const FString& ModuleName, EEng_ModuleState NewState)
{
    for (FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            Module.State = NewState;
            UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Module %s state changed to %d"), *ModuleName, (int32)NewState);
            return true;
        }
    }
    
    return false;
}

void UEng_ArchitecturalFramework::AddArchitecturalRule(const FEng_ArchitecturalRule& Rule)
{
    ArchitecturalRules.Add(Rule);
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Added rule %s"), *Rule.RuleName);
}

void UEng_ArchitecturalFramework::RemoveArchitecturalRule(const FString& RuleName)
{
    for (int32 i = 0; i < ArchitecturalRules.Num(); i++)
    {
        if (ArchitecturalRules[i].RuleName == RuleName)
        {
            ArchitecturalRules.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Removed rule %s"), *RuleName);
            break;
        }
    }
}

TArray<FEng_ArchitecturalRule> UEng_ArchitecturalFramework::GetActiveRules() const
{
    return ArchitecturalRules;
}

void UEng_ArchitecturalFramework::ValidateSystemArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Validating system architecture"));
    
    // Check module states
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.State == EEng_ModuleState::Error)
        {
            UE_LOG(LogTemp, Error, TEXT("ArchitecturalFramework: Module %s is in error state"), *Module.ModuleName);
        }
    }
    
    // Check performance metrics
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Performance outside acceptable limits"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Architecture validation complete"));
}

void UEng_ArchitecturalFramework::GenerateArchitecturalReport()
{
    FString Report = TEXT("=== ARCHITECTURAL FRAMEWORK REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Performance Profile: %d\n"), (int32)CurrentProfile);
    Report += FString::Printf(TEXT("Framework Initialized: %s\n"), bFrameworkInitialized ? TEXT("Yes") : TEXT("No"));
    
    Report += TEXT("\n--- REGISTERED MODULES ---\n");
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        Report += FString::Printf(TEXT("Module: %s | Priority: %d | State: %d\n"), 
                                  *Module.ModuleName, (int32)Module.Priority, (int32)Module.State);
    }
    
    Report += TEXT("\n--- CURRENT METRICS ---\n");
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), LastMetrics.FrameTime);
    Report += FString::Printf(TEXT("CPU Usage: %.2f%%\n"), LastMetrics.CPUUsage);
    Report += FString::Printf(TEXT("Memory Usage: %.2f%%\n"), LastMetrics.MemoryUsage);
    Report += FString::Printf(TEXT("Active Actors: %d\n"), LastMetrics.ActiveActors);
    Report += FString::Printf(TEXT("Ticking Components: %d\n"), LastMetrics.TickingComponents);
    
    Report += TEXT("\n--- ARCHITECTURAL RULES ---\n");
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        Report += FString::Printf(TEXT("Rule: %s | Mandatory: %s\n"), 
                                  *Rule.RuleName, Rule.bMandatory ? TEXT("Yes") : TEXT("No"));
    }
    
    // Save report to file
    FString FilePath = FPaths::ProjectLogDir() / TEXT("ArchitecturalReport.txt");
    FFileHelper::SaveStringToFile(Report, *FilePath);
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Report generated at %s"), *FilePath);
}

void UEng_ArchitecturalFramework::ResetFramework()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Resetting framework"));
    
    bFrameworkInitialized = false;
    RegisteredModules.Empty();
    ArchitecturalRules.Empty();
    LastMetrics = FEng_SystemMetrics();
    
    InitializeFramework();
}

void UEng_ArchitecturalFramework::UpdateSystemMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update frame time
    LastMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Update actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    LastMetrics.ActiveActors = AllActors.Num();
    
    // Count ticking components
    int32 TickingCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            TArray<UActorComponent*> Components = Actor->GetComponents().Array();
            for (UActorComponent* Component : Components)
            {
                if (Component && Component->PrimaryComponentTick.bCanEverTick)
                {
                    TickingCount++;
                }
            }
        }
    }
    LastMetrics.TickingComponents = TickingCount;
    
    // Update CPU and memory usage (simplified for now)
    LastMetrics.CPUUsage = FPlatformTime::GetSecondsPerCycle() * 100.0f;
    LastMetrics.MemoryUsage = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f * 1024.0f) * 100.0f;
}

void UEng_ArchitecturalFramework::CheckPerformanceLimits()
{
    if (CurrentProfile == EEng_PerformanceProfile::Development)
    {
        return; // No limits in development
    }
    
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Performance limits exceeded"));
        
        if (CurrentProfile == EEng_PerformanceProfile::Shipping)
        {
            // In shipping, we might want to take corrective action
            // For now, just log the issue
            UE_LOG(LogTemp, Error, TEXT("ArchitecturalFramework: Critical performance violation in shipping build"));
        }
    }
}

void UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    // Check that all module dependencies are satisfied
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        for (const FString& Dependency : Module.Dependencies)
        {
            if (!IsModuleRegistered(Dependency))
            {
                UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Module %s has unmet dependency: %s"), 
                       *Module.ModuleName, *Dependency);
            }
        }
    }
}

bool UEng_ArchitecturalFramework::IsModuleRegistered(const FString& ModuleName) const
{
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            return true;
        }
    }
    return false;
}

void UEng_ArchitecturalFramework::InitializeDefaultRules()
{
    // Performance rules
    FEng_ArchitecturalRule FrameTimeRule;
    FrameTimeRule.RuleName = TEXT("MaxFrameTime");
    FrameTimeRule.Description = TEXT("Maximum frame time should not exceed 33.33ms (30 FPS)");
    FrameTimeRule.bMandatory = true;
    FrameTimeRule.ViolationPenalty = 5.0f;
    AddArchitecturalRule(FrameTimeRule);
    
    FEng_ArchitecturalRule ActorLimitRule;
    ActorLimitRule.RuleName = TEXT("MaxActiveActors");
    ActorLimitRule.Description = TEXT("Maximum active actors should not exceed 10,000");
    ActorLimitRule.bMandatory = true;
    ActorLimitRule.ViolationPenalty = 3.0f;
    AddArchitecturalRule(ActorLimitRule);
    
    FEng_ArchitecturalRule MemoryRule;
    MemoryRule.RuleName = TEXT("MaxMemoryUsage");
    MemoryRule.Description = TEXT("Memory usage should not exceed 85%");
    MemoryRule.bMandatory = true;
    MemoryRule.ViolationPenalty = 4.0f;
    AddArchitecturalRule(MemoryRule);
}

// Constructor for AEng_ArchitecturalGameMode
AEng_ArchitecturalGameMode::AEng_ArchitecturalGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick every second
    
    // Create the architectural framework component
    ArchitecturalFramework = CreateDefaultSubobject<UEng_ArchitecturalFramework>(TEXT("ArchitecturalFramework"));
    bArchitectureValidated = false;
}

void AEng_ArchitecturalGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalGameMode: BeginPlay - Starting architectural validation"));
    
    if (ArchitecturalFramework)
    {
        // Framework will initialize itself in BeginPlay
        bArchitectureValidated = true;
    }
}

void AEng_ArchitecturalGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Periodic architecture validation
    if (ArchitecturalFramework && bArchitectureValidated)
    {
        if (!ArchitecturalFramework->IsPerformanceWithinLimits())
        {
            UE_LOG(LogTemp, Warning, TEXT("ArchitecturalGameMode: Performance outside limits"));
        }
    }
}

UEng_ArchitecturalFramework* AEng_ArchitecturalGameMode::GetArchitecturalFramework() const
{
    return ArchitecturalFramework;
}

bool AEng_ArchitecturalGameMode::IsSystemArchitectureValid() const
{
    if (!ArchitecturalFramework)
    {
        return false;
    }
    
    return ArchitecturalFramework->IsPerformanceWithinLimits() && bArchitectureValidated;
}

void AEng_ArchitecturalGameMode::ValidateAllSystems()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalGameMode: Validating all systems"));
    
    if (ArchitecturalFramework)
    {
        ArchitecturalFramework->ValidateSystemArchitecture();
        ArchitecturalFramework->GenerateArchitecturalReport();
    }
    
    bArchitectureValidated = IsSystemArchitectureValid();
}