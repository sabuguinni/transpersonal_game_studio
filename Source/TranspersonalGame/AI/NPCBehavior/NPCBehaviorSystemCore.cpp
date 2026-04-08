#include "NPCBehaviorSystemCore.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY(LogNPCBehavior);

UNPCBehaviorSystemCore::UNPCBehaviorSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 100ms para performance
    
    // Valores padrão
    SpeciesType = ENPCSpeciesType::SmallHerbivore;
    NPCName = TEXT("Unnamed NPC");
    UniqueID = 0;
    
    NeedsDecayRate = 0.1f;
    MemoryDecayRate = 0.05f;
    MaxMemoryEntries = 20;
    RoutineFlexibility = 0.3f;
    
    CurrentBehaviorState = ENPCBehaviorState::Idle;
    DomesticationLevel = ENPCDomesticationLevel::Wild;
    PlayerRelationship = 0.0f;
    
    TerritoryCenter = FVector::ZeroVector;
    TerritoryRadius = 1000.0f;
    bHasTerritory = false;
}

void UNPCBehaviorSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Gerar ID único se não foi definido
    if (UniqueID == 0)
    {
        UniqueID = FMath::RandRange(10000, 99999);
    }
    
    // Definir território inicial se não foi configurado
    if (!bHasTerritory && GetOwner())
    {
        TerritoryCenter = GetOwner()->GetActorLocation();
        bHasTerritory = true;
    }
    
    UE_LOG(LogNPCBehavior, Log, TEXT("NPC %s (ID: %d) initialized with species type %d"), 
           *NPCName, UniqueID, (int32)SpeciesType);
}

void UNPCBehaviorSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Atualizar necessidades
    UpdateNeeds(DeltaTime);
    
    // Decair memórias
    DecayMemories(DeltaTime);
    
    // Processar rotinas
    ProcessRoutines();
    
    // Atualizar estado comportamental
    UpdateBehaviorState();
}

void UNPCBehaviorSystemCore::AddMemoryEntry(AActor* Target, ENPCThreatLevel ThreatLevel, const FString& InteractionType)
{
    if (!Target)
    {
        return;
    }
    
    // Procurar entrada existente
    FNPCMemoryEntry* ExistingEntry = GetMemoryEntry(Target);
    
    if (ExistingEntry)
    {
        // Atualizar entrada existente
        ExistingEntry->LastKnownLocation = Target->GetActorLocation();
        ExistingEntry->ThreatLevel = ThreatLevel;
        ExistingEntry->MemoryStrength = FMath::Min(1.0f, ExistingEntry->MemoryStrength + 0.2f);
        ExistingEntry->LastSeenTime = GetWorld()->GetTimeSeconds();
        ExistingEntry->InteractionHistory += FString::Printf(TEXT("[%.1f] %s; "), 
                                                            GetWorld()->GetTimeSeconds(), *InteractionType);
    }
    else
    {
        // Criar nova entrada
        FNPCMemoryEntry NewEntry;
        NewEntry.TargetActor = Target;
        NewEntry.LastKnownLocation = Target->GetActorLocation();
        NewEntry.ThreatLevel = ThreatLevel;
        NewEntry.MemoryStrength = 1.0f;
        NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewEntry.InteractionHistory = FString::Printf(TEXT("[%.1f] %s; "), 
                                                     GetWorld()->GetTimeSeconds(), *InteractionType);
        
        MemoryEntries.Add(NewEntry);
        
        // Limitar número de memórias
        if (MemoryEntries.Num() > MaxMemoryEntries)
        {
            // Remover a memória mais fraca
            int32 WeakestIndex = 0;
            float WeakestStrength = MemoryEntries[0].MemoryStrength;
            
            for (int32 i = 1; i < MemoryEntries.Num(); i++)
            {
                if (MemoryEntries[i].MemoryStrength < WeakestStrength)
                {
                    WeakestStrength = MemoryEntries[i].MemoryStrength;
                    WeakestIndex = i;
                }
            }
            
            MemoryEntries.RemoveAt(WeakestIndex);
        }
    }
    
    UE_LOG(LogNPCBehavior, Verbose, TEXT("NPC %s added memory of %s with threat level %d"), 
           *NPCName, *Target->GetName(), (int32)ThreatLevel);
}

