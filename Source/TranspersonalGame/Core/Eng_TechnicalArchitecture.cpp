#include "Eng_TechnicalArchitecture.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

// ========================================================================================
// UEng_TechnicalArchitectureSubsystem Implementation
// ========================================================================================

void UEng_TechnicalArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Technical Architecture Subsystem Initializing"));
    
    // Set default performance profile
    CurrentProfile = EEng_PerformanceProfile::PC_High;
    
    // Initialize performance budgets
    InitializePerformanceProfiles();
    
    // Initialize default architectural rules
    InitializeDefaultRules();
    
    // Clear violation tracking
    ViolatingAgents.Empty();
    BlockedSystems.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Technical Architecture Subsystem Ready"));
}

void UEng_TechnicalArchitectureSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Technical Architecture Subsystem Shutting Down"));
    
    // Generate final compliance report
    GenerateArchitectureReport();
    
    Super::Deinitialize();
}

void UEng_TechnicalArchitectureSubsystem::SetPerformanceProfile(EEng_PerformanceProfile Profile)
{
    CurrentProfile = Profile;
    
    // Update performance budget based on profile
    switch (Profile)
    {
        case EEng_PerformanceProfile::Development:
            PerformanceBudget.MaxFrameTimeMS = 33.33f; // 30 FPS, relaxed for debugging
            PerformanceBudget.MaxActiveActors = 10000;
            PerformanceBudget.MaxPhysicsBodies = 2000;
            PerformanceBudget.MaxParticleCount = 50000;
            PerformanceBudget.MaxMemoryMB = 16384.0f; // 16 GB
            break;
            
        case EEng_PerformanceProfile::Console:
            PerformanceBudget.MaxFrameTimeMS = 33.33f; // 30 FPS
            PerformanceBudget.MaxActiveActors = 3000;
            PerformanceBudget.MaxPhysicsBodies = 500;
            PerformanceBudget.MaxParticleCount = 5000;
            PerformanceBudget.MaxMemoryMB = 6144.0f; // 6 GB (console constraints)
            break;
            
        case EEng_PerformanceProfile::PC_High:
            PerformanceBudget.MaxFrameTimeMS = 16.67f; // 60 FPS
            PerformanceBudget.MaxActiveActors = 5000;
            PerformanceBudget.MaxPhysicsBodies = 1000;
            PerformanceBudget.MaxParticleCount = 10000;
            PerformanceBudget.MaxMemoryMB = 8192.0f; // 8 GB
            break;
            
        case EEng_PerformanceProfile::PC_Ultra:
            PerformanceBudget.MaxFrameTimeMS = 8.33f; // 120 FPS
            PerformanceBudget.MaxActiveActors = 7500;
            PerformanceBudget.MaxPhysicsBodies = 1500;
            PerformanceBudget.MaxParticleCount = 15000;
            PerformanceBudget.MaxMemoryMB = 12288.0f; // 12 GB
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Performance Profile Set: %s"), 
           *UEnum::GetValueAsString(Profile));
}

FEng_PerformanceBudget UEng_TechnicalArchitectureSubsystem::GetCurrentPerformanceBudget() const
{
    return PerformanceBudget;
}

bool UEng_TechnicalArchitectureSubsystem::ValidatePerformanceCompliance(const FString& SystemName, float FrameTimeMS, int32 ActorCount)
{
    bool bCompliant = true;
    
    // Check frame time compliance
    if (FrameTimeMS > PerformanceBudget.MaxFrameTimeMS)
    {
        LogArchitecturalViolation(SystemName, 
            FString::Printf(TEXT("Frame time violation: %.2fms exceeds budget %.2fms"), 
                          FrameTimeMS, PerformanceBudget.MaxFrameTimeMS));
        bCompliant = false;
    }
    
    // Check actor count compliance
    if (ActorCount > PerformanceBudget.MaxActiveActors)
    {
        LogArchitecturalViolation(SystemName, 
            FString::Printf(TEXT("Actor count violation: %d exceeds budget %d"), 
                          ActorCount, PerformanceBudget.MaxActiveActors));
        bCompliant = false;
    }
    
    return bCompliant;
}

bool UEng_TechnicalArchitectureSubsystem::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority, const FString& AgentOwner)
{
    if (SystemName.IsEmpty() || AgentOwner.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT - Invalid system registration: %s by %s"), 
               *SystemName, *AgentOwner);
        return false;
    }
    
    RegisteredSystems.Add(SystemName, Priority);
    SystemOwners.Add(SystemName, AgentOwner);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - System Registered: %s (Priority: %s) by Agent: %s"), 
           *SystemName, *UEnum::GetValueAsString(Priority), *AgentOwner);
    
    return true;
}

