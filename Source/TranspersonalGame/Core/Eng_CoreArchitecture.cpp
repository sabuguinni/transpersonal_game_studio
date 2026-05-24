#include "Eng_CoreArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngineArchitect, Log, All);

UEng_CoreArchitectureSubsystem::UEng_CoreArchitectureSubsystem()
{
    // Initialize default performance limits
    MaxFrameTime = 33.33f; // 30 FPS minimum
    MaxCPUUsage = 80.0f;
    MaxMemoryUsage = 4096.0f; // 4GB
    MaxActiveActors = 10000;
}

void UEng_CoreArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Engine Architecture Subsystem Initialized"));
    
    // Register core modules by default
    FEng_ModuleInfo CoreModule;
    CoreModule.ModuleName = TEXT("CoreSystems");
    CoreModule.ModuleType = EEng_ModuleType::Physics;
    CoreModule.Layer = EEng_ArchitecturalLayer::Core;
    CoreModule.bIsActive = true;
    CoreModule.PerformanceWeight = 1.0f;
    RegisterModule(CoreModule);
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
}

void UEng_CoreArchitectureSubsystem::Deinitialize()
{
    UE_LOG(LogEngineArchitect, Log, TEXT("Engine Architecture Subsystem Deinitialized"));
    RegisteredModules.Empty();
    Super::Deinitialize();
}

void UEng_CoreArchitectureSubsystem::RegisterModule(const FEng_ModuleInfo& ModuleInfo)
{
    if (ModuleInfo.ModuleName.IsEmpty())
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Cannot register module with empty name"));
        return;
    }
    
    // Check if module already exists
    if (RegisteredModules.Contains(ModuleInfo.ModuleName))
    {
        UE_LOG(LogEngineArchitect, Warning, TEXT("Module %s already registered, updating..."), *ModuleInfo.ModuleName);
    }
    
    // Validate dependencies
    bool bDependenciesValid = true;
    for (const FString& Dependency : ModuleInfo.Dependencies)
    {
        if (!RegisteredModules.Contains(Dependency))
        {
            UE_LOG(LogEngineArchitect, Error, TEXT("Module %s has unmet dependency: %s"), *ModuleInfo.ModuleName, *Dependency);
            bDependenciesValid = false;
        }
    }
    
    RegisteredModules.Add(ModuleInfo.ModuleName, ModuleInfo);
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Registered module: %s (Type: %d, Layer: %d)"), 
           *ModuleInfo.ModuleName, 
           (int32)ModuleInfo.ModuleType, 
           (int32)ModuleInfo.Layer);
    
    OnModuleRegistered.Broadcast(ModuleInfo.ModuleName, bDependenciesValid);
}

void UEng_CoreArchitectureSubsystem::UnregisterModule(const FString& ModuleName)
{
    if (RegisteredModules.Contains(ModuleName))
    {
        RegisteredModules.Remove(ModuleName);
        UE_LOG(LogEngineArchitect, Log, TEXT("Unregistered module: %s"), *ModuleName);
    }
}

bool UEng_CoreArchitectureSubsystem::ValidateModuleDependencies(const FString& ModuleName)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        return false;
    }
    
    const FEng_ModuleInfo& ModuleInfo = RegisteredModules[ModuleName];
    
    for (const FString& Dependency : ModuleInfo.Dependencies)
    {
        if (!RegisteredModules.Contains(Dependency))
        {
            UE_LOG(LogEngineArchitect, Error, TEXT("Module %s missing dependency: %s"), *ModuleName, *Dependency);
            return false;
        }
        
        // Check if dependency is active
        const FEng_ModuleInfo& DepModule = RegisteredModules[Dependency];
        if (!DepModule.bIsActive)
        {
            UE_LOG(LogEngineArchitect, Warning, TEXT("Module %s depends on inactive module: %s"), *ModuleName, *Dependency);
        }
    }
    
    return true;
}

