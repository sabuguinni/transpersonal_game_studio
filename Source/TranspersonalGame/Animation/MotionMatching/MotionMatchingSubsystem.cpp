#include "MotionMatchingSubsystem.h"
#include "Engine/Engine.h"
#include "Animation/PoseSearch/PoseSearchDatabase.h"

void UMotionMatchingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultBlendTimes();
    
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSubsystem: Initialized successfully"));
}

void UMotionMatchingSubsystem::Deinitialize()
{
    CharacterDatabases.Empty();
    ContextBlendTimes.Empty();
    
    Super::Deinitialize();
}

UPoseSearchDatabase* UMotionMatchingSubsystem::SelectOptimalDatabase(const FMotionMatchingQuery& Query, const FString& CharacterType)
{
    // Verifica se temos bases de dados registadas para este tipo de personagem
    const FMotionMatchingDatabaseSet* DatabaseSet = CharacterDatabases.Find(CharacterType);
    if (!DatabaseSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingSubsystem: No databases registered for character type '%s'"), *CharacterType);
        return nullptr;
    }

    // Determina o contexto óptimo baseado no query
    EMotionMatchingContext OptimalContext = DetermineOptimalContext(Query);
    
    // Seleciona a base de dados apropriada
    UPoseSearchDatabase* SelectedDatabase = nullptr;
    
    switch (OptimalContext)
    {
        case EMotionMatchingContext::Locomotion:
            SelectedDatabase = DatabaseSet->LocomotionDatabase;
            break;
        case EMotionMatchingContext::Stealth:
            SelectedDatabase = DatabaseSet->StealthDatabase;
            break;
        case EMotionMatchingContext::Panic:
            SelectedDatabase = DatabaseSet->PanicDatabase;
            break;
        case EMotionMatchingContext::Exhausted:
            SelectedDatabase = DatabaseSet->ExhaustedDatabase;
            break;
        case EMotionMatchingContext::Injured:
            SelectedDatabase = DatabaseSet->InjuredDatabase;
            break;
        case EMotionMatchingContext::Gathering:
            SelectedDatabase = DatabaseSet->GatheringDatabase;
            break;
        case EMotionMatchingContext::Climbing:
            SelectedDatabase = DatabaseSet->ClimbingDatabase;
            break;
        case EMotionMatchingContext::Swimming:
            SelectedDatabase = DatabaseSet->SwimmingDatabase;
            break;
        case EMotionMatchingContext::Interaction:
            SelectedDatabase = DatabaseSet->InteractionDatabase;
            break;
        default:
            SelectedDatabase = DatabaseSet->LocomotionDatabase; // Fallback
            break;
    }
    
    // Fallback para locomotion se a base de dados específica não existir
    if (!SelectedDatabase && DatabaseSet->LocomotionDatabase)
    {
        SelectedDatabase = DatabaseSet->LocomotionDatabase;
        UE_LOG(LogTemp, Log, TEXT("MotionMatchingSubsystem: Using locomotion database as fallback for context %d"), (int32)OptimalContext);
    }
    
    return SelectedDatabase;
}

float UMotionMatchingSubsystem::CalculateOptimalBlendTime(EMotionMatchingContext FromContext, EMotionMatchingContext ToContext, float EmotionalIntensity)
{
    // Tempo base baseado no contexto de destino
    float BaseBlendTime = 0.3f;
    if (const float* FoundBlendTime = ContextBlendTimes.Find(ToContext))
    {
        BaseBlendTime = *FoundBlendTime;
    }
    
    // Modificadores baseados na transição
    float TransitionModifier = 1.0f;
    
    // Transições para estados de pânico devem ser mais rápidas
    if (ToContext == EMotionMatchingContext::Panic)
    {
        TransitionModifier = 0.5f;
    }
    // Transições para estados calmos devem ser mais lentas
    else if (FromContext == EMotionMatchingContext::Panic && ToContext == EMotionMatchingContext::Locomotion)
    {
        TransitionModifier = 2.0f;
    }
    // Transições entre estados similares são mais rápidas
    else if ((FromContext == EMotionMatchingContext::Stealth && ToContext == EMotionMatchingContext::Locomotion) ||
             (FromContext == EMotionMatchingContext::Locomotion && ToContext == EMotionMatchingContext::Stealth))
    {
        TransitionModifier = 0.8f;
    }
    
    // Aplica a intensidade emocional
    float EmotionalModifier = FMath::Lerp(0.7f, 1.5f, EmotionalIntensity);
    
    float FinalBlendTime = BaseBlendTime * TransitionModifier * EmotionalModifier;
    
    // Clamp para valores razoáveis
    return FMath::Clamp(FinalBlendTime, 0.1f, 2.0f);
}