bool UEng_TechnicalArchitectureSubsystem::ValidateSystemArchitecture(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        LogArchitecturalViolation(SystemName, TEXT("System not registered with Technical Architecture"));
        return false;
    }
    
    return CheckSystemCompliance(SystemName);
}

void UEng_TechnicalArchitectureSubsystem::BlockSystemOperation(const FString& SystemName, const FString& Reason)
{
    BlockedSystems.AddUnique(SystemName);
    
    UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT - SYSTEM BLOCKED: %s - Reason: %s"), 
           *SystemName, *Reason);
    
    // Add owning agent to violations list
    if (SystemOwners.Contains(SystemName))
    {
        ViolatingAgents.AddUnique(SystemOwners[SystemName]);
    }
}

void UEng_TechnicalArchitectureSubsystem::AddArchitecturalRule(const FEng_ArchitecturalRule& Rule)
{
    ArchitecturalRules.Add(Rule);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Architectural Rule Added: %s"), *Rule.RuleName);
}

bool UEng_TechnicalArchitectureSubsystem::ValidateAgentCompliance(const FString& AgentName)
{
    bool bCompliant = true;
    
    // Check if agent has any blocked systems
    for (const auto& SystemPair : SystemOwners)
    {
        if (SystemPair.Value == AgentName && BlockedSystems.Contains(SystemPair.Key))
        {
            bCompliant = false;
            break;
        }
    }
    
    // Update violations list
    if (!bCompliant)
    {
        ViolatingAgents.AddUnique(AgentName);
    }
    else
    {
        ViolatingAgents.Remove(AgentName);
    }
    
    return bCompliant;
}

TArray<FString> UEng_TechnicalArchitectureSubsystem::GetViolatingAgents() const
{
    return ViolatingAgents;
}

float UEng_TechnicalArchitectureSubsystem::GetCurrentMemoryUsageMB() const
{
    // Calculate total allocated memory
    float TotalMemoryMB = 0.0f;
    for (const auto& AllocationPair : SystemMemoryAllocations)
    {
        TotalMemoryMB += AllocationPair.Value;
    }
    
    return TotalMemoryMB;
}

bool UEng_TechnicalArchitectureSubsystem::RequestMemoryAllocation(const FString& SystemName, float SizeMB)
{
    float CurrentUsage = GetCurrentMemoryUsageMB();
    
    if (CurrentUsage + SizeMB > PerformanceBudget.MaxMemoryMB)
    {
        LogArchitecturalViolation(SystemName, 
            FString::Printf(TEXT("Memory allocation denied: %.2fMB would exceed budget %.2fMB"), 
                          CurrentUsage + SizeMB, PerformanceBudget.MaxMemoryMB));
        return false;
    }
    
    SystemMemoryAllocations.Add(SystemName, SizeMB);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Memory Allocated: %s = %.2fMB (Total: %.2fMB)"), 
           *SystemName, SizeMB, CurrentUsage + SizeMB);
    
    return true;
}

void UEng_TechnicalArchitectureSubsystem::ReleaseMemoryAllocation(const FString& SystemName, float SizeMB)
{
    if (SystemMemoryAllocations.Contains(SystemName))
    {
        SystemMemoryAllocations.Remove(SystemName);
        
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Memory Released: %s = %.2fMB"), 
               *SystemName, SizeMB);
    }
}

bool UEng_TechnicalArchitectureSubsystem::ValidateThreadSafety(const FString& SystemName)
{
    // Basic thread safety validation
    // In a full implementation, this would check for proper synchronization
    
    if (!SystemConstraints.bRequireThreadSafety)
    {
        return true; // Thread safety not enforced
    }
    
    // For now, assume all registered systems are thread-safe
    return RegisteredSystems.Contains(SystemName);
}

void UEng_TechnicalArchitectureSubsystem::ReportThreadSafetyViolation(const FString& SystemName, const FString& Details)
{
    LogArchitecturalViolation(SystemName, 
        FString::Printf(TEXT("Thread Safety Violation: %s"), *Details));
    
    BlockSystemOperation(SystemName, TEXT("Thread safety violation detected"));
}

