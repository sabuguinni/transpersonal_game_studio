#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every second
    
    bEnforceStrictCompilation = true;
    
    // Initialize required header includes for all modules
    RequiredHeaderIncludes.Add(TEXT("CoreMinimal.h"));
    RequiredHeaderIncludes.Add(TEXT("Engine/Engine.h"));
    RequiredHeaderIncludes.Add(TEXT("GameFramework/Actor.h"));
    RequiredHeaderIncludes.Add(TEXT("Components/ActorComponent.h"));
    
    // Initialize forbidden APIs that cause compilation issues
    ForbiddenAPIs.Add(TEXT("unreal.find_class"));
    ForbiddenAPIs.Add(TEXT("unreal.SystemLibrary.get_project_name"));
    ForbiddenAPIs.Add(TEXT("unreal.SystemLibrary.get_engine_version"));
    ForbiddenAPIs.Add(TEXT("escaped quotes in UPROPERTY"));
    
    LogArchitecturalEvent(TEXT("Architectural Framework initialized"));
}

void UEng_ArchitecturalFramework::BeginPlay()
{
    Super::BeginPlay();
    
    LogArchitecturalEvent(TEXT("Architectural Framework BeginPlay - Initializing core systems"));
    
    // Initialize core modules that must load first
    InitializeCoreModules();
    
    // Set up performance budgets for each agent type
    SetupDefaultPerformanceBudgets();
    
    // Initialize asset pipeline rules
    InitializeAssetPipelineRules();
    
    // Validate system integrity
    ValidateSystemIntegrity();
}

void UEng_ArchitecturalFramework::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Check module health
    CheckModuleHealth();
    
    // Enforce performance limits if enabled
    if (bEnforceStrictCompilation)
    {
        EnforcePerformanceLimits();
    }
}

void UEng_ArchitecturalFramework::RegisterModule(const FEng_ModuleSpec& ModuleSpec)
{
    LogArchitecturalEvent(FString::Printf(TEXT("Registering module: %s (Priority: %s)"), 
        *ModuleSpec.ModuleName, 
        *UEnum::GetValueAsString(ModuleSpec.Priority)));
    
    // Validate module dependencies before registration
    if (ValidateModuleDependencies(ModuleSpec.ModuleName))
    {
        RegisteredModules.Add(ModuleSpec);
        ModuleLoadStatus.Add(ModuleSpec.ModuleName, true);
        
        LogArchitecturalEvent(FString::Printf(TEXT("Module %s registered successfully"), *ModuleSpec.ModuleName));
    }
    else
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Module %s registration FAILED - dependency validation failed"), *ModuleSpec.ModuleName));
    }
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies(const FString& ModuleName)
{
    // Find the module spec
    for (const FEng_ModuleSpec& Module : RegisteredModules)
    {
        if (Module.ModuleName == ModuleName)
        {
            // Check if all dependencies are loaded
            for (const FString& Dependency : Module.Dependencies)
            {
                if (!ModuleLoadStatus.Contains(Dependency) || !ModuleLoadStatus[Dependency])
                {
                    LogArchitecturalEvent(FString::Printf(TEXT("Dependency validation failed: %s requires %s"), 
                        *ModuleName, *Dependency));
                    return false;
                }
            }
            return true;
        }
    }
    
    // Module not found, assume it's valid for now
    return true;
}

void UEng_ArchitecturalFramework::InitializeCoreModules()
{
    LogArchitecturalEvent(TEXT("Initializing core modules in priority order"));
    
    // Core Engine Module
    FEng_ModuleSpec CoreModule;
    CoreModule.ModuleName = TEXT("TranspersonalGameCore");
    CoreModule.Priority = EEng_ModulePriority::Critical;
    CoreModule.OwnerAgent = EEng_AgentType::Technical;
    CoreModule.MemoryBudgetMB = 50.0f;
    CoreModule.CPUBudgetMS = 2.0f;
    RegisterModule(CoreModule);
    
    // Studio Director Module
    FEng_ModuleSpec DirectorModule;
    DirectorModule.ModuleName = TEXT("StudioDirectorSystem");
    DirectorModule.Priority = EEng_ModulePriority::Critical;
    DirectorModule.OwnerAgent = EEng_AgentType::Management;
    DirectorModule.Dependencies.Add(TEXT("TranspersonalGameCore"));
    DirectorModule.MemoryBudgetMB = 25.0f;
    DirectorModule.CPUBudgetMS = 1.0f;
    RegisterModule(DirectorModule);
    
    // World Generation Module
    FEng_ModuleSpec WorldGenModule;
    WorldGenModule.ModuleName = TEXT("PCGWorldGenerator");
    WorldGenModule.Priority = EEng_ModulePriority::High;
    WorldGenModule.OwnerAgent = EEng_AgentType::WorldBuilding;
    WorldGenModule.Dependencies.Add(TEXT("TranspersonalGameCore"));
    WorldGenModule.MemoryBudgetMB = 100.0f;
    WorldGenModule.CPUBudgetMS = 5.0f;
    RegisterModule(WorldGenModule);
    
    // Character System Module
    FEng_ModuleSpec CharacterModule;
    CharacterModule.ModuleName = TEXT("TranspersonalCharacter");
    CharacterModule.Priority = EEng_ModulePriority::High;
    CharacterModule.OwnerAgent = EEng_AgentType::Character;
    CharacterModule.Dependencies.Add(TEXT("TranspersonalGameCore"));
    CharacterModule.MemoryBudgetMB = 75.0f;
    CharacterModule.CPUBudgetMS = 3.0f;
    RegisterModule(CharacterModule);
}

