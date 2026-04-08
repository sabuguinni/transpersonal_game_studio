#include "DinosaurMemorySystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UDinosaurMemorySystem::UDinosaurMemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick a cada segundo
}

void UDinosaurMemorySystem::BeginPlay()
{
    Super::BeginPlay();
    LastCleanupTime = GetCurrentGameTime();
}

void UDinosaurMemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetCurrentGameTime();
    
    // Limpeza periódica de memórias antigas
    if (CurrentTime - LastCleanupTime >= CleanupInterval)
    {
        CleanupOldMemories();
        LastCleanupTime = CurrentTime;
    }
}

void UDinosaurMemorySystem::AddMemory(const FDinosaurMemory& NewMemory)
{
    FDinosaurMemory Memory = NewMemory;
    Memory.Timestamp = GetCurrentGameTime();
    
    Memories.Add(Memory);
    
    // Remove memórias antigas se exceder o limite
    if (Memories.Num() > MaxMemories)
    {
        SortMemoriesByRelevance();
        Memories.RemoveAt(Memories.Num() - 1); // Remove a menos relevante
    }
}

void UDinosaurMemorySystem::AddSimpleMemory(EMemoryType MemoryType, FVector Location, float Intensity, float EmotionalValence)
{
    FDinosaurMemory NewMemory(MemoryType, Location, Intensity, EmotionalValence);
    AddMemory(NewMemory);
}

TArray<FDinosaurMemory> UDinosaurMemorySystem::GetMemoriesOfType(EMemoryType MemoryType, bool bOnlyValid) const
{
    TArray<FDinosaurMemory> FilteredMemories;
    float CurrentTime = GetCurrentGameTime();
    
    for (const FDinosaurMemory& Memory : Memories)
    {
        if (Memory.MemoryType == MemoryType)
        {
            if (!bOnlyValid || Memory.IsValid(CurrentTime, MaxMemoryAge))
            {
                FilteredMemories.Add(Memory);
            }
        }
    }
    
    return FilteredMemories;
}

TArray<FDinosaurMemory> UDinosaurMemorySystem::GetMemoriesNearLocation(FVector Location, float Radius, bool bOnlyValid) const
{
    TArray<FDinosaurMemory> NearbyMemories;
    float CurrentTime = GetCurrentGameTime();
    float RadiusSquared = Radius * Radius;
    
    for (const FDinosaurMemory& Memory : Memories)
    {
        float DistanceSquared = FVector::DistSquared(Memory.Location, Location);
        
        if (DistanceSquared <= RadiusSquared)
        {
            if (!bOnlyValid || Memory.IsValid(CurrentTime, MaxMemoryAge))
            {
                NearbyMemories.Add(Memory);
            }
        }
    }
    
    return NearbyMemories;
}

TArray<FDinosaurMemory> UDinosaurMemorySystem::GetMemoriesAboutActor(AActor* Actor, bool bOnlyValid) const
{
    TArray<FDinosaurMemory> ActorMemories;
    float CurrentTime = GetCurrentGameTime();
    
    if (!Actor)
    {
        return ActorMemories;
    }
    
    for (const FDinosaurMemory& Memory : Memories)
    {
        if (Memory.AssociatedActor.IsValid() && Memory.AssociatedActor.Get() == Actor)
        {
            if (!bOnlyValid || Memory.IsValid(CurrentTime, MaxMemoryAge))
            {
                ActorMemories.Add(Memory);
            }
        }
    }
    
    return ActorMemories;
}

float UDinosaurMemorySystem::GetPlayerImpression() const
{
    TArray<FDinosaurMemory> PlayerMemories;
    float CurrentTime = GetCurrentGameTime();
    
    // Coleta memórias relacionadas ao jogador
    for (const FDinosaurMemory& Memory : Memories)
    {
        if (Memory.MemoryType == EMemoryType::PlayerSighting ||
            Memory.MemoryType == EMemoryType::PlayerThreat ||
            Memory.MemoryType == EMemoryType::PlayerKindness ||
            Memory.MemoryType == EMemoryType::PlayerFeeding)
        {
            if (Memory.IsValid(CurrentTime, MaxMemoryAge))
            {
                PlayerMemories.Add(Memory);
            }
        }
    }
    
    if (PlayerMemories.Num() == 0)
    {
        return 0.0f; // Neutro - sem impressão
    }
    
    // Calcula impressão ponderada pela intensidade e recência
    float TotalImpression = 0.0f;
    float TotalWeight = 0.0f;
    
    for (const FDinosaurMemory& Memory : PlayerMemories)
    {
        float CurrentIntensity = Memory.GetCurrentIntensity(CurrentTime, MemoryDecayRate);
        float Weight = CurrentIntensity;
        
        TotalImpression += Memory.EmotionalValence * Weight;
        TotalWeight += Weight;
    }
    
    return TotalWeight > 0.0f ? TotalImpression / TotalWeight : 0.0f;
}