void UEng_TechnicalArchitectureSubsystem::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - ARCHITECTURE COMPLIANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Performance Profile: %s"), *UEnum::GetValueAsString(CurrentProfile));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Blocked Systems: %d"), BlockedSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Violating Agents: %d"), ViolatingAgents.Num());
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2fMB / %.2fMB"), 
           GetCurrentMemoryUsageMB(), PerformanceBudget.MaxMemoryMB);
    
    // List violating agents
    for (const FString& Agent : ViolatingAgents)
    {
        UE_LOG(LogTemp, Error, TEXT("VIOLATING AGENT: %s"), *Agent);
    }
    
    // List blocked systems
    for (const FString& System : BlockedSystems)
    {
        UE_LOG(LogTemp, Error, TEXT("BLOCKED SYSTEM: %s"), *System);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURE REPORT ==="));
}

void UEng_TechnicalArchitectureSubsystem::ValidateAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Validating All Systems..."));
    
    int32 ValidSystems = 0;
    int32 InvalidSystems = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (ValidateSystemArchitecture(SystemPair.Key))
        {
            ValidSystems++;
        }
        else
        {
            InvalidSystems++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - System Validation Complete: %d Valid, %d Invalid"), 
           ValidSystems, InvalidSystems);
}

void UEng_TechnicalArchitectureSubsystem::EnforceArchitecturalCompliance()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Enforcing Architectural Compliance..."));
    
    // Validate all agents
    TArray<FString> AllAgents;
    SystemOwners.GenerateValueArray(AllAgents);
    
    for (const FString& Agent : AllAgents)
    {
        ValidateAgentCompliance(Agent);
    }
    
    // Generate compliance report
    GenerateArchitectureReport();
}

void UEng_TechnicalArchitectureSubsystem::InitializeDefaultRules()
{
    // Rule 1: Performance Budget Compliance
    FEng_ArchitecturalRule PerformanceRule;
    PerformanceRule.RuleName = TEXT("Performance Budget Compliance");
    PerformanceRule.Description = TEXT("All systems must operate within defined performance budgets");
    PerformanceRule.Priority = EEng_SystemPriority::Critical;
    PerformanceRule.bMandatory = true;
    PerformanceRule.AffectedAgents = {TEXT("All Agents")};
    ArchitecturalRules.Add(PerformanceRule);
    
    // Rule 2: Modular Design Requirement
    FEng_ArchitecturalRule ModularRule;
    ModularRule.RuleName = TEXT("Modular Design Requirement");
    ModularRule.Description = TEXT("All systems must be modular and loosely coupled");
    ModularRule.Priority = EEng_SystemPriority::High;
    ModularRule.bMandatory = true;
    ModularRule.AffectedAgents = {TEXT("Agent #3"), TEXT("Agent #5"), TEXT("Agent #11"), TEXT("Agent #12")};
    ArchitecturalRules.Add(ModularRule);
    
    // Rule 3: Memory Pool Usage
    FEng_ArchitecturalRule MemoryRule;
    MemoryRule.RuleName = TEXT("Memory Pool Usage");
    MemoryRule.Description = TEXT("All dynamic allocations must use managed memory pools");
    MemoryRule.Priority = EEng_SystemPriority::High;
    MemoryRule.bMandatory = true;
    MemoryRule.AffectedAgents = {TEXT("Agent #3"), TEXT("Agent #13"), TEXT("Agent #17")};
    ArchitecturalRules.Add(MemoryRule);
    
    // Rule 4: Thread Safety
    FEng_ArchitecturalRule ThreadRule;
    ThreadRule.RuleName = TEXT("Thread Safety Requirement");
    ThreadRule.Description = TEXT("All shared data structures must be thread-safe");
    ThreadRule.Priority = EEng_SystemPriority::Critical;
    ThreadRule.bMandatory = true;
    ThreadRule.AffectedAgents = {TEXT("Agent #3"), TEXT("Agent #4"), TEXT("Agent #13")};
    ArchitecturalRules.Add(ThreadRule);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Default Architectural Rules Initialized: %d rules"), 
           ArchitecturalRules.Num());
}

void UEng_TechnicalArchitectureSubsystem::InitializePerformanceProfiles()
{
    // Set initial performance profile
    SetPerformanceProfile(CurrentProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Performance Profiles Initialized"));
}

bool UEng_TechnicalArchitectureSubsystem::CheckSystemCompliance(const FString& SystemName)
{
    // Check if system is blocked
    if (BlockedSystems.Contains(SystemName))
    {
        return false;
    }
    
    // Check memory allocation compliance
    if (SystemMemoryAllocations.Contains(SystemName))
    {
        float SystemMemory = SystemMemoryAllocations[SystemName];
        if (SystemMemory > PerformanceBudget.MaxMemoryMB * 0.1f) // No single system should use >10% of budget
        {
            LogArchitecturalViolation(SystemName, 
                FString::Printf(TEXT("Excessive memory usage: %.2fMB"), SystemMemory));
            return false;
        }
    }
    
    return true;
}

void UEng_TechnicalArchitectureSubsystem::LogArchitecturalViolation(const FString& SystemName, const FString& Violation)
{
    UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT - ARCHITECTURAL VIOLATION: %s - %s"), 
           *SystemName, *Violation);
    
    // Add to violations tracking
    if (SystemOwners.Contains(SystemName))
    {
        ViolatingAgents.AddUnique(SystemOwners[SystemName]);
    }
}