FNPCMemoryEntry* UNPCBehaviorSystemCore::GetMemoryEntry(AActor* Target)
{
    if (!Target)
    {
        return nullptr;
    }
    
    for (FNPCMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.TargetActor == Target)
        {
            return &Entry;
        }
    }
    
    return nullptr;
}

void UNPCBehaviorSystemCore::UpdateNeeds(float DeltaTime)
{
    // Decay natural das necessidades
    CurrentNeeds.Hunger = FMath::Max(0.0f, CurrentNeeds.Hunger - (NeedsDecayRate * DeltaTime));
    CurrentNeeds.Thirst = FMath::Max(0.0f, CurrentNeeds.Thirst - (NeedsDecayRate * DeltaTime * 1.5f)); // Sede decay mais rápido
    CurrentNeeds.Energy = FMath::Max(0.0f, CurrentNeeds.Energy - (NeedsDecayRate * DeltaTime * 0.5f)); // Energia decay mais lento
    CurrentNeeds.Social = FMath::Max(0.0f, CurrentNeeds.Social - (NeedsDecayRate * DeltaTime * 0.3f));
    
    // Safety e Comfort são influenciados pelo ambiente e não decaem naturalmente
    
    // Aplicar modificadores de personalidade
    float PersonalityModifier = 1.0f + (PersonalityTraits.Aggressiveness * 0.2f); // NPCs agressivos têm necessidades mais intensas
    CurrentNeeds.Hunger *= PersonalityModifier;
    CurrentNeeds.Thirst *= PersonalityModifier;
}

ENPCBehaviorState UNPCBehaviorSystemCore::GetMostUrgentNeed()
{
    float LowestNeed = 1.0f;
    ENPCBehaviorState UrgentState = ENPCBehaviorState::Idle;
    
    if (CurrentNeeds.Thirst < LowestNeed)
    {
        LowestNeed = CurrentNeeds.Thirst;
        UrgentState = ENPCBehaviorState::Drinking;
    }
    
    if (CurrentNeeds.Hunger < LowestNeed)
    {
        LowestNeed = CurrentNeeds.Hunger;
        UrgentState = ENPCBehaviorState::Foraging;
    }
    
    if (CurrentNeeds.Energy < LowestNeed)
    {
        LowestNeed = CurrentNeeds.Energy;
        UrgentState = ENPCBehaviorState::Resting;
    }
    
    if (CurrentNeeds.Safety < 0.3f) // Safety é crítica
    {
        UrgentState = ENPCBehaviorState::Fleeing;
    }
    
    if (CurrentNeeds.Social < LowestNeed && PersonalityTraits.Sociability > 0.6f)
    {
        LowestNeed = CurrentNeeds.Social;
        UrgentState = ENPCBehaviorState::Socializing;
    }
    
    return UrgentState;
}

bool UNPCBehaviorSystemCore::ShouldFollowRoutine()
{
    // Verificar se as necessidades básicas estão satisfeitas
    if (CurrentNeeds.Hunger < 0.3f || CurrentNeeds.Thirst < 0.3f || CurrentNeeds.Energy < 0.2f)
    {
        return false;
    }
    
    // Verificar se está em perigo
    if (CurrentNeeds.Safety < 0.5f)
    {
        return false;
    }
    
    // Aplicar flexibilidade da personalidade
    float RoutineChance = 1.0f - RoutineFlexibility;
    RoutineChance += PersonalityTraits.Patience * 0.3f; // NPCs pacientes seguem mais rotinas
    
    return FMath::RandRange(0.0f, 1.0f) < RoutineChance;
}

FNPCDailyRoutine UNPCBehaviorSystemCore::GetCurrentRoutine()
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    for (const FNPCDailyRoutine& Routine : DailyRoutines)
    {
        float EndTime = Routine.StartTime + (Routine.Duration / 24.0f);
        
        if (CurrentTime >= Routine.StartTime && CurrentTime <= EndTime)
        {
            return Routine;
        }
    }
    
    // Retornar rotina padrão se nenhuma for encontrada
    FNPCDailyRoutine DefaultRoutine;
    DefaultRoutine.BehaviorState = ENPCBehaviorState::Idle;
    return DefaultRoutine;
}