void UEng_CoreArchitectureSubsystem::EnforcePerformanceLimits()
{
    UpdatePerformanceMetrics();
    
    if (CurrentMetrics.FrameTime > MaxFrameTime)
    {
        FString ViolationMsg = FString::Printf(TEXT("Frame time exceeded: %.2fms > %.2fms"), 
                                               CurrentMetrics.FrameTime, MaxFrameTime);
        OnArchitecturalViolation.Broadcast(ViolationMsg);
        UE_LOG(LogEngineArchitect, Warning, TEXT("%s"), *ViolationMsg);
    }
    
    if (CurrentMetrics.ActiveActors > MaxActiveActors)
    {
        FString ViolationMsg = FString::Printf(TEXT("Too many active actors: %d > %d"), 
                                               CurrentMetrics.ActiveActors, MaxActiveActors);
        OnArchitecturalViolation.Broadcast(ViolationMsg);
        UE_LOG(LogEngineArchitect, Warning, TEXT("%s"), *ViolationMsg);
    }
}

FEng_PerformanceMetrics UEng_CoreArchitectureSubsystem::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

TArray<FEng_ModuleInfo> UEng_CoreArchitectureSubsystem::GetRegisteredModules()
{
    TArray<FEng_ModuleInfo> Modules;
    for (const auto& ModulePair : RegisteredModules)
    {
        Modules.Add(ModulePair.Value);
    }
    return Modules;
}

void UEng_CoreArchitectureSubsystem::ValidateArchitecturalCompliance()
{
    UE_LOG(LogEngineArchitect, Log, TEXT("Running architectural compliance validation..."));
    
    // Check layer dependencies
    for (int32 LayerIndex = 0; LayerIndex < (int32)EEng_ArchitecturalLayer::Platform; LayerIndex++)
    {
        EEng_ArchitecturalLayer Layer = (EEng_ArchitecturalLayer)LayerIndex;
        if (!ValidateLayerDependencies(Layer))
        {
            FString ViolationMsg = FString::Printf(TEXT("Layer dependency violation in layer %d"), LayerIndex);
            OnArchitecturalViolation.Broadcast(ViolationMsg);
        }
    }
    
    // Check module dependencies
    for (const auto& ModulePair : RegisteredModules)
    {
        if (!ValidateModuleDependencies(ModulePair.Key))
        {
            FString ViolationMsg = FString::Printf(TEXT("Module dependency violation: %s"), *ModulePair.Key);
            OnArchitecturalViolation.Broadcast(ViolationMsg);
        }
    }
    
    // Enforce performance limits
    EnforcePerformanceLimits();
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Architectural compliance validation complete"));
}

void UEng_CoreArchitectureSubsystem::UpdatePerformanceMetrics()
{
    // Get frame time from engine stats
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Get world and count actors
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActors = World->GetActorCount();
        
        // Count physics bodies (simplified)
        CurrentMetrics.PhysicsBodies = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.PhysicsBodies++;
                    }
                }
            }
        }
    }
    
    // Memory usage (simplified - platform specific implementations would be more accurate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
    
    // CPU usage would require platform-specific implementation
    CurrentMetrics.CPUUsage = 0.0f; // Placeholder
}

void UEng_CoreArchitectureSubsystem::CheckArchitecturalViolations()
{
    // This would contain more sophisticated violation detection
    // For now, just check basic performance metrics
    EnforcePerformanceLimits();
}

