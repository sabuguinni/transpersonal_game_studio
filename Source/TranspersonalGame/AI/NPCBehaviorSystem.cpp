#include "NPCBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UNPCBehaviorSystem::UNPCBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick a cada segundo para otimização
}

void UNPCBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar componentes de IA
    if (AActor* Owner = GetOwner())
    {
        if (APawn* OwnerPawn = Cast<APawn>(Owner))
        {
            if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
            {
                BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
                BlackboardComponent = AIController->GetBlackboardComponent();
            }
        }
    }
    
    // Gerar personalidade única se não foi definida
    if (PersonalityTraits.Aggression == 0.5f && PersonalityTraits.Curiosity == 0.5f) // Valores padrão
    {
        GenerateRandomPersonality();
    }
    
    // Configurar rotina diária padrão se vazia
    if (DailyRoutine.Num() == 0)
    {
        GenerateDefaultDailyRoutine();
    }
    
    // Atualizar blackboard inicial
    UpdateBlackboardValues();
}

void UNPCBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Atualizar rotina diária (a cada 30 segundos)
    if (CurrentTime - LastRoutineUpdate > 30.0f)
    {
        UpdateDailyRoutine();
        LastRoutineUpdate = CurrentTime;
    }
    
    // Atualizar domesticação (a cada 5 segundos)
    if (CurrentTime - LastDomesticationUpdate > 5.0f)
    {
        UpdateDomestication(DeltaTime);
        LastDomesticationUpdate = CurrentTime;
    }
    
    // Atualizar memória (a cada 60 segundos)
    if (CurrentTime - LastMemoryUpdate > 60.0f)
    {
        UpdateMemoryDecay(DeltaTime);
        LastMemoryUpdate = CurrentTime;
    }
    
    // Processar comportamento social
    ProcessSocialBehavior();
    
    // Atualizar blackboard
    UpdateBlackboardValues();
}

void UNPCBehaviorSystem::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        EDinosaurBehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        
        // Log da mudança de estado para debug
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed behavior from %d to %d"), 
               *GetOwner()->GetName(), (int32)PreviousState, (int32)NewState);
        
        // Atualizar blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)NewState);
        }
        
        // Trigger eventos específicos por estado
        OnBehaviorStateChanged(PreviousState, NewState);
    }
}

void UNPCBehaviorSystem::InteractWithPlayer(AActor* Player, float PositiveInteraction)
{
    if (!Player || !CanBeDomesticated())
    {
        return;
    }
    
    // Atualizar memória do jogador
    UpdateActorMemory(Player, PositiveInteraction * 0.1f);
    
    // Calcular progresso de domesticação baseado na personalidade
    float PersonalityModifier = (PersonalityTraits.Curiosity + (1.0f - PersonalityTraits.Fearfulness)) * 0.5f;
    float DomesticationGain = PositiveInteraction * PersonalityModifier * 0.05f;
    
    DomesticationProgress += DomesticationGain;
    
    // Verificar se avançou de nível
    if (DomesticationProgress >= 1.0f)
    {
        if (DomesticationLevel < EDomesticationLevel::Domesticated)
        {
            DomesticationLevel = (EDomesticationLevel)((int32)DomesticationLevel + 1);
            DomesticationProgress = 0.0f;
            
            UE_LOG(LogTemp, Log, TEXT("Dinosaur %s advanced to domestication level %d"), 
                   *GetOwner()->GetName(), (int32)DomesticationLevel);
        }
        else
        {
            DomesticationProgress = 1.0f; // Cap no máximo
        }
    }
}

float UNPCBehaviorSystem::GetFamiliarityWithActor(AActor* Actor) const
{
    if (const float* Familiarity = ActorMemory.Find(Actor))
    {
        return *Familiarity;
    }
    return 0.0f;
}

