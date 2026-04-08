#include "NPCMemorySystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNPCMemorySystem::UNPCMemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick a cada segundo
}

void UNPCMemorySystem::BeginPlay()
{
    Super::BeginPlay();
    
    LastMemoryProcessTime = GetWorld()->GetTimeSeconds();
    
    if (bLogMemoryOperations)
    {
        UE_LOG(LogTemp, Log, TEXT("NPCMemorySystem initialized for %s"), *GetOwner()->GetName());
    }
}

void UNPCMemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Processa decay de memórias no intervalo definido
    if (bEnableMemoryDecay && (CurrentTime - LastMemoryProcessTime) >= MemoryProcessingInterval)
    {
        ProcessMemoryDecay();
        LastMemoryProcessTime = CurrentTime;
    }
    
    // Remove memórias fracas se excedeu o limite
    if (Memories.Num() > MaxMemoryEntries)
    {
        RemoveWeakestMemories();
    }
}

void UNPCMemorySystem::AddMemory(const FNPCMemoryEntry& NewMemory)
{
    if (!IsMemoryValid(NewMemory))
    {
        if (bLogMemoryOperations)
        {
            UE_LOG(LogTemp, Warning, TEXT("Attempted to add invalid memory"));
        }
        return;
    }
    
    // Verifica se já existe uma memória similar e a reforça em vez de duplicar
    for (int32 i = 0; i < Memories.Num(); i++)
    {
        FNPCMemoryEntry& ExistingMemory = Memories[i];
        
        // Memória do mesmo actor
        if (NewMemory.RelatedActor.IsValid() && 
            ExistingMemory.RelatedActor.IsValid() && 
            NewMemory.RelatedActor == ExistingMemory.RelatedActor &&
            NewMemory.MemoryType == ExistingMemory.MemoryType)
        {
            ReinforceMemory(i, 0.2f);
            ExistingMemory.Description = NewMemory.Description; // Atualiza descrição
            ExistingMemory.EmotionalWeight = FMath::Lerp(ExistingMemory.EmotionalWeight, NewMemory.EmotionalWeight, 0.3f);
            return;
        }
        
        // Memória da mesma localização
        if (NewMemory.MemoryType == ExistingMemory.MemoryType &&
            FVector::Dist(NewMemory.Location, ExistingMemory.Location) < 200.0f)
        {
            ReinforceMemory(i, 0.1f);
            ExistingMemory.Description = NewMemory.Description;
            ExistingMemory.EmotionalWeight = FMath::Lerp(ExistingMemory.EmotionalWeight, NewMemory.EmotionalWeight, 0.2f);
            return;
        }
    }
    
    // Adiciona nova memória
    FNPCMemoryEntry MemoryToAdd = NewMemory;
    MemoryToAdd.Timestamp = FDateTime::Now();
    
    Memories.Add(MemoryToAdd);
    
    // Ordena por força se necessário
    SortMemoriesByStrength();
    
    OnMemoryAdded.Broadcast(MemoryToAdd);
    
    if (bLogMemoryOperations)
    {
        UE_LOG(LogTemp, Log, TEXT("Added memory: %s (Type: %s, Weight: %.2f)"), 
               *MemoryToAdd.Description, *MemoryToAdd.MemoryType, MemoryToAdd.EmotionalWeight);
    }
}

void UNPCMemorySystem::AddLocationMemory(const FVector& Location, const FString& Description, float EmotionalWeight)
{
    FNPCMemoryEntry NewMemory;
    NewMemory.MemoryType = TEXT("Location");
    NewMemory.Location = Location;
    NewMemory.Description = Description;
    NewMemory.EmotionalWeight = EmotionalWeight;
    NewMemory.MemoryStrength = 1.0f;
    
    AddMemory(NewMemory);
}

void UNPCMemorySystem::AddActorMemory(AActor* Actor, const FString& Description, float EmotionalWeight)
{
    if (!IsValid(Actor))
    {
        return;
    }
    
    FNPCMemoryEntry NewMemory;
    NewMemory.MemoryType = TEXT("Actor");
    NewMemory.RelatedActor = Actor;
    NewMemory.Location = Actor->GetActorLocation();
    NewMemory.Description = Description;
    NewMemory.EmotionalWeight = EmotionalWeight;
    NewMemory.MemoryStrength = 1.0f;
    
    AddMemory(NewMemory);
}

