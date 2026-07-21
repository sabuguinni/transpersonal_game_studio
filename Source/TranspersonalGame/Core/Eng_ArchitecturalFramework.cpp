#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    bFrameworkInitialized = false;
    CurrentProfile = EEng_PerformanceProfile::Development;
    MetricsUpdateInterval = 1.0f; // Update metrics every second
    LastMetricsUpdate = 0.0f;
}

void UEng_ArchitecturalFramework::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: BeginPlay - Initializing framework"));
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
        ValidateModuleDependencies();
        LastMetricsUpdate = 0.0f;
    }
}

void UEng_ArchitecturalFramework::InitializeFramework()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Initializing architectural framework"));
    
    // Clear existing data
    RegisteredModules.Empty();
    ArchitecturalRules.Empty();
    
    // Initialize default architectural rules
    InitializeDefaultRules();
    
    // Register core modules
    FEng_ModuleInfo CoreModule;
    CoreModule.ModuleName = TEXT("Core");
    CoreModule.Priority = EEng_SystemPriority::Critical;
    CoreModule.State = EEng_ModuleState::Active;
    RegisterModule(CoreModule);
    
    FEng_ModuleInfo PhysicsModule;
    PhysicsModule.ModuleName = TEXT("Physics");
    PhysicsModule.Priority = EEng_SystemPriority::Critical;
    PhysicsModule.State = EEng_ModuleState::Active;
    PhysicsModule.Dependencies.Add(TEXT("Core"));
    RegisterModule(PhysicsModule);
    
    FEng_ModuleInfo WorldGenModule;
    WorldGenModule.ModuleName = TEXT("WorldGeneration");
    WorldGenModule.Priority = EEng_SystemPriority::High;
    WorldGenModule.State = EEng_ModuleState::Initializing;
    WorldGenModule.Dependencies.Add(TEXT("Core"));
    WorldGenModule.Dependencies.Add(TEXT("Physics"));
    RegisterModule(WorldGenModule);
    
    bFrameworkInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Framework initialized with %d modules and %d rules"), 
           RegisteredModules.Num(), ArchitecturalRules.Num());
}

bool UEng_ArchitecturalFramework::RegisterModule(const FEng_ModuleInfo& ModuleInfo)
{
    if (IsModuleRegistered(ModuleInfo.ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Module %s already registered"), *ModuleInfo.ModuleName);
        return false;
    }
    
    FEng_ModuleInfo NewModule = ModuleInfo;
    NewModule.InitializationTime = FPlatformTime::Seconds();
    
    RegisteredModules.Add(NewModule);
    
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
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Failed to unregister module %s - not found"), *ModuleName);
    return false;
}

void UEng_ArchitecturalFramework::EnforceArchitecturalRules()
{
    if (!bFrameworkInitialized)
    {
        return;
    }
    
    int32 ViolationCount = 0;
    
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        // Check specific rules
        if (Rule.RuleName == TEXT("MaxActorCount"))
        {
            UWorld* World = GetWorld();
            if (World)
            {
                int32 ActorCount = World->GetActorCount();
                if (ActorCount > 10000) // Max 10k actors
                {
                    ViolationCount++;
                    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Rule violation - Too many actors: %d"), ActorCount);
                }
            }
        }
        else if (Rule.RuleName == TEXT("FrameTimeLimit"))
        {
            if (LastMetrics.FrameTime > 33.33f) // 30 FPS minimum
            {
                ViolationCount++;
                UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Rule violation - Frame time too high: %f ms"), LastMetrics.FrameTime);
            }
        }
    }
    
    if (ViolationCount > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitecturalFramework: %d architectural rule violations detected"), ViolationCount);
    }
}

FEng_SystemMetrics UEng_ArchitecturalFramework::GetCurrentMetrics() const
{
    return LastMetrics;
}

void UEng_ArchitecturalFramework::SetPerformanceProfile(EEng_PerformanceProfile Profile)
{
    CurrentProfile = Profile;
    
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
    
    UE_LOG(LogTemp, Log, TEXT("ArchitecturalFramework: Performance profile set to %d"), (int32)Profile);
}

bool UEng_ArchitecturalFramework::IsPerformanceWithinLimits() const
{
    switch (CurrentProfile)
    {
        case EEng_PerformanceProfile::Development:
            return true; // No limits in development
        case EEng_PerformanceProfile::Testing:
            return LastMetrics.FrameTime < 50.0f && LastMetrics.MemoryUsage < 80.0f;
        case EEng_PerformanceProfile::Shipping:
            return LastMetrics.FrameTime < 33.33f && LastMetrics.MemoryUsage < 70.0f;
        default:
            return false;
    }
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
            return;
        }
    }
}

TArray<FEng_ArchitecturalRule> UEng_ArchitecturalFramework::GetActiveRules() const
{
    return ArchitecturalRules;
}

void UEng_ArchitecturalFramework::ValidateSystemArchitecture()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Validating system architecture"));
    
    EnforceArchitecturalRules();
    ValidateModuleDependencies();
    
    int32 ActiveModules = 0;
    int32 ErrorModules = 0;
    
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        if (Module.State == EEng_ModuleState::Active)
        {
            ActiveModules++;
        }
        else if (Module.State == EEng_ModuleState::Error)
        {
            ErrorModules++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Validation complete - %d active modules, %d error modules"), 
           ActiveModules, ErrorModules);
}