void UNPCBehaviorSystem::UpdateActorMemory(AActor* Actor, float FamiliarityChange)
{
    if (!Actor)
    {
        return;
    }
    
    float CurrentFamiliarity = GetFamiliarityWithActor(Actor);
    float NewFamiliarity = FMath::Clamp(CurrentFamiliarity + FamiliarityChange, -1.0f, 1.0f);
    
    ActorMemory.Add(Actor, NewFamiliarity);
    
    // Limitar tamanho da memória (manter apenas os 20 atores mais relevantes)
    if (ActorMemory.Num() > 20)
    {
        // Encontrar o ator com menor familiaridade absoluta
        AActor* LeastRelevantActor = nullptr;
        float LowestRelevance = FLT_MAX;
        
        for (const auto& MemoryEntry : ActorMemory)
        {
            float Relevance = FMath::Abs(MemoryEntry.Value);
            if (Relevance < LowestRelevance)
            {
                LowestRelevance = Relevance;
                LeastRelevantActor = MemoryEntry.Key;
            }
        }
        
        if (LeastRelevantActor)
        {
            ActorMemory.Remove(LeastRelevantActor);
        }
    }
}

EDinosaurBehaviorState UNPCBehaviorSystem::GetCurrentScheduledBehavior() const
{
    FDailyRoutineSchedule ActiveSchedule = GetActiveRoutineSchedule();
    return ActiveSchedule.ScheduledBehavior;
}

FVector UNPCBehaviorSystem::GetCurrentScheduledLocation() const
{
    FDailyRoutineSchedule ActiveSchedule = GetActiveRoutineSchedule();
    return ActiveSchedule.PreferredLocation;
}

bool UNPCBehaviorSystem::CanBeDomesticated() const
{
    // Apenas herbívoros pequenos podem ser domesticados
    return Species == EDinosaurSpecies::Compsognathus ||
           Species == EDinosaurSpecies::Parasaurolophus_Juvenile ||
           Species == EDinosaurSpecies::Triceratops_Juvenile;
}

bool UNPCBehaviorSystem::IsHerbivore() const
{
    return Species == EDinosaurSpecies::Compsognathus ||
           Species == EDinosaurSpecies::Parasaurolophus_Juvenile ||
           Species == EDinosaurSpecies::Triceratops_Juvenile ||
           Species == EDinosaurSpecies::Triceratops_Adult ||
           Species == EDinosaurSpecies::Brachiosaurus ||
           Species == EDinosaurSpecies::Stegosaurus;
}

bool UNPCBehaviorSystem::IsCarnivore() const
{
    return Species == EDinosaurSpecies::Velociraptor ||
           Species == EDinosaurSpecies::Dilophosaurus ||
           Species == EDinosaurSpecies::Tyrannosaurus ||
           Species == EDinosaurSpecies::Allosaurus ||
           Species == EDinosaurSpecies::Carnotaurus ||
           Species == EDinosaurSpecies::Plesiosaur;
}

bool UNPCBehaviorSystem::IsApexPredator() const
{
    return Species == EDinosaurSpecies::Tyrannosaurus ||
           Species == EDinosaurSpecies::Allosaurus ||
           Species == EDinosaurSpecies::Carnotaurus;
}

void UNPCBehaviorSystem::UpdateDailyRoutine()
{
    EDinosaurBehaviorState ScheduledBehavior = GetCurrentScheduledBehavior();
    
    // Aplicar flexibilidade da rotina
    bool ShouldFollowSchedule = FMath::RandRange(0.0f, 1.0f) > RoutineFlexibility;
    
    if (ShouldFollowSchedule)
    {
        // Verificar se o estado atual é muito diferente do programado
        if (CurrentBehaviorState != ScheduledBehavior)
        {
            // Considerar fatores externos antes de mudar
            if (!IsInDanger() && !IsInteractingWithPlayer())
            {
                SetBehaviorState(ScheduledBehavior);
            }
        }
    }
}

