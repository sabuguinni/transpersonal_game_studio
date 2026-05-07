#include "Combat_EmotionalIntegrationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCombat_EmotionalIntegrationSystem::UCombat_EmotionalIntegrationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    EmotionalUpdateRate = 0.5f;
    CombatDetectionRange = 1500.0f;
    ThreatAssessmentRange = 2000.0f;
    LastEmotionalUpdate = 0.0f;
}

void UCombat_EmotionalIntegrationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Procurar sistema emocional no mesmo actor
    EmotionalSystem = GetOwner()->FindComponentByClass<UNPC_EmotionalBehaviorSystem>();
    
    if (!EmotionalSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_EmotionalIntegrationSystem: Nenhum sistema emocional encontrado no actor %s"), *GetOwner()->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Combat_EmotionalIntegrationSystem: Sistema emocional conectado em %s"), *GetOwner()->GetName());
    }
    
    // Inicializar dados de combate
    CombatData.CombatState = ECombat_EmotionalCombatState::Neutral;
    CombatData.bIsInCombat = false;
    CombatData.LastCombatTime = GetWorld()->GetTimeSeconds();
}

void UCombat_EmotionalIntegrationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    UpdateEmotionalCombatState(DeltaTime);
    
    // Atualizar a cada intervalo definido
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastEmotionalUpdate >= EmotionalUpdateRate)
    {
        ProcessEmotionalTriggers();
        ScanForThreatsAndAllies();
        UpdateCombatMetrics(DeltaTime);
        LastEmotionalUpdate = CurrentTime;
    }
}

void UCombat_EmotionalIntegrationSystem::UpdateEmotionalCombatState(float DeltaTime)
{
    if (!EmotionalSystem)
    {
        return;
    }
    
    // Obter dados emocionais atuais
    FNPC_EmotionalState EmotionalState = EmotionalSystem->GetCurrentEmotionalState();
    
    // Mapear emoções para estados de combate
    if (EmotionalState.Fear > 0.7f)
    {
        HandleCombatStateTransition(ECombat_EmotionalCombatState::Fearful);
    }
    else if (EmotionalState.Aggression > 0.8f)
    {
        HandleCombatStateTransition(ECombat_EmotionalCombatState::Enraged);
    }
    else if (EmotionalState.Aggression > 0.5f)
    {
        HandleCombatStateTransition(ECombat_EmotionalCombatState::Aggressive);
    }
    else if (EmotionalState.SocialBond > 0.6f && NearbyAllies.Num() > 0)
    {
        HandleCombatStateTransition(ECombat_EmotionalCombatState::Protective);
    }
    else if (CombatData.bIsInCombat)
    {
        HandleCombatStateTransition(ECombat_EmotionalCombatState::Defensive);
    }
    else
    {
        HandleCombatStateTransition(ECombat_EmotionalCombatState::Neutral);
    }
    
    // Atualizar níveis baseados nas emoções
    CombatData.AggressionLevel = FMath::Clamp(EmotionalState.Aggression, 0.0f, 1.0f);
    CombatData.FearLevel = FMath::Clamp(EmotionalState.Fear, 0.0f, 1.0f);
    CombatData.ProtectiveInstinct = FMath::Clamp(EmotionalState.SocialBond, 0.0f, 1.0f);
    
    ProcessEmotionalInfluence(DeltaTime);
}

void UCombat_EmotionalIntegrationSystem::ProcessEmotionalTriggers()
{
    if (!EmotionalSystem)
    {
        return;
    }
    
    // Verificar ameaças próximas
    for (AActor* Threat : NearbyThreats)
    {
        if (IsValid(Threat))
        {
            AssessThreatLevel(Threat);
        }
    }
    
    // Verificar aliados em perigo
    for (AActor* Ally : NearbyAllies)
    {
        if (IsValid(Ally))
        {
            // Verificar se o aliado está em combate ou sob ameaça
            UCombat_EmotionalIntegrationSystem* AllyCombatSystem = Ally->FindComponentByClass<UCombat_EmotionalIntegrationSystem>();
            if (AllyCombatSystem && AllyCombatSystem->CombatData.bIsInCombat)
            {
                HandleProtectiveResponse(Ally);
            }
        }
    }
}

