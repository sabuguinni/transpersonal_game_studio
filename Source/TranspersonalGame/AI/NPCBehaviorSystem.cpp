#include "NPCBehaviorSystem.h"
#include "NPCMemoryComponent.h"
#include "NPCRoutineComponent.h"
#include "NPCSocialComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorSystem::UNPCBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Atualiza 10 vezes por segundo

    // Criar componentes do sistema
    MemoryComponent = CreateDefaultSubobject<UNPCMemoryComponent>(TEXT("MemoryComponent"));
    RoutineComponent = CreateDefaultSubobject<UNPCRoutineComponent>(TEXT("RoutineComponent"));
    SocialComponent = CreateDefaultSubobject<UNPCSocialComponent>(TEXT("SocialComponent"));
}

void UNPCBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();

    // Inicializar personalidade com valores aleatórios se não foi configurada
    if (Personality.Aggressiveness == 0.5f && Personality.Curiosity == 0.5f) // Valores padrão
    {
        // Gerar personalidade única baseada no tipo de dinossauro
        GenerateUniquePersonality();
    }

    // Configurar nível inicial de domesticação baseado no tipo
    if (CanBeDomesticated())
    {
        DomesticationLevel = EDomesticationLevel::Wary;
    }
    else
    {
        DomesticationLevel = EDomesticationLevel::Wild;
    }

    // Inicializar componentes
    if (MemoryComponent)
    {
        MemoryComponent->Initialize(MemoryDuration);
    }

    if (RoutineComponent)
    {
        RoutineComponent->Initialize(DinosaurType, Personality);
    }

    if (SocialComponent)
    {
        SocialComponent->Initialize(DinosaurType, Personality);
    }
}

void UNPCBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateBehaviorLogic(DeltaTime);
    ProcessNearbyActors();
    CleanupOldMemories();
}

void UNPCBehaviorSystem::SetBehaviorState(ENPCBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        ENPCBehaviorState OldState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;

        // Disparar evento
        OnBehaviorStateChanged.Broadcast(OldState, NewState);

        // Notificar componentes
        if (RoutineComponent)
        {
            RoutineComponent->OnBehaviorStateChanged(OldState, NewState);
        }

        if (SocialComponent)
        {
            SocialComponent->OnBehaviorStateChanged(OldState, NewState);
        }
    }
}

void UNPCBehaviorSystem::AddMemoryEntry(AActor* Actor, float EmotionalWeight, const FString& MemoryType)
{
    if (!Actor) return;

    // Procurar entrada existente
    for (FNPCMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.RememberedActor == Actor)
        {
            // Atualizar entrada existente
            Entry.LastKnownLocation = Actor->GetActorLocation();
            Entry.EmotionalWeight = FMath::Clamp(Entry.EmotionalWeight + EmotionalWeight, -10.0f, 10.0f);
            Entry.LastSeenTime = GetWorld()->GetTimeSeconds();
            Entry.MemoryType = MemoryType;
            return;
        }
    }

    // Criar nova entrada
    FNPCMemoryEntry NewEntry;
    NewEntry.RememberedActor = Actor;
    NewEntry.LastKnownLocation = Actor->GetActorLocation();
    NewEntry.EmotionalWeight = EmotionalWeight;
    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewEntry.MemoryType = MemoryType;

    MemoryEntries.Add(NewEntry);

    // Notificar componente de memória
    if (MemoryComponent)
    {
        MemoryComponent->OnMemoryAdded(NewEntry);
    }
}

FNPCMemoryEntry UNPCBehaviorSystem::GetMemoryOfActor(AActor* Actor)
{
    for (const FNPCMemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.RememberedActor == Actor)
        {
            return Entry;
        }
    }

    // Retornar entrada vazia se não encontrado
    return FNPCMemoryEntry();
}

void UNPCBehaviorSystem::ModifyTrustLevel(float DeltaAmount)
{
    float OldTrust = TrustLevel;
    TrustLevel = FMath::Clamp(TrustLevel + DeltaAmount, 0.0f, 1.0f);

    if (OldTrust != TrustLevel)
    {
        UpdateDomesticationLevel();
    }
}

bool UNPCBehaviorSystem::CanBeDomesticated() const
{
    // Apenas herbívoros pequenos podem ser domesticados
    return DinosaurType == EDinosaurBehaviorType::Herbivore && 
           Personality.Aggressiveness < 0.7f &&
           Personality.Fearfulness < 0.8f;
}

void UNPCBehaviorSystem::UpdateDomesticationLevel()
{
    if (!CanBeDomesticated()) return;

    EDomesticationLevel OldLevel = DomesticationLevel;
    EDomesticationLevel NewLevel = DomesticationLevel;

    // Determinar novo nível baseado no trust level
    if (TrustLevel >= 0.9f)
    {
        NewLevel = EDomesticationLevel::Domesticated;
    }
    else if (TrustLevel >= 0.75f)
    {
        NewLevel = EDomesticationLevel::Bonded;
    }
    else if (TrustLevel >= 0.6f)
    {
        NewLevel = EDomesticationLevel::Friendly;
    }
    else if (TrustLevel >= 0.4f)
    {
        NewLevel = EDomesticationLevel::Tolerant;
    }
    else if (TrustLevel >= 0.25f)
    {
        NewLevel = EDomesticationLevel::Curious;
    }
    else if (TrustLevel >= 0.1f)
    {
        NewLevel = EDomesticationLevel::Wary;
    }
    else
    {
        NewLevel = EDomesticationLevel::Wild;
    }

    if (OldLevel != NewLevel)
    {
        DomesticationLevel = NewLevel;
        OnDomesticationLevelChanged.Broadcast(OldLevel, NewLevel);
    }
}

