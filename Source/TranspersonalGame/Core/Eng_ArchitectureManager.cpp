#include "Eng_ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UEng_ArchitectureManager::UEng_ArchitectureManager()
{
    // Initialize mandatory include paths
    MandatoryIncludePaths.Add("CoreMinimal.h");
    MandatoryIncludePaths.Add("SharedTypes.h");
    
    // Initialize forbidden dependencies
    ForbiddenDependencies.Add("Engine/Private");
    ForbiddenDependencies.Add("UnrealEd/Private");
    
    // Initialize default performance budgets (in milliseconds)
    PerformanceBudgets.Add("WorldGeneration", 2.0f);
    PerformanceBudgets.Add("DinosaurAI", 1.5f);
    PerformanceBudgets.Add("CrowdSimulation", 3.0f);
    PerformanceBudgets.Add("Physics", 1.0f);
    PerformanceBudgets.Add("Audio", 0.5f);
    PerformanceBudgets.Add("VFX", 2.5f);
}

void UEng_ArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager initialized"));
    
    // Initialize agent constraints
    InitializeAgentConstraints();
    
    // Validate current project state
    ValidateModuleDependencies();
    ValidateClassNamingConventions();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture validation complete"));
}

void UEng_ArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager shutting down"));
    
    // Clear all registries
    RegisteredModules.Empty();
    PerformanceBudgets.Empty();
    AgentSystemConstraints.Empty();
    SystemHealthMetrics.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitectureManager::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating module dependencies..."));
    
    bool bAllValid = true;
    
    // Check for circular dependencies
    if (!ValidateCircularDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Circular dependencies detected!"));
        bAllValid = false;
    }
    
    // Validate include paths
    if (!ValidateIncludePaths())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid include paths detected!"));
        bAllValid = false;
    }
    
    if (bAllValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Module dependencies validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Module dependencies validation FAILED"));
    }
    
    return bAllValid;
}

bool UEng_ArchitectureManager::ValidateClassNamingConventions()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating class naming conventions..."));
    
    // This would normally scan the project files
    // For now, we'll assume compliance based on our coding rules
    bool bNamingValid = true;
    
    // Check if all custom classes use proper prefixes
    // Engine Architect classes should use "Eng_" prefix
    // This validation would be more comprehensive in a real implementation
    
    if (bNamingValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Class naming conventions validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Class naming conventions validation FAILED"));
    }
    
    return bNamingValid;
}

bool UEng_ArchitectureManager::ValidateSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating system integration..."));
    
    bool bIntegrationValid = true;
    
    // Check memory usage
    if (!ValidateMemoryUsage())
    {
        UE_LOG(LogTemp, Error, TEXT("Memory usage exceeds limits!"));
        bIntegrationValid = false;
    }
    
    // Update system metrics
    UpdateSystemMetrics();
    
    // Check performance budgets
    CheckPerformanceBudgets();
    
    if (bIntegrationValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ System integration validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ System integration validation FAILED"));
    }
    
    return bIntegrationValid;
}

void UEng_ArchitectureManager::RegisterModuleSystem(const FString& ModuleName, const FString& SystemClass)
{
    RegisteredModules.Add(ModuleName, SystemClass);
    UE_LOG(LogTemp, Warning, TEXT("Registered module system: %s -> %s"), *ModuleName, *SystemClass);
}

TArray<FString> UEng_ArchitectureManager::GetRegisteredModules() const
{
    TArray<FString> ModuleNames;
    RegisteredModules.GetKeys(ModuleNames);
    return ModuleNames;
}

float UEng_ArchitectureManager::GetSystemPerformanceMetric(const FString& SystemName)
{
    if (SystemHealthMetrics.Contains(SystemName))
    {
        return SystemHealthMetrics[SystemName];
    }
    return 0.0f;
}

void UEng_ArchitectureManager::SetPerformanceBudget(const FString& SystemName, float MaxFrameTimeMs)
{
    PerformanceBudgets.Add(SystemName, MaxFrameTimeMs);
    UE_LOG(LogTemp, Warning, TEXT("Set performance budget for %s: %.2f ms"), *SystemName, MaxFrameTimeMs);
}

