#include "ArchitectureCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "Misc/DateTime.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchitecture, Log, All);

void UArchitectureCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchitecture, Log, TEXT("Architecture Core System Initializing..."));
    
    // Initialize default world size
    WorldSizeKm = 1.0f;
    
    // Register core systems
    InitializeCoreSystems();
    
    UE_LOG(LogArchitecture, Log, TEXT("Architecture Core System Initialized"));
}

void UArchitectureCore::Deinitialize()
{
    UE_LOG(LogArchitecture, Log, TEXT("Architecture Core System Shutting Down..."));
    
    RegisteredSystems.Empty();
    SystemPerformanceData.Empty();
    
    Super::Deinitialize();
}

bool UArchitectureCore::RegisterSystem(const FSystemInfo& SystemInfo)
{
    if (SystemInfo.SystemName.IsEmpty())
    {
        UE_LOG(LogArchitecture, Error, TEXT("Cannot register system with empty name"));
        return false;
    }

    if (RegisteredSystems.Contains(SystemInfo.SystemName))
    {
        UE_LOG(LogArchitecture, Warning, TEXT("System '%s' already registered, updating..."), *SystemInfo.SystemName);
    }

    // Validate dependencies
    if (!ValidateSystemDependencies(SystemInfo.SystemName))
    {
        UE_LOG(LogArchitecture, Error, TEXT("System '%s' has invalid dependencies"), *SystemInfo.SystemName);
        return false;
    }

    RegisteredSystems.Add(SystemInfo.SystemName, SystemInfo);
    SystemPerformanceData.Add(SystemInfo.SystemName, 0.0f);

    UE_LOG(LogArchitecture, Log, TEXT("Registered system: %s [Priority: %d, Layer: %d]"), 
           *SystemInfo.SystemName, 
           (int32)SystemInfo.Priority, 
           (int32)SystemInfo.Layer);

    return true;
}

bool UArchitectureCore::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogArchitecture, Warning, TEXT("Cannot unregister unknown system: %s"), *SystemName);
        return false;
    }

    RegisteredSystems.Remove(SystemName);
    SystemPerformanceData.Remove(SystemName);

    UE_LOG(LogArchitecture, Log, TEXT("Unregistered system: %s"), *SystemName);
    return true;
}

FSystemInfo UArchitectureCore::GetSystemInfo(const FString& SystemName) const
{
    if (const FSystemInfo* Info = RegisteredSystems.Find(SystemName))
    {
        return *Info;
    }

    return FSystemInfo(); // Return default
}

TArray<FSystemInfo> UArchitectureCore::GetAllSystems() const
{
    TArray<FSystemInfo> Systems;
    RegisteredSystems.GenerateValueArray(Systems);
    return Systems;
}

void UArchitectureCore::SetPerformanceBudget(const FString& SystemName, float BudgetMs)
{
    if (FSystemInfo* Info = RegisteredSystems.Find(SystemName))
    {
        Info->PerformanceBudget = BudgetMs;
        UE_LOG(LogArchitecture, Log, TEXT("Set performance budget for %s: %.2fms"), *SystemName, BudgetMs);
    }
}

float UArchitectureCore::GetSystemPerformance(const FString& SystemName) const
{
    if (const float* Performance = SystemPerformanceData.Find(SystemName))
    {
        return *Performance;
    }
    return 0.0f;
}

bool UArchitectureCore::IsPerformanceWithinBudget(const FString& SystemName) const
{
    const FSystemInfo* Info = RegisteredSystems.Find(SystemName);
    const float* Performance = SystemPerformanceData.Find(SystemName);
    
    if (Info && Performance)
    {
        return *Performance <= Info->PerformanceBudget;
    }
    
    return true; // Assume OK if no data
}

bool UArchitectureCore::ValidateArchitecture() const
{
    TArray<FString> Violations = GetArchitectureViolations();
    return Violations.Num() == 0;
}

