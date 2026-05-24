#include "Core_PhysicsArchitecturalCompliance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"

// Physics Architectural Compliance Subsystem Implementation

void UCore_PhysicsArchitecturalComplianceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Physics Architectural Compliance Subsystem: Initializing"));
    
    // Initialize default metrics
    CurrentMetrics = FCore_PhysicsComplianceMetrics();
    CurrentPerformanceProfile = EEng_PerformanceProfile::Development;
    
    // Get Technical Architecture subsystem reference
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        TechnicalArchitecture = GameInstance->GetSubsystem<UEng_TechnicalArchitectureSubsystem>();
        if (TechnicalArchitecture)
        {
            UE_LOG(LogTemp, Log, TEXT("Physics Compliance: Connected to Technical Architecture"));
            RegisterWithTechnicalArchitecture();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics Compliance: Technical Architecture subsystem not found"));
        }
    }
    
    // Start compliance validation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ComplianceValidationTimer,
            this,
            &UCore_PhysicsArchitecturalComplianceSubsystem::UpdateComplianceMetrics,
            1.0f, // Every second
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Architectural Compliance Subsystem: Initialized successfully"));
}

void UCore_PhysicsArchitecturalComplianceSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Architectural Compliance Subsystem: Deinitializing"));
    
    // Clear compliance timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ComplianceValidationTimer);
    }
    
    // Clear registered systems
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

bool UCore_PhysicsArchitecturalComplianceSubsystem::RegisterPhysicsSystem(const FString& SystemName, int32 Priority, float MaxFrameTime, int32 MaxBodies, bool bIsThreadSafe)
{
    UE_LOG(LogTemp, Log, TEXT("Physics Compliance: Registering system '%s' with priority %d"), *SystemName, Priority);
    
    FCore_PhysicsSystemRegistration Registration;
    Registration.SystemName = SystemName;
    Registration.SystemPriority = Priority;
    Registration.MaxFrameTime = MaxFrameTime;
    Registration.MaxPhysicsBodies = MaxBodies;
    Registration.bThreadSafe = bIsThreadSafe;
    Registration.bRegisteredWithArchitecture = false;
    
    // Register with Technical Architecture if available
    if (TechnicalArchitecture)
    {
        FEng_SystemRegistration ArchRegistration;
        ArchRegistration.SystemName = SystemName;
        ArchRegistration.AgentName = TEXT("Core Systems Programmer");
        ArchRegistration.Priority = Priority;
        ArchRegistration.bIsThreadSafe = bIsThreadSafe;
        ArchRegistration.MaxFrameTimeMs = MaxFrameTime;
        ArchRegistration.MaxMemoryMB = 100.0f; // Default physics memory budget
        
        if (TechnicalArchitecture->RegisterSystem(ArchRegistration))
        {
            Registration.bRegisteredWithArchitecture = true;
            UE_LOG(LogTemp, Log, TEXT("Physics Compliance: System '%s' registered with Technical Architecture"), *SystemName);
        }
    }
    
    RegisteredSystems.Add(SystemName, Registration);
    return true;
}

bool UCore_PhysicsArchitecturalComplianceSubsystem::UnregisterPhysicsSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        // Unregister from Technical Architecture
        if (TechnicalArchitecture)
        {
            TechnicalArchitecture->UnregisterSystem(SystemName);
        }
        
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("Physics Compliance: Unregistered system '%s'"), *SystemName);
        return true;
    }
    
    return false;
}

FCore_PhysicsComplianceMetrics UCore_PhysicsArchitecturalComplianceSubsystem::GetCurrentComplianceMetrics() const
{
    return CurrentMetrics;
}