void UCombat_EmotionalIntegrationSystem::HandleCombatStateTransition(ECombat_EmotionalCombatState NewState)
{
    if (CombatData.CombatState != NewState)
    {
        ECombat_EmotionalCombatState OldState = CombatData.CombatState;
        CombatData.CombatState = NewState;
        
        UE_LOG(LogTemp, Log, TEXT("%s: Transição de combate emocional: %d -> %d"), 
               *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
        
        ApplyEmotionalCombatModifiers();
    }
}

void UCombat_EmotionalIntegrationSystem::AssessThreatLevel(AActor* PotentialThreat)
{
    if (!IsValid(PotentialThreat) || !GetOwner())
    {
        return;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialThreat->GetActorLocation());
    float ThreatLevel = 1.0f - (Distance / ThreatAssessmentRange);
    ThreatLevel = FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
    
    // Aumentar agressão baseado no nível de ameaça
    if (ThreatLevel > 0.5f && EmotionalSystem)
    {
        EmotionalSystem->TriggerEmotionalResponse(ENPC_EmotionalTrigger::ThreatDetected, ThreatLevel);
        
        // Considerar entrar em combate
        if (ShouldEngageInCombat(PotentialThreat))
        {
            CombatData.bIsInCombat = true;
            CombatData.CurrentTarget = PotentialThreat;
            CombatData.LastCombatTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void UCombat_EmotionalIntegrationSystem::ModifyAggressionBasedOnEmotion(float EmotionalIntensity)
{
    float AggressionModifier = EmotionalIntensity * 0.3f;
    CombatData.AggressionLevel = FMath::Clamp(CombatData.AggressionLevel + AggressionModifier, 0.0f, 1.0f);
    
    // Aplicar modificador ao sistema emocional
    if (EmotionalSystem)
    {
        EmotionalSystem->ModifyEmotionalState(ENPC_EmotionalTrigger::CombatEngagement, AggressionModifier);
    }
}

void UCombat_EmotionalIntegrationSystem::HandleProtectiveResponse(AActor* ThreatenedAlly)
{
    if (!IsValid(ThreatenedAlly) || !EmotionalSystem)
    {
        return;
    }
    
    // Aumentar instinto protetor
    CombatData.ProtectiveInstinct = FMath::Min(CombatData.ProtectiveInstinct + 0.2f, 1.0f);
    
    // Trigger resposta emocional
    EmotionalSystem->TriggerEmotionalResponse(ENPC_EmotionalTrigger::AllyInDanger, 0.8f);
    
    // Entrar em modo protetor
    HandleCombatStateTransition(ECombat_EmotionalCombatState::Protective);
    
    UE_LOG(LogTemp, Log, TEXT("%s: Resposta protetiva ativada para %s"), 
           *GetOwner()->GetName(), *ThreatenedAlly->GetName());
}

void UCombat_EmotionalIntegrationSystem::ProcessFearResponse(AActor* ThreatSource)
{
    if (!IsValid(ThreatSource) || !EmotionalSystem)
    {
        return;
    }
    
    // Aumentar medo
    CombatData.FearLevel = FMath::Min(CombatData.FearLevel + 0.3f, 1.0f);
    
    // Trigger resposta de medo
    EmotionalSystem->TriggerEmotionalResponse(ENPC_EmotionalTrigger::ThreatDetected, CombatData.FearLevel);
    
    // Reduzir efetividade de combate
    CombatData.CombatEffectiveness *= (1.0f - CombatData.FearLevel * 0.5f);
    
    UE_LOG(LogTemp, Log, TEXT("%s: Resposta de medo ativada. Efetividade: %.2f"), 
           *GetOwner()->GetName(), CombatData.CombatEffectiveness);
}

bool UCombat_EmotionalIntegrationSystem::ShouldEngageInCombat(AActor* PotentialTarget)
{
    if (!IsValid(PotentialTarget))
    {
        return false;
    }
    
    // Fatores para decidir engajar em combate
    float EngagementScore = 0.0f;
    
    // Agressão aumenta chance de engajar
    EngagementScore += CombatData.AggressionLevel * 0.4f;
    
    // Instinto protetor aumenta chance se há aliados próximos
    if (NearbyAllies.Num() > 0)
    {
        EngagementScore += CombatData.ProtectiveInstinct * 0.3f;
    }
    
    // Medo reduz chance de engajar
    EngagementScore -= CombatData.FearLevel * 0.5f;
    
    // Distância influencia decisão
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialTarget->GetActorLocation());
    float DistanceFactor = 1.0f - (Distance / CombatDetectionRange);
    EngagementScore += DistanceFactor * 0.2f;
    
    return EngagementScore > 0.5f;
}

float UCombat_EmotionalIntegrationSystem::CalculateCombatEffectiveness()
{
    float Effectiveness = 1.0f;
    
    // Estado emocional afeta efetividade
    switch (CombatData.CombatState)
    {
        case ECombat_EmotionalCombatState::Enraged:
            Effectiveness *= 1.3f; // Mais dano mas menos precisão
            break;
        case ECombat_EmotionalCombatState::Aggressive:
            Effectiveness *= 1.1f;
            break;
        case ECombat_EmotionalCombatState::Protective:
            Effectiveness *= 1.2f; // Motivação extra
            break;
        case ECombat_EmotionalCombatState::Fearful:
            Effectiveness *= 0.6f; // Redução significativa
            break;
        case ECombat_EmotionalCombatState::Defensive:
            Effectiveness *= 0.9f;
            break;
        default:
            break;
    }
    
    // Medo sempre reduz efetividade
    Effectiveness *= (1.0f - CombatData.FearLevel * 0.4f);
    
    CombatData.CombatEffectiveness = FMath::Clamp(Effectiveness, 0.1f, 2.0f);
    return CombatData.CombatEffectiveness;
}

void UCombat_EmotionalIntegrationSystem::ApplyEmotionalCombatModifiers()
{
    CalculateCombatEffectiveness();
    
    // Log da mudança de estado
    UE_LOG(LogTemp, Log, TEXT("%s: Estado de combate emocional: %d | Efetividade: %.2f"), 
           *GetOwner()->GetName(), (int32)CombatData.CombatState, CombatData.CombatEffectiveness);
}

void UCombat_EmotionalIntegrationSystem::ScanForThreatsAndAllies()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    NearbyThreats.Empty();
    NearbyAllies.Empty();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor) || Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= ThreatAssessmentRange)
        {
            FString ActorName = Actor->GetName().ToLower();
            
            // Identificar dinossauros como ameaças
            if (ActorName.Contains(TEXT("dino")) || ActorName.Contains(TEXT("rex")) || 
                ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachio")))
            {
                NearbyThreats.Add(Actor);
            }
            // Identificar outros NPCs como aliados potenciais
            else if (ActorName.Contains(TEXT("npc")) || ActorName.Contains(TEXT("primitive")) || 
                     ActorName.Contains(TEXT("human")))
            {
                NearbyAllies.Add(Actor);
            }
        }
    }
}