void UEng_ArchitecturalFramework::SetPerformanceBudget(EEng_AgentType AgentType, const FEng_PerformanceBudget& Budget)
{
    AgentBudgets.Add(AgentType, Budget);
    
    LogArchitecturalEvent(FString::Printf(TEXT("Performance budget set for %s: %d actors, %.2fMB memory, %.2fms CPU"), 
        *UEnum::GetValueAsString(AgentType),
        Budget.MaxActorsPerFrame,
        Budget.MaxMemoryUsageMB,
        Budget.MaxCPUTimeMS));
}

bool UEng_ArchitecturalFramework::CheckPerformanceCompliance(EEng_AgentType AgentType)
{
    if (!AgentBudgets.Contains(AgentType))
    {
        return true; // No budget set, assume compliant
    }
    
    const FEng_PerformanceBudget& Budget = AgentBudgets[AgentType];
    
    // Get current world
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Count actors in the world
    int32 ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        ActorCount++;
    }
    
    // Check if we're within budget
    bool bCompliant = ActorCount <= Budget.MaxActorsPerFrame;
    
    if (!bCompliant)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Performance compliance FAILED for %s: %d actors (limit: %d)"), 
            *UEnum::GetValueAsString(AgentType),
            ActorCount,
            Budget.MaxActorsPerFrame));
    }
    
    return bCompliant;
}

void UEng_ArchitecturalFramework::EnforcePerformanceLimits()
{
    // Check compliance for all agent types
    for (const auto& BudgetPair : AgentBudgets)
    {
        if (!CheckPerformanceCompliance(BudgetPair.Key))
        {
            LogArchitecturalEvent(FString::Printf(TEXT("Performance limit exceeded for %s - taking corrective action"), 
                *UEnum::GetValueAsString(BudgetPair.Key)));
            
            // Could implement automatic actor culling here if needed
        }
    }
}

void UEng_ArchitecturalFramework::AddAssetRule(const FEng_AssetPipelineRule& Rule)
{
    AssetRules.Add(Rule);
    
    LogArchitecturalEvent(FString::Printf(TEXT("Asset rule added for %s: %s naming, %dx%d max texture"), 
        *Rule.AssetType,
        *Rule.NamingConvention,
        Rule.MaxTextureSizeX,
        Rule.MaxTextureSizeY));
}

bool UEng_ArchitecturalFramework::ValidateAssetCompliance(const FString& AssetPath)
{
    // Basic validation - check if path follows conventions
    for (const FEng_AssetPipelineRule& Rule : AssetRules)
    {
        if (AssetPath.Contains(Rule.AssetType))
        {
            if (!AssetPath.Contains(Rule.RequiredPath))
            {
                LogArchitecturalEvent(FString::Printf(TEXT("Asset compliance FAILED: %s not in required path %s"), 
                    *AssetPath, *Rule.RequiredPath));
                return false;
            }
            
            if (!AssetPath.Contains(Rule.NamingConvention))
            {
                LogArchitecturalEvent(FString::Printf(TEXT("Asset compliance FAILED: %s doesn't follow naming convention %s"), 
                    *AssetPath, *Rule.NamingConvention));
                return false;
            }
        }
    }
    
    return true;
}