void UNPCMemorySystem::AddEventMemory(const FString& EventType, const FString& Description, float EmotionalWeight, const FVector& Location)
{
    FNPCMemoryEntry NewMemory;
    NewMemory.MemoryType = EventType;
    NewMemory.Location = Location.IsZero() ? GetOwner()->GetActorLocation() : Location;
    NewMemory.Description = Description;
    NewMemory.EmotionalWeight = EmotionalWeight;
    NewMemory.MemoryStrength = 1.0f;
    
    AddMemory(NewMemory);
}

void UNPCMemorySystem::ReinforceMemory(int32 MemoryIndex, float StrengthBonus)
{
    if (!Memories.IsValidIndex(MemoryIndex))
    {
        return;
    }
    
    FNPCMemoryEntry& Memory = Memories[MemoryIndex];
    Memory.MemoryStrength = FMath::Clamp(Memory.MemoryStrength + StrengthBonus, 0.0f, 2.0f);
    Memory.ReinforcementCount++;
    Memory.Timestamp = FDateTime::Now(); // Atualiza timestamp
    
    OnMemoryReinforced.Broadcast(Memory);
    
    if (bLogMemoryOperations)
    {
        UE_LOG(LogTemp, Log, TEXT("Reinforced memory: %s (New Strength: %.2f)"), 
               *Memory.Description, Memory.MemoryStrength);
    }
}

void UNPCMemorySystem::ForgetMemory(int32 MemoryIndex)
{
    if (!Memories.IsValidIndex(MemoryIndex))
    {
        return;
    }
    
    FNPCMemoryEntry ForgottenMemory = Memories[MemoryIndex];
    Memories.RemoveAt(MemoryIndex);
    
    OnMemoryForgotten.Broadcast(ForgottenMemory);
    
    if (bLogMemoryOperations)
    {
        UE_LOG(LogTemp, Log, TEXT("Forgot memory: %s"), *ForgottenMemory.Description);
    }
}

void UNPCMemorySystem::ClearAllMemories()
{
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        OnMemoryForgotten.Broadcast(Memory);
    }
    
    Memories.Empty();
    
    if (bLogMemoryOperations)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleared all memories"));
    }
}

TArray<FNPCMemoryEntry> UNPCMemorySystem::QueryMemories(const FMemoryQuery& Query) const
{
    TArray<FNPCMemoryEntry> Results;
    
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        bool bMatches = true;
        
        // Filtro por tipo
        if (!Query.MemoryType.IsEmpty() && Memory.MemoryType != Query.MemoryType)
        {
            bMatches = false;
        }
        
        // Filtro por actor
        if (bMatches && Query.RelatedActor && Memory.RelatedActor != Query.RelatedActor)
        {
            bMatches = false;
        }
        
        // Filtro por localização
        if (bMatches && !Query.SearchLocation.IsZero())
        {
            float Distance = FVector::Dist(Memory.Location, Query.SearchLocation);
            if (Distance > Query.SearchRadius)
            {
                bMatches = false;
            }
        }
        
        // Filtro por peso emocional
        if (bMatches && Memory.EmotionalWeight < Query.MinEmotionalWeight)
        {
            bMatches = false;
        }
        
        // Filtro por idade
        if (bMatches && Query.MaxAge > 0.0f)
        {
            FTimespan Age = FDateTime::Now() - Memory.Timestamp;
            if (Age.GetTotalDays() > Query.MaxAge)
            {
                bMatches = false;
            }
        }
        
        if (bMatches)
        {
            Results.Add(Memory);
        }
    }
    
    // Ordena por força se solicitado
    if (Query.bSortByStrength)
    {
        Results.Sort([](const FNPCMemoryEntry& A, const FNPCMemoryEntry& B) {
            return A.MemoryStrength > B.MemoryStrength;
        });
    }
    
    return Results;
}

TArray<FNPCMemoryEntry> UNPCMemorySystem::GetMemoriesByType(const FString& MemoryType) const
{
    FMemoryQuery Query;
    Query.MemoryType = MemoryType;
    return QueryMemories(Query);
}