bool UCore_PhysicsArchitecturalComplianceSubsystem::ValidatePhysicsCompliance()
{
    UpdateComplianceMetrics();
    
    CurrentMetrics.ComplianceViolations.Empty();
    bool bCompliant = true;
    
    // Get performance budget from Technical Architecture
    if (TechnicalArchitecture)
    {
        FEng_PerformanceBudget Budget = TechnicalArchitecture->GetPerformanceBudget();
        
        // Check frame time compliance
        if (CurrentMetrics.PhysicsFrameTime > Budget.MaxFrameTimeMs)
        {
            CurrentMetrics.ComplianceViolations.Add(
                FString::Printf(TEXT("Physics frame time %.2fms exceeds budget %.2fms"), 
                CurrentMetrics.PhysicsFrameTime, Budget.MaxFrameTimeMs)
            );
            bCompliant = false;
        }
        
        // Check physics bodies count
        if (CurrentMetrics.ActivePhysicsBodies > Budget.MaxPhysicsBodies)
        {
            CurrentMetrics.ComplianceViolations.Add(
                FString::Printf(TEXT("Physics bodies %d exceeds budget %d"), 
                CurrentMetrics.ActivePhysicsBodies, Budget.MaxPhysicsBodies)
            );
            bCompliant = false;
        }
        
        // Check memory usage
        if (CurrentMetrics.MemoryUsageMB > Budget.MaxMemoryMB)
        {
            CurrentMetrics.ComplianceViolations.Add(
                FString::Printf(TEXT("Physics memory %.2fMB exceeds budget %.2fMB"), 
                CurrentMetrics.MemoryUsageMB, Budget.MaxMemoryMB)
            );
            bCompliant = false;
        }
    }
    
    // Validate individual systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!IsSystemCompliant(SystemPair.Value))
        {
            CurrentMetrics.ComplianceViolations.Add(
                FString::Printf(TEXT("System '%s' is not compliant"), *SystemPair.Key)
            );
            bCompliant = false;
        }
    }
    
    CurrentMetrics.bWithinPerformanceBudget = bCompliant;
    
    // Report violations to Technical Architecture
    if (!bCompliant && TechnicalArchitecture)
    {
        for (const FString& Violation : CurrentMetrics.ComplianceViolations)
        {
            TechnicalArchitecture->ReportViolation(TEXT("Core Physics Systems"), TEXT("Performance"), Violation);
        }
    }
    
    return bCompliant;
}

TArray<FString> UCore_PhysicsArchitecturalComplianceSubsystem::GetComplianceViolations() const
{
    return CurrentMetrics.ComplianceViolations;
}

bool UCore_PhysicsArchitecturalComplianceSubsystem::IsWithinPerformanceBudget() const
{
    return CurrentMetrics.bWithinPerformanceBudget;
}

void UCore_PhysicsArchitecturalComplianceSubsystem::SetPerformanceProfile(EEng_PerformanceProfile Profile)
{
    CurrentPerformanceProfile = Profile;
    UE_LOG(LogTemp, Log, TEXT("Physics Compliance: Performance profile set to %d"), (int32)Profile);
}

bool UCore_PhysicsArchitecturalComplianceSubsystem::RegisterWithTechnicalArchitecture()
{
    if (!TechnicalArchitecture)
    {
        return false;
    }
    
    // Register the Physics Compliance system itself
    FEng_SystemRegistration Registration;
    Registration.SystemName = TEXT("Physics Architectural Compliance");
    Registration.AgentName = TEXT("Core Systems Programmer");
    Registration.Priority = 5; // High priority for compliance monitoring
    Registration.bIsThreadSafe = true;
    Registration.MaxFrameTimeMs = 1.0f; // Minimal overhead
    Registration.MaxMemoryMB = 10.0f; // Minimal memory usage
    
    return TechnicalArchitecture->RegisterSystem(Registration);
}

void UCore_PhysicsArchitecturalComplianceSubsystem::ReportComplianceViolation(const FString& SystemName, const FString& ViolationType, const FString& Details)
{
    UE_LOG(LogTemp, Warning, TEXT("Physics Compliance Violation - System: %s, Type: %s, Details: %s"), 
           *SystemName, *ViolationType, *Details);
    
    if (TechnicalArchitecture)
    {
        TechnicalArchitecture->ReportViolation(SystemName, ViolationType, Details);
    }
}

void UCore_PhysicsArchitecturalComplianceSubsystem::UpdateComplianceMetrics()
{
    // Update physics frame time (simplified - would need actual physics profiling)
    CurrentMetrics.PhysicsFrameTime = FPlatformTime::Seconds() * 1000.0f; // Convert to ms
    
    // Count active physics bodies in the world
    CurrentMetrics.ActivePhysicsBodies = 0;
    CurrentMetrics.CollisionChecksPerFrame = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.ActivePhysicsBodies++;
                    }
                }
            }
        }
    }
    
    // Update memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Validate compliance
    ValidatePhysicsCompliance();
}