void UMotionMatchingSubsystem::RegisterCharacterDatabases(const FString& CharacterType, const FMotionMatchingDatabaseSet& DatabaseSet)
{
    CharacterDatabases.Add(CharacterType, DatabaseSet);
    UE_LOG(LogTemp, Log, TEXT("MotionMatchingSubsystem: Registered databases for character type '%s'"), *CharacterType);
}

FMotionMatchingQuery UMotionMatchingSubsystem::ApplyEmotionalModifiers(const FMotionMatchingQuery& BaseQuery, float FearLevel, float ExhaustionLevel, float InjuryLevel)
{
    FMotionMatchingQuery ModifiedQuery = BaseQuery;
    
    // Aplica modificadores de medo
    if (FearLevel > 0.0f)
    {
        // Medo aumenta a velocidade desejada (fugir)
        ModifiedQuery.DesiredVelocity *= (1.0f + FearLevel * 0.5f);
        
        // Medo aumenta a taxa de viragem (olhar à volta)
        ModifiedQuery.TurnRate += FearLevel * 180.0f;
        
        // Medo afecta a posição futura (movimento mais errático)
        FVector FearOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f) * FearLevel,
            FMath::RandRange(-100.0f, 100.0f) * FearLevel,
            0.0f
        );
        ModifiedQuery.FuturePosition += FearOffset;
    }
    
    // Aplica modificadores de exaustão
    if (ExhaustionLevel > 0.0f)
    {
        // Exaustão reduz a velocidade desejada
        ModifiedQuery.DesiredVelocity *= (1.0f - ExhaustionLevel * 0.6f);
        
        // Exaustão reduz a capacidade de viragem rápida
        ModifiedQuery.TurnRate *= (1.0f - ExhaustionLevel * 0.4f);
    }
    
    // Aplica modificadores de ferimentos
    if (InjuryLevel > 0.0f)
    {
        // Ferimentos reduzem drasticamente a velocidade
        ModifiedQuery.DesiredVelocity *= (1.0f - InjuryLevel * 0.8f);
        
        // Ferimentos afectam a estabilidade do movimento
        ModifiedQuery.TurnRate *= (1.0f - InjuryLevel * 0.6f);
        
        // Ferimentos afectam a capacidade de planeamento futuro
        ModifiedQuery.FuturePosition *= (1.0f - InjuryLevel * 0.3f);
    }
    
    // Actualiza os níveis no query
    ModifiedQuery.FearLevel = FearLevel;
    ModifiedQuery.ExhaustionLevel = ExhaustionLevel;
    ModifiedQuery.InjuryLevel = InjuryLevel;
    
    return ModifiedQuery;
}

void UMotionMatchingSubsystem::InitializeDefaultBlendTimes()
{
    ContextBlendTimes.Empty();
    
    // Define tempos de blend por contexto
    ContextBlendTimes.Add(EMotionMatchingContext::Locomotion, 0.3f);
    ContextBlendTimes.Add(EMotionMatchingContext::Stealth, 0.5f);
    ContextBlendTimes.Add(EMotionMatchingContext::Panic, 0.15f);
    ContextBlendTimes.Add(EMotionMatchingContext::Exhausted, 0.8f);
    ContextBlendTimes.Add(EMotionMatchingContext::Injured, 1.0f);
    ContextBlendTimes.Add(EMotionMatchingContext::Gathering, 0.4f);
    ContextBlendTimes.Add(EMotionMatchingContext::Climbing, 0.6f);
    ContextBlendTimes.Add(EMotionMatchingContext::Swimming, 0.7f);
    ContextBlendTimes.Add(EMotionMatchingContext::Interaction, 0.5f);
}