void UNPCBehaviorSystem::UpdateDomestication(float DeltaTime)
{
    if (DomesticationLevel == EDomesticationLevel::Wild)
    {
        return; // Não há domesticação para decair
    }
    
    // Decair domesticação se não houver interação recente com o jogador
    float DecayAmount = DomesticationDecayRate * (DeltaTime / 86400.0f); // Por dia
    
    DomesticationProgress -= DecayAmount;
    
    if (DomesticationProgress < 0.0f)
    {
        if (DomesticationLevel > EDomesticationLevel::Wild)
        {
            DomesticationLevel = (EDomesticationLevel)((int32)DomesticationLevel - 1);
            DomesticationProgress = 1.0f;
        }
        else
        {
            DomesticationProgress = 0.0f;
        }
    }
}

void UNPCBehaviorSystem::UpdateMemoryDecay(float DeltaTime)
{
    float DecayAmount = MemoryDecayRate * (DeltaTime / 86400.0f); // Por dia
    
    TArray<AActor*> ActorsToRemove;
    
    for (auto& MemoryEntry : ActorMemory)
    {
        // Decair memórias em direção ao neutro (0.0)
        if (MemoryEntry.Value > 0.0f)
        {
            MemoryEntry.Value = FMath::Max(0.0f, MemoryEntry.Value - DecayAmount);
        }
        else if (MemoryEntry.Value < 0.0f)
        {
            MemoryEntry.Value = FMath::Min(0.0f, MemoryEntry.Value + DecayAmount);
        }
        
        // Remover memórias muito fracas
        if (FMath::Abs(MemoryEntry.Value) < 0.01f)
        {
            ActorsToRemove.Add(MemoryEntry.Key);
        }
    }
    
    // Limpar memórias fracas
    for (AActor* Actor : ActorsToRemove)
    {
        ActorMemory.Remove(Actor);
    }
}

void UNPCBehaviorSystem::ProcessSocialBehavior()
{
    // Implementar comportamento social básico
    // Esta função será expandida com lógica de matilha, hierarquia, etc.
    
    if (IsHerbivore() && PackMembers.Num() > 0)
    {
        // Herbívoros tendem a ficar próximos do grupo
        ProcessHerdBehavior();
    }
    else if (IsCarnivore() && Species == EDinosaurSpecies::Velociraptor)
    {
        // Velociraptors caçam em grupo
        ProcessPackHuntingBehavior();
    }
}

float UNPCBehaviorSystem::GetCurrentTimeOfDay() const
{
    // Implementação simplificada - assumindo ciclo de 24 minutos = 24 horas
    float WorldTime = GetWorld()->GetTimeSeconds();
    float DayLength = 1440.0f; // 24 minutos em segundos
    return FMath::Fmod(WorldTime, DayLength) / DayLength;
}

FDailyRoutineSchedule UNPCBehaviorSystem::GetActiveRoutineSchedule() const
{
    float CurrentTime = GetCurrentTimeOfDay();
    
    for (const FDailyRoutineSchedule& Schedule : DailyRoutine)
    {
        if (CurrentTime >= Schedule.StartTime && CurrentTime <= Schedule.EndTime)
        {
            return Schedule;
        }
    }
    
    // Retornar rotina padrão se nenhuma for encontrada
    FDailyRoutineSchedule DefaultSchedule;
    DefaultSchedule.ScheduledBehavior = EDinosaurBehaviorState::Idle;
    return DefaultSchedule;
}

void UNPCBehaviorSystem::UpdateBlackboardValues()
{
    if (!BlackboardComponent)
    {
        return;
    }
    
    // Atualizar valores do blackboard para uso nas Behavior Trees
    BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), (uint8)CurrentBehaviorState);
    BlackboardComponent->SetValueAsEnum(TEXT("DomesticationLevel"), (uint8)DomesticationLevel);
    BlackboardComponent->SetValueAsFloat(TEXT("DomesticationProgress"), DomesticationProgress);
    BlackboardComponent->SetValueAsVector(TEXT("ScheduledLocation"), GetCurrentScheduledLocation());
    BlackboardComponent->SetValueAsFloat(TEXT("Aggression"), PersonalityTraits.Aggression);
    BlackboardComponent->SetValueAsFloat(TEXT("Curiosity"), PersonalityTraits.Curiosity);
    BlackboardComponent->SetValueAsFloat(TEXT("Fearfulness"), PersonalityTraits.Fearfulness);
}

