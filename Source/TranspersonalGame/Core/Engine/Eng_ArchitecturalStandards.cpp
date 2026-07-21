#include "Eng_ArchitecturalStandards.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UEng_ArchitecturalStandards::UEng_ArchitecturalStandards()
{
    MaxFrameTime = 16.67f; // 60 FPS target
    MaxActorsPerBiome = 10000; // Performance limit per biome
}

void UEng_ArchitecturalStandards::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeStandards();
    UE_LOG(LogTemp, Warning, TEXT("Eng_ArchitecturalStandards initialized with 19 agent standards"));
}

void UEng_ArchitecturalStandards::Deinitialize()
{
    AgentStandards.Empty();
    BiomeCenters.Empty();
    Super::Deinitialize();
}

void UEng_ArchitecturalStandards::InitializeStandards()
{
    SetupAgentStandards();
    
    // Initialize biome centers
    BiomeCenters.Empty();
    BiomeCenters.Add(FVector(0.0f, 0.0f, 0.0f));           // Savana
    BiomeCenters.Add(FVector(-50000.0f, -45000.0f, 0.0f)); // Pantano
    BiomeCenters.Add(FVector(-45000.0f, 40000.0f, 0.0f));  // Floresta
    BiomeCenters.Add(FVector(55000.0f, 0.0f, 0.0f));       // Deserto
    BiomeCenters.Add(FVector(40000.0f, 50000.0f, 0.0f));   // Montanha
}

void UEng_ArchitecturalStandards::SetupAgentStandards()
{
    AgentStandards.Empty();

    // Agent #1 - Studio Director
    FEng_ModuleStandard DirectorStandard;
    DirectorStandard.ModuleName = TEXT("Studio Director");
    DirectorStandard.MinimumFilesRequired = 2;
    DirectorStandard.MinimumUE5Commands = 2;
    DirectorStandard.bRequiresBiomeDistribution = false;
    DirectorStandard.RequiredInterfaces.Add(TEXT("Production Coordination"));
    AgentStandards.Add(1, DirectorStandard);

    // Agent #2 - Engine Architect
    FEng_ModuleStandard ArchitectStandard;
    ArchitectStandard.ModuleName = TEXT("Engine Architect");
    ArchitectStandard.MinimumFilesRequired = 4;
    ArchitectStandard.MinimumUE5Commands = 3;
    ArchitectStandard.bRequiresBiomeDistribution = true;
    ArchitectStandard.RequiredInterfaces.Add(TEXT("Biome Management"));
    ArchitectStandard.RequiredInterfaces.Add(TEXT("Architectural Compliance"));
    AgentStandards.Add(2, ArchitectStandard);

    // Agent #3 - Core Systems Programmer
    FEng_ModuleStandard CoreStandard;
    CoreStandard.ModuleName = TEXT("Core Systems");
    CoreStandard.MinimumFilesRequired = 6;
    CoreStandard.MinimumUE5Commands = 2;
    CoreStandard.bRequiresBiomeDistribution = false;
    CoreStandard.RequiredInterfaces.Add(TEXT("Physics System"));
    CoreStandard.RequiredInterfaces.Add(TEXT("Collision System"));
    AgentStandards.Add(3, CoreStandard);

    // Agent #4 - Performance Optimizer
    FEng_ModuleStandard PerfStandard;
    PerfStandard.ModuleName = TEXT("Performance Optimizer");
    PerfStandard.MinimumFilesRequired = 4;
    PerfStandard.MinimumUE5Commands = 3;
    PerfStandard.bRequiresBiomeDistribution = false;
    PerfStandard.RequiredInterfaces.Add(TEXT("Performance Monitoring"));
    PerfStandard.RequiredInterfaces.Add(TEXT("LOD Management"));
    AgentStandards.Add(4, PerfStandard);

    // Agent #5 - Procedural World Generator
    FEng_ModuleStandard WorldGenStandard;
    WorldGenStandard.ModuleName = TEXT("World Generator");
    WorldGenStandard.MinimumFilesRequired = 6;
    WorldGenStandard.MinimumUE5Commands = 4;
    WorldGenStandard.bRequiresBiomeDistribution = true;
    WorldGenStandard.RequiredInterfaces.Add(TEXT("PCG System"));
    WorldGenStandard.RequiredInterfaces.Add(TEXT("Terrain Generation"));
    AgentStandards.Add(5, WorldGenStandard);

    // Agents #6-19 with appropriate standards
    for (int32 AgentID = 6; AgentID <= 19; AgentID++)
    {
        FEng_ModuleStandard GenericStandard;
        GenericStandard.ModuleName = FString::Printf(TEXT("Agent_%d"), AgentID);
        GenericStandard.MinimumFilesRequired = (AgentID <= 10) ? 4 : 2;
        GenericStandard.MinimumUE5Commands = (AgentID <= 10) ? 3 : 2;
        GenericStandard.bRequiresBiomeDistribution = (AgentID == 6 || AgentID == 9 || AgentID >= 11);
        
        if (AgentID >= 11 && AgentID <= 15)
        {
            GenericStandard.RequiredInterfaces.Add(TEXT("AI System"));
        }
        
        AgentStandards.Add(AgentID, GenericStandard);
    }
}

