#include "CombatAIArchitecture.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Atualiza 10x por segundo

    // Criar componente de percepção
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    
    // Configurar percepção visual
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configurar percepção auditiva
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configurar percepção de dano
    UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
    
    // Adicionar configurações ao componente de percepção
    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearingConfig);
    PerceptionComponent->ConfigureSense(*DamageConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Gerar personalidade única para este dinossauro
    GenerateUniquePersonality();
    
    // Configurar eventos de percepção
    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &UCombatAIComponent::OnPerceptionUpdated);
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &UCombatAIComponent::OnTargetPerceptionUpdated);
    }
    
    // Obter referência ao Blackboard do AI Controller
    if (AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController()))
    {
        CombatBlackboard = AIController->GetBlackboardComponent();
        
        // Inicializar Behavior Tree se especificado
        if (CombatBehaviorTree && AIController->GetBehaviorTreeComponent())
        {
            AIController->RunBehaviorTree(CombatBehaviorTree);
        }
    }
    
    // Configurar timer para atualização de estado
    GetWorld()->GetTimerManager().SetTimer(StateUpdateTimer, this, &UCombatAIComponent::UpdateCombatState, 1.0f, true);
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Atualizar memória de combate
    CombatMemory.TimeSincePlayerSeen += DeltaTime;
    
    // Decaimento do nível de ameaça ao longo do tempo
    if (CombatMemory.TimeSincePlayerSeen > 30.0f && CombatMemory.CurrentThreatLevel != EThreatLevel::None)
    {
        UpdateThreatLevel(EThreatLevel::None);
    }
}

void UCombatAIComponent::SetCombatState(ECombatState NewState)
{
    if (CurrentCombatState == NewState) return;
    
    ECombatState PreviousState = CurrentCombatState;
    CurrentCombatState = NewState;
    
    // Atualizar Blackboard
    if (CombatBlackboard)
    {
        CombatBlackboard->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
    }
    
    // Log para debugging
    UE_LOG(LogTemp, Warning, TEXT("Combat State Changed: %s -> %s"), 
           *UEnum::GetValueAsString(PreviousState), 
           *UEnum::GetValueAsString(NewState));
}

void UCombatAIComponent::UpdateThreatLevel(EThreatLevel NewThreatLevel)
{
    if (CombatMemory.CurrentThreatLevel == NewThreatLevel) return;
    
    CombatMemory.CurrentThreatLevel = NewThreatLevel;
    
    // Atualizar Blackboard
    if (CombatBlackboard)
    {
        CombatBlackboard->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(NewThreatLevel));
    }
    
    // Ajustar comportamento baseado no nível de ameaça
    switch (NewThreatLevel)
    {
        case EThreatLevel::Critical:
            SetCombatState(ECombatState::Fighting);
            break;
        case EThreatLevel::High:
            SetCombatState(ECombatState::Engaging);
            break;
        case EThreatLevel::Medium:
            SetCombatState(ECombatState::Stalking);
            break;
        case EThreatLevel::Low:
            SetCombatState(ECombatState::Hunting);
            break;
        case EThreatLevel::None:
            SetCombatState(ECombatState::Idle);
            break;
    }
}

AActor* UCombatAIComponent::FindBestTarget()
{
    AActor* BestTarget = nullptr;
    float HighestPriority = 0.0f;
    
    // Obter todos os atores percebidos
    TArray<AActor*> PerceivedActors;
    PerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
    
    for (AActor* Actor : PerceivedActors)
    {
        // Verificar se o ator implementa a interface de alvo de combate
        if (Actor->Implements<UCombatTarget>())
        {
            ICombatTarget* CombatTarget = Cast<ICombatTarget>(Actor);
            if (CombatTarget && CombatTarget->Execute_CanBeAttacked(Actor))
            {
                float Priority = CombatTarget->Execute_GetTargetPriority(Actor);
                
                // Modificar prioridade baseado na personalidade
                if (Actor->IsA<APawn>()) // Assumindo que o jogador é um Pawn
                {
                    Priority *= (1.0f + Personality.Aggression);
                }
                
                if (Priority > HighestPriority)
                {
                    HighestPriority = Priority;
                    BestTarget = Actor;
                }
            }
        }
    }
    
    return BestTarget;
}

