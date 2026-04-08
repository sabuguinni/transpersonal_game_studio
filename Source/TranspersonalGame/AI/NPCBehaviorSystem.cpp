#include "NPCBehaviorSystem.h"
#include "DinosaurNPC.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameplayTagsManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

void UNPCBehaviorSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior System initialized"));
    
    // Carregar data table de espécies se configurado
    if (SpeciesDataTable.IsValid())
    {
        UDataTable* LoadedTable = SpeciesDataTable.LoadSynchronous();
        if (LoadedTable)
        {
            UE_LOG(LogTemp, Warning, TEXT("Species data table loaded successfully"));
        }
    }
}

void UNPCBehaviorSystem::Deinitialize()
{
    // Limpar timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RoutineUpdateTimer);
    }
    
    RegisteredNPCs.Empty();
    
    Super::Deinitialize();
}

void UNPCBehaviorSystem::RegisterNPC(ADinosaurNPC* NPC)
{
    if (!NPC)
    {
        return;
    }
    
    // Adicionar à lista se não estiver já registado
    TWeakObjectPtr<ADinosaurNPC> WeakNPC(NPC);
    if (!RegisteredNPCs.Contains(WeakNPC))
    {
        RegisteredNPCs.Add(WeakNPC);
        UE_LOG(LogTemp, Log, TEXT("NPC %s registered with behavior system"), *NPC->GetName());
        
        // Se é o primeiro NPC, iniciar o timer de updates
        if (RegisteredNPCs.Num() == 1)
        {
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(
                    RoutineUpdateTimer,
                    this,
                    &UNPCBehaviorSystem::UpdateNPCRoutines,
                    RoutineUpdateInterval,
                    true
                );
            }
        }
    }
}

void UNPCBehaviorSystem::UnregisterNPC(ADinosaurNPC* NPC)
{
    if (!NPC)
    {
        return;
    }
    
    TWeakObjectPtr<ADinosaurNPC> WeakNPC(NPC);
    RegisteredNPCs.Remove(WeakNPC);
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s unregistered from behavior system"), *NPC->GetName());
    
    // Se não há mais NPCs, parar o timer
    if (RegisteredNPCs.Num() == 0)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(RoutineUpdateTimer);
        }
    }
}

void UNPCBehaviorSystem::UpdateNPCRoutines(float DeltaTime)
{
    // Limpar NPCs inválidos
    for (int32 i = RegisteredNPCs.Num() - 1; i >= 0; i--)
    {
        if (!RegisteredNPCs[i].IsValid())
        {
            RegisteredNPCs.RemoveAt(i);
        }
    }
    
    // Processar cada NPC válido
    for (const TWeakObjectPtr<ADinosaurNPC>& WeakNPC : RegisteredNPCs)
    {
        if (ADinosaurNPC* NPC = WeakNPC.Get())
        {
            ProcessNPCRoutine(NPC);
            UpdateNPCNeeds(NPC, DeltaTime);
        }
    }
}

void UNPCBehaviorSystem::UpdateNPCNeeds(ADinosaurNPC* NPC, float DeltaTime)
{
    if (!NPC)
    {
        return;
    }
    
    // Obter necessidades actuais do NPC
    FDinosaurNeeds CurrentNeeds = NPC->GetCurrentNeeds();
    
    // Calcular decay baseado no tempo e actividade
    float DecayMultiplier = NeedsDecayRate * DeltaTime;
    
    // Diferentes estados afectam diferentes necessidades
    EDinosaurBehaviorState CurrentState = NPC->GetCurrentBehaviorState();
    
    switch (CurrentState)
    {
        case EDinosaurBehaviorState::Foraging:
            CurrentNeeds.Hunger = FMath::Min(100.0f, CurrentNeeds.Hunger + 10.0f * DeltaTime);
            CurrentNeeds.Energy -= 5.0f * DecayMultiplier;
            break;
            
        case EDinosaurBehaviorState::Drinking:
            CurrentNeeds.Thirst = FMath::Min(100.0f, CurrentNeeds.Thirst + 15.0f * DeltaTime);
            break;
            
        case EDinosaurBehaviorState::Resting:
            CurrentNeeds.Energy = FMath::Min(100.0f, CurrentNeeds.Energy + 8.0f * DeltaTime);
            CurrentNeeds.Comfort = FMath::Min(100.0f, CurrentNeeds.Comfort + 5.0f * DeltaTime);
            break;
            
        case EDinosaurBehaviorState::Socializing:
            CurrentNeeds.Social = FMath::Min(100.0f, CurrentNeeds.Social + 12.0f * DeltaTime);
            CurrentNeeds.Energy -= 2.0f * DecayMultiplier;
            break;
            
        case EDinosaurBehaviorState::Fleeing:
            CurrentNeeds.Energy -= 15.0f * DecayMultiplier;
            CurrentNeeds.Safety = FMath::Max(0.0f, CurrentNeeds.Safety - 20.0f * DeltaTime);
            break;
            
        default:
            // Decay normal para estados neutros
            CurrentNeeds.Hunger = FMath::Max(0.0f, CurrentNeeds.Hunger - 3.0f * DecayMultiplier);
            CurrentNeeds.Thirst = FMath::Max(0.0f, CurrentNeeds.Thirst - 4.0f * DecayMultiplier);
            CurrentNeeds.Energy = FMath::Max(0.0f, CurrentNeeds.Energy - 2.0f * DecayMultiplier);
            CurrentNeeds.Social = FMath::Max(0.0f, CurrentNeeds.Social - 1.0f * DecayMultiplier);
            break;
    }
    
    // Aplicar as necessidades actualizadas ao NPC
    NPC->SetCurrentNeeds(CurrentNeeds);
    
    // Actualizar blackboard se disponível
    if (UBlackboardComponent* Blackboard = NPC->GetBlackboardComponent())
    {
        Blackboard->SetValueAsFloat(FName("Hunger"), CurrentNeeds.Hunger);
        Blackboard->SetValueAsFloat(FName("Thirst"), CurrentNeeds.Thirst);
        Blackboard->SetValueAsFloat(FName("Energy"), CurrentNeeds.Energy);
        Blackboard->SetValueAsFloat(FName("Social"), CurrentNeeds.Social);
        Blackboard->SetValueAsFloat(FName("Safety"), CurrentNeeds.Safety);
        Blackboard->SetValueAsFloat(FName("Comfort"), CurrentNeeds.Comfort);
    }
}