float UDinosaurMemorySystem::GetLocationFamiliarity(FVector Location, float Radius) const
{
    TArray<FDinosaurMemory> NearbyMemories = GetMemoriesNearLocation(Location, Radius, true);
    
    if (NearbyMemories.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalFamiliarity = 0.0f;
    float CurrentTime = GetCurrentGameTime();
    
    for (const FDinosaurMemory& Memory : NearbyMemories)
    {
        float CurrentIntensity = Memory.GetCurrentIntensity(CurrentTime, MemoryDecayRate);
        TotalFamiliarity += CurrentIntensity;
    }
    
    // Normaliza baseado no número de memórias (mais memórias = mais familiar)
    return FMath::Min(1.0f, TotalFamiliarity / 5.0f); // Máximo de familiaridade com 5 memórias intensas
}

bool UDinosaurMemorySystem::HasNegativeMemoriesAtLocation(FVector Location, float Radius) const
{
    TArray<FDinosaurMemory> NearbyMemories = GetMemoriesNearLocation(Location, Radius, true);
    
    for (const FDinosaurMemory& Memory : NearbyMemories)
    {
        if (Memory.EmotionalValence < -0.3f) // Considera negativo se valência < -0.3
        {
            return true;
        }
    }
    
    return false;
}

FVector UDinosaurMemorySystem::GetBestKnownLocationForActivity(EMemoryType ActivityType) const
{
    TArray<FDinosaurMemory> ActivityMemories = GetMemoriesOfType(ActivityType, true);
    
    if (ActivityMemories.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    // Encontra a localização com melhor combinação de valência positiva e intensidade
    FVector BestLocation = FVector::ZeroVector;
    float BestScore = -1.0f;
    float CurrentTime = GetCurrentGameTime();
    
    for (const FDinosaurMemory& Memory : ActivityMemories)
    {
        float CurrentIntensity = Memory.GetCurrentIntensity(CurrentTime, MemoryDecayRate);
        float Score = (Memory.EmotionalValence + 1.0f) * 0.5f * CurrentIntensity; // Normaliza valência para [0,1]
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestLocation = Memory.Location;
        }
    }
    
    return BestLocation;
}

void UDinosaurMemorySystem::CleanupOldMemories()
{
    RemoveExpiredMemories();
    
    // Se ainda há muitas memórias, remove as menos relevantes
    if (Memories.Num() > MaxMemories)
    {
        SortMemoriesByRelevance();
        int32 MemoriesToRemove = Memories.Num() - MaxMemories;
        Memories.RemoveAt(Memories.Num() - MemoriesToRemove, MemoriesToRemove);
    }
}

void UDinosaurMemorySystem::ClearAllMemories()
{
    Memories.Empty();
}

float UDinosaurMemorySystem::GetCurrentGameTime() const
{
    UWorld* World = GetWorld();
    return World ? World->GetTimeSeconds() : 0.0f;
}

void UDinosaurMemorySystem::RemoveExpiredMemories()
{
    float CurrentTime = GetCurrentGameTime();
    
    Memories.RemoveAll([CurrentTime, this](const FDinosaurMemory& Memory)
    {
        return !Memory.IsValid(CurrentTime, MaxMemoryAge);
    });
}

void UDinosaurMemorySystem::SortMemoriesByRelevance()
{
    float CurrentTime = GetCurrentGameTime();
    
    Memories.Sort([CurrentTime, this](const FDinosaurMemory& A, const FDinosaurMemory& B)
    {
        float ScoreA = A.GetCurrentIntensity(CurrentTime, MemoryDecayRate) * (FMath::Abs(A.EmotionalValence) + 0.5f);
        float ScoreB = B.GetCurrentIntensity(CurrentTime, MemoryDecayRate) * (FMath::Abs(B.EmotionalValence) + 0.5f);
        return ScoreA > ScoreB; // Ordena do mais relevante para o menos relevante
    });
}