void UNPCBehaviorSystem::GenerateRandomPersonality()
{
    // Gerar traços de personalidade únicos com base na espécie
    float SpeciesModifier = GetSpeciesPersonalityModifier();
    
    PersonalityTraits.Aggression = FMath::Clamp(FMath::RandRange(0.0f, 1.0f) * SpeciesModifier.X, 0.0f, 1.0f);
    PersonalityTraits.Curiosity = FMath::Clamp(FMath::RandRange(0.0f, 1.0f), 0.0f, 1.0f);
    PersonalityTraits.Sociability = FMath::Clamp(FMath::RandRange(0.0f, 1.0f), 0.0f, 1.0f);
    PersonalityTraits.Fearfulness = FMath::Clamp(FMath::RandRange(0.0f, 1.0f) * SpeciesModifier.Y, 0.0f, 1.0f);
    PersonalityTraits.Intelligence = FMath::Clamp(FMath::RandRange(0.0f, 1.0f) * SpeciesModifier.Z, 0.0f, 1.0f);
    
    // Gerar variações físicas
    PersonalityTraits.SizeVariation = FVector(
        FMath::RandRange(0.8f, 1.2f),
        FMath::RandRange(0.8f, 1.2f),
        FMath::RandRange(0.8f, 1.2f)
    );
    
    PersonalityTraits.ColorVariation = FLinearColor(
        FMath::RandRange(0.7f, 1.3f),
        FMath::RandRange(0.7f, 1.3f),
        FMath::RandRange(0.7f, 1.3f),
        1.0f
    );
}

FVector UNPCBehaviorSystem::GetSpeciesPersonalityModifier() const
{
    // Retorna modificadores para (Agressão, Medo, Inteligência) baseados na espécie
    switch (Species)
    {
        case EDinosaurSpecies::Tyrannosaurus:
            return FVector(1.5f, 0.3f, 0.8f); // Muito agressivo, pouco medroso, inteligência média
        case EDinosaurSpecies::Velociraptor:
            return FVector(1.2f, 0.5f, 1.4f); // Agressivo, moderadamente medroso, muito inteligente
        case EDinosaurSpecies::Compsognathus:
            return FVector(0.4f, 1.3f, 1.0f); // Pouco agressivo, muito medroso, inteligência normal
        case EDinosaurSpecies::Triceratops_Adult:
            return FVector(0.8f, 0.6f, 0.7f); // Moderadamente agressivo, moderadamente medroso, menos inteligente
        default:
            return FVector(1.0f, 1.0f, 1.0f); // Sem modificação
    }
}