bool UCombatAIComponent::CanSeeTarget(AActor* Target)
{
    if (!Target || !PerceptionComponent) return false;
    
    FActorPerceptionBlueprintInfo Info;
    return PerceptionComponent->GetActorsPerception(Target, Info) && Info.bIsHostile;
}

float UCombatAIComponent::GetDistanceToTarget(AActor* Target)
{
    if (!Target) return -1.0f;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

void UCombatAIComponent::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        // Se detectou o jogador
        if (Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled())
        {
            CombatMemory.LastKnownPlayerLocation = Actor->GetActorLocation();
            CombatMemory.TimeSincePlayerSeen = 0.0f;
            
            // Adicionar à lista de avistamentos
            CombatMemory.PlayerSightings.Add(Actor->GetActorLocation());
            if (CombatMemory.PlayerSightings.Num() > 10)
            {
                CombatMemory.PlayerSightings.RemoveAt(0);
            }
            
            // Calcular nível de ameaça baseado na distância e personalidade
            float Distance = GetDistanceToTarget(Actor);
            EThreatLevel NewThreatLevel = EThreatLevel::Low;
            
            if (Distance < 500.0f)
                NewThreatLevel = EThreatLevel::Critical;
            else if (Distance < 1000.0f)
                NewThreatLevel = EThreatLevel::High;
            else if (Distance < 1500.0f)
                NewThreatLevel = EThreatLevel::Medium;
            
            // Modificar baseado na personalidade
            if (Personality.Aggression > 0.7f)
            {
                NewThreatLevel = static_cast<EThreatLevel>(FMath::Min(static_cast<int32>(NewThreatLevel) + 1, static_cast<int32>(EThreatLevel::Critical)));
            }
            
            UpdateThreatLevel(NewThreatLevel);
        }
    }
}

void UCombatAIComponent::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;
    
    // Atualizar Blackboard com informações do estímulo
    if (CombatBlackboard)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            CombatBlackboard->SetValueAsVector(TEXT("LastKnownTargetLocation"), Stimulus.StimulusLocation);
            CombatBlackboard->SetValueAsObject(TEXT("CurrentTarget"), Actor);
        }
        else
        {
            // Perdeu o alvo
            CombatBlackboard->ClearValue(TEXT("CurrentTarget"));
        }
    }
}

bool UCombatAIComponent::ShouldEngageTarget(AActor* Target)
{
    if (!Target) return false;
    
    float Distance = GetDistanceToTarget(Target);
    
    // Baseado no papel de combate
    switch (CombatRole)
    {
        case EDinosaurCombatRole::Apex_Predator:
            return Personality.Aggression > 0.3f; // Predadores apex são confiantes
            
        case EDinosaurCombatRole::Pack_Hunter:
            // Precisa de aliados próximos ou alta agressividade
            return (CombatMemory.KnownAllies.Num() > 0) || (Personality.Aggression > 0.6f);
            
        case EDinosaurCombatRole::Ambush_Predator:
            // Só ataca se pode emboscar (distância ideal)
            return (Distance > 200.0f && Distance < 800.0f) && Personality.Aggression > 0.4f;
            
        case EDinosaurCombatRole::Territorial_Guard:
            // Ataca se o alvo está no território
            return Personality.Territoriality > 0.5f;
            
        case EDinosaurCombatRole::Opportunist:
            // Ataca alvos fracos ou feridos
            return Personality.Aggression > 0.5f && Distance < 600.0f;
            
        case EDinosaurCombatRole::Passive_Herbivore:
            // Só ataca em legítima defesa
            return CombatMemory.CurrentThreatLevel >= EThreatLevel::Critical;
    }
    
    return false;
}

bool UCombatAIComponent::ShouldRetreat()
{
    // Baseado na coragem e saúde atual
    float HealthPercentage = 1.0f; // TODO: Obter saúde real do dinossauro
    
    // Covardes recuam mais facilmente
    float RetreatThreshold = 1.0f - Personality.Courage;
    
    // Feridos recuam mais facilmente
    if (HealthPercentage < 0.5f)
    {
        RetreatThreshold += 0.3f;
    }
    
    // Nível de ameaça muito alto força recuo
    if (CombatMemory.CurrentThreatLevel >= EThreatLevel::Critical && Personality.Courage < 0.8f)
    {
        return true;
    }
    
    return FMath::RandRange(0.0f, 1.0f) < RetreatThreshold;
}