// ========================================================================================
// UEng_TechnicalArchitectureComponent Implementation
// ========================================================================================

UEng_TechnicalArchitectureComponent::UEng_TechnicalArchitectureComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms
    
    ActorPriority = EEng_SystemPriority::Medium;
    bEnforcePerformanceLimits = true;
    MaxFrameTimeMS = 16.67f; // 60 FPS default
    CurrentFrameTime = 0.0f;
    bIsCompliant = true;
}

void UEng_TechnicalArchitectureComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with Technical Architecture Subsystem
    if (UEng_TechnicalArchitectureSubsystem* ArchSubsystem = 
        GetWorld()->GetGameInstance()->GetSubsystem<UEng_TechnicalArchitectureSubsystem>())
    {
        FString ActorName = GetOwner()->GetName();
        ArchSubsystem->RegisterSystem(ActorName, ActorPriority, TEXT("Actor Component"));
        
        // Get current performance budget
        FEng_PerformanceBudget Budget = ArchSubsystem->GetCurrentPerformanceBudget();
        MaxFrameTimeMS = Budget.MaxFrameTimeMS;
    }
    
    ValidateActorCompliance();
}

void UEng_TechnicalArchitectureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnforcePerformanceLimits)
    {
        CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        CheckPerformanceCompliance();
    }
}

void UEng_TechnicalArchitectureComponent::ValidateActorCompliance()
{
    bIsCompliant = true;
    ComplianceViolations.Empty();
    
    // Validate component architecture
    ValidateComponentArchitecture();
    
    // Check performance compliance
    CheckPerformanceCompliance();
    
    if (!bIsCompliant)
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT - Actor %s is NOT compliant"), 
               *GetOwner()->GetName());
    }
}

bool UEng_TechnicalArchitectureComponent::IsActorCompliant() const
{
    return bIsCompliant;
}

void UEng_TechnicalArchitectureComponent::ReportPerformanceMetrics(float FrameTime, int32 ComponentCount)
{
    CurrentFrameTime = FrameTime;
    
    if (UEng_TechnicalArchitectureSubsystem* ArchSubsystem = 
        GetWorld()->GetGameInstance()->GetSubsystem<UEng_TechnicalArchitectureSubsystem>())
    {
        FString ActorName = GetOwner()->GetName();
        ArchSubsystem->ValidatePerformanceCompliance(ActorName, FrameTime, 1); // 1 actor
    }
}

void UEng_TechnicalArchitectureComponent::SetSystemPriority(EEng_SystemPriority Priority)
{
    ActorPriority = Priority;
    
    // Re-register with new priority
    if (UEng_TechnicalArchitectureSubsystem* ArchSubsystem = 
        GetWorld()->GetGameInstance()->GetSubsystem<UEng_TechnicalArchitectureSubsystem>())
    {
        FString ActorName = GetOwner()->GetName();
        ArchSubsystem->RegisterSystem(ActorName, ActorPriority, TEXT("Actor Component"));
    }
}

void UEng_TechnicalArchitectureComponent::CheckPerformanceCompliance()
{
    if (!bEnforcePerformanceLimits)
    {
        return;
    }
    
    if (CurrentFrameTime > MaxFrameTimeMS)
    {
        ReportViolation(FString::Printf(TEXT("Frame time violation: %.2fms > %.2fms"), 
                                      CurrentFrameTime, MaxFrameTimeMS));
    }
}

void UEng_TechnicalArchitectureComponent::ValidateComponentArchitecture()
{
    // Check component count
    TArray<UActorComponent*> Components = GetOwner()->GetRootComponent()->GetAttachChildren();
    if (Components.Num() > 50) // Arbitrary limit for demonstration
    {
        ReportViolation(FString::Printf(TEXT("Excessive component count: %d"), Components.Num()));
    }
}

void UEng_TechnicalArchitectureComponent::ReportViolation(const FString& Violation)
{
    bIsCompliant = false;
    ComplianceViolations.AddUnique(Violation);
    
    UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT - Actor Violation: %s - %s"), 
           *GetOwner()->GetName(), *Violation);
}