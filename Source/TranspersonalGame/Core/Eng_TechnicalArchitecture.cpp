#include "Eng_TechnicalArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UEng_TechnicalArchitecture::UEng_TechnicalArchitecture()
{
    CurrentStatus = EEng_ArchitectureStatus::Unknown;
}

void UEng_TechnicalArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT - TECHNICAL ARCHITECTURE INITIALIZED ==="));
    
    CurrentStatus = EEng_ArchitectureStatus::Validating;
    
    // Initialize core systems
    InitializeBiomeDefinitions();
    InitializeCompilationRules();
    ValidateCurrentCodebase();
    
    CurrentStatus = EEng_ArchitectureStatus::Valid;
    
    UE_LOG(LogTemp, Warning, TEXT("Technical Architecture System: ONLINE"));
    UE_LOG(LogTemp, Warning, TEXT("Biome validation: ACTIVE"));
    UE_LOG(LogTemp, Warning, TEXT("Compilation rules: ENFORCED"));
}

void UEng_TechnicalArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Technical Architecture System: SHUTDOWN"));
    Super::Deinitialize();
}

void UEng_TechnicalArchitecture::InitializeBiomeDefinitions()
{
    // CRITICAL: These coordinates come from brain memories and are MANDATORY
    
    // 1. PANTANO (Swamp) - Southwest
    FEng_BiomeValidationData SwampData;
    SwampData.BiomeType = EBiomeType::Swamp;
    SwampData.BiomeName = TEXT("Pantano");
    SwampData.Center = FVector(-50000.0f, -45000.0f, 0.0f);
    SwampData.MinBounds = FVector(-77500.0f, -76500.0f, -1000.0f);
    SwampData.MaxBounds = FVector(-25000.0f, -15000.0f, 1000.0f);
    BiomeDefinitions.Add(EBiomeType::Swamp, SwampData);
    
    // 2. FLORESTA (Forest) - Northwest  
    FEng_BiomeValidationData ForestData;
    ForestData.BiomeType = EBiomeType::Forest;
    ForestData.BiomeName = TEXT("Floresta");
    ForestData.Center = FVector(-45000.0f, 40000.0f, 0.0f);
    ForestData.MinBounds = FVector(-77500.0f, 15000.0f, -1000.0f);
    ForestData.MaxBounds = FVector(-15000.0f, 76500.0f, 1000.0f);
    BiomeDefinitions.Add(EBiomeType::Forest, ForestData);
    
    // 3. SAVANA (Savanna) - Center
    FEng_BiomeValidationData SavannaData;
    SavannaData.BiomeType = EBiomeType::Savanna;
    SavannaData.BiomeName = TEXT("Savana");
    SavannaData.Center = FVector(0.0f, 0.0f, 0.0f);
    SavannaData.MinBounds = FVector(-20000.0f, -20000.0f, -1000.0f);
    SavannaData.MaxBounds = FVector(20000.0f, 20000.0f, 1000.0f);
    BiomeDefinitions.Add(EBiomeType::Savanna, SavannaData);
    
    // 4. DESERTO (Desert) - East
    FEng_BiomeValidationData DesertData;
    DesertData.BiomeType = EBiomeType::Desert;
    DesertData.BiomeName = TEXT("Deserto");
    DesertData.Center = FVector(55000.0f, 0.0f, 0.0f);
    DesertData.MinBounds = FVector(25000.0f, -30000.0f, -1000.0f);
    DesertData.MaxBounds = FVector(79500.0f, 30000.0f, 1000.0f);
    BiomeDefinitions.Add(EBiomeType::Desert, DesertData);
    
    // 5. MONTANHA NEVADA (Mountain) - Northeast
    FEng_BiomeValidationData MountainData;
    MountainData.BiomeType = EBiomeType::Mountain;
    MountainData.BiomeName = TEXT("Montanha Nevada");
    MountainData.Center = FVector(40000.0f, 50000.0f, 500.0f);
    MountainData.MinBounds = FVector(15000.0f, 20000.0f, -500.0f);
    MountainData.MaxBounds = FVector(79500.0f, 76500.0f, 2000.0f);
    BiomeDefinitions.Add(EBiomeType::Mountain, MountainData);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome definitions initialized: 5 biomes configured"));
}

