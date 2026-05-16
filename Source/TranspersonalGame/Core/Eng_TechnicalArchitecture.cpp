#include "Eng_TechnicalArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"

UEng_TechnicalArchitecture::UEng_TechnicalArchitecture()
{
    ProjectVersion = TEXT("1.0.0");
    WorldSizeKm = 10.0f;
    bWorldPartitionEnabled = true;
    MaxConcurrentActors = 50000;
}

void UEng_TechnicalArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architecture Manager initialized"));
    
    InitializeDefaultModules();
    InitializeDefaultPerformanceRules();
    InitializeDefaultBiomeArchitectures();
    ValidateWorldPartitionRequirements();
}

void UEng_TechnicalArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Technical Architecture Manager deinitialized"));
    Super::Deinitialize();
}

void UEng_TechnicalArchitecture::InitializeDefaultModules()
{
    // Core Module
    FEng_ModuleSpec CoreModule;
    CoreModule.ModuleType = EEng_ModuleType::Core;
    CoreModule.ModuleName = TEXT("TranspersonalGame Core");
    CoreModule.MemoryBudget = EEng_MemoryBudget::Critical;
    CoreModule.MaxActorsPerBiome = 100;
    CoreModule.TargetFrameTime = 16.67f;
    CoreModule.bRequiresWorldPartition = false;
    CoreModule.AllowedAssetTypes = {TEXT("Blueprint"), TEXT("C++"), TEXT("DataTable")};
    ModuleSpecifications.Add(CoreModule);

    // World Generation Module
    FEng_ModuleSpec WorldGenModule;
    WorldGenModule.ModuleType = EEng_ModuleType::WorldGeneration;
    WorldGenModule.ModuleName = TEXT("Procedural World Generator");
    WorldGenModule.Dependencies = {TEXT("Core")};
    WorldGenModule.MemoryBudget = EEng_MemoryBudget::High;
    WorldGenModule.MaxActorsPerBiome = 2000;
    WorldGenModule.TargetFrameTime = 16.67f;
    WorldGenModule.bRequiresWorldPartition = true;
    WorldGenModule.AllowedAssetTypes = {TEXT("Landscape"), TEXT("StaticMesh"), TEXT("Material")};
    ModuleSpecifications.Add(WorldGenModule);

    // Environment Module
    FEng_ModuleSpec EnvironmentModule;
    EnvironmentModule.ModuleType = EEng_ModuleType::Environment;
    EnvironmentModule.ModuleName = TEXT("Environment Art System");
    EnvironmentModule.Dependencies = {TEXT("Core"), TEXT("WorldGeneration")};
    EnvironmentModule.MemoryBudget = EEng_MemoryBudget::High;
    EnvironmentModule.MaxActorsPerBiome = 5000;
    EnvironmentModule.TargetFrameTime = 16.67f;
    EnvironmentModule.bRequiresWorldPartition = true;
    EnvironmentModule.AllowedAssetTypes = {TEXT("StaticMesh"), TEXT("Material"), TEXT("Texture"), TEXT("Foliage")};
    ModuleSpecifications.Add(EnvironmentModule);

    // Character Module
    FEng_ModuleSpec CharacterModule;
    CharacterModule.ModuleType = EEng_ModuleType::Character;
    CharacterModule.ModuleName = TEXT("Character System");
    CharacterModule.Dependencies = {TEXT("Core")};
    CharacterModule.MemoryBudget = EEng_MemoryBudget::Medium;
    CharacterModule.MaxActorsPerBiome = 50;
    CharacterModule.TargetFrameTime = 16.67f;
    CharacterModule.bRequiresWorldPartition = false;
    CharacterModule.AllowedAssetTypes = {TEXT("SkeletalMesh"), TEXT("Animation"), TEXT("Blueprint")};
    ModuleSpecifications.Add(CharacterModule);

    // AI Module
    FEng_ModuleSpec AIModule;
    AIModule.ModuleType = EEng_ModuleType::AI;
    AIModule.ModuleName = TEXT("AI Behavior System");
    AIModule.Dependencies = {TEXT("Core"), TEXT("Character")};
    AIModule.MemoryBudget = EEng_MemoryBudget::Medium;
    AIModule.MaxActorsPerBiome = 200;
    AIModule.TargetFrameTime = 16.67f;
    AIModule.bRequiresWorldPartition = false;
    AIModule.AllowedAssetTypes = {TEXT("BehaviorTree"), TEXT("Blackboard"), TEXT("Blueprint")};
    ModuleSpecifications.Add(AIModule);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default module specifications"), ModuleSpecifications.Num());
}