void UEng_ArchitecturalFramework::GenerateArchitecturalReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== ARCHITECTURAL FRAMEWORK REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += FString::Printf(TEXT("Framework Initialized: %s\n"), bFrameworkInitialized ? TEXT("Yes") : TEXT("No"));
    ReportContent += FString::Printf(TEXT("Performance Profile: %d\n"), (int32)CurrentProfile);
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("=== REGISTERED MODULES ===\n");
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        ReportContent += FString::Printf(TEXT("Module: %s | Priority: %d | State: %d | Dependencies: %d\n"),
                                       *Module.ModuleName, (int32)Module.Priority, (int32)Module.State, Module.Dependencies.Num());
    }
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("=== ARCHITECTURAL RULES ===\n");
    for (const FEng_ArchitecturalRule& Rule : ArchitecturalRules)
    {
        ReportContent += FString::Printf(TEXT("Rule: %s | Mandatory: %s | Penalty: %.2f\n"),
                                       *Rule.RuleName, Rule.bMandatory ? TEXT("Yes") : TEXT("No"), Rule.ViolationPenalty);
    }
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("=== SYSTEM METRICS ===\n");
    ReportContent += FString::Printf(TEXT("Frame Time: %.2f ms\n"), LastMetrics.FrameTime);
    ReportContent += FString::Printf(TEXT("CPU Usage: %.2f%%\n"), LastMetrics.CPUUsage);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.2f%%\n"), LastMetrics.MemoryUsage);
    ReportContent += FString::Printf(TEXT("Active Actors: %d\n"), LastMetrics.ActiveActors);
    ReportContent += FString::Printf(TEXT("Ticking Components: %d\n"), LastMetrics.TickingComponents);
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *ReportContent);
}

void UEng_ArchitecturalFramework::ResetFramework()
{
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Resetting framework"));
    
    bFrameworkInitialized = false;
    RegisteredModules.Empty();
    ArchitecturalRules.Empty();
    LastMetricsUpdate = 0.0f;
    
    InitializeFramework();
}

void UEng_ArchitecturalFramework::UpdateSystemMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update frame time
    LastMetrics.FrameTime = FPlatformTime::ToMilliseconds(FPlatformTime::Cycles());
    
    // Update actor count
    LastMetrics.ActiveActors = World->GetActorCount();
    
    // Count ticking components
    LastMetrics.TickingComponents = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            LastMetrics.TickingComponents += Actor->GetRootComponent() ? 1 : 0;
        }
    }
    
    // Estimate CPU and memory usage (simplified)
    LastMetrics.CPUUsage = FMath::Clamp(LastMetrics.FrameTime / 33.33f * 100.0f, 0.0f, 100.0f);
    LastMetrics.MemoryUsage = FMath::Clamp(LastMetrics.ActiveActors / 10000.0f * 100.0f, 0.0f, 100.0f);
}

void UEng_ArchitecturalFramework::CheckPerformanceLimits()
{
    if (CurrentProfile == EEng_PerformanceProfile::Development)
    {
        return; // No limits in development
    }
    
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitecturalFramework: Performance limits exceeded - Frame: %.2f ms, Memory: %.2f%%"),
               LastMetrics.FrameTime, LastMetrics.MemoryUsage);
    }
}

void UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    for (const FEng_ModuleInfo& Module : RegisteredModules)
    {
        for (const FString& Dependency : Module.Dependencies)
        {
            if (!IsModuleRegistered(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("ArchitecturalFramework: Module %s has unregistered dependency %s"),
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
    // Max actor count rule
    FEng_ArchitecturalRule ActorRule;
    ActorRule.RuleName = TEXT("MaxActorCount");
    ActorRule.Description = TEXT("Maximum number of actors allowed in the world");
    ActorRule.bMandatory = true;
    ActorRule.ViolationPenalty = 5.0f;
    AddArchitecturalRule(ActorRule);
    
    // Frame time limit rule
    FEng_ArchitecturalRule FrameRule;
    FrameRule.RuleName = TEXT("FrameTimeLimit");
    FrameRule.Description = TEXT("Maximum frame time for smooth gameplay");
    FrameRule.bMandatory = true;
    FrameRule.ViolationPenalty = 10.0f;
    AddArchitecturalRule(FrameRule);
    
    // Memory usage rule
    FEng_ArchitecturalRule MemoryRule;
    MemoryRule.RuleName = TEXT("MemoryUsageLimit");
    MemoryRule.Description = TEXT("Maximum memory usage percentage");
    MemoryRule.bMandatory = false;
    MemoryRule.ViolationPenalty = 3.0f;
    AddArchitecturalRule(MemoryRule);
}

// AEng_ArchitecturalGameMode Implementation

AEng_ArchitecturalGameMode::AEng_ArchitecturalGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick every second
    
    // Create architectural framework component
    ArchitecturalFramework = CreateDefaultSubobject<UEng_ArchitecturalFramework>(TEXT("ArchitecturalFramework"));
    bArchitectureValidated = false;
}

void AEng_ArchitecturalGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalGameMode: BeginPlay - Starting architectural validation"));
    
    if (ArchitecturalFramework)
    {
        ArchitecturalFramework->InitializeFramework();
        bArchitectureValidated = true;
    }
}

void AEng_ArchitecturalGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (ArchitecturalFramework && bArchitectureValidated)
    {
        ArchitecturalFramework->EnforceArchitecturalRules();
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
    UE_LOG(LogTemp, Warning, TEXT("ArchitecturalGameMode: Validating all systems"));
    
    if (ArchitecturalFramework)
    {
        ArchitecturalFramework->ValidateSystemArchitecture();
        ArchitecturalFramework->GenerateArchitecturalReport();
    }
}