bool UEng_CoreArchitectureSubsystem::ValidateLayerDependencies(EEng_ArchitecturalLayer Layer)
{
    // Core architectural rule: Higher layers cannot depend on lower layers
    // Core -> Gameplay -> Content -> Presentation -> Platform
    
    for (const auto& ModulePair : RegisteredModules)
    {
        const FEng_ModuleInfo& Module = ModulePair.Value;
        if (Module.Layer == Layer)
        {
            for (const FString& DepName : Module.Dependencies)
            {
                if (RegisteredModules.Contains(DepName))
                {
                    const FEng_ModuleInfo& DepModule = RegisteredModules[DepName];
                    if ((int32)DepModule.Layer > (int32)Layer)
                    {
                        UE_LOG(LogEngineArchitect, Error, TEXT("Layer violation: %s (Layer %d) depends on %s (Layer %d)"), 
                               *Module.ModuleName, (int32)Layer, *DepName, (int32)DepModule.Layer);
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

// Architectural Compliance Component Implementation
UEng_ArchitecturalComplianceComponent::UEng_ArchitecturalComplianceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    AssignedModule = EEng_ModuleType::Physics;
    PerformanceBudget = 1.0f;
    bEnforceStrictCompliance = true;
    LastValidationTime = 0.0f;
}

void UEng_ArchitecturalComplianceComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogEngineArchitect, Log, TEXT("Architectural Compliance Component started on %s"), 
           *GetOwner()->GetName());
    
    ValidateActorCompliance();
}

void UEng_ArchitecturalComplianceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastValidationTime > 5.0f) // Validate every 5 seconds
    {
        ValidateActorCompliance();
        LastValidationTime = CurrentTime;
    }
}

void UEng_ArchitecturalComplianceComponent::ValidateActorCompliance()
{
    if (!GetOwner())
    {
        return;
    }
    
    ValidateNamingConventions();
    ValidateComponentStructure();
    ValidatePerformanceImpact();
}

bool UEng_ArchitecturalComplianceComponent::CheckPerformanceCompliance()
{
    // Simple performance check - more sophisticated metrics could be added
    if (UEng_CoreArchitectureSubsystem* ArchSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UEng_CoreArchitectureSubsystem>())
    {
        FEng_PerformanceMetrics Metrics = ArchSubsystem->GetCurrentPerformanceMetrics();
        
        if (Metrics.FrameTime > 33.33f) // 30 FPS threshold
        {
            ReportViolation(TEXT("Performance"), TEXT("Frame time exceeds 30 FPS threshold"));
            return false;
        }
    }
    
    return true;
}

void UEng_ArchitecturalComplianceComponent::ReportViolation(const FString& ViolationType, const FString& Description)
{
    FString ViolationMessage = FString::Printf(TEXT("[%s] %s: %s"), 
                                               *GetOwner()->GetName(), 
                                               *ViolationType, 
                                               *Description);
    
    ViolationHistory.Add(ViolationMessage);
    
    UE_LOG(LogEngineArchitect, Warning, TEXT("Architectural Violation: %s"), *ViolationMessage);
    
    // Broadcast to architecture subsystem
    if (UEng_CoreArchitectureSubsystem* ArchSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UEng_CoreArchitectureSubsystem>())
    {
        ArchSubsystem->OnArchitecturalViolation.Broadcast(ViolationMessage);
    }
}

void UEng_ArchitecturalComplianceComponent::ValidateNamingConventions()
{
    FString ActorName = GetOwner()->GetName();
    
    // Check for proper prefixing based on module type
    bool bValidPrefix = false;
    switch (AssignedModule)
    {
        case EEng_ModuleType::Physics:
            bValidPrefix = ActorName.StartsWith(TEXT("Phys_")) || ActorName.StartsWith(TEXT("Core_"));
            break;
        case EEng_ModuleType::Character:
            bValidPrefix = ActorName.StartsWith(TEXT("Char_")) || ActorName.StartsWith(TEXT("Player_"));
            break;
        case EEng_ModuleType::AI:
            bValidPrefix = ActorName.StartsWith(TEXT("AI_")) || ActorName.StartsWith(TEXT("NPC_"));
            break;
        default:
            bValidPrefix = true; // Allow other modules for now
            break;
    }
    
    if (!bValidPrefix && bEnforceStrictCompliance)
    {
        ReportViolation(TEXT("Naming"), TEXT("Actor name does not follow module naming conventions"));
    }
}

void UEng_ArchitecturalComplianceComponent::ValidateComponentStructure()
{
    // Check component count and types
    TArray<UActorComponent*> Components = GetOwner()->GetComponents().Array();
    
    if (Components.Num() > 20) // Arbitrary limit for demonstration
    {
        ReportViolation(TEXT("Structure"), TEXT("Actor has too many components (>20)"));
    }
    
    // Check for required components based on module type
    if (AssignedModule == EEng_ModuleType::Physics)
    {
        bool bHasPhysicsComponent = false;
        for (UActorComponent* Component : Components)
        {
            if (Cast<UPrimitiveComponent>(Component))
            {
                bHasPhysicsComponent = true;
                break;
            }
        }
        
        if (!bHasPhysicsComponent)
        {
            ReportViolation(TEXT("Structure"), TEXT("Physics module actor missing primitive component"));
        }
    }
}

void UEng_ArchitecturalComplianceComponent::ValidatePerformanceImpact()
{
    // Simple performance validation
    if (!CheckPerformanceCompliance())
    {
        ReportViolation(TEXT("Performance"), TEXT("Actor contributing to performance issues"));
    }
    
    // Check tick frequency
    if (PrimaryComponentTick.TickInterval < 0.1f && bEnforceStrictCompliance)
    {
        ReportViolation(TEXT("Performance"), TEXT("Component ticking too frequently (<0.1s)"));
    }
}