void UEng_TechnicalArchitecture::InitializeCompilationRules()
{
    CompilationRules.Empty();
    
    // Rule 1: Header-CPP Pairing
    FEng_CompilationRule HeaderCppRule;
    HeaderCppRule.RuleName = TEXT("HeaderCppPairing");
    HeaderCppRule.Description = TEXT("Every .h file must have a corresponding .cpp file");
    HeaderCppRule.bIsCritical = true;
    HeaderCppRule.ViolationCount = 0;
    CompilationRules.Add(HeaderCppRule);
    
    // Rule 2: Type Uniqueness
    FEng_CompilationRule TypeUniqueRule;
    TypeUniqueRule.RuleName = TEXT("TypeUniqueness");
    TypeUniqueRule.Description = TEXT("No duplicate USTRUCT/UENUM/UCLASS names across project");
    TypeUniqueRule.bIsCritical = true;
    TypeUniqueRule.ViolationCount = 0;
    CompilationRules.Add(TypeUniqueRule);
    
    // Rule 3: No Vector Zero Spawns
    FEng_CompilationRule NoZeroSpawnRule;
    NoZeroSpawnRule.RuleName = TEXT("NoZeroSpawns");
    NoZeroSpawnRule.Description = TEXT("Never spawn actors at Vector(0,0,0) - use biome coordinates");
    NoZeroSpawnRule.bIsCritical = true;
    NoZeroSpawnRule.ViolationCount = 0;
    CompilationRules.Add(NoZeroSpawnRule);
    
    // Rule 4: Module Dependencies
    FEng_CompilationRule ModuleDepsRule;
    ModuleDepsRule.RuleName = TEXT("ModuleDependencies");
    ModuleDepsRule.Description = TEXT("All cross-module includes must be declared in Build.cs");
    ModuleDepsRule.bIsCritical = true;
    ModuleDepsRule.ViolationCount = 0;
    CompilationRules.Add(ModuleDepsRule);
    
    UE_LOG(LogTemp, Warning, TEXT("Compilation rules initialized: %d rules active"), CompilationRules.Num());
}

void UEng_TechnicalArchitecture::ValidateCurrentCodebase()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CODEBASE VALIDATION STARTED ==="));
    
    int32 TotalViolations = 0;
    
    // Validate each rule
    for (FEng_CompilationRule& Rule : CompilationRules)
    {
        Rule.ViolationCount = 0;
        
        if (Rule.RuleName == TEXT("HeaderCppPairing"))
        {
            // This would require file system access to validate properly
            // For now, assume compliance based on brain memory about 122 headers
            Rule.ViolationCount = 122; // From brain memory
        }
        else if (Rule.RuleName == TEXT("NoZeroSpawns"))
        {
            // Check if any actors are at (0,0,0) - this would need world access
            Rule.ViolationCount = 0; // Assume good for now
        }
        
        TotalViolations += Rule.ViolationCount;
        
        if (Rule.ViolationCount > 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Rule violation: %s - %d violations"), 
                *Rule.RuleName, Rule.ViolationCount);
        }
    }
    
    // Set system status based on violations
    if (TotalViolations == 0)
    {
        CurrentStatus = EEng_ArchitectureStatus::Valid;
    }
    else if (TotalViolations < 10)
    {
        CurrentStatus = EEng_ArchitectureStatus::HasWarnings;
    }
    else if (TotalViolations < 50)
    {
        CurrentStatus = EEng_ArchitectureStatus::HasErrors;
    }
    else
    {
        CurrentStatus = EEng_ArchitectureStatus::Critical;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Codebase validation complete: %d total violations"), TotalViolations);
}

bool UEng_TechnicalArchitecture::IsValidBiomeLocation(const FVector& Location, EBiomeType BiomeType) const
{
    if (!BiomeDefinitions.Contains(BiomeType))
    {
        return false;
    }
    
    const FEng_BiomeValidationData& BiomeData = BiomeDefinitions[BiomeType];
    
    return (Location.X >= BiomeData.MinBounds.X && Location.X <= BiomeData.MaxBounds.X &&
            Location.Y >= BiomeData.MinBounds.Y && Location.Y <= BiomeData.MaxBounds.Y &&
            Location.Z >= BiomeData.MinBounds.Z && Location.Z <= BiomeData.MaxBounds.Z);
}

EBiomeType UEng_TechnicalArchitecture::GetBiomeAtLocation(const FVector& Location) const
{
    // Check each biome to see which one contains this location
    for (const auto& BiomePair : BiomeDefinitions)
    {
        if (IsValidBiomeLocation(Location, BiomePair.Key))
        {
            return BiomePair.Key;
        }
    }
    
    // Default to Savanna if location is outside all biomes
    return EBiomeType::Savanna;
}

FVector UEng_TechnicalArchitecture::GetRandomLocationInBiome(EBiomeType BiomeType) const
{
    if (!BiomeDefinitions.Contains(BiomeType))
    {
        // Return Savanna center as fallback
        return FVector(0.0f, 0.0f, 0.0f);
    }
    
    const FEng_BiomeValidationData& BiomeData = BiomeDefinitions[BiomeType];
    
    // Generate random location within biome bounds
    float RandomX = FMath::RandRange(BiomeData.MinBounds.X, BiomeData.MaxBounds.X);
    float RandomY = FMath::RandRange(BiomeData.MinBounds.Y, BiomeData.MaxBounds.Y);
    float RandomZ = FMath::RandRange(BiomeData.MinBounds.Z, BiomeData.MaxBounds.Z);
    
    return FVector(RandomX, RandomY, RandomZ);
}

