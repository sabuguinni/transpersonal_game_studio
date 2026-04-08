#include "NPCBehaviorAgent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogNPCBehavior, Log, All);

// ===== UAdvancedNPCBehaviorComponent Implementation =====

UAdvancedNPCBehaviorComponent::UAdvancedNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 100ms para performance
    
    // Configurações padrão
    bAutoActivate = true;
    bWantsInitializeComponent = true;
    
    // Inicializar valores padrão
    CurrentEmotionalState = ENPCEmotionalState::Calm;
    CurrentActivity = ENPCActivity::Idle;
    StressLevel = 0.0f;
    EnergyLevel = 1.0f;
    SocialBattery = 1.0f;
    
    // Configurações de memória
    MaxMemoryEntries = 100;
    MemoryDecayRate = 0.001f; // Decay muito lento
    ImportantMemoryThreshold = 0.7f;
    
    // Configurações de rotina
    bUsesDailyRoutines = true;
    RoutineFlexibility = 0.3f;
    
    // Configurações sociais
    SocialRadius = 1500.0f;
    MaxRelationships = 20;
    RelationshipDecayRate = 0.0001f;
    
    // Configurações de personalidade
    PersonalityStability = 0.9f; // Personalidade bastante estável
    EmotionalVolatility = 0.2f;
    
    UE_LOG(LogNPCBehavior, Log, TEXT("AdvancedNPCBehaviorComponent: Component created"));
}

void UAdvancedNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar personalidade se não foi definida
    if (Personality.Openness == 0.0f && Personality.Conscientiousness == 0.0f)
    {
        GenerateRandomPersonality();
    }
    
    // Calcular traços dominantes
    Personality.CalculateDominantTraits();
    
    // Configurar rotinas diárias se necessário
    if (bUsesDailyRoutines && DailyRoutines.Num() == 0)
    {
        GenerateDefaultDailyRoutines();
    }
    
    // Inicializar sistema de memória
    InitializeMemorySystem();
    
    // Configurar timers
    SetupTimers();
    
    UE_LOG(LogNPCBehavior, Log, TEXT("AdvancedNPCBehaviorComponent: BeginPlay completed for %s"), 
           *GetOwner()->GetName());
}

void UAdvancedNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Atualizar sistemas principais
    UpdateEmotionalState(DeltaTime);
    UpdateNeeds(DeltaTime);
    UpdateMemorySystem(DeltaTime);
    UpdateRelationships(DeltaTime);
    UpdateDailyRoutine(DeltaTime);
    
    // Debug info (apenas em builds de desenvolvimento)
    #if WITH_EDITOR
    if (bShowDebugInfo)
    {
        DisplayDebugInfo();
    }
    #endif
}

void UAdvancedNPCBehaviorComponent::GenerateRandomPersonality()
{
    // Gerar personalidade aleatória mas balanceada
    Personality.Openness = FMath::RandRange(0.2f, 0.8f);
    Personality.Conscientiousness = FMath::RandRange(0.2f, 0.8f);
    Personality.Extraversion = FMath::RandRange(0.2f, 0.8f);
    Personality.Agreeableness = FMath::RandRange(0.2f, 0.8f);
    Personality.Neuroticism = FMath::RandRange(0.2f, 0.8f);
    
    // Traços específicos do contexto
    Personality.Curiosity = FMath::RandRange(0.1f, 0.9f);
    Personality.Empathy = FMath::RandRange(0.2f, 0.8f);
    Personality.Ambition = FMath::RandRange(0.1f, 0.7f);
    Personality.Loyalty = FMath::RandRange(0.3f, 0.9f);
    Personality.Independence = FMath::RandRange(0.2f, 0.8f);
    Personality.RiskTolerance = FMath::RandRange(0.1f, 0.6f);
    Personality.SocialNeed = FMath::RandRange(0.2f, 0.8f);
    Personality.TrustLevel = FMath::RandRange(0.3f, 0.7f);
    Personality.AdaptabilityRate = FMath::RandRange(0.2f, 0.8f);
    Personality.StressResistance = FMath::RandRange(0.3f, 0.9f);
    
    UE_LOG(LogNPCBehavior, Log, TEXT("Generated random personality for %s"), *GetOwner()->GetName());
}