EMotionMatchingContext UMotionMatchingSubsystem::DetermineOptimalContext(const FMotionMatchingQuery& Query)
{
    // Calcula prioridades para cada contexto
    TMap<EMotionMatchingContext, float> ContextPriorities;
    
    ContextPriorities.Add(EMotionMatchingContext::Locomotion, CalculateContextPriority(EMotionMatchingContext::Locomotion, Query));
    ContextPriorities.Add(EMotionMatchingContext::Stealth, CalculateContextPriority(EMotionMatchingContext::Stealth, Query));
    ContextPriorities.Add(EMotionMatchingContext::Panic, CalculateContextPriority(EMotionMatchingContext::Panic, Query));
    ContextPriorities.Add(EMotionMatchingContext::Exhausted, CalculateContextPriority(EMotionMatchingContext::Exhausted, Query));
    ContextPriorities.Add(EMotionMatchingContext::Injured, CalculateContextPriority(EMotionMatchingContext::Injured, Query));
    ContextPriorities.Add(EMotionMatchingContext::Gathering, CalculateContextPriority(EMotionMatchingContext::Gathering, Query));
    ContextPriorities.Add(EMotionMatchingContext::Climbing, CalculateContextPriority(EMotionMatchingContext::Climbing, Query));
    ContextPriorities.Add(EMotionMatchingContext::Swimming, CalculateContextPriority(EMotionMatchingContext::Swimming, Query));
    ContextPriorities.Add(EMotionMatchingContext::Interaction, CalculateContextPriority(EMotionMatchingContext::Interaction, Query));
    
    // Encontra o contexto com maior prioridade
    EMotionMatchingContext BestContext = EMotionMatchingContext::Locomotion;
    float HighestPriority = 0.0f;
    
    for (const auto& ContextPair : ContextPriorities)
    {
        if (ContextPair.Value > HighestPriority)
        {
            HighestPriority = ContextPair.Value;
            BestContext = ContextPair.Key;
        }
    }
    
    return BestContext;
}

float UMotionMatchingSubsystem::CalculateContextPriority(EMotionMatchingContext Context, const FMotionMatchingQuery& Query)
{
    float Priority = 0.0f;
    
    switch (Context)
    {
        case EMotionMatchingContext::Panic:
            // Prioridade alta se há perigo ou medo alto
            Priority = (Query.bIsInDanger ? 1.0f : 0.0f) + (Query.FearLevel * FearInfluenceWeight);
            break;
            
        case EMotionMatchingContext::Injured:
            // Prioridade baseada no nível de ferimentos
            Priority = Query.InjuryLevel * InjuryInfluenceWeight;
            break;
            
        case EMotionMatchingContext::Exhausted:
            // Prioridade baseada na exaustão
            Priority = Query.ExhaustionLevel * ExhaustionInfluenceWeight;
            break;
            
        case EMotionMatchingContext::Stealth:
            // Prioridade se está a esconder-se ou há perigo moderado
            Priority = (Query.bIsHiding ? 0.8f : 0.0f) + (Query.FearLevel * 0.5f);
            break;
            
        case EMotionMatchingContext::Climbing:
            // Prioridade baseada na inclinação do terreno
            Priority = FMath::Abs(Query.TerrainSlope) > 45.0f ? 1.0f : 0.0f;
            break;
            
        case EMotionMatchingContext::Locomotion:
            // Contexto base - sempre tem alguma prioridade
            Priority = 0.1f;
            // Aumenta se não há outros contextos mais importantes
            if (Query.FearLevel < 0.3f && Query.ExhaustionLevel < 0.5f && Query.InjuryLevel < 0.2f)
            {
                Priority = 0.5f;
            }
            break;
            
        default:
            // Contextos específicos baseados no query context
            Priority = (Query.Context == Context) ? 0.7f : 0.0f;
            break;
    }
    
    return FMath::Clamp(Priority, 0.0f, 1.0f);
}