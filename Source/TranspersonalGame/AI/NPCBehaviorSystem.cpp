#include "NPCBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UNPCBehaviorSystem::UNPCBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Valores padrão
    Species = EDinosaurSpecies::Triceratops;
    ThreatLevel = EDinosaurThreatLevel::LowThreat;
    bCanBeDomesticated = true;
    CurrentDomesticationStage = EDomesticationStage::Wild;
    
    // Sistema de memória
    MemoryDuration = 300.0f; // 5 minutos
    MaxMemoryEntries = 20;
    
    // Sistema de domesticação
    DomesticationProgress = 0.0f;
    PlayerFamiliarity = 0.0f;
    DomesticationRate = 0.1f;
    
    // Estado inicial
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    
    // Timers
    MemoryUpdateTimer = 0.0f;
    RoutineUpdateTimer = 0.0f;
    DomesticationUpdateTimer = 0.0f;
    
    CachedTimeOfDay = 0.0f;
    bRoutineNeedsUpdate = true;
}

void UNPCBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar rotina padrão se não estiver definida
    if (DailyRoutine.Num() == 0)
    {
        SetupDefaultRoutine();
    }
    
    // Gerar características individuais se não estiverem definidas
    if (IndividualTraits.IndividualName == TEXT("Unnamed"))
    {
        GenerateIndividualTraits();
    }
}

void UNPCBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMemory(DeltaTime);
    UpdateDailyRoutine(DeltaTime);
    UpdateDomestication(DeltaTime);
}

void UNPCBehaviorSystem::AddMemoryEntry(AActor* Actor, float ThreatLevel, bool bIsPlayerRelated)
{
    if (!Actor)
        return;
        
    // Verificar se já temos memória deste actor
    for (int32 i = 0; i < MemoryEntries.Num(); i++)
    {
        if (MemoryEntries[i].Actor == Actor)
        {
            // Atualizar entrada existente
            MemoryEntries[i].LastKnownLocation = Actor->GetActorLocation();
            MemoryEntries[i].Timestamp = GetWorld()->GetTimeSeconds();
            MemoryEntries[i].ThreatLevel = ThreatLevel;
            MemoryEntries[i].bIsPlayerRelated = bIsPlayerRelated;
            
            // Aumentar familiaridade se for relacionado ao jogador
            if (bIsPlayerRelated)
            {
                MemoryEntries[i].FamiliarityLevel = FMath::Clamp(MemoryEntries[i].FamiliarityLevel + 0.1f, 0.0f, 1.0f);
            }
            return;
        }
    }
    
    // Criar nova entrada
    FNPCMemoryEntry NewEntry;
    NewEntry.Actor = Actor;
    NewEntry.LastKnownLocation = Actor->GetActorLocation();
    NewEntry.Timestamp = GetWorld()->GetTimeSeconds();
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.bIsPlayerRelated = bIsPlayerRelated;
    NewEntry.FamiliarityLevel = bIsPlayerRelated ? 0.1f : 0.0f;
    
    MemoryEntries.Add(NewEntry);
    
    // Limpar memórias antigas se excedermos o limite
    if (MemoryEntries.Num() > MaxMemoryEntries)
    {
        CleanupOldMemories();
    }
}

FNPCMemoryEntry UNPCBehaviorSystem::GetMemoryOfActor(AActor* Actor)
{
    for (const FNPCMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Actor == Actor)
        {
            return Entry;
        }
    }
    
    // Retornar entrada vazia se não encontrar
    return FNPCMemoryEntry();
}

bool UNPCBehaviorSystem::HasMemoryOfActor(AActor* Actor)
{
    for (const FNPCMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Actor == Actor)
        {
            return true;
        }
    }
    return false;
}

void UNPCBehaviorSystem::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        
        // Notificar mudança de estado para o Behavior Tree
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (UBlackboardComponent* BlackboardComp = OwnerPawn->GetController() ? 
                Cast<UBlackboardComponent>(OwnerPawn->GetController()->GetComponentByClass(UBlackboardComponent::StaticClass())) : nullptr)
            {
                BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
            }
        }
    }
}

