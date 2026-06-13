#include "Eng_CoreArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void UEng_CoreArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    TargetFPS = 60.0f;
    CurrentFPS = 0.0f;
    
    // Register core engine systems
    RegisterCoreSystem("CoreArchitecture", "1.0.0");
    RegisterCoreSystem("PhysicsSystem", "1.0.0");
    RegisterCoreSystem("SurvivalSystem", "1.0.0");
    RegisterCoreSystem("GameplaySystem", "1.0.0");
    
    // Set up module dependencies
    RegisterModuleDependency("PhysicsSystem", "CoreArchitecture");
    RegisterModuleDependency("SurvivalSystem", "CoreArchitecture");
    RegisterModuleDependency("GameplaySystem", "CoreArchitecture");
    RegisterModuleDependency("GameplaySystem", "SurvivalSystem");
    
    // Initialize system health tracking
    ReportSystemHealth("CoreArchitecture", true);
    ReportSystemHealth("PhysicsSystem", true);
    ReportSystemHealth("SurvivalSystem", true);
    ReportSystemHealth("GameplaySystem", true);
    
    UE_LOG(LogTemp, Warning, TEXT("Core Architecture System Initialized"));
}

void UEng_CoreArchitecture::Deinitialize()
{
    RegisteredSystems.Empty();
    ModuleDependencies.Empty();
    SystemHealthStatus.Empty();
    
    Super::Deinitialize();
}

void UEng_CoreArchitecture::RegisterCoreSystem(const FString& SystemName, const FString& SystemVersion)
{
    RegisteredSystems.Add(SystemName, SystemVersion);
    SystemHealthStatus.Add(SystemName, true);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered System: %s v%s"), *SystemName, *SystemVersion);
}

bool UEng_CoreArchitecture::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

void UEng_CoreArchitecture::SetPerformanceTarget(float InTargetFPS)
{
    TargetFPS = InTargetFPS;
    UE_LOG(LogTemp, Warning, TEXT("Performance Target Set: %.1f FPS"), TargetFPS);
}

float UEng_CoreArchitecture::GetCurrentFPS() const
{
    if (GEngine && GEngine->GetWorld())
    {
        return 1.0f / GEngine->GetWorld()->GetDeltaSeconds();
    }
    return 0.0f;
}

void UEng_CoreArchitecture::RegisterModuleDependency(const FString& ModuleName, const FString& DependsOn)
{
    if (!ModuleDependencies.Contains(ModuleName))
    {
        ModuleDependencies.Add(ModuleName, TArray<FString>());
    }
    
    ModuleDependencies[ModuleName].AddUnique(DependsOn);
    UE_LOG(LogTemp, Warning, TEXT("Module Dependency: %s depends on %s"), *ModuleName, *DependsOn);
}

TArray<FString> UEng_CoreArchitecture::GetModuleDependencies(const FString& ModuleName) const
{
    if (ModuleDependencies.Contains(ModuleName))
    {
        return ModuleDependencies[ModuleName];
    }
    return TArray<FString>();
}

void UEng_CoreArchitecture::ReportSystemHealth(const FString& SystemName, bool bIsHealthy)
{
    SystemHealthStatus.Add(SystemName, bIsHealthy);
    
    if (!bIsHealthy)
    {
        UE_LOG(LogTemp, Error, TEXT("System Health Issue: %s is unhealthy"), *SystemName);
    }
}

bool UEng_CoreArchitecture::GetSystemHealth(const FString& SystemName) const
{
    if (SystemHealthStatus.Contains(SystemName))
    {
        return SystemHealthStatus[SystemName];
    }
    return false;
}

void UEng_CoreArchitecture::UpdatePerformanceMetrics()
{
    CurrentFPS = GetCurrentFPS();
    
    if (CurrentFPS < TargetFPS * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: FPS %.1f below target %.1f"), CurrentFPS, TargetFPS);
    }
}

void UEng_CoreArchitecture::ValidateSystemDependencies()
{
    for (const auto& ModulePair : ModuleDependencies)
    {
        const FString& ModuleName = ModulePair.Key;
        const TArray<FString>& Dependencies = ModulePair.Value;
        
        for (const FString& Dependency : Dependencies)
        {
            if (!IsSystemRegistered(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("Dependency Error: %s requires %s but it's not registered"), *ModuleName, *Dependency);
                ReportSystemHealth(ModuleName, false);
            }
        }
    }
}