void UEng_ArchitecturalFramework::PrepareCommercialAssetIntegration()
{
    LogArchitecturalEvent(TEXT("Preparing for commercial asset integration"));
    
    // Set up asset rules for commercial dinosaur assets
    FEng_AssetPipelineRule DinosaurRule;
    DinosaurRule.AssetType = TEXT("SkeletalMesh");
    DinosaurRule.RequiredPath = TEXT("/Game/Assets/Dinosaurs/");
    DinosaurRule.NamingConvention = TEXT("SK_Dino_");
    DinosaurRule.MaxTextureSizeX = 4096;
    DinosaurRule.MaxTextureSizeY = 4096;
    DinosaurRule.bRequiresLODChain = true;
    AddAssetRule(DinosaurRule);
    
    // Set up asset rules for biome assets
    FEng_AssetPipelineRule BiomeRule;
    BiomeRule.AssetType = TEXT("StaticMesh");
    BiomeRule.RequiredPath = TEXT("/Game/Assets/Biomes/");
    BiomeRule.NamingConvention = TEXT("SM_Biome_");
    BiomeRule.MaxTextureSizeX = 2048;
    BiomeRule.MaxTextureSizeY = 2048;
    BiomeRule.bRequiresLODChain = true;
    AddAssetRule(BiomeRule);
    
    LogArchitecturalEvent(TEXT("Commercial asset integration rules established"));
}

void UEng_ArchitecturalFramework::EnforceCompilationRules()
{
    LogArchitecturalEvent(TEXT("Enforcing compilation rules across all modules"));
    
    // This would typically scan source files and validate them
    // For now, we'll just log the enforcement
    for (const FString& RequiredHeader : RequiredHeaderIncludes)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Required header: %s"), *RequiredHeader));
    }
    
    for (const FString& ForbiddenAPI : ForbiddenAPIs)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Forbidden API: %s"), *ForbiddenAPI));
    }
}

bool UEng_ArchitecturalFramework::ValidateHeaderStructure(const FString& HeaderPath)
{
    // Basic validation - in a real implementation, this would parse the file
    LogArchitecturalEvent(FString::Printf(TEXT("Validating header structure: %s"), *HeaderPath));
    
    // Check file exists
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.FileExists(*HeaderPath))
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Header validation FAILED: %s does not exist"), *HeaderPath));
        return false;
    }
    
    return true;
}

void UEng_ArchitecturalFramework::GenerateModuleBuildFiles()
{
    LogArchitecturalEvent(TEXT("Generating module build files"));
    
    // This would generate .Build.cs files for each module
    // For now, we'll just validate the existing structure
    for (const FEng_ModuleSpec& Module : RegisteredModules)
    {
        LogArchitecturalEvent(FString::Printf(TEXT("Module build file required for: %s"), *Module.ModuleName));
    }
}

bool UEng_ArchitecturalFramework::IsModuleLoaded(const FString& ModuleName) const
{
    return ModuleLoadStatus.Contains(ModuleName) && ModuleLoadStatus[ModuleName];
}

TArray<FString> UEng_ArchitecturalFramework::GetLoadedModules() const
{
    TArray<FString> LoadedModules;
    
    for (const auto& StatusPair : ModuleLoadStatus)
    {
        if (StatusPair.Value)
        {
            LoadedModules.Add(StatusPair.Key);
        }
    }
    
    return LoadedModules;
}

float UEng_ArchitecturalFramework::GetCurrentPerformanceScore() const
{
    float TotalScore = 100.0f;
    int32 CheckedAgents = 0;
    
    for (const auto& BudgetPair : AgentBudgets)
    {
        if (!const_cast<UEng_ArchitecturalFramework*>(this)->CheckPerformanceCompliance(BudgetPair.Key))
        {
            TotalScore -= 20.0f; // Deduct 20 points per non-compliant agent
        }
        CheckedAgents++;
    }
    
    return FMath::Max(0.0f, TotalScore);
}

int32 UEng_ArchitecturalFramework::GetTotalAssetsRegistered() const
{
    return AssetRules.Num();
}

void UEng_ArchitecturalFramework::EmergencyModuleShutdown(const FString& ModuleName)
{
    LogArchitecturalEvent(FString::Printf(TEXT("EMERGENCY SHUTDOWN: Module %s"), *ModuleName));
    
    if (ModuleLoadStatus.Contains(ModuleName))
    {
        ModuleLoadStatus[ModuleName] = false;
    }
}

void UEng_ArchitecturalFramework::ResetArchitecturalFramework()
{
    LogArchitecturalEvent(TEXT("RESETTING ARCHITECTURAL FRAMEWORK"));
    
    RegisteredModules.Empty();
    ModuleLoadStatus.Empty();
    AgentBudgets.Empty();
    AssetRules.Empty();
    
    // Reinitialize core systems
    InitializeCoreModules();
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Message)
{
    FString LogMessage = FString::Printf(TEXT("[ENGINE ARCHITECT] %s"), *Message);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, LogMessage);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
}