void UNPCBehaviorSystem::GenerateDefaultDailyRoutine()
{
    DailyRoutine.Empty();
    
    if (IsHerbivore())
    {
        // Rotina herbívora: forragear, beber, descansar
        FDailyRoutineSchedule MorningForaging;
        MorningForaging.StartTime = 0.25f; // 6h da manhã
        MorningForaging.EndTime = 0.45f;   // 11h da manhã
        MorningForaging.ScheduledBehavior = EDinosaurBehaviorState::Foraging;
        MorningForaging.Priority = 0.8f;
        DailyRoutine.Add(MorningForaging);
        
        FDailyRoutineSchedule MiddayRest;
        MiddayRest.StartTime = 0.45f; // 11h da manhã
        MiddayRest.EndTime = 0.65f;   // 3h da tarde
        MiddayRest.ScheduledBehavior = EDinosaurBehaviorState::Resting;
        MiddayRest.Priority = 0.6f;
        DailyRoutine.Add(MiddayRest);
        
        FDailyRoutineSchedule EveningDrinking;
        EveningDrinking.StartTime = 0.65f; // 3h da tarde
        EveningDrinking.EndTime = 0.75f;   // 6h da tarde
        EveningDrinking.ScheduledBehavior = EDinosaurBehaviorState::Drinking;
        EveningDrinking.Priority = 0.9f;
        DailyRoutine.Add(EveningDrinking);
    }
    else if (IsCarnivore())
    {
        // Rotina carnívora: caçar, descansar
        FDailyRoutineSchedule DawnHunt;
        DawnHunt.StartTime = 0.2f;  // 5h da manhã
        DawnHunt.EndTime = 0.35f;   // 8h da manhã
        DawnHunt.ScheduledBehavior = EDinosaurBehaviorState::Hunting;
        DawnHunt.Priority = 0.9f;
        DailyRoutine.Add(DawnHunt);
        
        FDailyRoutineSchedule DayRest;
        DayRest.StartTime = 0.35f; // 8h da manhã
        DayRest.EndTime = 0.7f;    // 5h da tarde
        DayRest.ScheduledBehavior = EDinosaurBehaviorState::Resting;
        DayRest.Priority = 0.5f;
        DailyRoutine.Add(DayRest);
        
        FDailyRoutineSchedule DuskHunt;
        DuskHunt.StartTime = 0.7f;  // 5h da tarde
        DuskHunt.EndTime = 0.85f;   // 8h da noite
        DuskHunt.ScheduledBehavior = EDinosaurBehaviorState::Hunting;
        DuskHunt.Priority = 0.9f;
        DailyRoutine.Add(DuskHunt);
    }
}

bool UNPCBehaviorSystem::IsInDanger() const
{
    // Implementação básica - verificar se há predadores próximos
    // Esta função será expandida com lógica mais complexa
    return false;
}

bool UNPCBehaviorSystem::IsInteractingWithPlayer() const
{
    // Verificar se o jogador está próximo e há interação ativa
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        return Distance < 500.0f; // 5 metros
    }
    return false;
}

void UNPCBehaviorSystem::ProcessHerdBehavior()
{
    // Lógica básica de comportamento de rebanho
    // Manter-se próximo aos membros do grupo
    if (PackMembers.Num() > 0)
    {
        FVector AveragePosition = FVector::ZeroVector;
        int32 ValidMembers = 0;
        
        for (AActor* Member : PackMembers)
        {
            if (IsValid(Member))
            {
                AveragePosition += Member->GetActorLocation();
                ValidMembers++;
            }
        }
        
        if (ValidMembers > 0)
        {
            AveragePosition /= ValidMembers;
            
            // Se estiver muito longe do grupo, mudar comportamento para se aproximar
            float DistanceToHerd = FVector::Dist(GetOwner()->GetActorLocation(), AveragePosition);
            if (DistanceToHerd > 1000.0f) // 10 metros
            {
                if (BlackboardComponent)
                {
                    BlackboardComponent->SetValueAsVector(TEXT("HerdCenter"), AveragePosition);
                    BlackboardComponent->SetValueAsBool(TEXT("ShouldReturnToHerd"), true);
                }
            }
        }
    }
}

void UNPCBehaviorSystem::ProcessPackHuntingBehavior()
{
    // Lógica básica de caça em grupo para Velociraptors
    if (Species == EDinosaurSpecies::Velociraptor && PackMembers.Num() > 0)
    {
        // Coordenar com outros membros do grupo para caça
        if (CurrentBehaviorState == EDinosaurBehaviorState::Hunting)
        {
            // Implementar lógica de coordenação de caça
            // Esta função será expandida com comportamento mais sofisticado
        }
    }
}

void UNPCBehaviorSystem::OnBehaviorStateChanged(EDinosaurBehaviorState PreviousState, EDinosaurBehaviorState NewState)
{
    // Callback para mudanças de estado - pode ser usado para animações, efeitos, etc.
    
    // Exemplo: Se mudou para estado de fuga, aumentar velocidade
    if (NewState == EDinosaurBehaviorState::Fleeing)
    {
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsFloat(TEXT("MovementSpeed"), 1.5f); // 50% mais rápido
        }
    }
    else
    {
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsFloat(TEXT("MovementSpeed"), 1.0f); // Velocidade normal
        }
    }
}