void UAdvancedNPCBehaviorComponent::GenerateDefaultDailyRoutines()
{
    // Rotina básica baseada na personalidade
    FNPCDailyRoutine MorningRoutine;
    MorningRoutine.RoutineName = TEXT("Morning Activities");
    MorningRoutine.StartTimeOfDay = 0.25f; // 6:00 AM
    MorningRoutine.Duration = 0.125f; // 3 horas
    MorningRoutine.Priority = 3;
    MorningRoutine.bCanBeInterrupted = true;
    
    if (Personality.Conscientiousness > 0.6f)
    {
        MorningRoutine.RequiredConditions.Add(TEXT("WellRested"));
    }
    
    DailyRoutines.Add(MorningRoutine);
    
    // Rotina de meio-dia
    FNPCDailyRoutine MiddayRoutine;
    MiddayRoutine.RoutineName = TEXT("Midday Work");
    MiddayRoutine.StartTimeOfDay = 0.4f; // 9:36 AM
    MiddayRoutine.Duration = 0.25f; // 6 horas
    MiddayRoutine.Priority = 4;
    MiddayRoutine.bCanBeInterrupted = false;
    
    DailyRoutines.Add(MiddayRoutine);
    
    // Rotina noturna
    FNPCDailyRoutine EveningRoutine;
    EveningRoutine.RoutineName = TEXT("Evening Rest");
    EveningRoutine.StartTimeOfDay = 0.8f; // 7:12 PM
    EveningRoutine.Duration = 0.2f; // 4.8 horas
    EveningRoutine.Priority = 2;
    EveningRoutine.bCanBeInterrupted = true;
    
    if (Personality.SocialNeed > 0.5f)
    {
        EveningRoutine.RequiredConditions.Add(TEXT("SocialInteraction"));
    }
    
    DailyRoutines.Add(EveningRoutine);
    
    UE_LOG(LogNPCBehavior, Log, TEXT("Generated %d daily routines for %s"), 
           DailyRoutines.Num(), *GetOwner()->GetName());
}

void UAdvancedNPCBehaviorComponent::InitializeMemorySystem()
{
    // Limpar memórias antigas se existirem
    MemoryEntries.Empty();
    
    // Criar memória inicial do local de spawn
    FNPCMemoryEntry SpawnMemory;
    SpawnMemory.MemoryType = TEXT("Location");
    SpawnMemory.Location = GetOwner()->GetActorLocation();
    SpawnMemory.Description = TEXT("Birth/Spawn location");
    SpawnMemory.EmotionalWeight = 0.5f;
    SpawnMemory.MemoryStrength = 1.0f;
    SpawnMemory.ReinforcementCount = 1;
    
    AddMemoryEntry(SpawnMemory);
    
    UE_LOG(LogNPCBehavior, Log, TEXT("Memory system initialized for %s"), *GetOwner()->GetName());
}

void UAdvancedNPCBehaviorComponent::SetupTimers()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Timer para atualizações menos frequentes (a cada 5 segundos)
    GetWorld()->GetTimerManager().SetTimer(
        SlowUpdateTimer,
        this,
        &UAdvancedNPCBehaviorComponent::SlowUpdate,
        5.0f,
        true
    );
    
    // Timer para atualizações de relacionamentos (a cada 30 segundos)
    GetWorld()->GetTimerManager().SetTimer(
        RelationshipUpdateTimer,
        this,
        &UAdvancedNPCBehaviorComponent::UpdateNearbyRelationships,
        30.0f,
        true
    );
}