void UEng_ArchitecturalFramework::ValidateSystemIntegrity()
{
    LogArchitecturalEvent(TEXT("Validating system integrity"));
    
    // Check that critical modules are loaded
    TArray<FString> CriticalModules = {
        TEXT("TranspersonalGameCore"),
        TEXT("StudioDirectorSystem")
    };
    
    for (const FString& CriticalModule : CriticalModules)
    {
        if (!IsModuleLoaded(CriticalModule))
        {
            LogArchitecturalEvent(FString::Printf(TEXT("CRITICAL: Module %s is not loaded!"), *CriticalModule));
        }
    }
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    // Update performance metrics every tick
    // This is where we'd collect real-time performance data
    
    UWorld* World = GetWorld();
    if (World)
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        
        // Update global budget tracking
        GlobalBudget.MaxActorsPerFrame = FMath::Max(GlobalBudget.MaxActorsPerFrame, ActorCount);
    }
}

void UEng_ArchitecturalFramework::CheckModuleHealth()
{
    // Check the health of all registered modules
    for (const FEng_ModuleSpec& Module : RegisteredModules)
    {
        if (!IsModuleLoaded(Module.ModuleName))
        {
            LogArchitecturalEvent(FString::Printf(TEXT("Module health check FAILED: %s is not loaded"), *Module.ModuleName));
        }
    }
}

void UEng_ArchitecturalFramework::SetupDefaultPerformanceBudgets()
{
    // Technical Agents (Architecture, Core, Performance)
    FEng_PerformanceBudget TechnicalBudget;
    TechnicalBudget.TargetTier = EEng_PerformanceTier::Ultra;
    TechnicalBudget.MaxActorsPerFrame = 500;
    TechnicalBudget.MaxMemoryUsageMB = 100.0f;
    TechnicalBudget.MaxCPUTimeMS = 2.0f;
    SetPerformanceBudget(EEng_AgentType::Technical, TechnicalBudget);
    
    // World Building Agents (World, Environment, Architecture, Lighting)
    FEng_PerformanceBudget WorldBudget;
    WorldBudget.TargetTier = EEng_PerformanceTier::High;
    WorldBudget.MaxActorsPerFrame = 2000;
    WorldBudget.MaxMemoryUsageMB = 300.0f;
    WorldBudget.MaxCPUTimeMS = 5.0f;
    SetPerformanceBudget(EEng_AgentType::WorldBuilding, WorldBudget);
    
    // AI Agents (NPC, Combat, Crowd)
    FEng_PerformanceBudget AIBudget;
    AIBudget.TargetTier = EEng_PerformanceTier::High;
    AIBudget.MaxActorsPerFrame = 1000;
    AIBudget.MaxMemoryUsageMB = 200.0f;
    AIBudget.MaxCPUTimeMS = 8.0f;
    SetPerformanceBudget(EEng_AgentType::AI, AIBudget);
    
    // Character Agents
    FEng_PerformanceBudget CharacterBudget;
    CharacterBudget.TargetTier = EEng_PerformanceTier::Ultra;
    CharacterBudget.MaxActorsPerFrame = 100;
    CharacterBudget.MaxMemoryUsageMB = 150.0f;
    CharacterBudget.MaxCPUTimeMS = 3.0f;
    SetPerformanceBudget(EEng_AgentType::Character, CharacterBudget);
}

void UEng_ArchitecturalFramework::InitializeAssetPipelineRules()
{
    // Static Mesh Rules
    FEng_AssetPipelineRule StaticMeshRule;
    StaticMeshRule.AssetType = TEXT("StaticMesh");
    StaticMeshRule.RequiredPath = TEXT("/Game/Assets/StaticMeshes/");
    StaticMeshRule.NamingConvention = TEXT("SM_");
    StaticMeshRule.MaxTextureSizeX = 2048;
    StaticMeshRule.MaxTextureSizeY = 2048;
    StaticMeshRule.bRequiresLODChain = true;
    AddAssetRule(StaticMeshRule);
    
    // Skeletal Mesh Rules
    FEng_AssetPipelineRule SkeletalMeshRule;
    SkeletalMeshRule.AssetType = TEXT("SkeletalMesh");
    SkeletalMeshRule.RequiredPath = TEXT("/Game/Assets/SkeletalMeshes/");
    SkeletalMeshRule.NamingConvention = TEXT("SK_");
    SkeletalMeshRule.MaxTextureSizeX = 4096;
    SkeletalMeshRule.MaxTextureSizeY = 4096;
    SkeletalMeshRule.bRequiresLODChain = true;
    AddAssetRule(SkeletalMeshRule);
    
    // Material Rules
    FEng_AssetPipelineRule MaterialRule;
    MaterialRule.AssetType = TEXT("Material");
    MaterialRule.RequiredPath = TEXT("/Game/Assets/Materials/");
    MaterialRule.NamingConvention = TEXT("M_");
    MaterialRule.MaxTextureSizeX = 2048;
    MaterialRule.MaxTextureSizeY = 2048;
    MaterialRule.bRequiresLODChain = false;
    AddAssetRule(MaterialRule);
}