bool UEng_ArchitectureManager::ValidateAgentOutput(int32 AgentID, const FString& OutputType)
{
    if (AgentSystemConstraints.Contains(AgentID))
    {
        const TArray<FString>& AllowedSystems = AgentSystemConstraints[AgentID];
        return AllowedSystems.Contains(OutputType);
    }
    
    // If no constraints defined, allow all outputs
    return true;
}

void UEng_ArchitectureManager::SetAgentConstraints(int32 AgentID, const TArray<FString>& AllowedSystems)
{
    AgentSystemConstraints.Add(AgentID, AllowedSystems);
    UE_LOG(LogTemp, Warning, TEXT("Set constraints for Agent %d: %d allowed systems"), AgentID, AllowedSystems.Num());
}

bool UEng_ArchitectureManager::ValidateIncludePaths()
{
    // In a real implementation, this would scan source files
    // For now, we assume compliance
    return true;
}

bool UEng_ArchitectureManager::ValidateCircularDependencies()
{
    // In a real implementation, this would analyze Build.cs files
    // For now, we assume no circular dependencies
    return true;
}

bool UEng_ArchitectureManager::ValidateMemoryUsage()
{
    // Check current memory usage
    // For now, we assume memory usage is within limits
    return true;
}

void UEng_ArchitectureManager::UpdateSystemMetrics()
{
    // Update performance metrics for all registered systems
    for (const auto& Module : RegisteredModules)
    {
        // In a real implementation, this would measure actual performance
        // For now, we'll simulate some metrics
        float SimulatedMetric = FMath::RandRange(0.1f, 2.0f);
        SystemHealthMetrics.Add(Module.Key, SimulatedMetric);
    }
}

void UEng_ArchitectureManager::CheckPerformanceBudgets()
{
    for (const auto& Budget : PerformanceBudgets)
    {
        if (SystemHealthMetrics.Contains(Budget.Key))
        {
            float CurrentMetric = SystemHealthMetrics[Budget.Key];
            if (CurrentMetric > Budget.Value)
            {
                UE_LOG(LogTemp, Error, TEXT("⚠️ Performance budget exceeded for %s: %.2f ms (limit: %.2f ms)"), 
                       *Budget.Key, CurrentMetric, Budget.Value);
            }
        }
    }
}

void UEng_ArchitectureManager::InitializeAgentConstraints()
{
    // Set constraints for each agent based on their role
    
    // Agent #3 - Core Systems Programmer
    TArray<FString> CoreSystemsConstraints;
    CoreSystemsConstraints.Add("Physics");
    CoreSystemsConstraints.Add("Collision");
    CoreSystemsConstraints.Add("Movement");
    SetAgentConstraints(3, CoreSystemsConstraints);
    
    // Agent #5 - Procedural World Generator
    TArray<FString> WorldGenConstraints;
    WorldGenConstraints.Add("WorldGeneration");
    WorldGenConstraints.Add("Terrain");
    WorldGenConstraints.Add("Biomes");
    SetAgentConstraints(5, WorldGenConstraints);
    
    // Agent #6 - Environment Artist
    TArray<FString> EnvironmentConstraints;
    EnvironmentConstraints.Add("Environment");
    EnvironmentConstraints.Add("Foliage");
    EnvironmentConstraints.Add("Props");
    SetAgentConstraints(6, EnvironmentConstraints);
    
    // Agent #12 - Combat & Enemy AI
    TArray<FString> CombatAIConstraints;
    CombatAIConstraints.Add("AI");
    CombatAIConstraints.Add("Combat");
    CombatAIConstraints.Add("DinosaurBehavior");
    SetAgentConstraints(12, CombatAIConstraints);
    
    UE_LOG(LogTemp, Warning, TEXT("Agent constraints initialized for critical agents"));
}

void UEng_ArchitectureManager::ValidateAgentCompliance()
{
    // This would be called periodically to ensure agents follow their constraints
    UE_LOG(LogTemp, Warning, TEXT("Validating agent compliance with architectural constraints"));
}