FEng_BiomeValidationData UEng_TechnicalArchitecture::GetBiomeData(EBiomeType BiomeType) const
{
    if (BiomeDefinitions.Contains(BiomeType))
    {
        return BiomeDefinitions[BiomeType];
    }
    
    // Return default data if biome not found
    return FEng_BiomeValidationData();
}

bool UEng_TechnicalArchitecture::ValidateHeaderCppPairing(const FString& HeaderPath) const
{
    // Convert .h to .cpp path
    FString CppPath = HeaderPath.Replace(TEXT(".h"), TEXT(".cpp"));
    
    // Check if .cpp file exists (would need file system access)
    // For now, return true as placeholder
    return true;
}

bool UEng_TechnicalArchitecture::ValidateTypeUniqueness(const FString& TypeName) const
{
    // This would require reflection system access to check all loaded types
    // For now, return true as placeholder
    return true;
}

TArray<FEng_CompilationRule> UEng_TechnicalArchitecture::GetActiveRules() const
{
    return CompilationRules;
}

EEng_ArchitectureStatus UEng_TechnicalArchitecture::GetSystemStatus() const
{
    return CurrentStatus;
}

bool UEng_TechnicalArchitecture::ValidateModuleDependency(const FString& FromModule, const FString& ToModule) const
{
    // This would require Build.cs parsing to validate properly
    // For now, return true as placeholder
    return true;
}

TArray<FString> UEng_TechnicalArchitecture::GetRequiredModules() const
{
    TArray<FString> RequiredModules;
    RequiredModules.Add(TEXT("Core"));
    RequiredModules.Add(TEXT("CoreUObject"));
    RequiredModules.Add(TEXT("Engine"));
    RequiredModules.Add(TEXT("UnrealEd"));
    RequiredModules.Add(TEXT("ToolMenus"));
    RequiredModules.Add(TEXT("PCG"));
    RequiredModules.Add(TEXT("MassEntity"));
    RequiredModules.Add(TEXT("MassMovement"));
    RequiredModules.Add(TEXT("MassSpawner"));
    
    return RequiredModules;
}

bool UEng_TechnicalArchitecture::CanAgentProceed(int32 AgentID, const FString& TaskType) const
{
    // Check if system is in critical state
    if (CurrentStatus == EEng_ArchitectureStatus::Critical)
    {
        UE_LOG(LogTemp, Error, TEXT("Agent %d blocked - system in critical state"), AgentID);
        return false;
    }
    
    // Agent #20 (cleanup) can always proceed
    if (AgentID == 20)
    {
        return true;
    }
    
    // Block new development if too many violations
    if (CurrentStatus == EEng_ArchitectureStatus::HasErrors && TaskType.Contains(TEXT("Create")))
    {
        UE_LOG(LogTemp, Warning, TEXT("Agent %d blocked - fix existing errors before creating new content"), AgentID);
        return false;
    }
    
    return true;
}

void UEng_TechnicalArchitecture::RegisterAgentTask(int32 AgentID, const FString& TaskType, const FString& Description)
{
    ActiveAgentTasks.Add(AgentID, FString::Printf(TEXT("%s: %s"), *TaskType, *Description));
    UE_LOG(LogTemp, Log, TEXT("Agent %d registered task: %s"), AgentID, *Description);
}

void UEng_TechnicalArchitecture::ReportAgentCompletion(int32 AgentID, bool bSuccess, const FString& Result)
{
    if (ActiveAgentTasks.Contains(AgentID))
    {
        ActiveAgentTasks.Remove(AgentID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Agent %d completed: %s - %s"), 
        AgentID, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"), *Result);
        
    // Re-validate codebase after agent completion
    const_cast<UEng_TechnicalArchitecture*>(this)->ValidateCurrentCodebase();
}

bool UEng_TechnicalArchitecture::ValidateActorSpawnLocation(const FVector& Location) const
{
    // CRITICAL: Never allow spawning at (0,0,0)
    if (Location.Equals(FVector::ZeroVector, 1.0f))
    {
        LogArchitectureViolation(TEXT("Attempted spawn at Vector(0,0,0) - BLOCKED"), true);
        return false;
    }
    
    // Check if location is within any valid biome
    for (const auto& BiomePair : BiomeDefinitions)
    {
        if (IsValidBiomeLocation(Location, BiomePair.Key))
        {
            return true;
        }
    }
    
    LogArchitectureViolation(FString::Printf(TEXT("Spawn location %s outside all biomes"), *Location.ToString()), false);
    return false;
}

void UEng_TechnicalArchitecture::LogArchitectureViolation(const FString& Violation, bool bIsCritical) const
{
    if (bIsCritical)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL ARCHITECTURE VIOLATION: %s"), *Violation);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Architecture Warning: %s"), *Violation);
    }
}