void UEng_TechnicalArchitecture::InitializeDefaultPerformanceRules()
{
    // Environment Performance Rule
    FEng_PerformanceRule EnvironmentRule;
    EnvironmentRule.RuleName = TEXT("Environment Asset Performance");
    EnvironmentRule.ApplicableModule = EEng_ModuleType::Environment;
    EnvironmentRule.MaxTriangles = 5000;
    EnvironmentRule.MaxTextureSize = 2048;
    EnvironmentRule.MaxDrawDistance = 10000.0f;
    EnvironmentRule.bRequiresLOD = true;
    EnvironmentRule.LODLevels = 4;
    PerformanceRules.Add(EnvironmentRule);

    // Character Performance Rule
    FEng_PerformanceRule CharacterRule;
    CharacterRule.RuleName = TEXT("Character Asset Performance");
    CharacterRule.ApplicableModule = EEng_ModuleType::Character;
    CharacterRule.MaxTriangles = 15000;
    CharacterRule.MaxTextureSize = 4096;
    CharacterRule.MaxDrawDistance = 5000.0f;
    CharacterRule.bRequiresLOD = true;
    CharacterRule.LODLevels = 3;
    PerformanceRules.Add(CharacterRule);

    // AI Performance Rule
    FEng_PerformanceRule AIRule;
    AIRule.RuleName = TEXT("AI System Performance");
    AIRule.ApplicableModule = EEng_ModuleType::AI;
    AIRule.MaxTriangles = 1000;
    AIRule.MaxTextureSize = 512;
    AIRule.MaxDrawDistance = 2000.0f;
    AIRule.bRequiresLOD = false;
    AIRule.LODLevels = 1;
    PerformanceRules.Add(AIRule);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default performance rules"), PerformanceRules.Num());
}