TArray<FNPCMemoryEntry> UNPCMemorySystem::GetMemoriesOfActor(AActor* Actor) const
{
    FMemoryQuery Query;
    Query.RelatedActor = Actor;
    return QueryMemories(Query);
}

TArray<FNPCMemoryEntry> UNPCMemorySystem::GetMemoriesNearLocation(const FVector& Location, float Radius) const
{
    FMemoryQuery Query;
    Query.SearchLocation = Location;
    Query.SearchRadius = Radius;
    return QueryMemories(Query);
}

FNPCMemoryEntry UNPCMemorySystem::GetStrongestMemory() const
{
    if (Memories.Num() == 0)
    {
        return FNPCMemoryEntry();
    }
    
    const FNPCMemoryEntry* StrongestMemory = &Memories[0];
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (Memory.MemoryStrength > StrongestMemory->MemoryStrength)
        {
            StrongestMemory = &Memory;
        }
    }
    
    return *StrongestMemory;
}

FNPCMemoryEntry UNPCMemorySystem::GetMostRecentMemory() const
{
    if (Memories.Num() == 0)
    {
        return FNPCMemoryEntry();
    }
    
    const FNPCMemoryEntry* MostRecent = &Memories[0];
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (Memory.Timestamp > MostRecent->Timestamp)
        {
            MostRecent = &Memory;
        }
    }
    
    return *MostRecent;
}

bool UNPCMemorySystem::HasMemoryOfActor(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (Memory.RelatedActor.IsValid() && Memory.RelatedActor.Get() == Actor)
        {
            return true;
        }
    }
    
    return false;
}

bool UNPCMemorySystem::HasMemoryOfLocation(const FVector& Location, float Tolerance) const
{
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (FVector::Dist(Memory.Location, Location) <= Tolerance)
        {
            return true;
        }
    }
    
    return false;
}

float UNPCMemorySystem::GetEmotionalAssociationWithActor(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return 0.0f;
    }
    
    float TotalWeight = 0.0f;
    float TotalStrength = 0.0f;
    
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (Memory.RelatedActor.IsValid() && Memory.RelatedActor.Get() == Actor)
        {
            TotalWeight += Memory.EmotionalWeight * Memory.MemoryStrength;
            TotalStrength += Memory.MemoryStrength;
        }
    }
    
    return TotalStrength > 0.0f ? TotalWeight / TotalStrength : 0.0f;
}

float UNPCMemorySystem::GetEmotionalAssociationWithLocation(const FVector& Location, float Radius) const
{
    float TotalWeight = 0.0f;
    float TotalStrength = 0.0f;
    
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        float Distance = FVector::Dist(Memory.Location, Location);
        if (Distance <= Radius)
        {
            // Peso inversamente proporcional à distância
            float DistanceWeight = 1.0f - (Distance / Radius);
            float EffectiveStrength = Memory.MemoryStrength * DistanceWeight;
            
            TotalWeight += Memory.EmotionalWeight * EffectiveStrength;
            TotalStrength += EffectiveStrength;
        }
    }
    
    return TotalStrength > 0.0f ? TotalWeight / TotalStrength : 0.0f;
}

TArray<AActor*> UNPCMemorySystem::GetKnownActors() const
{
    TArray<AActor*> KnownActors;
    
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (Memory.RelatedActor.IsValid())
        {
            KnownActors.AddUnique(Memory.RelatedActor.Get());
        }
    }
    
    return KnownActors;
}

TArray<FVector> UNPCMemorySystem::GetKnownLocations() const
{
    TArray<FVector> KnownLocations;
    
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        if (!Memory.Location.IsZero())
        {
            KnownLocations.AddUnique(Memory.Location);
        }
    }
    
    return KnownLocations;
}