TArray<FString> UArchitectureCore::GetArchitectureViolations() const
{
    TArray<FString> Violations;

    // Check World Partition requirement
    if (IsWorldPartitionRequired())
    {
        // TODO: Check if World Partition is actually enabled
        // This would require access to the current world
        UE_LOG(LogArchitecture, Log, TEXT("World Partition validation required for %fkm² world"), WorldSizeKm);
    }

    // Check performance budgets
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        const FSystemInfo& Info = SystemPair.Value;

        if (!IsPerformanceWithinBudget(SystemName))
        {
            float CurrentPerf = GetSystemPerformance(SystemName);
            Violations.Add(FString::Printf(TEXT("System '%s' exceeds performance budget: %.2fms > %.2fms"), 
                                         *SystemName, CurrentPerf, Info.PerformanceBudget));
        }
    }

    // Check system dependencies
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        if (!ValidateSystemDependencies(SystemName))
        {
            Violations.Add(FString::Printf(TEXT("System '%s' has unresolved dependencies"), *SystemName));
        }
    }

    return Violations;
}

bool UArchitectureCore::IsWorldPartitionRequired() const
{
    return WorldSizeKm > WORLD_PARTITION_THRESHOLD;
}

void UArchitectureCore::SetWorldSize(float SizeKm)
{
    WorldSizeKm = SizeKm;
    
    if (IsWorldPartitionRequired())
    {
        UE_LOG(LogArchitecture, Warning, TEXT("World size %.2fkm² requires World Partition"), SizeKm);
    }
    
    UE_LOG(LogArchitecture, Log, TEXT("World size set to %.2fkm²"), SizeKm);
}

void UArchitectureCore::InitializeCoreSystems()
{
    // Register core engine systems
    FSystemInfo PhysicsSystem;
    PhysicsSystem.SystemName = TEXT("Physics");
    PhysicsSystem.Priority = ESystemPriority::Critical;
    PhysicsSystem.Layer = EArchitectureLayer::Engine;
    PhysicsSystem.PerformanceBudget = 3.0f; // 3ms budget
    RegisterSystem(PhysicsSystem);

    FSystemInfo RenderingSystem;
    RenderingSystem.SystemName = TEXT("Rendering");
    RenderingSystem.Priority = ESystemPriority::Critical;
    RenderingSystem.Layer = EArchitectureLayer::Engine;
    RenderingSystem.PerformanceBudget = 10.0f; // 10ms budget
    RegisterSystem(RenderingSystem);

    FSystemInfo AISystem;
    AISystem.SystemName = TEXT("AI");
    AISystem.Priority = ESystemPriority::High;
    AISystem.Layer = EArchitectureLayer::Gameplay;
    AISystem.PerformanceBudget = 2.0f; // 2ms budget
    AISystem.Dependencies.Add(TEXT("Physics"));
    RegisterSystem(AISystem);

    FSystemInfo AnimationSystem;
    AnimationSystem.SystemName = TEXT("Animation");
    AnimationSystem.Priority = ESystemPriority::High;
    AnimationSystem.Layer = EArchitectureLayer::Gameplay;
    AnimationSystem.PerformanceBudget = 2.5f; // 2.5ms budget
    AnimationSystem.Dependencies.Add(TEXT("Physics"));
    RegisterSystem(AnimationSystem);

    FSystemInfo AudioSystem;
    AudioSystem.SystemName = TEXT("Audio");
    AudioSystem.Priority = ESystemPriority::High;
    AudioSystem.Layer = EArchitectureLayer::Gameplay;
    AudioSystem.PerformanceBudget = 1.0f; // 1ms budget
    RegisterSystem(AudioSystem);

    UE_LOG(LogArchitecture, Log, TEXT("Initialized %d core systems"), RegisteredSystems.Num());
}

