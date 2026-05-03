#include "ProductionCoordinator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UProductionCoordinator::UProductionCoordinator()
{
    // Initialize milestone tracking
    InitializeMilestone1Tasks();
    InitializeBiomeData();
}

void UProductionCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Sistema inicializado"));
    
    // Validate current map state
    ValidateMapState();
}

void UProductionCoordinator::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Sistema terminado"));
    Super::Deinitialize();
}

void UProductionCoordinator::InitializeMilestone1Tasks()
{
    // Milestone 1: "Walk Around" tasks
    Milestone1Tasks.Empty();
    
    // Core character movement
    Milestone1Tasks.Add(TEXT("ThirdPersonCharacter"), false);
    Milestone1Tasks.Add(TEXT("CameraBoom"), false);
    Milestone1Tasks.Add(TEXT("FollowCamera"), false);
    Milestone1Tasks.Add(TEXT("WASDMovement"), false);
    Milestone1Tasks.Add(TEXT("RunJump"), false);
    
    // World and environment
    Milestone1Tasks.Add(TEXT("LandscapeTerrain"), false);
    Milestone1Tasks.Add(TEXT("BasicTerrain"), false);
    Milestone1Tasks.Add(TEXT("StaticDinosaurs"), false);
    Milestone1Tasks.Add(TEXT("DinosaurMeshes"), false);
    
    // Lighting and atmosphere
    Milestone1Tasks.Add(TEXT("DirectionalLight"), false);
    Milestone1Tasks.Add(TEXT("SkyAtmosphere"), false);
    Milestone1Tasks.Add(TEXT("ExponentialFog"), false);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: %d tarefas Milestone 1 inicializadas"), Milestone1Tasks.Num());
}

void UProductionCoordinator::InitializeBiomeData()
{
    BiomeData.Empty();
    
    // Biome 1: PANTANO (sudoeste)
    FDir_BiomeData PantanoData;
    PantanoData.Center = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoData.MinBounds = FVector(-77500.0f, -76500.0f, -1000.0f);
    PantanoData.MaxBounds = FVector(-25000.0f, -15000.0f, 1000.0f);
    PantanoData.Name = TEXT("Pantano");
    BiomeData.Add(EDir_BiomeType::Swamp, PantanoData);
    
    // Biome 2: FLORESTA (noroeste)
    FDir_BiomeData FlorestaData;
    FlorestaData.Center = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaData.MinBounds = FVector(-77500.0f, 15000.0f, -1000.0f);
    FlorestaData.MaxBounds = FVector(-15000.0f, 76500.0f, 1000.0f);
    FlorestaData.Name = TEXT("Floresta");
    BiomeData.Add(EDir_BiomeType::Forest, FlorestaData);
    
    // Biome 3: SAVANA (centro)
    FDir_BiomeData SavanaData;
    SavanaData.Center = FVector(0.0f, 0.0f, 0.0f);
    SavanaData.MinBounds = FVector(-20000.0f, -20000.0f, -1000.0f);
    SavanaData.MaxBounds = FVector(20000.0f, 20000.0f, 1000.0f);
    SavanaData.Name = TEXT("Savana");
    BiomeData.Add(EDir_BiomeType::Savanna, SavanaData);
    
    // Biome 4: DESERTO (leste)
    FDir_BiomeData DesertoData;
    DesertoData.Center = FVector(55000.0f, 0.0f, 0.0f);
    DesertoData.MinBounds = FVector(25000.0f, -30000.0f, -1000.0f);
    DesertoData.MaxBounds = FVector(79500.0f, 30000.0f, 1000.0f);
    DesertoData.Name = TEXT("Deserto");
    BiomeData.Add(EDir_BiomeType::Desert, DesertoData);
    
    // Biome 5: MONTANHA NEVADA (nordeste)
    FDir_BiomeData MontanhaData;
    MontanhaData.Center = FVector(40000.0f, 50000.0f, 500.0f);
    MontanhaData.MinBounds = FVector(15000.0f, 20000.0f, -500.0f);
    MontanhaData.MaxBounds = FVector(79500.0f, 76500.0f, 2000.0f);
    MontanhaData.Name = TEXT("Montanha Nevada");
    BiomeData.Add(EDir_BiomeType::Mountain, MontanhaData);
    
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: %d biomas inicializados"), BiomeData.Num());
}

bool UProductionCoordinator::IsMilestone1Complete() const
{
    for (const auto& Task : Milestone1Tasks)
    {
        if (!Task.Value)
        {
            return false;
        }
    }
    return true;
}