float UNPCMemorySystem::GetAverageMemoryStrength() const
{
    if (Memories.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalStrength = 0.0f;
    for (const FNPCMemoryEntry& Memory : Memories)
    {
        TotalStrength += Memory.MemoryStrength;
    }
    
    return TotalStrength / Memories.Num();
}

void UNPCMemorySystem::ProcessMemoryDecay()
{
    TArray<int32> MemoriesToForget;
    
    for (int32 i = 0; i < Memories.Num(); i++)
    {
        FNPCMemoryEntry& Memory = Memories[i];
        
        if (ShouldForgetMemory(Memory))
        {
            MemoriesToForget.Add(i);
            continue;
        }
        
        // Aplica decay
        float DecayAmount = CalculateMemoryDecay(Memory, MemoryProcessingInterval);
        Memory.MemoryStrength = FMath::Max(Memory.MemoryStrength - DecayAmount, 0.0f);
    }
    
    // Remove memórias esquecidas (em ordem reversa para não afetar índices)
    for (int32 i = MemoriesToForget.Num() - 1; i >= 0; i--)
    {
        ForgetMemory(MemoriesToForget[i]);
    }
}

void UNPCMemorySystem::RemoveWeakestMemories()
{
    // Ordena por força (mais fraca primeiro)
    Memories.Sort([](const FNPCMemoryEntry& A, const FNPCMemoryEntry& B) {
        return A.MemoryStrength < B.MemoryStrength;
    });
    
    // Remove as mais fracas até ficar dentro do limite
    while (Memories.Num() > MaxMemoryEntries)
    {
        ForgetMemory(0);
    }
    
    // Reordena por força (mais forte primeiro)
    SortMemoriesByStrength();
}

bool UNPCMemorySystem::ShouldForgetMemory(const FNPCMemoryEntry& Memory) const
{
    // Memórias críticas não são esquecidas
    if (Memory.MemoryStrength >= DecaySettings.CriticalMemoryThreshold)
    {
        return false;
    }
    
    // Esquece se a força caiu abaixo do mínimo
    return Memory.MemoryStrength <= DecaySettings.MinimumStrength;
}

float UNPCMemorySystem::CalculateMemoryDecay(const FNPCMemoryEntry& Memory, float DeltaTime) const
{
    float BaseDecay = DecaySettings.BaseDecayRate * (DeltaTime / 86400.0f); // Converte para dias
    
    // Memórias emocionais decaem mais devagar
    float EmotionalModifier = 1.0f - (FMath::Abs(Memory.EmotionalWeight) * DecaySettings.EmotionalWeightMultiplier);
    EmotionalModifier = FMath::Clamp(EmotionalModifier, 0.1f, 1.0f);
    
    // Reforços reduzem o decay
    float ReinforcementModifier = 1.0f - (Memory.ReinforcementCount * DecaySettings.ReinforcementBonus);
    ReinforcementModifier = FMath::Clamp(ReinforcementModifier, 0.1f, 1.0f);
    
    return BaseDecay * EmotionalModifier * ReinforcementModifier;
}

int32 UNPCMemorySystem::FindMemoryIndex(const FNPCMemoryEntry& Memory) const
{
    for (int32 i = 0; i < Memories.Num(); i++)
    {
        if (Memories[i].Timestamp == Memory.Timestamp && 
            Memories[i].Description == Memory.Description)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void UNPCMemorySystem::SortMemoriesByStrength()
{
    Memories.Sort([](const FNPCMemoryEntry& A, const FNPCMemoryEntry& B) {
        return A.MemoryStrength > B.MemoryStrength;
    });
}

FString UNPCMemorySystem::GenerateMemoryID() const
{
    return FString::Printf(TEXT("Memory_%s_%d"), *GetOwner()->GetName(), Memories.Num());
}

bool UNPCMemorySystem::IsMemoryValid(const FNPCMemoryEntry& Memory) const
{
    // Verifica se tem pelo menos tipo e descrição
    if (Memory.MemoryType.IsEmpty() || Memory.Description.IsEmpty())
    {
        return false;
    }
    
    // Verifica se a força está no range válido
    if (Memory.MemoryStrength < 0.0f || Memory.MemoryStrength > 2.0f)
    {
        return false;
    }
    
    // Verifica se o peso emocional está no range válido
    if (Memory.EmotionalWeight < -1.0f || Memory.EmotionalWeight > 1.0f)
    {
        return false;
    }
    
    return true;
}

void UNPCMemorySystem::ValidateMemoryIntegrity()
{
    TArray<int32> InvalidMemories;
    
    for (int32 i = 0; i < Memories.Num(); i++)
    {
        if (!IsMemoryValid(Memories[i]))
        {
            InvalidMemories.Add(i);
        }
    }
    
    // Remove memórias inválidas
    for (int32 i = InvalidMemories.Num() - 1; i >= 0; i--)
    {
        ForgetMemory(InvalidMemories[i]);
    }
}