FEng_ModuleStandard UEng_ArchitecturalStandards::GetStandardForAgent(int32 AgentID) const
{
    if (const FEng_ModuleStandard* Standard = AgentStandards.Find(AgentID))
    {
        return *Standard;
    }
    return FEng_ModuleStandard();
}

void UEng_ArchitecturalStandards::SetStandardForAgent(int32 AgentID, const FEng_ModuleStandard& Standard)
{
    AgentStandards.Add(AgentID, Standard);
}

FEng_ComplianceReport UEng_ArchitecturalStandards::ValidateAgentCompliance(int32 AgentID, int32 FilesCreated, int32 UE5Commands) const
{
    FEng_ComplianceReport Report;
    Report.AgentName = FString::Printf(TEXT("Agent_%d"), AgentID);
    Report.FilesCreated = FilesCreated;
    Report.UE5CommandsExecuted = UE5Commands;

    FEng_ModuleStandard Standard = GetStandardForAgent(AgentID);
    
    bool bFileCompliance = ValidateAgentFileCount(AgentID, FilesCreated);
    bool bCommandCompliance = ValidateAgentCommandCount(AgentID, UE5Commands);
    
    Report.bMeetsStandards = bFileCompliance && bCommandCompliance;
    
    if (!bFileCompliance)
    {
        Report.ViolationReasons.Add(FString::Printf(TEXT("Insufficient files: %d/%d"), FilesCreated, Standard.MinimumFilesRequired));
    }
    
    if (!bCommandCompliance)
    {
        Report.ViolationReasons.Add(FString::Printf(TEXT("Insufficient UE5 commands: %d/%d"), UE5Commands, Standard.MinimumUE5Commands));
    }

    return Report;
}

bool UEng_ArchitecturalStandards::IsAgentCompliant(int32 AgentID, int32 FilesCreated, int32 UE5Commands) const
{
    return ValidateAgentFileCount(AgentID, FilesCreated) && ValidateAgentCommandCount(AgentID, UE5Commands);
}

TArray<FString> UEng_ArchitecturalStandards::GetComplianceViolations(int32 AgentID, int32 FilesCreated, int32 UE5Commands) const
{
    FEng_ComplianceReport Report = ValidateAgentCompliance(AgentID, FilesCreated, UE5Commands);
    return Report.ViolationReasons;
}

bool UEng_ArchitecturalStandards::ValidateBiomeDistribution(const TArray<FVector>& SpawnLocations) const
{
    if (SpawnLocations.Num() == 0 || BiomeCenters.Num() == 0)
    {
        return false;
    }

    TArray<int32> BiomeCounts;
    BiomeCounts.Init(0, BiomeCenters.Num());

    // Count actors per biome
    for (const FVector& Location : SpawnLocations)
    {
        int32 ClosestBiome = 0;
        float MinDistance = FLT_MAX;

        for (int32 i = 0; i < BiomeCenters.Num(); i++)
        {
            float Distance = FVector::Dist2D(Location, BiomeCenters[i]);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                ClosestBiome = i;
            }
        }

        if (BiomeCounts.IsValidIndex(ClosestBiome))
        {
            BiomeCounts[ClosestBiome]++;
        }
    }

    // Validate distribution (each biome should have at least 10% of total)
    int32 MinRequired = FMath::Max(1, SpawnLocations.Num() / 10);
    for (int32 Count : BiomeCounts)
    {
        if (Count < MinRequired)
        {
            return false;
        }
    }

    return true;
}