void UNPCBehaviorSystem::AddMemoryEvent(ADinosaurNPC* NPC, const FGameplayTag& EventType, const FVector& Location, AActor* RelatedActor)
{
    if (!NPC)
    {
        return;
    }
    
    // Implementar sistema de memória
    // Por agora, apenas log para debug
    FString ActorName = RelatedActor ? RelatedActor->GetName() : TEXT("None");
    UE_LOG(LogTemp, Log, TEXT("Memory event for %s: %s at %s (Actor: %s)"), 
           *NPC->GetName(), 
           *EventType.ToString(), 
           *Location.ToString(), 
           *ActorName);
    
    // TODO: Implementar armazenamento persistente de memórias
    // TODO: Implementar sistema de esquecimento baseado em tempo
    // TODO: Implementar influência das memórias no comportamento
}

void UNPCBehaviorSystem::ProcessDomesticationInteraction(ADinosaurNPC* NPC, AActor* Player, float InteractionStrength)
{
    if (!NPC || !Player)
    {
        return;
    }
    
    // Verificar se a espécie pode ser domesticada
    FDinosaurSpeciesData SpeciesData = GetSpeciesData(NPC->GetSpeciesName());
    if (!SpeciesData.bCanBeDomesticated)
    {
        return;
    }
    
    // Calcular progresso de domesticação baseado na personalidade e interação
    FDinosaurPersonality Personality = NPC->GetPersonality();
    
    float DomesticationProgress = InteractionStrength;
    
    // Modificadores baseados na personalidade
    DomesticationProgress *= (100.0f - Personality.Fearfulness) / 100.0f; // Menos medo = mais fácil
    DomesticationProgress *= (Personality.Curiosity + 50.0f) / 150.0f;    // Mais curiosidade = mais fácil
    DomesticationProgress *= (100.0f - Personality.Aggressiveness) / 100.0f; // Menos agressão = mais fácil
    
    // Aplicar dificuldade da espécie
    DomesticationProgress *= (100.0f - SpeciesData.DomesticationDifficulty) / 100.0f;
    
    // Actualizar progresso no NPC
    float CurrentProgress = NPC->GetDomesticationProgress();
    float NewProgress = FMath::Clamp(CurrentProgress + DomesticationProgress, 0.0f, 100.0f);
    NPC->SetDomesticationProgress(NewProgress);
    
    UE_LOG(LogTemp, Log, TEXT("Domestication interaction: %s progress %.2f -> %.2f"), 
           *NPC->GetName(), CurrentProgress, NewProgress);
    
    // Adicionar evento de memória
    FGameplayTag DomesticationTag = FGameplayTag::RequestGameplayTag(FName("Event.Domestication.Interaction"));
    AddMemoryEvent(NPC, DomesticationTag, Player->GetActorLocation(), Player);
}

FDinosaurSpeciesData UNPCBehaviorSystem::GetSpeciesData(const FString& SpeciesName) const
{
    FDinosaurSpeciesData DefaultData;
    
    if (!SpeciesDataTable.IsValid())
    {
        return DefaultData;
    }
    
    UDataTable* LoadedTable = SpeciesDataTable.LoadSynchronous();
    if (!LoadedTable)
    {
        return DefaultData;
    }
    
    FDinosaurSpeciesData* FoundData = LoadedTable->FindRow<FDinosaurSpeciesData>(FName(*SpeciesName), TEXT(""));
    return FoundData ? *FoundData : DefaultData;
}