void UNPCBehaviorSystemCore::ModifyPlayerRelationship(float Delta)
{
    PlayerRelationship = FMath::Clamp(PlayerRelationship + Delta, -1.0f, 1.0f);
    
    // Atualizar nível de domesticação baseado no relacionamento
    if (PlayerRelationship > 0.8f)
    {
        DomesticationLevel = ENPCDomesticationLevel::Bonded;
    }
    else if (PlayerRelationship > 0.6f)
    {
        DomesticationLevel = ENPCDomesticationLevel::Friendly;
    }
    else if (PlayerRelationship > 0.4f)
    {
        DomesticationLevel = ENPCDomesticationLevel::Tolerant;
    }
    else if (PlayerRelationship > 0.2f)
    {
        DomesticationLevel = ENPCDomesticationLevel::Curious;
    }
    else if (PlayerRelationship > -0.2f)
    {
        DomesticationLevel = ENPCDomesticationLevel::Wary;
    }
    else
    {
        DomesticationLevel = ENPCDomesticationLevel::Wild;
    }
    
    UE_LOG(LogNPCBehavior, Log, TEXT("NPC %s relationship with player: %.2f (Domestication: %d)"), 
           *NPCName, PlayerRelationship, (int32)DomesticationLevel);
}

bool UNPCBehaviorSystemCore::IsInTerritory(const FVector& Location)
{
    if (!bHasTerritory)
    {
        return true; // Sem território definido, considera sempre dentro
    }
    
    float Distance = FVector::Dist(Location, TerritoryCenter);
    return Distance <= TerritoryRadius;
}

float UNPCBehaviorSystemCore::GetDistanceFromTerritory(const FVector& Location)
{
    if (!bHasTerritory)
    {
        return 0.0f;
    }
    
    float Distance = FVector::Dist(Location, TerritoryCenter);
    return FMath::Max(0.0f, Distance - TerritoryRadius);
}

void UNPCBehaviorSystemCore::DecayMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        FNPCMemoryEntry& Entry = MemoryEntries[i];
        
        // Calcular decay baseado no tempo
        float TimeSinceLastSeen = CurrentTime - Entry.LastSeenTime;
        float DecayAmount = MemoryDecayRate * DeltaTime * (1.0f + TimeSinceLastSeen * 0.1f);
        
        Entry.MemoryStrength -= DecayAmount;
        
        // Remover memórias muito fracas
        if (Entry.MemoryStrength <= 0.0f)
        {
            UE_LOG(LogNPCBehavior, Verbose, TEXT("NPC %s forgot about %s"), 
                   *NPCName, Entry.TargetActor ? *Entry.TargetActor->GetName() : TEXT("Unknown"));
            MemoryEntries.RemoveAt(i);
        }
    }
}

void UNPCBehaviorSystemCore::ProcessRoutines()
{
    if (!ShouldFollowRoutine())
    {
        return;
    }
    
    FNPCDailyRoutine CurrentRoutine = GetCurrentRoutine();
    
    // Se a rotina atual é diferente do estado comportamental, considerar mudança
    if (CurrentRoutine.BehaviorState != CurrentBehaviorState)
    {
        // Aplicar prioridade da rotina vs necessidades urgentes
        ENPCBehaviorState UrgentNeed = GetMostUrgentNeed();
        
        if (CurrentRoutine.Priority > 0.7f || UrgentNeed == ENPCBehaviorState::Idle)
        {
            CurrentBehaviorState = CurrentRoutine.BehaviorState;
        }
    }
}

float UNPCBehaviorSystemCore::GetCurrentTimeOfDay()
{
    // Implementação simples - pode ser expandida para usar sistema de tempo do jogo
    if (UWorld* World = GetWorld())
    {
        float GameTime = World->GetTimeSeconds();
        float DayLength = 1200.0f; // 20 minutos = 1 dia do jogo
        return FMath::Fmod(GameTime / DayLength, 1.0f);
    }
    
    return 0.0f;
}

void UNPCBehaviorSystemCore::UpdateBehaviorState()
{
    // Verificar se precisa mudar estado baseado em necessidades urgentes
    ENPCBehaviorState UrgentNeed = GetMostUrgentNeed();
    
    if (UrgentNeed != ENPCBehaviorState::Idle && UrgentNeed != CurrentBehaviorState)
    {
        // Priorizar necessidades críticas
        if (UrgentNeed == ENPCBehaviorState::Fleeing || 
            (UrgentNeed == ENPCBehaviorState::Drinking && CurrentNeeds.Thirst < 0.2f) ||
            (UrgentNeed == ENPCBehaviorState::Foraging && CurrentNeeds.Hunger < 0.2f))
        {
            CurrentBehaviorState = UrgentNeed;
        }
    }
}