void UAdvancedNPCBehaviorComponent::UpdateEmotionalState(float DeltaTime)
{
    // Calcular novo estado emocional baseado em vários fatores
    float EmotionalChange = 0.0f;
    
    // Influência da personalidade
    if (Personality.Neuroticism > 0.6f)
    {
        EmotionalChange += (StressLevel - 0.5f) * EmotionalVolatility * DeltaTime;
    }
    
    // Influência das necessidades
    if (EnergyLevel < 0.3f)
    {
        EmotionalChange -= 0.1f * DeltaTime;
    }
    
    if (SocialBattery < 0.2f && Personality.SocialNeed > 0.6f)
    {
        EmotionalChange -= 0.15f * DeltaTime;
    }
    
    // Aplicar mudança emocional gradual
    // (Implementação simplificada - em produção seria mais complexa)
    
    // Decaimento natural do stress
    StressLevel = FMath::Max(0.0f, StressLevel - (0.1f * DeltaTime));
}

void UAdvancedNPCBehaviorComponent::UpdateNeeds(float DeltaTime)
{
    // Redução gradual de energia
    EnergyLevel = FMath::Max(0.0f, EnergyLevel - (0.05f * DeltaTime));
    
    // Redução de bateria social para extrovertidos
    if (Personality.Extraversion > 0.5f)
    {
        SocialBattery = FMath::Max(0.0f, SocialBattery - (0.02f * DeltaTime));
    }
    else
    {
        // Introvertidos recuperam bateria social quando sozinhos
        SocialBattery = FMath::Min(1.0f, SocialBattery + (0.01f * DeltaTime));
    }
}

void UAdvancedNPCBehaviorComponent::UpdateMemorySystem(float DeltaTime)
{
    // Aplicar decay às memórias
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        FNPCMemoryEntry& Memory = MemoryEntries[i];
        
        // Memórias importantes não decaem
        if (FMath::Abs(Memory.EmotionalWeight) > ImportantMemoryThreshold)
        {
            continue;
        }
        
        // Aplicar decay
        Memory.MemoryStrength -= MemoryDecayRate * DeltaTime;
        
        // Remover memórias muito fracas
        if (Memory.MemoryStrength <= 0.1f)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
    
    // Limitar número total de memórias
    if (MemoryEntries.Num() > MaxMemoryEntries)
    {
        // Remover as memórias mais fracas
        MemoryEntries.Sort([](const FNPCMemoryEntry& A, const FNPCMemoryEntry& B) {
            return A.MemoryStrength > B.MemoryStrength;
        });
        
        MemoryEntries.SetNum(MaxMemoryEntries);
    }
}

void UAdvancedNPCBehaviorComponent::UpdateRelationships(float DeltaTime)
{
    // Aplicar decay aos relacionamentos
    for (auto& RelationshipPair : Relationships)
    {
        FNPCRelationship& Relationship = RelationshipPair.Value;
        
        // Calcular tempo desde última interação
        FTimespan TimeSinceInteraction = FDateTime::Now() - Relationship.LastInteraction;
        float DaysSinceInteraction = TimeSinceInteraction.GetTotalDays();
        
        // Aplicar decay baseado no tempo
        if (DaysSinceInteraction > 1.0f)
        {
            float DecayAmount = RelationshipDecayRate * DaysSinceInteraction * DeltaTime;
            
            if (Relationship.RelationshipStrength > 0.0f)
            {
                Relationship.RelationshipStrength = FMath::Max(0.0f, 
                    Relationship.RelationshipStrength - DecayAmount);
            }
            else
            {
                Relationship.RelationshipStrength = FMath::Min(0.0f, 
                    Relationship.RelationshipStrength + DecayAmount);
            }
        }
    }
    
    // Remover relacionamentos muito fracos
    for (auto It = Relationships.CreateIterator(); It; ++It)
    {
        if (FMath::Abs(It->Value.RelationshipStrength) < 0.1f)
        {
            It.RemoveCurrent();
        }
    }
}