void UEng_TechnicalArchitecture::InitializeDefaultBiomeArchitectures()
{
    // Savanna Biome
    FEng_BiomeArchitecture SavannaBiome;
    SavannaBiome.BiomeType = EBiomeType::Savanna;
    SavannaBiome.WorldCoordinates = FVector(0.0f, 0.0f, 0.0f);
    SavannaBiome.BiomeRadius = 200000.0f;
    SavannaBiome.MinActorCount = 500;
    SavannaBiome.MaxActorCount = 2000;
    SavannaBiome.AllowedAssetCategories = {TEXT("Grass"), TEXT("Trees"), TEXT("Rocks"), TEXT("Dinosaurs")};
    SavannaBiome.MemoryAllocation = EEng_MemoryBudget::High;
    SavannaBiome.bRequiresStreamingLOD = true;
    BiomeArchitectures.Add(SavannaBiome);

    // Swamp Biome
    FEng_BiomeArchitecture SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.WorldCoordinates = FVector(-5000000.0f, -4500000.0f, 0.0f);
    SwampBiome.BiomeRadius = 200000.0f;
    SwampBiome.MinActorCount = 500;
    SwampBiome.MaxActorCount = 2000;
    SwampBiome.AllowedAssetCategories = {TEXT("Vegetation"), TEXT("Water"), TEXT("Mud"), TEXT("Amphibians")};
    SwampBiome.MemoryAllocation = EEng_MemoryBudget::High;
    SwampBiome.bRequiresStreamingLOD = true;
    BiomeArchitectures.Add(SwampBiome);

    // Forest Biome
    FEng_BiomeArchitecture ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.WorldCoordinates = FVector(-4500000.0f, 4000000.0f, 0.0f);
    ForestBiome.BiomeRadius = 200000.0f;
    ForestBiome.MinActorCount = 500;
    ForestBiome.MaxActorCount = 3000;
    ForestBiome.AllowedAssetCategories = {TEXT("Trees"), TEXT("Undergrowth"), TEXT("Logs"), TEXT("Forest_Dinosaurs")};
    ForestBiome.MemoryAllocation = EEng_MemoryBudget::High;
    ForestBiome.bRequiresStreamingLOD = true;
    BiomeArchitectures.Add(ForestBiome);

    // Desert Biome
    FEng_BiomeArchitecture DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.WorldCoordinates = FVector(5500000.0f, 0.0f, 0.0f);
    DesertBiome.BiomeRadius = 200000.0f;
    DesertBiome.MinActorCount = 500;
    DesertBiome.MaxActorCount = 1500;
    DesertBiome.AllowedAssetCategories = {TEXT("Sand"), TEXT("Cacti"), TEXT("Rocks"), TEXT("Desert_Dinosaurs")};
    DesertBiome.MemoryAllocation = EEng_MemoryBudget::Medium;
    DesertBiome.bRequiresStreamingLOD = true;
    BiomeArchitectures.Add(DesertBiome);

    // Mountain Biome
    FEng_BiomeArchitecture MountainBiome;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.WorldCoordinates = FVector(4000000.0f, 5000000.0f, 100000.0f);
    MountainBiome.BiomeRadius = 200000.0f;
    MountainBiome.MinActorCount = 500;
    MountainBiome.MaxActorCount = 1800;
    MountainBiome.AllowedAssetCategories = {TEXT("Rocks"), TEXT("Cliffs"), TEXT("Alpine_Plants"), TEXT("Flying_Dinosaurs")};
    MountainBiome.MemoryAllocation = EEng_MemoryBudget::Medium;
    MountainBiome.bRequiresStreamingLOD = true;
    BiomeArchitectures.Add(MountainBiome);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d biome architectures"), BiomeArchitectures.Num());
}

void UEng_TechnicalArchitecture::ValidateWorldPartitionRequirements()
{
    if (WorldSizeKm > 4.0f)
    {
        bWorldPartitionEnabled = true;
        UE_LOG(LogTemp, Warning, TEXT("World Partition enabled for %f km world"), WorldSizeKm);
    }
    else
    {
        bWorldPartitionEnabled = false;
        UE_LOG(LogTemp, Warning, TEXT("World Partition disabled for %f km world"), WorldSizeKm);
    }
}

void UEng_TechnicalArchitecture::RegisterModule(const FEng_ModuleSpec& ModuleSpec)
{
    // Check if module already exists
    for (int32 i = 0; i < ModuleSpecifications.Num(); i++)
    {
        if (ModuleSpecifications[i].ModuleType == ModuleSpec.ModuleType)
        {
            ModuleSpecifications[i] = ModuleSpec;
            UE_LOG(LogTemp, Warning, TEXT("Updated module specification: %s"), *ModuleSpec.ModuleName);
            return;
        }
    }
    
    ModuleSpecifications.Add(ModuleSpec);
    UE_LOG(LogTemp, Warning, TEXT("Registered new module: %s"), *ModuleSpec.ModuleName);
}