TArray<FVector> UEng_ArchitecturalStandards::GetRequiredBiomeDistribution(int32 NumActors) const
{
    TArray<FVector> DistributedLocations;
    
    if (BiomeCenters.Num() == 0 || NumActors <= 0)
    {
        return DistributedLocations;
    }

    int32 ActorsPerBiome = NumActors / BiomeCenters.Num();
    int32 Remainder = NumActors % BiomeCenters.Num();

    for (int32 BiomeIndex = 0; BiomeIndex < BiomeCenters.Num(); BiomeIndex++)
    {
        int32 ActorsForThisBiome = ActorsPerBiome + (BiomeIndex < Remainder ? 1 : 0);
        FVector BiomeCenter = BiomeCenters[BiomeIndex];

        for (int32 i = 0; i < ActorsForThisBiome; i++)
        {
            // Generate random point within 15km radius of biome center
            float Angle = FMath::RandRange(0.0f, 2.0f * PI);
            float Distance = FMath::RandRange(0.0f, 15000.0f);
            
            FVector Location = BiomeCenter + FVector(
                Distance * FMath::Cos(Angle),
                Distance * FMath::Sin(Angle),
                100.0f
            );
            
            DistributedLocations.Add(Location);
        }
    }

    return DistributedLocations;
}

bool UEng_ArchitecturalStandards::ValidateFileNaming(const FString& FileName, int32 AgentID) const
{
    FString RequiredPrefix = GetRequiredFilePrefix(AgentID);
    return FileName.StartsWith(RequiredPrefix);
}

FString UEng_ArchitecturalStandards::GetRequiredFilePrefix(int32 AgentID) const
{
    switch (AgentID)
    {
        case 1: return TEXT("Director_");
        case 2: return TEXT("Eng_");
        case 3: return TEXT("Core_");
        case 4: return TEXT("Perf_");
        case 5: return TEXT("WorldGen_");
        case 6: return TEXT("Env_");
        case 7: return TEXT("Arch_");
        case 8: return TEXT("Light_");
        case 9: return TEXT("Char_");
        case 10: return TEXT("Anim_");
        case 11: return TEXT("NPC_");
        case 12: return TEXT("Combat_");
        case 13: return TEXT("Crowd_");
        case 14: return TEXT("Quest_");
        case 15: return TEXT("Narrative_");
        case 16: return TEXT("Audio_");
        case 17: return TEXT("VFX_");
        case 18: return TEXT("QA_");
        case 19: return TEXT("Build_");
        default: return TEXT("Unknown_");
    }
}

bool UEng_ArchitecturalStandards::ValidatePerformanceRequirements(float FrameTime, int32 ActorCount) const
{
    if (FrameTime > MaxFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance violation: Frame time %.2fms exceeds limit %.2fms"), FrameTime, MaxFrameTime);
        return false;
    }

    if (ActorCount > MaxActorsPerBiome * 5) // Total across all biomes
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance violation: Actor count %d exceeds limit %d"), ActorCount, MaxActorsPerBiome * 5);
        return false;
    }

    return true;
}

void UEng_ArchitecturalStandards::EnforcePerformanceLimits()
{
    // Implementation for enforcing performance limits
    UE_LOG(LogTemp, Log, TEXT("Enforcing performance limits: Max frame time %.2fms, Max actors per biome %d"), MaxFrameTime, MaxActorsPerBiome);
}

bool UEng_ArchitecturalStandards::ValidateAgentFileCount(int32 AgentID, int32 FilesCreated) const
{
    FEng_ModuleStandard Standard = GetStandardForAgent(AgentID);
    return FilesCreated >= Standard.MinimumFilesRequired;
}

bool UEng_ArchitecturalStandards::ValidateAgentCommandCount(int32 AgentID, int32 UE5Commands) const
{
    FEng_ModuleStandard Standard = GetStandardForAgent(AgentID);
    return UE5Commands >= Standard.MinimumUE5Commands;
}