void UAdvancedNPCBehaviorComponent::UpdateDailyRoutine(float DeltaTime)
{
    if (!bUsesDailyRoutines || DailyRoutines.Num() == 0)
    {
        return;
    }
    
    // Obter tempo atual do dia (0.0 - 1.0)
    float CurrentTimeOfDay = GetCurrentTimeOfDay();
    
    // Encontrar rotina ativa
    FNPCDailyRoutine* ActiveRoutine = nullptr;
    int32 HighestPriority = 0;
    
    for (FNPCDailyRoutine& Routine : DailyRoutines)
    {
        float RoutineEnd = Routine.StartTimeOfDay + Routine.Duration;
        
        // Verificar se estamos no período da rotina
        bool bInTimeRange = false;
        if (RoutineEnd <= 1.0f)
        {
            // Rotina não cruza meia-noite
            bInTimeRange = (CurrentTimeOfDay >= Routine.StartTimeOfDay && 
                           CurrentTimeOfDay <= RoutineEnd);
        }
        else
        {
            // Rotina cruza meia-noite
            bInTimeRange = (CurrentTimeOfDay >= Routine.StartTimeOfDay || 
                           CurrentTimeOfDay <= (RoutineEnd - 1.0f));
        }
        
        if (bInTimeRange && Routine.Priority > HighestPriority)
        {
            // Verificar condições necessárias
            bool bConditionsMet = true;
            for (const FString& Condition : Routine.RequiredConditions)
            {
                if (!CheckCondition(Condition))
                {
                    bConditionsMet = false;
                    break;
                }
            }
            
            if (bConditionsMet)
            {
                ActiveRoutine = &Routine;
                HighestPriority = Routine.Priority;
            }
        }
    }
    
    // Atualizar rotina atual se necessário
    if (ActiveRoutine && CurrentRoutine != ActiveRoutine->RoutineName)
    {
        CurrentRoutine = ActiveRoutine->RoutineName;
        OnRoutineChanged.Broadcast(CurrentRoutine);
        
        UE_LOG(LogNPCBehavior, Log, TEXT("%s started routine: %s"), 
               *GetOwner()->GetName(), *CurrentRoutine);
    }
}

float UAdvancedNPCBehaviorComponent::GetCurrentTimeOfDay() const
{
    // Implementação simplificada - em produção usaria o sistema de tempo do jogo
    if (UWorld* World = GetWorld())
    {
        float GameTime = World->GetTimeSeconds();
        float DayLength = 1200.0f; // 20 minutos = 1 dia do jogo
        return FMath::Fmod(GameTime / DayLength, 1.0f);
    }
    
    return 0.0f;
}

bool UAdvancedNPCBehaviorComponent::CheckCondition(const FString& Condition) const
{
    // Verificar condições básicas
    if (Condition == TEXT("WellRested"))
    {
        return EnergyLevel > 0.7f;
    }
    else if (Condition == TEXT("SocialInteraction"))
    {
        return SocialBattery > 0.3f;
    }
    else if (Condition == TEXT("LowStress"))
    {
        return StressLevel < 0.3f;
    }
    
    // Condição desconhecida - assumir verdadeira
    return true;
}

void UAdvancedNPCBehaviorComponent::SlowUpdate()
{
    // Atualizações menos críticas que podem ser feitas com menos frequência
    UpdatePersonalityEvolution();
    CleanupOldMemories();
    EvaluateGoals();
}

void UAdvancedNPCBehaviorComponent::UpdateNearbyRelationships()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    // Encontrar NPCs próximos
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        
        if (Distance <= SocialRadius)
        {
            // Verificar se já temos relacionamento
            if (!Relationships.Contains(Actor))
            {
                // Criar novo relacionamento
                FNPCRelationship NewRelationship;
                NewRelationship.RelatedActor = Actor;
                NewRelationship.RelationshipType = ENPCRelationshipType::Unknown;
                NewRelationship.RelationshipStrength = 0.0f;
                
                Relationships.Add(Actor, NewRelationship);
                
                UE_LOG(LogNPCBehavior, Log, TEXT("%s discovered new NPC: %s"), 
                       *GetOwner()->GetName(), *Actor->GetName());
            }
        }
    }
}