float UNPCBehaviorSystem::GetPersonalityTrait(const FString& TraitName) const
{
    if (TraitName == TEXT("Aggressiveness"))
        return Personality.Aggressiveness;
    else if (TraitName == TEXT("Curiosity"))
        return Personality.Curiosity;
    else if (TraitName == TEXT("Fearfulness"))
        return Personality.Fearfulness;
    else if (TraitName == TEXT("Sociability"))
        return Personality.Sociability;
    else if (TraitName == TEXT("Intelligence"))
        return Personality.Intelligence;
    else if (TraitName == TEXT("Territoriality"))
        return Personality.Territoriality;

    return 0.5f; // Valor padrão
}

void UNPCBehaviorSystem::ReactToPlayerAction(const FString& ActionType, float Intensity)
{
    float TrustDelta = 0.0f;

    // Calcular mudança de confiança baseada na ação e personalidade
    if (ActionType == TEXT("FeedFood"))
    {
        TrustDelta = 0.1f * Intensity * (1.0f - Personality.Fearfulness);
    }
    else if (ActionType == TEXT("SlowApproach"))
    {
        TrustDelta = 0.05f * Intensity * Personality.Curiosity;
    }
    else if (ActionType == TEXT("FastMovement"))
    {
        TrustDelta = -0.1f * Intensity * Personality.Fearfulness;
    }
    else if (ActionType == TEXT("LoudNoise"))
    {
        TrustDelta = -0.15f * Intensity * Personality.Fearfulness;
    }
    else if (ActionType == TEXT("Attack"))
    {
        TrustDelta = -0.5f * Intensity;
    }

    ModifyTrustLevel(TrustDelta);

    // Adicionar à memória
    if (AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        AddMemoryEntry(Player, TrustDelta * 2.0f, ActionType);
    }
}

void UNPCBehaviorSystem::UpdateBehaviorLogic(float DeltaTime)
{
    LastBehaviorUpdate += DeltaTime;

    if (LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        LastBehaviorUpdate = 0.0f;

        // Determinar próximo estado comportamental
        ENPCBehaviorState NextState = DetermineNextBehaviorState();
        
        if (NextState != CurrentBehaviorState)
        {
            SetBehaviorState(NextState);
        }
    }
}

void UNPCBehaviorSystem::CleanupOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    MemoryEntries.RemoveAll([CurrentTime, this](const FNPCMemoryEntry& Entry)
    {
        return (CurrentTime - Entry.LastSeenTime) > MemoryDuration;
    });
}

ENPCBehaviorState UNPCBehaviorSystem::DetermineNextBehaviorState()
{
    // Lógica básica para determinar próximo estado
    // Esta função será expandida com IA mais complexa

    // Verificar se há ameaças próximas
    if (AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
        
        if (DistanceToPlayer < PerceptionRadius)
        {
            FNPCMemoryEntry PlayerMemory = GetMemoryOfActor(Player);
            
            // Se tem memória negativa do jogador, fugir
            if (PlayerMemory.EmotionalWeight < -2.0f)
            {
                return ENPCBehaviorState::Fleeing;
            }
            // Se é curioso e não tem medo, investigar
            else if (Personality.Curiosity > 0.6f && Personality.Fearfulness < 0.4f)
            {
                return ENPCBehaviorState::Investigating;
            }
            // Se tem medo, ficar alerta
            else if (Personality.Fearfulness > 0.6f)
            {
                return ENPCBehaviorState::Idle; // Estado alerta
            }
        }
    }

    // Comportamentos baseados no tipo de dinossauro
    switch (DinosaurType)
    {
        case EDinosaurBehaviorType::Herbivore:
            // Alternar entre foraging e idle
            return (FMath::RandRange(0.0f, 1.0f) < 0.7f) ? ENPCBehaviorState::Foraging : ENPCBehaviorState::Idle;
            
        case EDinosaurBehaviorType::Predator:
            // Alternar entre hunting e patrolling
            return (FMath::RandRange(0.0f, 1.0f) < 0.4f) ? ENPCBehaviorState::Hunting : ENPCBehaviorState::Patrolling;
            
        default:
            return ENPCBehaviorState::Idle;
    }
}

void UNPCBehaviorSystem::ProcessNearbyActors()
{
    // Esta função será implementada para processar atores próximos
    // e atualizar memórias baseadas em percepção
}

void UNPCBehaviorSystem::GenerateUniquePersonality()
{
    // Gerar personalidade baseada no tipo de dinossauro com variação aleatória
    switch (DinosaurType)
    {
        case EDinosaurBehaviorType::Herbivore:
            Personality.Aggressiveness = FMath::RandRange(0.1f, 0.4f);
            Personality.Fearfulness = FMath::RandRange(0.4f, 0.8f);
            Personality.Sociability = FMath::RandRange(0.3f, 0.7f);
            break;
            
        case EDinosaurBehaviorType::Predator:
            Personality.Aggressiveness = FMath::RandRange(0.6f, 0.9f);
            Personality.Fearfulness = FMath::RandRange(0.1f, 0.4f);
            Personality.Sociability = FMath::RandRange(0.2f, 0.5f);
            break;
            
        default:
            // Valores médios para outros tipos
            Personality.Aggressiveness = FMath::RandRange(0.3f, 0.7f);
            Personality.Fearfulness = FMath::RandRange(0.3f, 0.7f);
            Personality.Sociability = FMath::RandRange(0.3f, 0.7f);
            break;
    }

    // Traços comuns com variação
    Personality.Curiosity = FMath::RandRange(0.2f, 0.8f);
    Personality.Intelligence = FMath::RandRange(0.3f, 0.7f);
    Personality.Territoriality = FMath::RandRange(0.2f, 0.8f);
}