void UCombat_EmotionalIntegrationSystem::UpdateCombatMetrics(float DeltaTime)
{
    // Decaimento natural dos níveis emocionais
    float DecayRate = 0.1f * DeltaTime;
    
    CombatData.AggressionLevel = FMath::Max(CombatData.AggressionLevel - DecayRate, 0.0f);
    CombatData.FearLevel = FMath::Max(CombatData.FearLevel - DecayRate, 0.0f);
    
    // Verificar se ainda está em combate
    if (CombatData.bIsInCombat && CombatData.CurrentTarget)
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CombatData.CurrentTarget->GetActorLocation());
        if (Distance > CombatDetectionRange * 1.5f)
        {
            // Sair de combate se o alvo está muito longe
            CombatData.bIsInCombat = false;
            CombatData.CurrentTarget = nullptr;
            HandleCombatStateTransition(ECombat_EmotionalCombatState::Neutral);
        }
    }
}

void UCombat_EmotionalIntegrationSystem::ProcessEmotionalInfluence(float DeltaTime)
{
    if (!EmotionalSystem)
    {
        return;
    }
    
    // Influenciar o sistema emocional baseado no estado de combate
    FNPC_EmotionalState CurrentState = EmotionalSystem->GetCurrentEmotionalState();
    
    if (CombatData.bIsInCombat)
    {
        // Combate aumenta stress e pode aumentar agressão
        EmotionalSystem->ModifyEmotionalState(ENPC_EmotionalTrigger::CombatEngagement, 0.1f * DeltaTime);
    }
    
    // Presença de aliados reduz medo
    if (NearbyAllies.Num() > 0)
    {
        float AllyComfort = FMath::Min(NearbyAllies.Num() * 0.1f, 0.3f);
        CombatData.FearLevel = FMath::Max(CombatData.FearLevel - AllyComfort * DeltaTime, 0.0f);
    }
}