void UCore_PhysicsArchitecturalComplianceSubsystem::ValidateSystemPerformance()
{
    // Implementation for system-specific performance validation
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FCore_PhysicsSystemRegistration& System = SystemPair.Value;
        
        // Check if system is within its performance limits
        if (CurrentMetrics.PhysicsFrameTime > System.MaxFrameTime)
        {
            ReportComplianceViolation(System.SystemName, TEXT("Performance"), 
                FString::Printf(TEXT("Frame time %.2fms exceeds system limit %.2fms"), 
                CurrentMetrics.PhysicsFrameTime, System.MaxFrameTime));
        }
    }
}

void UCore_PhysicsArchitecturalComplianceSubsystem::CheckMemoryUsage()
{
    // Memory usage validation logic
    if (TechnicalArchitecture)
    {
        FEng_PerformanceBudget Budget = TechnicalArchitecture->GetPerformanceBudget();
        if (CurrentMetrics.MemoryUsageMB > Budget.MaxMemoryMB * 0.8f) // 80% warning threshold
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics memory usage approaching limit: %.2fMB / %.2fMB"), 
                   CurrentMetrics.MemoryUsageMB, Budget.MaxMemoryMB);
        }
    }
}

void UCore_PhysicsArchitecturalComplianceSubsystem::ValidateThreadSafety()
{
    // Thread safety validation for registered systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!SystemPair.Value.bThreadSafe)
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics system '%s' is not thread-safe"), *SystemPair.Key);
        }
    }
}

bool UCore_PhysicsArchitecturalComplianceSubsystem::IsSystemCompliant(const FCore_PhysicsSystemRegistration& System) const
{
    // Check if system is registered with Technical Architecture
    if (!System.bRegisteredWithArchitecture)
    {
        return false;
    }
    
    // Check performance compliance
    if (CurrentMetrics.PhysicsFrameTime > System.MaxFrameTime)
    {
        return false;
    }
    
    // Check physics bodies limit
    if (CurrentMetrics.ActivePhysicsBodies > System.MaxPhysicsBodies)
    {
        return false;
    }
    
    return true;
}

// Physics Architectural Compliance Component Implementation

UCore_PhysicsArchitecturalComplianceComponent::UCore_PhysicsArchitecturalComplianceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for compliance checking
    
    bEnableComplianceMonitoring = true;
    ComplianceCheckInterval = 1.0f;
    bAutoReportViolations = true;
    
    MaxPhysicsFrameTime = 16.67f; // 60 FPS
    MaxCollisionChecks = 100;
    MaxMemoryUsageMB = 50.0f;
    
    bIsCompliant = true;
    CurrentViolations.Empty();
    
    LastFrameTime = 0.0f;
    LastCollisionCount = 0;
    LastMemoryUsage = 0.0f;
    ComplianceCheckTimer = 0.0f;
}

void UCore_PhysicsArchitecturalComplianceComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get compliance subsystem reference
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        ComplianceSubsystem = GameInstance->GetSubsystem<UCore_PhysicsArchitecturalComplianceSubsystem>();
        if (!ComplianceSubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics Compliance Component: Subsystem not found"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics Compliance Component: Started monitoring for actor %s"), 
           *GetOwner()->GetName());
}

void UCore_PhysicsArchitecturalComplianceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("Physics Compliance Component: Stopped monitoring for actor %s"), 
           *GetOwner()->GetName());
    
    Super::EndPlay(EndPlayReason);
}

void UCore_PhysicsArchitecturalComplianceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableComplianceMonitoring)
    {
        return;
    }
    
    ComplianceCheckTimer += DeltaTime;
    
    if (ComplianceCheckTimer >= ComplianceCheckInterval)
    {
        ValidateActorCompliance();
        ComplianceCheckTimer = 0.0f;
    }
}