void UAdvancedNPCBehaviorComponent::UpdatePersonalityEvolution()
{
    // Personalidade evolui muito lentamente baseada em experiências
    // Implementação simplificada
}

void UAdvancedNPCBehaviorComponent::CleanupOldMemories()
{
    // Remover memórias muito antigas e irrelevantes
    FDateTime CutoffDate = FDateTime::Now() - FTimespan::FromDays(30.0);
    
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        const FNPCMemoryEntry& Memory = MemoryEntries[i];
        
        if (Memory.Timestamp < CutoffDate && 
            FMath::Abs(Memory.EmotionalWeight) < 0.3f &&
            Memory.MemoryStrength < 0.5f)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

void UAdvancedNPCBehaviorComponent::EvaluateGoals()
{
    // Avaliar e atualizar objetivos pessoais
    // Implementação simplificada
}

void UAdvancedNPCBehaviorComponent::AddMemoryEntry(const FNPCMemoryEntry& NewMemory)
{
    // Verificar se já existe memória similar
    for (FNPCMemoryEntry& ExistingMemory : MemoryEntries)
    {
        if (ExistingMemory.MemoryType == NewMemory.MemoryType &&
            FVector::Dist(ExistingMemory.Location, NewMemory.Location) < 100.0f)
        {
            // Reforçar memória existente
            ExistingMemory.ReinforcementCount++;
            ExistingMemory.MemoryStrength = FMath::Min(1.0f, 
                ExistingMemory.MemoryStrength + 0.1f);
            ExistingMemory.Timestamp = FDateTime::Now();
            return;
        }
    }
    
    // Adicionar nova memória
    MemoryEntries.Add(NewMemory);
    
    UE_LOG(LogNPCBehavior, Log, TEXT("%s added memory: %s"), 
           *GetOwner()->GetName(), *NewMemory.Description);
}

void UAdvancedNPCBehaviorComponent::ModifyRelationship(AActor* OtherActor, 
                                                       float StrengthChange, 
                                                       ENPCRelationshipType NewType)
{
    if (!OtherActor)
    {
        return;
    }
    
    FNPCRelationship* Relationship = Relationships.Find(OtherActor);
    
    if (!Relationship)
    {
        // Criar novo relacionamento
        FNPCRelationship NewRelationship;
        NewRelationship.RelatedActor = OtherActor;
        NewRelationship.RelationshipType = NewType;
        NewRelationship.RelationshipStrength = StrengthChange;
        
        Relationships.Add(OtherActor, NewRelationship);
    }
    else
    {
        // Modificar relacionamento existente
        Relationship->RelationshipStrength = FMath::Clamp(
            Relationship->RelationshipStrength + StrengthChange, -1.0f, 1.0f);
        
        if (NewType != ENPCRelationshipType::Unknown)
        {
            Relationship->RelationshipType = NewType;
        }
        
        Relationship->LastInteraction = FDateTime::Now();
        
        if (StrengthChange > 0.0f)
        {
            Relationship->PositiveInteractions++;
        }
        else if (StrengthChange < 0.0f)
        {
            Relationship->NegativeInteractions++;
        }
    }
    
    UE_LOG(LogNPCBehavior, Log, TEXT("%s relationship with %s changed by %.2f"), 
           *GetOwner()->GetName(), *OtherActor->GetName(), StrengthChange);
}

#if WITH_EDITOR
void UAdvancedNPCBehaviorComponent::DisplayDebugInfo()
{
    if (!GetWorld() || !GetOwner())
    {
        return;
    }
    
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FString DebugText = FString::Printf(
        TEXT("NPC: %s\\nPersonality: O:%.1f C:%.1f E:%.1f A:%.1f N:%.1f\\nEnergy: %.1f Stress: %.1f Social: %.1f\\nRoutine: %s\\nMemories: %d Relationships: %d"),
        *GetOwner()->GetName(),
        Personality.Openness, Personality.Conscientiousness, Personality.Extraversion,
        Personality.Agreeableness, Personality.Neuroticism,
        EnergyLevel, StressLevel, SocialBattery,
        *CurrentRoutine,
        MemoryEntries.Num(), Relationships.Num()
    );
    
    DrawDebugString(GetWorld(), ActorLocation + FVector(0, 0, 200), DebugText, 
                   nullptr, FColor::White, 0.0f, true);
}
#endif

// ===== FNPCPersonality Implementation =====

void FNPCPersonality::CalculateDominantTraits()
{
    DominantTraits.Empty();
    
    // Analisar traços baseados nos valores
    if (Openness > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::Curious);
    if (Conscientiousness > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::Analytical);
    if (Extraversion > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::Extrovert);
    else if (Extraversion < 0.3f) DominantTraits.Add(ENPCPersonalityTrait::Introvert);
    
    if (Agreeableness > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::Cooperative);
    else if (Agreeableness < 0.3f) DominantTraits.Add(ENPCPersonalityTrait::Competitive);
    
    if (Neuroticism > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::Pessimistic);
    else if (Neuroticism < 0.3f) DominantTraits.Add(ENPCPersonalityTrait::Optimistic);
    
    // Traços específicos do contexto
    if (RiskTolerance > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::Brave);
    else if (RiskTolerance < 0.3f) DominantTraits.Add(ENPCPersonalityTrait::Cautious);
    
    if (AdaptabilityRate > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::AdaptabilityHigh);
    if (Independence > 0.7f) DominantTraits.Add(ENPCPersonalityTrait::NaturalSurvivor);
}

float FNPCPersonality::CalculateCompatibility(const FNPCPersonality& Other) const
{
    float Compatibility = 0.0f;
    
    // Calcular compatibilidade baseada em diferenças
    Compatibility += 1.0f - FMath::Abs(Agreeableness - Other.Agreeableness);
    Compatibility += 1.0f - FMath::Abs(Empathy - Other.Empathy);
    Compatibility += 1.0f - FMath::Abs(TrustLevel - Other.TrustLevel);
    
    // Algumas diferenças podem ser complementares
    if (FMath::Abs(Independence - Other.Independence) > 0.3f)
    {
        Compatibility += 0.2f; // Independência complementar pode ser boa
    }
    
    // Normalizar para 0.0 - 1.0
    return FMath::Clamp(Compatibility / 4.0f, 0.0f, 1.0f);
}

void FNPCPersonality::ModifyFromExperience(const FString& ExperienceType, float Intensity)
{
    // Modificar personalidade baseada em experiências
    // Mudanças são muito pequenas para manter estabilidade
    float ChangeAmount = Intensity * 0.01f; // Máximo 1% de mudança por experiência
    
    if (ExperienceType == TEXT("Betrayal"))
    {
        TrustLevel = FMath::Max(0.0f, TrustLevel - ChangeAmount);
        Neuroticism = FMath::Min(1.0f, Neuroticism + ChangeAmount * 0.5f);
    }
    else if (ExperienceType == TEXT("Success"))
    {
        Neuroticism = FMath::Max(0.0f, Neuroticism - ChangeAmount);
        Conscientiousness = FMath::Min(1.0f, Conscientiousness + ChangeAmount * 0.3f);
    }
    else if (ExperienceType == TEXT("SocialPositive"))
    {
        Agreeableness = FMath::Min(1.0f, Agreeableness + ChangeAmount);
        Empathy = FMath::Min(1.0f, Empathy + ChangeAmount * 0.5f);
    }
    else if (ExperienceType == TEXT("Trauma"))
    {
        StressResistance = FMath::Max(0.0f, StressResistance - ChangeAmount);
        RiskTolerance = FMath::Max(0.0f, RiskTolerance - ChangeAmount * 0.7f);
    }
}