float UProductionCoordinator::GetMilestone1Progress() const
{
    if (Milestone1Tasks.Num() == 0)
    {
        return 0.0f;
    }
    
    int32 CompletedTasks = 0;
    for (const auto& Task : Milestone1Tasks)
    {
        if (Task.Value)
        {
            CompletedTasks++;
        }
    }
    
    return static_cast<float>(CompletedTasks) / static_cast<float>(Milestone1Tasks.Num());
}

void UProductionCoordinator::UpdateTaskProgress(const FString& AgentName, const FString& TaskName, bool bCompleted)
{
    if (Milestone1Tasks.Contains(TaskName))
    {
        Milestone1Tasks[TaskName] = bCompleted;
        UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agente %s atualizou tarefa %s: %s"), 
               *AgentName, *TaskName, bCompleted ? TEXT("COMPLETA") : TEXT("PENDENTE"));
    }
}

void UProductionCoordinator::RegisterAgent(const FString& AgentName, int32 Priority)
{
    RegisteredAgents.Add(AgentName, Priority);
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Agente %s registado com prioridade %d"), *AgentName, Priority);
}

TArray<FString> UProductionCoordinator::GetActiveAgents() const
{
    TArray<FString> ActiveAgents;
    RegisteredAgents.GetKeys(ActiveAgents);
    return ActiveAgents;
}

bool UProductionCoordinator::CanAgentProceed(const FString& AgentName) const
{
    // Check if there are critical issues blocking this agent
    for (const FString& Issue : CriticalIssues)
    {
        if (Issue.Contains(AgentName) || Issue.Contains(TEXT("BLOQUEIO_GLOBAL")))
        {
            return false;
        }
    }
    return true;
}

void UProductionCoordinator::ReportCriticalIssue(const FString& AgentName, const FString& Issue)
{
    FString FormattedIssue = FString::Printf(TEXT("[%s] %s"), *AgentName, *Issue);
    CriticalIssues.AddUnique(FormattedIssue);
    UE_LOG(LogTemp, Error, TEXT("ProductionCoordinator: PROBLEMA CRÍTICO - %s"), *FormattedIssue);
}

TArray<FString> UProductionCoordinator::GetCriticalIssues() const
{
    return CriticalIssues;
}

void UProductionCoordinator::ClearCriticalIssue(const FString& Issue)
{
    CriticalIssues.Remove(Issue);
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Problema resolvido - %s"), *Issue);
}

FVector UProductionCoordinator::GetBiomeCenter(EDir_BiomeType BiomeType) const
{
    if (const FDir_BiomeData* Data = BiomeData.Find(BiomeType))
    {
        return Data->Center;
    }
    return FVector::ZeroVector;
}

FVector UProductionCoordinator::GetRandomLocationInBiome(EDir_BiomeType BiomeType) const
{
    if (const FDir_BiomeData* Data = BiomeData.Find(BiomeType))
    {
        FVector RandomOffset;
        RandomOffset.X = FMath::RandRange(-5000.0f, 5000.0f);
        RandomOffset.Y = FMath::RandRange(-5000.0f, 5000.0f);
        RandomOffset.Z = 0.0f;
        
        FVector RandomLocation = Data->Center + RandomOffset;
        
        // Clamp to biome bounds
        RandomLocation.X = FMath::Clamp(RandomLocation.X, Data->MinBounds.X, Data->MaxBounds.X);
        RandomLocation.Y = FMath::Clamp(RandomLocation.Y, Data->MinBounds.Y, Data->MaxBounds.Y);
        RandomLocation.Z = FMath::Clamp(RandomLocation.Z, Data->MinBounds.Z, Data->MaxBounds.Z);
        
        return RandomLocation;
    }
    return FVector::ZeroVector;
}

bool UProductionCoordinator::IsLocationInBiome(const FVector& Location, EDir_BiomeType BiomeType) const
{
    if (const FDir_BiomeData* Data = BiomeData.Find(BiomeType))
    {
        return Location.X >= Data->MinBounds.X && Location.X <= Data->MaxBounds.X &&
               Location.Y >= Data->MinBounds.Y && Location.Y <= Data->MaxBounds.Y &&
               Location.Z >= Data->MinBounds.Z && Location.Z <= Data->MaxBounds.Z;
    }
    return false;
}

void UProductionCoordinator::ValidateMapState()
{
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: A validar estado do MinPlayableMap..."));
    
    // This will be expanded to validate critical map elements
    // For now, just log that validation is happening
    UE_LOG(LogTemp, Warning, TEXT("ProductionCoordinator: Validação de estado completa"));
}