EDinosaurBehaviorState UNPCBehaviorSystem::GetOptimalBehaviorState(ADinosaurNPC* NPC) const
{
    if (!NPC)
    {
        return EDinosaurBehaviorState::Idle;
    }
    
    FDinosaurNeeds Needs = NPC->GetCurrentNeeds();
    FDinosaurPersonality Personality = NPC->GetPersonality();
    
    // Prioridades críticas (sobrevivência)
    if (Needs.Safety < 20.0f)
    {
        return EDinosaurBehaviorState::Fleeing;
    }
    
    if (Needs.Thirst < 30.0f)
    {
        return EDinosaurBehaviorState::Drinking;
    }
    
    if (Needs.Hunger < 25.0f)
    {
        return EDinosaurBehaviorState::Foraging;
    }
    
    if (Needs.Energy < 20.0f)
    {
        return EDinosaurBehaviorState::Resting;
    }
    
    // Prioridades sociais e comportamentais
    if (Needs.Social < 40.0f && Personality.Sociability > 60.0f)
    {
        return EDinosaurBehaviorState::Socializing;
    }
    
    // Comportamentos baseados na personalidade
    if (Personality.Territoriality > 70.0f && FMath::RandRange(0.0f, 100.0f) < 30.0f)
    {
        return EDinosaurBehaviorState::Territorial;
    }
    
    if (Personality.Curiosity > 70.0f && FMath::RandRange(0.0f, 100.0f) < 20.0f)
    {
        return EDinosaurBehaviorState::Foraging; // Exploração como forrageamento
    }
    
    return EDinosaurBehaviorState::Idle;
}

void UNPCBehaviorSystem::ProcessNPCRoutine(ADinosaurNPC* NPC)
{
    if (!NPC)
    {
        return;
    }
    
    // Obter hora actual do dia (assumindo sistema de dia/noite)
    float CurrentTimeOfDay = 12.0f; // TODO: Obter do sistema de tempo do jogo
    
    // Obter rotina da espécie
    FDinosaurSpeciesData SpeciesData = GetSpeciesData(NPC->GetSpeciesName());
    
    // Encontrar a actividade mais apropriada para a hora actual
    FDailyRoutineEntry BestActivity;
    float BestScore = -1.0f;
    
    for (const FDailyRoutineEntry& Activity : SpeciesData.DefaultRoutine)
    {
        float TimeDifference = FMath::Abs(Activity.TimeOfDay - CurrentTimeOfDay);
        if (TimeDifference > 12.0f)
        {
            TimeDifference = 24.0f - TimeDifference; // Considerar wrap-around
        }
        
        // Score baseado na proximidade temporal e prioridade
        float Score = Activity.Priority * (1.0f - (TimeDifference / 12.0f));
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestActivity = Activity;
        }
    }
    
    // Verificar se deve mudar de estado
    EDinosaurBehaviorState OptimalState = GetOptimalBehaviorState(NPC);
    EDinosaurBehaviorState RoutineState = BestActivity.PreferredState;
    
    // Priorizar necessidades sobre rotina
    EDinosaurBehaviorState TargetState = (BestScore > 50.0f) ? RoutineState : OptimalState;
    
    if (ShouldSwitchBehaviorState(NPC, TargetState))
    {
        NPC->SetCurrentBehaviorState(TargetState);
        
        // Actualizar blackboard
        if (UBlackboardComponent* Blackboard = NPC->GetBlackboardComponent())
        {
            Blackboard->SetValueAsEnum(FName("BehaviorState"), static_cast<uint8>(TargetState));
        }
    }
}

float UNPCBehaviorSystem::CalculateNeedPriority(const FDinosaurNeeds& Needs, EDinosaurBehaviorState State) const
{
    switch (State)
    {
        case EDinosaurBehaviorState::Drinking:
            return 100.0f - Needs.Thirst;
        case EDinosaurBehaviorState::Foraging:
            return 100.0f - Needs.Hunger;
        case EDinosaurBehaviorState::Resting:
            return 100.0f - Needs.Energy;
        case EDinosaurBehaviorState::Socializing:
            return 100.0f - Needs.Social;
        case EDinosaurBehaviorState::Fleeing:
            return 100.0f - Needs.Safety;
        default:
            return 0.0f;
    }
}

bool UNPCBehaviorSystem::ShouldSwitchBehaviorState(ADinosaurNPC* NPC, EDinosaurBehaviorState NewState) const
{
    if (!NPC)
    {
        return false;
    }
    
    EDinosaurBehaviorState CurrentState = NPC->GetCurrentBehaviorState();
    
    // Sempre permitir mudança se o estado é diferente e crítico
    if (NewState == EDinosaurBehaviorState::Fleeing)
    {
        return true;
    }
    
    // Evitar mudanças muito frequentes
    float TimeSinceLastChange = NPC->GetTimeSinceLastBehaviorChange();
    if (TimeSinceLastChange < 30.0f && CurrentState != EDinosaurBehaviorState::Idle)
    {
        return false;
    }
    
    return CurrentState != NewState;
}