FEng_ModuleSpec UEng_TechnicalArchitecture::GetModuleSpec(EEng_ModuleType ModuleType)
{
    for (const FEng_ModuleSpec& Spec : ModuleSpecifications)
    {
        if (Spec.ModuleType == ModuleType)
        {
            return Spec;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Module specification not found for type: %d"), (int32)ModuleType);
    return FEng_ModuleSpec();
}

bool UEng_TechnicalArchitecture::ValidateModuleDependencies(EEng_ModuleType ModuleType)
{
    FEng_ModuleSpec ModuleSpec = GetModuleSpec(ModuleType);
    
    for (const FString& Dependency : ModuleSpec.Dependencies)
    {
        bool bDependencyFound = false;
        for (const FEng_ModuleSpec& Spec : ModuleSpecifications)
        {
            if (Spec.ModuleName.Contains(Dependency))
            {
                bDependencyFound = true;
                break;
            }
        }
        
        if (!bDependencyFound)
        {
            UE_LOG(LogTemp, Error, TEXT("Module dependency not satisfied: %s requires %s"), *ModuleSpec.ModuleName, *Dependency);
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEng_TechnicalArchitecture::GetModuleDependencies(EEng_ModuleType ModuleType)
{
    FEng_ModuleSpec ModuleSpec = GetModuleSpec(ModuleType);
    return ModuleSpec.Dependencies;
}

void UEng_TechnicalArchitecture::AddPerformanceRule(const FEng_PerformanceRule& Rule)
{
    PerformanceRules.Add(Rule);
    UE_LOG(LogTemp, Warning, TEXT("Added performance rule: %s"), *Rule.RuleName);
}

FEng_PerformanceRule UEng_TechnicalArchitecture::GetPerformanceRule(EEng_ModuleType ModuleType)
{
    for (const FEng_PerformanceRule& Rule : PerformanceRules)
    {
        if (Rule.ApplicableModule == ModuleType)
        {
            return Rule;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("No performance rule found for module type: %d"), (int32)ModuleType);
    return FEng_PerformanceRule();
}

bool UEng_TechnicalArchitecture::ValidateAssetPerformance(const FString& AssetPath, EEng_ModuleType ModuleType)
{
    FEng_PerformanceRule Rule = GetPerformanceRule(ModuleType);
    
    // TODO: Implement actual asset validation logic
    // For now, return true as placeholder
    UE_LOG(LogTemp, Warning, TEXT("Validating asset performance: %s for module %d"), *AssetPath, (int32)ModuleType);
    return true;
}

int32 UEng_TechnicalArchitecture::GetMaxTrianglesForModule(EEng_ModuleType ModuleType)
{
    FEng_PerformanceRule Rule = GetPerformanceRule(ModuleType);
    return Rule.MaxTriangles;
}

void UEng_TechnicalArchitecture::SetupBiomeArchitecture(EBiomeType BiomeType, FVector Coordinates, float Radius)
{
    for (FEng_BiomeArchitecture& Biome : BiomeArchitectures)
    {
        if (Biome.BiomeType == BiomeType)
        {
            Biome.WorldCoordinates = Coordinates;
            Biome.BiomeRadius = Radius;
            UE_LOG(LogTemp, Warning, TEXT("Updated biome architecture for %d at coordinates (%f, %f, %f)"), 
                   (int32)BiomeType, Coordinates.X, Coordinates.Y, Coordinates.Z);
            return;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Biome architecture not found for type: %d"), (int32)BiomeType);
}

FEng_BiomeArchitecture UEng_TechnicalArchitecture::GetBiomeArchitecture(EBiomeType BiomeType)
{
    for (const FEng_BiomeArchitecture& Biome : BiomeArchitectures)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    UE_LOG(LogTemp, Error, TEXT("Biome architecture not found for type: %d"), (int32)BiomeType);
    return FEng_BiomeArchitecture();
}

bool UEng_TechnicalArchitecture::CanSpawnActorInBiome(EBiomeType BiomeType, const FString& AssetCategory)
{
    FEng_BiomeArchitecture Biome = GetBiomeArchitecture(BiomeType);
    
    for (const FString& AllowedCategory : Biome.AllowedAssetCategories)
    {
        if (AssetCategory.Contains(AllowedCategory))
        {
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Asset category %s not allowed in biome %d"), *AssetCategory, (int32)BiomeType);
    return false;
}

int32 UEng_TechnicalArchitecture::GetActorBudgetForBiome(EBiomeType BiomeType)
{
    FEng_BiomeArchitecture Biome = GetBiomeArchitecture(BiomeType);
    return Biome.MaxActorCount;
}

float UEng_TechnicalArchitecture::GetMemoryBudgetMB(EEng_MemoryBudget BudgetType)
{
    switch (BudgetType)
    {
        case EEng_MemoryBudget::Critical:
            return 512.0f;
        case EEng_MemoryBudget::High:
            return 256.0f;
        case EEng_MemoryBudget::Medium:
            return 128.0f;
        case EEng_MemoryBudget::Low:
            return 64.0f;
        case EEng_MemoryBudget::Minimal:
            return 32.0f;
        default:
            return 128.0f;
    }
}

bool UEng_TechnicalArchitecture::CheckMemoryUsage(EEng_ModuleType ModuleType)
{
    // TODO: Implement actual memory monitoring
    // For now, return true as placeholder
    UE_LOG(LogTemp, Warning, TEXT("Checking memory usage for module: %d"), (int32)ModuleType);
    return true;
}

void UEng_TechnicalArchitecture::OptimizeMemoryForBiome(EBiomeType BiomeType)
{
    FEng_BiomeArchitecture Biome = GetBiomeArchitecture(BiomeType);
    UE_LOG(LogTemp, Warning, TEXT("Optimizing memory for biome %d with budget %f MB"), 
           (int32)BiomeType, GetMemoryBudgetMB(Biome.MemoryAllocation));
}

void UEng_TechnicalArchitecture::SetWorldSize(float SizeInKm)
{
    WorldSizeKm = SizeInKm;
    ValidateWorldPartitionRequirements();
    UE_LOG(LogTemp, Warning, TEXT("World size set to %f km"), SizeInKm);
}

bool UEng_TechnicalArchitecture::ShouldUseWorldPartition()
{
    return bWorldPartitionEnabled;
}

FVector UEng_TechnicalArchitecture::GetWorldBounds()
{
    float WorldSizeCm = WorldSizeKm * 100000.0f; // Convert km to cm
    return FVector(WorldSizeCm, WorldSizeCm, 100000.0f);
}

TArray<int32> UEng_TechnicalArchitecture::GetAllowedAgentsForModule(EEng_ModuleType ModuleType)
{
    TArray<int32> AllowedAgents;
    
    switch (ModuleType)
    {
        case EEng_ModuleType::Core:
            AllowedAgents = {1, 2, 3, 19, 20}; // Director, Architect, Core Systems, Integration, Build
            break;
        case EEng_ModuleType::WorldGeneration:
            AllowedAgents = {5}; // Procedural World Generator
            break;
        case EEng_ModuleType::Environment:
            AllowedAgents = {6, 7}; // Environment Artist, Architecture & Interior
            break;
        case EEng_ModuleType::Character:
            AllowedAgents = {9, 10}; // Character Artist, Animation
            break;
        case EEng_ModuleType::AI:
            AllowedAgents = {11, 12, 13}; // NPC Behavior, Combat AI, Crowd Simulation
            break;
        case EEng_ModuleType::Audio:
            AllowedAgents = {16}; // Audio Agent
            break;
        case EEng_ModuleType::VFX:
            AllowedAgents = {17}; // VFX Agent
            break;
        default:
            AllowedAgents = {1, 2}; // Default to Director and Architect only
            break;
    }
    
    return AllowedAgents;
}

bool UEng_TechnicalArchitecture::CanAgentModifyModule(int32 AgentNumber, EEng_ModuleType ModuleType)
{
    TArray<int32> AllowedAgents = GetAllowedAgentsForModule(ModuleType);
    return AllowedAgents.Contains(AgentNumber);
}

void UEng_TechnicalArchitecture::RegisterAgentActivity(int32 AgentNumber, EEng_ModuleType ModuleType, const FString& Activity)
{
    if (CanAgentModifyModule(AgentNumber, ModuleType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %d activity in module %d: %s"), AgentNumber, (int32)ModuleType, *Activity);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Agent %d not authorized to modify module %d"), AgentNumber, (int32)ModuleType);
    }
}

bool UEng_TechnicalArchitecture::ValidateSystemArchitecture()
{
    bool bIsValid = true;
    
    // Validate all module dependencies
    for (const FEng_ModuleSpec& Module : ModuleSpecifications)
    {
        if (!ValidateModuleDependencies(Module.ModuleType))
        {
            bIsValid = false;
        }
    }
    
    // Validate world partition requirements
    if (WorldSizeKm > 4.0f && !bWorldPartitionEnabled)
    {
        UE_LOG(LogTemp, Error, TEXT("World Partition required for worlds larger than 4km"));
        bIsValid = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System architecture validation: %s"), bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
    return bIsValid;
}

TArray<FString> UEng_TechnicalArchitecture::GetArchitectureViolations()
{
    TArray<FString> Violations;
    
    // Check module dependencies
    for (const FEng_ModuleSpec& Module : ModuleSpecifications)
    {
        if (!ValidateModuleDependencies(Module.ModuleType))
        {
            Violations.Add(FString::Printf(TEXT("Module %s has unresolved dependencies"), *Module.ModuleName));
        }
    }
    
    // Check world partition
    if (WorldSizeKm > 4.0f && !bWorldPartitionEnabled)
    {
        Violations.Add(TEXT("World Partition required for large worlds"));
    }
    
    return Violations;
}

FString UEng_TechnicalArchitecture::GenerateArchitectureReport()
{
    FString Report = TEXT("=== TECHNICAL ARCHITECTURE REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Project Version: %s\n"), *ProjectVersion);
    Report += FString::Printf(TEXT("World Size: %.1f km\n"), WorldSizeKm);
    Report += FString::Printf(TEXT("World Partition: %s\n"), bWorldPartitionEnabled ? TEXT("Enabled") : TEXT("Disabled"));
    Report += FString::Printf(TEXT("Max Concurrent Actors: %d\n\n"), MaxConcurrentActors);
    
    Report += FString::Printf(TEXT("Registered Modules: %d\n"), ModuleSpecifications.Num());
    for (const FEng_ModuleSpec& Module : ModuleSpecifications)
    {
        Report += FString::Printf(TEXT("- %s (Memory: %.0f MB, Max Actors: %d)\n"), 
                                  *Module.ModuleName, GetMemoryBudgetMB(Module.MemoryBudget), Module.MaxActorsPerBiome);
    }
    
    Report += FString::Printf(TEXT("\nBiome Architectures: %d\n"), BiomeArchitectures.Num());
    for (const FEng_BiomeArchitecture& Biome : BiomeArchitectures)
    {
        Report += FString::Printf(TEXT("- Biome %d: %d-%d actors, %.0f MB\n"), 
                                  (int32)Biome.BiomeType, Biome.MinActorCount, Biome.MaxActorCount, 
                                  GetMemoryBudgetMB(Biome.MemoryAllocation));
    }
    
    TArray<FString> Violations = GetArchitectureViolations();
    if (Violations.Num() > 0)
    {
        Report += TEXT("\nArchitecture Violations:\n");
        for (const FString& Violation : Violations)
        {
            Report += FString::Printf(TEXT("- %s\n"), *Violation);
        }
    }
    else
    {
        Report += TEXT("\nNo architecture violations detected.\n");
    }
    
    return Report;
}

void UEng_TechnicalArchitecture::RefreshArchitectureRules()
{
    UE_LOG(LogTemp, Warning, TEXT("Refreshing architecture rules..."));
    
    ModuleSpecifications.Empty();
    PerformanceRules.Empty();
    BiomeArchitectures.Empty();
    
    InitializeDefaultModules();
    InitializeDefaultPerformanceRules();
    InitializeDefaultBiomeArchitectures();
    
    UE_LOG(LogTemp, Warning, TEXT("Architecture rules refreshed successfully"));
}