bool UCore_PhysicsArchitecturalComplianceComponent::ValidateActorCompliance()
{
    CurrentViolations.Empty();
    bIsCompliant = true;
    
    // Check physics performance
    CheckPhysicsPerformance();
    
    // Check collision compliance
    CheckCollisionCompliance();
    
    // Check memory compliance
    CheckMemoryCompliance();
    
    // Update compliance status
    UpdateComplianceStatus();
    
    // Auto-report violations if enabled
    if (bAutoReportViolations && !bIsCompliant && ComplianceSubsystem)
    {
        for (const FString& Violation : CurrentViolations)
        {
            ComplianceSubsystem->ReportComplianceViolation(
                GetOwner()->GetName(), 
                TEXT("Actor Compliance"), 
                Violation
            );
        }
    }
    
    return bIsCompliant;
}

FCore_PhysicsComplianceMetrics UCore_PhysicsArchitecturalComplianceComponent::GetActorComplianceMetrics() const
{
    FCore_PhysicsComplianceMetrics Metrics;
    Metrics.PhysicsFrameTime = LastFrameTime;
    Metrics.CollisionChecksPerFrame = LastCollisionCount;
    Metrics.MemoryUsageMB = LastMemoryUsage;
    Metrics.bWithinPerformanceBudget = bIsCompliant;
    Metrics.ComplianceViolations = CurrentViolations;
    
    return Metrics;
}

void UCore_PhysicsArchitecturalComplianceComponent::ReportActorViolation(const FString& ViolationType, const FString& Details)
{
    CurrentViolations.Add(FString::Printf(TEXT("%s: %s"), *ViolationType, *Details));
    bIsCompliant = false;
    
    if (ComplianceSubsystem)
    {
        ComplianceSubsystem->ReportComplianceViolation(GetOwner()->GetName(), ViolationType, Details);
    }
}

void UCore_PhysicsArchitecturalComplianceComponent::SetComplianceLimits(float MaxFrameTime, int32 MaxCollisions, float MaxMemory)
{
    MaxPhysicsFrameTime = MaxFrameTime;
    MaxCollisionChecks = MaxCollisions;
    MaxMemoryUsageMB = MaxMemory;
    
    UE_LOG(LogTemp, Log, TEXT("Physics Compliance: Updated limits for actor %s - Frame: %.2fms, Collisions: %d, Memory: %.2fMB"),
           *GetOwner()->GetName(), MaxFrameTime, MaxCollisions, MaxMemory);
}

void UCore_PhysicsArchitecturalComplianceComponent::CheckPhysicsPerformance()
{
    // Simplified physics performance check
    float CurrentFrameTime = FPlatformTime::Seconds() * 1000.0f;
    LastFrameTime = CurrentFrameTime;
    
    if (LastFrameTime > MaxPhysicsFrameTime)
    {
        CurrentViolations.Add(FString::Printf(TEXT("Physics frame time %.2fms exceeds limit %.2fms"), 
                                            LastFrameTime, MaxPhysicsFrameTime));
        bIsCompliant = false;
    }
}

void UCore_PhysicsArchitecturalComplianceComponent::CheckCollisionCompliance()
{
    // Count collision components on this actor
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    GetOwner()->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    LastCollisionCount = PrimitiveComponents.Num();
    
    if (LastCollisionCount > MaxCollisionChecks)
    {
        CurrentViolations.Add(FString::Printf(TEXT("Collision checks %d exceeds limit %d"), 
                                            LastCollisionCount, MaxCollisionChecks));
        bIsCompliant = false;
    }
}

void UCore_PhysicsArchitecturalComplianceComponent::CheckMemoryCompliance()
{
    // Simplified memory usage estimation
    LastMemoryUsage = sizeof(*GetOwner()) / (1024.0f * 1024.0f); // Convert to MB
    
    if (LastMemoryUsage > MaxMemoryUsageMB)
    {
        CurrentViolations.Add(FString::Printf(TEXT("Memory usage %.2fMB exceeds limit %.2fMB"), 
                                            LastMemoryUsage, MaxMemoryUsageMB));
        bIsCompliant = false;
    }
}

void UCore_PhysicsArchitecturalComplianceComponent::UpdateComplianceStatus()
{
    bIsCompliant = CurrentViolations.Num() == 0;
    
    if (!bIsCompliant)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor %s has %d compliance violations"), 
               *GetOwner()->GetName(), CurrentViolations.Num());
    }
}