FDailyRoutineEntry UNPCBehaviorSystem::GetCurrentRoutineForTime(float TimeOfDay)
{
    if (DailyRoutine.Num() == 0)
    {
        return FDailyRoutineEntry();
    }
    
    // Encontrar a rotina mais apropriada para o horário atual
    FDailyRoutineEntry BestMatch = DailyRoutine[0];
    float BestTimeDifference = FMath::Abs(TimeOfDay - DailyRoutine[0].TimeOfDay);
    
    for (const FDailyRoutineEntry& Entry : DailyRoutine)
    {
        float TimeDifference = FMath::Abs(TimeOfDay - Entry.TimeOfDay);
        
        // Considerar o wrap-around do dia (0.0 e 1.0 são o mesmo momento)
        float WrapDifference = FMath::Min(
            FMath::Abs(TimeOfDay - (Entry.TimeOfDay + 1.0f)),
            FMath::Abs((TimeOfDay + 1.0f) - Entry.TimeOfDay)
        );
        
        TimeDifference = FMath::Min(TimeDifference, WrapDifference);
        
        if (TimeDifference < BestTimeDifference)
        {
            BestTimeDifference = TimeDifference;
            BestMatch = Entry;
        }
    }
    
    return BestMatch;
}

void UNPCBehaviorSystem::ProcessPlayerInteraction(float InteractionQuality, float DeltaTime)
{
    if (!bCanBeDomesticated)
        return;
        
    // Processar interação baseada na qualidade (-1.0 = negativa, 1.0 = positiva)
    float FamiliarityGain = InteractionQuality * DomesticationRate * DeltaTime;
    
    // Modificar ganho baseado na personalidade
    switch (IndividualTraits.PrimaryPersonality)
    {
        case EDinosaurPersonality::Cautious:
            FamiliarityGain *= 0.5f; // Mais lento para ganhar confiança
            break;
        case EDinosaurPersonality::Curious:
            FamiliarityGain *= 1.5f; // Mais rápido para interagir
            break;
        case EDinosaurPersonality::Aggressive:
            FamiliarityGain *= 0.3f; // Muito difícil de domesticar
            break;
        case EDinosaurPersonality::Docile:
            FamiliarityGain *= 2.0f; // Mais fácil de domesticar
            break;
        default:
            break;
    }
    
    PlayerFamiliarity = FMath::Clamp(PlayerFamiliarity + FamiliarityGain, 0.0f, 1.0f);
    
    // Atualizar estágio de domesticação baseado na familiaridade
    UpdateDomesticationStage();
}

bool UNPCBehaviorSystem::CanAdvanceDomestication()
{
    // Verificar se pode avançar para o próximo estágio
    float RequiredFamiliarity = GetRequiredFamiliarityForNextStage();
    return PlayerFamiliarity >= RequiredFamiliarity;
}

float UNPCBehaviorSystem::GetReactionToActor(AActor* Actor)
{
    if (!Actor)
        return 0.0f;
        
    float Reaction = 0.0f;
    
    // Verificar memória do actor
    if (HasMemoryOfActor(Actor))
    {
        FNPCMemoryEntry Memory = GetMemoryOfActor(Actor);
        
        // Reação baseada na memória
        Reaction = Memory.FamiliarityLevel - Memory.ThreatLevel;
        
        // Se for relacionado ao jogador e estamos domesticados, reação mais positiva
        if (Memory.bIsPlayerRelated && CurrentDomesticationStage >= EDomesticationStage::Accepting)
        {
            Reaction += 0.5f;
        }
    }
    else
    {
        // Primeira impressão baseada na personalidade
        float BaseThreat = CalculateActorThreatLevel(Actor);
        
        switch (IndividualTraits.PrimaryPersonality)
        {
            case EDinosaurPersonality::Cautious:
                Reaction = -BaseThreat * 1.5f;
                break;
            case EDinosaurPersonality::Curious:
                Reaction = BaseThreat > 0.7f ? -BaseThreat : 0.3f;
                break;
            case EDinosaurPersonality::Aggressive:
                Reaction = BaseThreat > 0.5f ? -BaseThreat * 2.0f : 0.2f;
                break;
            case EDinosaurPersonality::Social:
                Reaction = 0.2f - BaseThreat;
                break;
            default:
                Reaction = -BaseThreat;
                break;
        }
    }
    
    return FMath::Clamp(Reaction, -1.0f, 1.0f);
}