float UCombatAIComponent::GetPersonalityModifier(const FString& ModifierName)
{
    if (ModifierName == "Aggression") return Personality.Aggression;
    if (ModifierName == "Intelligence") return Personality.Intelligence;
    if (ModifierName == "Curiosity") return Personality.Curiosity;
    if (ModifierName == "Territoriality") return Personality.Territoriality;
    if (ModifierName == "Sociability") return Personality.Sociability;
    if (ModifierName == "Courage") return Personality.Courage;
    
    return 0.5f; // Valor padrão
}

void UCombatAIComponent::UpdateCombatState()
{
    // Lógica para transições automáticas de estado baseada em condições
    AActor* CurrentTarget = FindBestTarget();
    
    if (CurrentTarget)
    {
        float Distance = GetDistanceToTarget(CurrentTarget);
        
        switch (CurrentCombatState)
        {
            case ECombatState::Idle:
                if (CanSeeTarget(CurrentTarget))
                {
                    SetCombatState(ECombatState::Stalking);
                }
                break;
                
            case ECombatState::Stalking:
                if (ShouldEngageTarget(CurrentTarget) && Distance < 1000.0f)
                {
                    SetCombatState(ECombatState::Engaging);
                }
                else if (!CanSeeTarget(CurrentTarget))
                {
                    SetCombatState(ECombatState::Hunting);
                }
                break;
                
            case ECombatState::Engaging:
                if (Distance < 200.0f)
                {
                    SetCombatState(ECombatState::Fighting);
                }
                else if (ShouldRetreat())
                {
                    SetCombatState(ECombatState::Retreating);
                }
                break;
                
            case ECombatState::Fighting:
                if (ShouldRetreat())
                {
                    SetCombatState(ECombatState::Retreating);
                }
                else if (Distance > 500.0f)
                {
                    SetCombatState(ECombatState::Engaging);
                }
                break;
                
            case ECombatState::Retreating:
                if (Distance > 2000.0f)
                {
                    SetCombatState(ECombatState::Idle);
                }
                break;
        }
    }
    else
    {
        // Sem alvos visíveis
        if (CurrentCombatState != ECombatState::Idle && CombatMemory.TimeSincePlayerSeen > 10.0f)
        {
            SetCombatState(ECombatState::Idle);
        }
    }
}

void UCombatAIComponent::GenerateUniquePersonality()
{
    // Gerar personalidade baseada no papel de combate com variação aleatória
    switch (CombatRole)
    {
        case EDinosaurCombatRole::Apex_Predator:
            Personality.Aggression = FMath::RandRange(0.7f, 1.0f);
            Personality.Intelligence = FMath::RandRange(0.6f, 0.9f);
            Personality.Courage = FMath::RandRange(0.8f, 1.0f);
            Personality.Territoriality = FMath::RandRange(0.7f, 1.0f);
            Personality.Sociability = FMath::RandRange(0.0f, 0.3f);
            break;
            
        case EDinosaurCombatRole::Pack_Hunter:
            Personality.Aggression = FMath::RandRange(0.6f, 0.8f);
            Personality.Intelligence = FMath::RandRange(0.7f, 1.0f);
            Personality.Courage = FMath::RandRange(0.5f, 0.8f);
            Personality.Sociability = FMath::RandRange(0.7f, 1.0f);
            break;
            
        case EDinosaurCombatRole::Ambush_Predator:
            Personality.Aggression = FMath::RandRange(0.5f, 0.8f);
            Personality.Intelligence = FMath::RandRange(0.6f, 0.9f);
            Personality.Courage = FMath::RandRange(0.4f, 0.7f);
            Personality.Curiosity = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case EDinosaurCombatRole::Passive_Herbivore:
            Personality.Aggression = FMath::RandRange(0.0f, 0.3f);
            Personality.Courage = FMath::RandRange(0.2f, 0.6f);
            Personality.Sociability = FMath::RandRange(0.5f, 1.0f);
            Personality.Curiosity = FMath::RandRange(0.4f, 0.8f);
            break;
    }
    
    // Adicionar variação aleatória para tornar cada indivíduo único
    Personality.Curiosity = FMath::Clamp(Personality.Curiosity + FMath::RandRange(-0.2f, 0.2f), 0.0f, 1.0f);
}