bool UArchitectureCore::ValidateSystemDependencies(const FString& SystemName) const
{
    const FSystemInfo* Info = RegisteredSystems.Find(SystemName);
    if (!Info)
    {
        return false;
    }

    // Check if all dependencies are registered
    for (const FString& Dependency : Info->Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            UE_LOG(LogArchitecture, Error, TEXT("System '%s' depends on unregistered system '%s'"), 
                   *SystemName, *Dependency);
            return false;
        }
    }

    return true;
}

void UArchitectureCore::UpdatePerformanceMetrics()
{
    // This would be called by a performance monitoring system
    // For now, just log that we're tracking performance
    static float LastUpdateTime = 0.0f;
    float CurrentTime = FPlatformTime::Seconds();
    
    if (CurrentTime - LastUpdateTime > 1.0f) // Update every second
    {
        UE_LOG(LogArchitecture, VeryVerbose, TEXT("Performance metrics updated for %d systems"), 
               SystemPerformanceData.Num());
        LastUpdateTime = CurrentTime;
    }
}

// Architecture Validation Component Implementation

UArchitectureValidationComponent::UArchitectureValidationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Validate once per second
    
    bEnforcePerformanceLimits = true;
    MaxRenderComplexity = 1000.0f;
    MaxTriangleCount = 10000;
}

void UArchitectureValidationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Perform initial validation
    if (!ValidateActor())
    {
        TArray<FString> Errors = GetValidationErrors();
        for (const FString& Error : Errors)
        {
            UE_LOG(LogArchitecture, Warning, TEXT("Validation Error on %s: %s"), 
                   *GetOwner()->GetName(), *Error);
        }
    }
}

void UArchitectureValidationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnforcePerformanceLimits)
    {
        // Periodic validation
        ValidateActor();
    }
}

bool UArchitectureValidationComponent::ValidateActor() const
{
    if (!GetOwner())
    {
        return false;
    }

    bool bIsValid = true;

    bIsValid &= ValidateRenderComplexity();
    bIsValid &= ValidateTriangleCount();
    bIsValid &= ValidateComponentStructure();

    return bIsValid;
}

TArray<FString> UArchitectureValidationComponent::GetValidationErrors() const
{
    TArray<FString> Errors;

    if (!ValidateRenderComplexity())
    {
        Errors.Add(FString::Printf(TEXT("Render complexity exceeds limit: %.1f"), MaxRenderComplexity));
    }

    if (!ValidateTriangleCount())
    {
        Errors.Add(FString::Printf(TEXT("Triangle count exceeds limit: %d"), MaxTriangleCount));
    }

    if (!ValidateComponentStructure())
    {
        Errors.Add(TEXT("Component structure violates architecture guidelines"));
    }

    return Errors;
}

bool UArchitectureValidationComponent::ValidateRenderComplexity() const
{
    // TODO: Implement actual render complexity calculation
    // This would analyze mesh complexity, material complexity, etc.
    return true;
}

bool UArchitectureValidationComponent::ValidateTriangleCount() const
{
    if (!GetOwner())
    {
        return false;
    }

    int32 TotalTriangles = 0;
    
    // Count triangles from all mesh components
    TArray<UMeshComponent*> MeshComponents;
    GetOwner()->GetComponents<UMeshComponent>(MeshComponents);
    
    for (UMeshComponent* MeshComp : MeshComponents)
    {
        if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(MeshComp))
        {
            if (UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh())
            {
                if (StaticMesh->GetRenderData() && StaticMesh->GetRenderData()->LODResources.Num() > 0)
                {
                    TotalTriangles += StaticMesh->GetRenderData()->LODResources[0].GetNumTriangles();
                }
            }
        }
    }

    return TotalTriangles <= MaxTriangleCount;
}

bool UArchitectureValidationComponent::ValidateComponentStructure() const
{
    if (!GetOwner())
    {
        return false;
    }

    // Check for common architecture violations
    TArray<UActorComponent*> Components = GetOwner()->GetRootComponent()->GetAttachChildren();
    
    // Example rule: No more than 10 components per actor (performance guideline)
    if (Components.Num() > 10)
    {
        return false;
    }

    return true;
}