bool UNPCBehaviorSystem::ShouldFleeFromActor(AActor* Actor)
{
    float Reaction = GetReactionToActor(Actor);
    float FleeThreshold = -IndividualTraits.FearThreshold;
    
    return Reaction < FleeThreshold;
}

bool UNPCBehaviorSystem::ShouldInvestigateActor(AActor* Actor)
{
    float Reaction = GetReactionToActor(Actor);
    float InvestigateThreshold = IndividualTraits.CuriosityLevel * 0.5f;
    
    // Só investigar se não for uma ameaça e tivermos curiosidade
    return Reaction > -0.3f && IndividualTraits.CuriosityLevel > InvestigateThreshold;
}

void UNPCBehaviorSystem::UpdateMemory(float DeltaTime)
{
    MemoryUpdateTimer += DeltaTime;
    
    if (MemoryUpdateTimer >= 1.0f) // Atualizar memória a cada segundo
    {
        MemoryUpdateTimer = 0.0f;
        CleanupOldMemories();
    }
}

void UNPCBehaviorSystem::UpdateDailyRoutine(float DeltaTime)
{
    RoutineUpdateTimer += DeltaTime;
    
    if (RoutineUpdateTimer >= 5.0f) // Verificar rotina a cada 5 segundos
    {
        RoutineUpdateTimer = 0.0f;
        
        // Obter horário atual do dia (implementação simplificada)
        float CurrentTime = GetWorld()->GetTimeSeconds();
        CachedTimeOfDay = FMath::Fmod(CurrentTime / 3600.0f, 24.0f) / 24.0f; // Converter para 0.0-1.0
        
        // Verificar se precisamos mudar de rotina
        FDailyRoutineEntry NewRoutine = GetCurrentRoutineForTime(CachedTimeOfDay);
        
        if (NewRoutine.BehaviorState != CurrentBehaviorState)
        {
            CurrentRoutineEntry = NewRoutine;
            SetBehaviorState(NewRoutine.BehaviorState);
        }
    }
}

void UNPCBehaviorSystem::UpdateDomestication(float DeltaTime)
{
    DomesticationUpdateTimer += DeltaTime;
    
    if (DomesticationUpdateTimer >= 2.0f) // Verificar domesticação a cada 2 segundos
    {
        DomesticationUpdateTimer = 0.0f;
        
        // Decaimento natural da familiaridade se não houver interação
        if (PlayerFamiliarity > 0.0f)
        {
            float DecayRate = 0.01f; // 1% por verificação
            PlayerFamiliarity = FMath::Max(0.0f, PlayerFamiliarity - DecayRate);
            
            UpdateDomesticationStage();
        }
    }
}

float UNPCBehaviorSystem::CalculateActorThreatLevel(AActor* Actor)
{
    if (!Actor)
        return 0.0f;
        
    // Implementação básica - pode ser expandida
    float ThreatLevel = 0.0f;
    
    // Verificar se é o jogador
    if (Actor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        // Threat level do jogador baseado no estágio de domesticação
        switch (CurrentDomesticationStage)
        {
            case EDomesticationStage::Wild:
                ThreatLevel = 0.7f;
                break;
            case EDomesticationStage::Aware:
                ThreatLevel = 0.5f;
                break;
            case EDomesticationStage::Curious:
                ThreatLevel = 0.3f;
                break;
            case EDomesticationStage::Accepting:
                ThreatLevel = 0.1f;
                break;
            default:
                ThreatLevel = 0.0f;
                break;
        }
    }
    else
    {
        // Verificar se é outro dinossauro
        if (UNPCBehaviorSystem* OtherNPC = Actor->FindComponentByClass<UNPCBehaviorSystem>())
        {
            // Calcular ameaça baseada na espécie e características
            bool bIsNaturalEnemy = IndividualTraits.NaturalEnemies.Contains(OtherNPC->Species);
            
            if (bIsNaturalEnemy)
            {
                ThreatLevel = 0.8f;
            }
            else if (OtherNPC->ThreatLevel > ThreatLevel)
            {
                ThreatLevel = static_cast<float>(OtherNPC->ThreatLevel) / 4.0f; // Normalizar enum
            }
        }
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void UNPCBehaviorSystem::CleanupOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remover memórias expiradas
    MemoryEntries.RemoveAll([CurrentTime, this](const FNPCMemoryEntry& Entry)
    {
        return (CurrentTime - Entry.Timestamp) > MemoryDuration;
    });
    
    // Se ainda temos muitas memórias, remover as mais antigas
    if (MemoryEntries.Num() > MaxMemoryEntries)
    {
        MemoryEntries.Sort([](const FNPCMemoryEntry& A, const FNPCMemoryEntry& B)
        {
            return A.Timestamp < B.Timestamp;
        });
        
        int32 ToRemove = MemoryEntries.Num() - MaxMemoryEntries;
        MemoryEntries.RemoveAt(0, ToRemove);
    }
}

void UNPCBehaviorSystem::SetupDefaultRoutine()
{
    // Rotina padrão para herbívoros
    DailyRoutine.Empty();
    
    // Madrugada - Descanso
    FDailyRoutineEntry Rest;
    Rest.TimeOfDay = 0.0f; // Meia-noite
    Rest.BehaviorState = EDinosaurBehaviorState::Resting;
    Rest.Duration = 6.0f;
    Rest.Priority = 0.9f;
    DailyRoutine.Add(Rest);
    
    // Manhã - Forrageamento
    FDailyRoutineEntry MorningForage;
    MorningForage.TimeOfDay = 0.25f; // 6h
    MorningForage.BehaviorState = EDinosaurBehaviorState::Foraging;
    MorningForage.Duration = 3.0f;
    MorningForage.Priority = 0.8f;
    DailyRoutine.Add(MorningForage);
    
    // Meio-dia - Socialização
    FDailyRoutineEntry Midday;
    Midday.TimeOfDay = 0.5f; // 12h
    Midday.BehaviorState = EDinosaurBehaviorState::Socializing;
    Midday.Duration = 2.0f;
    Midday.Priority = 0.6f;
    DailyRoutine.Add(Midday);
    
    // Tarde - Beber água
    FDailyRoutineEntry Afternoon;
    Afternoon.TimeOfDay = 0.75f; // 18h
    Afternoon.BehaviorState = EDinosaurBehaviorState::Drinking;
    Afternoon.Duration = 1.0f;
    Afternoon.Priority = 0.8f;
    DailyRoutine.Add(Afternoon);
}

void UNPCBehaviorSystem::GenerateIndividualTraits()
{
    // Gerar características únicas para este indivíduo
    TArray<FString> NamePool = {
        TEXT("Alpha"), TEXT("Beta"), TEXT("Gamma"), TEXT("Delta"),
        TEXT("Spike"), TEXT("Crest"), TEXT("Horn"), TEXT("Scale"),
        TEXT("Thunder"), TEXT("Storm"), TEXT("River"), TEXT("Stone")
    };
    
    IndividualTraits.IndividualName = NamePool[FMath::RandRange(0, NamePool.Num() - 1)];
    
    // Personalidades baseadas na espécie
    switch (Species)
    {
        case EDinosaurSpecies::Triceratops:
            IndividualTraits.PrimaryPersonality = EDinosaurPersonality::Cautious;
            IndividualTraits.SecondaryPersonality = EDinosaurPersonality::Social;
            break;
        case EDinosaurSpecies::Compsognathus:
            IndividualTraits.PrimaryPersonality = EDinosaurPersonality::Curious;
            IndividualTraits.SecondaryPersonality = EDinosaurPersonality::Skittish;
            break;
        case EDinosaurSpecies::TyrannosaurusRex:
            IndividualTraits.PrimaryPersonality = EDinosaurPersonality::Aggressive;
            IndividualTraits.SecondaryPersonality = EDinosaurPersonality::Territorial;
            break;
        default:
            IndividualTraits.PrimaryPersonality = EDinosaurPersonality::Cautious;
            IndividualTraits.SecondaryPersonality = EDinosaurPersonality::Curious;
            break;
    }
    
    // Valores aleatórios com variação baseada na personalidade
    IndividualTraits.AggressionLevel = FMath::RandRange(0.2f, 0.8f);
    IndividualTraits.CuriosityLevel = FMath::RandRange(0.3f, 0.9f);
    IndividualTraits.SocialLevel = FMath::RandRange(0.1f, 0.7f);
    IndividualTraits.IntelligenceLevel = FMath::RandRange(0.4f, 0.8f);
    IndividualTraits.FearThreshold = FMath::RandRange(0.3f, 0.7f);
    IndividualTraits.TerritorialRadius = FMath::RandRange(500.0f, 2000.0f);
}

float UNPCBehaviorSystem::GetRequiredFamiliarityForNextStage()
{
    switch (CurrentDomesticationStage)
    {
        case EDomesticationStage::Wild: return 0.1f;
        case EDomesticationStage::Aware: return 0.25f;
        case EDomesticationStage::Curious: return 0.4f;
        case EDomesticationStage::Cautious: return 0.6f;
        case EDomesticationStage::Accepting: return 0.75f;
        case EDomesticationStage::Trusting: return 0.9f;
        case EDomesticationStage::Bonded: return 1.0f;
        default: return 1.0f;
    }
}

void UNPCBehaviorSystem::UpdateDomesticationStage()
{
    EDomesticationStage NewStage = CurrentDomesticationStage;
    
    if (PlayerFamiliarity >= 1.0f)
        NewStage = EDomesticationStage::Domesticated;
    else if (PlayerFamiliarity >= 0.9f)
        NewStage = EDomesticationStage::Bonded;
    else if (PlayerFamiliarity >= 0.75f)
        NewStage = EDomesticationStage::Trusting;
    else if (PlayerFamiliarity >= 0.6f)
        NewStage = EDomesticationStage::Accepting;
    else if (PlayerFamiliarity >= 0.4f)
        NewStage = EDomesticationStage::Cautious;
    else if (PlayerFamiliarity >= 0.25f)
        NewStage = EDomesticationStage::Curious;
    else if (PlayerFamiliarity >= 0.1f)
        NewStage = EDomesticationStage::Aware;
    else
        NewStage = EDomesticationStage::Wild;
        
    if (NewStage != CurrentDomesticationStage)
    {
        CurrentDomesticationStage = NewStage;
        
        // Notificar mudança para o Blackboard
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (UBlackboardComponent* BlackboardComp = OwnerPawn->GetController() ? 
                Cast<UBlackboardComponent>(OwnerPawn->GetController()->GetComponentByClass(UBlackboardComponent::StaticClass())) : nullptr)
            {
                BlackboardComp->SetValueAsEnum(TEXT("DomesticationStage"), static_cast<uint8>(NewStage));
                BlackboardComp->SetValueAsFloat(TEXT("PlayerFamiliarity"), PlayerFamiliarity);
            }
        }
    }
}