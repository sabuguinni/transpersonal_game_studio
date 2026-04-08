#include "BehavioralAnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IKRigDefinition.h"
#include "ControlRig.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBehavioralAnimation, Log, All);

UBehavioralAnimationSystem::UBehavioralAnimationSystem()
{
    // Configuração padrão do sistema
    SystemConfig = FBehavioralAnimationConfig();
    
    // Inicializar mapeamentos de arquétipos
    InitializeArchetypeMappings();
}

void UBehavioralAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBehavioralAnimation, Log, TEXT("Behavioral Animation System initialized"));
    
    // Carregar configurações do sistema
    LoadSystemConfiguration();
    
    // Inicializar pools de animação
    InitializeAnimationPools();
    
    // Configurar timers para atualizações periódicas
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            EmotionalUpdateTimer,
            this,
            &UBehavioralAnimationSystem::UpdateEmotionalStates,
            SystemConfig.EmotionalUpdateInterval,
            true
        );
        
        World->GetTimerManager().SetTimer(
            GestureUpdateTimer,
            this,
            &UBehavioralAnimationSystem::UpdateProceduralGestures,
            SystemConfig.GestureUpdateInterval,
            true
        );
    }
}

void UBehavioralAnimationSystem::Deinitialize()
{
    // Limpar timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EmotionalUpdateTimer);
        World->GetTimerManager().ClearTimer(GestureUpdateTimer);
    }
    
    // Limpar registros
    RegisteredCharacters.Empty();
    ArchetypeConfigurations.Empty();
    ActiveGestures.Empty();
    
    UE_LOG(LogBehavioralAnimation, Log, TEXT("Behavioral Animation System deinitialized"));
    
    Super::Deinitialize();
}

void UBehavioralAnimationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!SystemConfig.bSystemEnabled)
    {
        return;
    }
    
    // Atualizar personagens registrados
    UpdateRegisteredCharacters(DeltaTime);
    
    // Processar fila de gestos
    ProcessGestureQueue(DeltaTime);
    
    // Atualizar estatísticas de performance
    UpdatePerformanceStats(DeltaTime);
}

bool UBehavioralAnimationSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return !IsRunningDedicatedServer();
}

void UBehavioralAnimationSystem::RegisterCharacter(ACharacter* Character, EBodyLanguageArchetype Archetype)
{
    if (!Character)
    {
        UE_LOG(LogBehavioralAnimation, Warning, TEXT("Attempted to register null character"));
        return;
    }
    
    if (RegisteredCharacters.Contains(Character))
    {
        UE_LOG(LogBehavioralAnimation, Warning, TEXT("Character %s is already registered"), *Character->GetName());
        return;
    }
    
    // Criar configuração de personagem
    FCharacterBehavioralData CharacterData;
    CharacterData.Character = Character;
    CharacterData.Archetype = Archetype;
    CharacterData.BodyLanguageConfig = GetArchetypeBodyLanguage(Archetype);
    CharacterData.EmotionalState = FEmotionalState();
    CharacterData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Registrar personagem
    RegisteredCharacters.Add(Character, CharacterData);
    
    // Aplicar configuração inicial
    ApplyArchetypeConfiguration(Character, Archetype);
    
    UE_LOG(LogBehavioralAnimation, Log, TEXT("Character %s registered with archetype %s"), 
        *Character->GetName(), 
        *UEnum::GetValueAsString(Archetype));
}

void UBehavioralAnimationSystem::UnregisterCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    if (RegisteredCharacters.Contains(Character))
    {
        // Limpar gestos ativos
        RemoveActiveGesturesForCharacter(Character);
        
        // Remover do registro
        RegisteredCharacters.Remove(Character);
        
        UE_LOG(LogBehavioralAnimation, Log, TEXT("Character %s unregistered"), *Character->GetName());
    }
}

void UBehavioralAnimationSystem::UpdateCharacterEmotionalState(ACharacter* Character, const FEmotionalState& NewState)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    FCharacterBehavioralData& CharacterData = RegisteredCharacters[Character];
    FEmotionalState PreviousState = CharacterData.EmotionalState;
    CharacterData.EmotionalState = NewState;
    
    // Verificar se houve mudança significativa
    if (HasSignificantEmotionalChange(PreviousState, NewState))
    {
        // Aplicar nova configuração baseada no estado emocional
        ApplyEmotionalConfiguration(Character, NewState);
        
        // Disparar gesto emocional se apropriado
        TriggerEmotionalGesture(Character, NewState);
    }
}

void UBehavioralAnimationSystem::TriggerContextualGesture(ACharacter* Character, const FString& GestureContext, float Priority)
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return;
    }
    
    const FCharacterBehavioralData& CharacterData = RegisteredCharacters[Character];
    
    // Buscar gesto apropriado para o contexto
    UAnimMontage* GestureMontage = FindContextualGesture(CharacterData.Archetype, GestureContext);
    
    if (GestureMontage)
    {
        // Adicionar à fila de gestos
        FGestureQueueEntry GestureEntry;
        GestureEntry.Character = Character;
        GestureEntry.GestureMontage = GestureMontage;
        GestureEntry.Priority = Priority;
        GestureEntry.Context = GestureContext;
        GestureEntry.QueueTime = GetWorld()->GetTimeSeconds();
        
        GestureQueue.Add(GestureEntry);
        
        // Ordenar fila por prioridade
        GestureQueue.Sort([](const FGestureQueueEntry& A, const FGestureQueueEntry& B) {
            return A.Priority > B.Priority;
        });
    }
}

FArchetypeBodyLanguage UBehavioralAnimationSystem::GetArchetypeBodyLanguage(EBodyLanguageArchetype Archetype) const
{
    if (const FArchetypeBodyLanguage* Found = ArchetypeBodyLanguageMap.Find(Archetype))
    {
        return *Found;
    }
    
    // Retornar configuração padrão
    return FArchetypeBodyLanguage();
}

void UBehavioralAnimationSystem::SetArchetypeBodyLanguage(EBodyLanguageArchetype Archetype, const FArchetypeBodyLanguage& BodyLanguage)
{
    ArchetypeBodyLanguageMap.Add(Archetype, BodyLanguage);
    
    // Atualizar todos os personagens com este arquétipo
    for (auto& CharacterPair : RegisteredCharacters)
    {
        if (CharacterPair.Value.Archetype == Archetype)
        {
            CharacterPair.Value.BodyLanguageConfig = BodyLanguage;
            ApplyBodyLanguageConfiguration(CharacterPair.Key, BodyLanguage);
        }
    }
}

bool UBehavioralAnimationSystem::IsCharacterInEmotionalState(ACharacter* Character, EEmotionalState State) const
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return false;
    }
    
    const FCharacterBehavioralData& CharacterData = RegisteredCharacters[Character];
    
    // Determinar estado emocional dominante
    EEmotionalState DominantState = GetDominantEmotionalState(CharacterData.EmotionalState);
    
    return DominantState == State;
}

float UBehavioralAnimationSystem::GetEmotionalIntensity(ACharacter* Character, EEmotionalState State) const
{
    if (!Character || !RegisteredCharacters.Contains(Character))
    {
        return 0.0f;
    }
    
    const FCharacterBehavioralData& CharacterData = RegisteredCharacters[Character];
    
    switch (State)
    {
        case EEmotionalState::Fearful:
            return CharacterData.EmotionalState.Fear;
        case EEmotionalState::Excited:
            return CharacterData.EmotionalState.Confidence;
        case EEmotionalState::Curious:
            return CharacterData.EmotionalState.Curiosity;
        case EEmotionalState::Angry:
            return CharacterData.EmotionalState.Alertness;
        default:
            return 0.0f;
    }
}

void UBehavioralAnimationSystem::LoadSystemConfiguration()
{
    // Carregar configuração de arquivo ou usar padrões
    SystemConfig.bSystemEnabled = true;
    SystemConfig.MaxRegisteredCharacters = 50;
    SystemConfig.EmotionalUpdateInterval = 0.5f;
    SystemConfig.GestureUpdateInterval = 0.1f;
    SystemConfig.bUseProceduralGestures = true;
    SystemConfig.bUseEmotionalAnimations = true;
    SystemConfig.bUseAdaptiveBodyLanguage = true;
    SystemConfig.GlobalGestureFrequency = 1.0f;
    SystemConfig.EmotionalResponsiveness = 0.7f;
    SystemConfig.bEnableDebugMode = false;
}

void UBehavioralAnimationSystem::InitializeArchetypeMappings()
{
    // Configurar linguagem corporal para cada arquétipo
    
    // Paleontologista - Curiosidade Científica
    FArchetypeBodyLanguage ScientistBodyLanguage;
    ScientistBodyLanguage.Archetype = EBodyLanguageArchetype::ScientificCuriosity;
    ScientistBodyLanguage.MovementPersonality = EMovementPersonality::Contemplative;
    ScientistBodyLanguage.PostureWeight = 0.8f;
    ScientistBodyLanguage.GestureFrequency = 0.6f;
    ScientistBodyLanguage.EyeContactLevel = 0.9f;
    ScientistBodyLanguage.PersonalSpaceRadius = 120.0f;
    ScientistBodyLanguage.MovementSpeed = 0.8f;
    ScientistBodyLanguage.TensionLevel = 0.2f;
    ScientistBodyLanguage.HeadTiltAngle = 5.0f;
    ScientistBodyLanguage.ShoulderTension = 0.1f;
    ScientistBodyLanguage.HandGestureStyle = 0.8f;
    ScientistBodyLanguage.BlinkRate = 0.8f;
    ScientistBodyLanguage.BreathingDepth = 0.9f;
    ArchetypeBodyLanguageMap.Add(EBodyLanguageArchetype::ScientificCuriosity, ScientistBodyLanguage);
    
    // Sobrevivente - Vigilância Constante
    FArchetypeBodyLanguage SurvivorBodyLanguage;
    SurvivorBodyLanguage.Archetype = EBodyLanguageArchetype::ConstantVigilance;
    SurvivorBodyLanguage.MovementPersonality = EMovementPersonality::Hyperalert;
    SurvivorBodyLanguage.PostureWeight = 1.0f;
    SurvivorBodyLanguage.GestureFrequency = 0.3f;
    SurvivorBodyLanguage.EyeContactLevel = 0.5f;
    SurvivorBodyLanguage.PersonalSpaceRadius = 200.0f;
    SurvivorBodyLanguage.MovementSpeed = 1.2f;
    SurvivorBodyLanguage.TensionLevel = 0.8f;
    SurvivorBodyLanguage.HeadTiltAngle = 0.0f;
    SurvivorBodyLanguage.ShoulderTension = 0.7f;
    SurvivorBodyLanguage.HandGestureStyle = 0.2f;
    SurvivorBodyLanguage.BlinkRate = 1.5f;
    SurvivorBodyLanguage.BreathingDepth = 0.6f;
    ArchetypeBodyLanguageMap.Add(EBodyLanguageArchetype::ConstantVigilance, SurvivorBodyLanguage);
    
    // Explorador - Confiança Territorial
    FArchetypeBodyLanguage ExplorerBodyLanguage;
    ExplorerBodyLanguage.Archetype = EBodyLanguageArchetype::TerritorialConfidence;
    ExplorerBodyLanguage.MovementPersonality = EMovementPersonality::Confident;
    ExplorerBodyLanguage.PostureWeight = 1.0f;
    ExplorerBodyLanguage.GestureFrequency = 0.7f;
    ExplorerBodyLanguage.EyeContactLevel = 0.8f;
    ExplorerBodyLanguage.PersonalSpaceRadius = 150.0f;
    ExplorerBodyLanguage.MovementSpeed = 1.1f;
    ExplorerBodyLanguage.TensionLevel = 0.3f;
    ExplorerBodyLanguage.HeadTiltAngle = -2.0f;
    ExplorerBodyLanguage.ShoulderTension = 0.2f;
    ExplorerBodyLanguage.HandGestureStyle = 0.6f;
    ExplorerBodyLanguage.BlinkRate = 0.9f;
    ExplorerBodyLanguage.BreathingDepth = 0.8f;
    ArchetypeBodyLanguageMap.Add(EBodyLanguageArchetype::TerritorialConfidence, ExplorerBodyLanguage);
}

void UBehavioralAnimationSystem::InitializeAnimationPools()
{
    // Inicializar pools de animações por categoria
    // Isso seria normalmente carregado de assets
    
    UE_LOG(LogBehavioralAnimation, Log, TEXT("Animation pools initialized"));
}

void UBehavioralAnimationSystem::UpdateRegisteredCharacters(float DeltaTime)
{
    for (auto& CharacterPair : RegisteredCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterBehavioralData& CharacterData = CharacterPair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Atualizar estado emocional baseado no ambiente
        UpdateCharacterEmotionalResponse(Character, CharacterData, DeltaTime);
        
        // Atualizar linguagem corporal
        UpdateCharacterBodyLanguage(Character, CharacterData, DeltaTime);
        
        // Processar gestos automáticos
        ProcessAutomaticGestures(Character, CharacterData, DeltaTime);
    }
}

void UBehavioralAnimationSystem::UpdateEmotionalStates()
{
    for (auto& CharacterPair : RegisteredCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterBehavioralData& CharacterData = CharacterPair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Decaimento natural das emoções
        ApplyEmotionalDecay(CharacterData.EmotionalState, SystemConfig.EmotionalUpdateInterval);
        
        // Verificar triggers ambientais
        CheckEnvironmentalEmotionalTriggers(Character, CharacterData);
    }
}

void UBehavioralAnimationSystem::UpdateProceduralGestures()
{
    if (!SystemConfig.bUseProceduralGestures)
    {
        return;
    }
    
    for (auto& CharacterPair : RegisteredCharacters)
    {
        ACharacter* Character = CharacterPair.Key;
        FCharacterBehavioralData& CharacterData = CharacterPair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Verificar se deve disparar gesto procedural
        if (ShouldTriggerProceduralGesture(Character, CharacterData))
        {
            TriggerRandomGestureForArchetype(Character, CharacterData.Archetype);
        }
    }
}

void UBehavioralAnimationSystem::ProcessGestureQueue(float DeltaTime)
{
    for (int32 i = GestureQueue.Num() - 1; i >= 0; --i)
    {
        FGestureQueueEntry& Entry = GestureQueue[i];
        
        if (!IsValid(Entry.Character))
        {
            GestureQueue.RemoveAt(i);
            continue;
        }
        
        // Verificar se o personagem pode executar o gesto
        if (CanCharacterExecuteGesture(Entry.Character))
        {
            ExecuteGesture(Entry);
            GestureQueue.RemoveAt(i);
        }
        else if (GetWorld()->GetTimeSeconds() - Entry.QueueTime > 5.0f) // Timeout de 5 segundos
        {
            GestureQueue.RemoveAt(i);
        }
    }
}

void UBehavioralAnimationSystem::UpdatePerformanceStats(float DeltaTime)
{
    // Atualizar estatísticas de performance para debug
    PerformanceStats.RegisteredCharacterCount = RegisteredCharacters.Num();
    PerformanceStats.ActiveGestureCount = ActiveGestures.Num();
    PerformanceStats.QueuedGestureCount = GestureQueue.Num();
    PerformanceStats.LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void UBehavioralAnimationSystem::ApplyArchetypeConfiguration(ACharacter* Character, EBodyLanguageArchetype Archetype)
{
    if (!Character)
    {
        return;
    }
    
    const FArchetypeBodyLanguage BodyLanguage = GetArchetypeBodyLanguage(Archetype);
    ApplyBodyLanguageConfiguration(Character, BodyLanguage);
}

void UBehavioralAnimationSystem::ApplyBodyLanguageConfiguration(ACharacter* Character, const FArchetypeBodyLanguage& BodyLanguage)
{
    if (!Character)
    {
        return;
    }
    
    // Aplicar configurações de movimento
    if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
    {
        MovementComponent->MaxWalkSpeed *= BodyLanguage.MovementSpeed;
    }
    
    // Aplicar configurações de animação via Animation Blueprint
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        // Definir variáveis de linguagem corporal no Animation Blueprint
        AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
    }
}

void UBehavioralAnimationSystem::ApplyEmotionalConfiguration(ACharacter* Character, const FEmotionalState& EmotionalState)
{
    if (!Character)
    {
        return;
    }
    
    // Aplicar modificações baseadas no estado emocional
    // Isso seria implementado com base no Animation Blueprint específico
}

bool UBehavioralAnimationSystem::HasSignificantEmotionalChange(const FEmotionalState& Previous, const FEmotionalState& Current) const
{
    const float Threshold = 0.2f; // 20% de mudança
    
    return FMath::Abs(Previous.Fear - Current.Fear) > Threshold ||
           FMath::Abs(Previous.Confidence - Current.Confidence) > Threshold ||
           FMath::Abs(Previous.Alertness - Current.Alertness) > Threshold ||
           FMath::Abs(Previous.Curiosity - Current.Curiosity) > Threshold;
}

void UBehavioralAnimationSystem::TriggerEmotionalGesture(ACharacter* Character, const FEmotionalState& EmotionalState)
{
    if (!Character)
    {
        return;
    }
    
    // Determinar tipo de gesto baseado no estado emocional dominante
    EEmotionalState DominantState = GetDominantEmotionalState(EmotionalState);
    
    FString GestureContext;
    switch (DominantState)
    {
        case EEmotionalState::Fearful:
            GestureContext = TEXT("Fear");
            break;
        case EEmotionalState::Curious:
            GestureContext = TEXT("Curiosity");
            break;
        case EEmotionalState::Excited:
            GestureContext = TEXT("Excitement");
            break;
        case EEmotionalState::Angry:
            GestureContext = TEXT("Anger");
            break;
        default:
            GestureContext = TEXT("Neutral");
            break;
    }
    
    TriggerContextualGesture(Character, GestureContext, 0.7f);
}

UAnimMontage* UBehavioralAnimationSystem::FindContextualGesture(EBodyLanguageArchetype Archetype, const FString& Context) const
{
    // Buscar gesto apropriado nos pools de animação
    // Isso seria implementado com base nos assets carregados
    
    return nullptr; // Placeholder
}

EEmotionalState UBehavioralAnimationSystem::GetDominantEmotionalState(const FEmotionalState& EmotionalState) const
{
    float MaxValue = 0.0f;
    EEmotionalState DominantState = EEmotionalState::Neutral;
    
    if (EmotionalState.Fear > MaxValue)
    {
        MaxValue = EmotionalState.Fear;
        DominantState = EEmotionalState::Fearful;
    }
    
    if (EmotionalState.Confidence > MaxValue)
    {
        MaxValue = EmotionalState.Confidence;
        DominantState = EEmotionalState::Excited;
    }
    
    if (EmotionalState.Curiosity > MaxValue)
    {
        MaxValue = EmotionalState.Curiosity;
        DominantState = EEmotionalState::Curious;
    }
    
    if (EmotionalState.Alertness > MaxValue)
    {
        MaxValue = EmotionalState.Alertness;
        DominantState = EEmotionalState::Angry;
    }
    
    return DominantState;
}

void UBehavioralAnimationSystem::UpdateCharacterEmotionalResponse(ACharacter* Character, FCharacterBehavioralData& CharacterData, float DeltaTime)
{
    // Atualizar resposta emocional baseada no ambiente
    // Isso incluiria detecção de ameaças, descobertas, etc.
}

void UBehavioralAnimationSystem::UpdateCharacterBodyLanguage(ACharacter* Character, FCharacterBehavioralData& CharacterData, float DeltaTime)
{
    // Atualizar linguagem corporal baseada no estado atual
}

void UBehavioralAnimationSystem::ProcessAutomaticGestures(ACharacter* Character, FCharacterBehavioralData& CharacterData, float DeltaTime)
{
    // Processar gestos automáticos baseados no arquétipo e estado
}

void UBehavioralAnimationSystem::ApplyEmotionalDecay(FEmotionalState& EmotionalState, float DeltaTime) const
{
    const float DecayRate = 0.1f; // 10% por segundo
    
    EmotionalState.Fear = FMath::Max(0.0f, EmotionalState.Fear - DecayRate * DeltaTime);
    EmotionalState.Alertness = FMath::Max(0.0f, EmotionalState.Alertness - DecayRate * DeltaTime);
    EmotionalState.Exhaustion = FMath::Max(0.0f, EmotionalState.Exhaustion - DecayRate * DeltaTime);
}

void UBehavioralAnimationSystem::CheckEnvironmentalEmotionalTriggers(ACharacter* Character, FCharacterBehavioralData& CharacterData)
{
    // Verificar triggers ambientais que afetam emoções
    // Isso incluiria detecção de dinossauros, sons, etc.
}

bool UBehavioralAnimationSystem::ShouldTriggerProceduralGesture(ACharacter* Character, const FCharacterBehavioralData& CharacterData) const
{
    // Determinar se deve disparar um gesto procedural
    float GestureChance = CharacterData.BodyLanguageConfig.GestureFrequency * SystemConfig.GlobalGestureFrequency;
    
    return FMath::RandRange(0.0f, 1.0f) < GestureChance * 0.01f; // 1% por frame na frequência máxima
}

void UBehavioralAnimationSystem::TriggerRandomGestureForArchetype(ACharacter* Character, EBodyLanguageArchetype Archetype)
{
    // Disparar gesto aleatório apropriado para o arquétipo
    FString GestureContext = TEXT("Idle");
    
    switch (Archetype)
    {
        case EBodyLanguageArchetype::ScientificCuriosity:
            GestureContext = TEXT("Thinking");
            break;
        case EBodyLanguageArchetype::ConstantVigilance:
            GestureContext = TEXT("Alert");
            break;
        case EBodyLanguageArchetype::TerritorialConfidence:
            GestureContext = TEXT("Confident");
            break;
        default:
            GestureContext = TEXT("Neutral");
            break;
    }
    
    TriggerContextualGesture(Character, GestureContext, 0.3f);
}

bool UBehavioralAnimationSystem::CanCharacterExecuteGesture(ACharacter* Character) const
{
    if (!Character)
    {
        return false;
    }
    
    // Verificar se o personagem não está ocupado com outras animações
    if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
    {
        return !AnimInstance->IsAnyMontagePlaying();
    }
    
    return true;
}

void UBehavioralAnimationSystem::ExecuteGesture(const FGestureQueueEntry& GestureEntry)
{
    if (!GestureEntry.Character || !GestureEntry.GestureMontage)
    {
        return;
    }
    
    // Executar o gesto
    if (UAnimInstance* AnimInstance = GestureEntry.Character->GetMesh()->GetAnimInstance())
    {
        float PlayRate = 1.0f;
        
        // Ajustar velocidade baseada no arquétipo
        if (RegisteredCharacters.Contains(GestureEntry.Character))
        {
            const FCharacterBehavioralData& CharacterData = RegisteredCharacters[GestureEntry.Character];
            PlayRate = CharacterData.BodyLanguageConfig.MovementSpeed;
        }
        
        AnimInstance->Montage_Play(GestureEntry.GestureMontage, PlayRate);
        
        // Adicionar aos gestos ativos
        FActiveGesture ActiveGesture;
        ActiveGesture.Character = GestureEntry.Character;
        ActiveGesture.Montage = GestureEntry.GestureMontage;
        ActiveGesture.StartTime = GetWorld()->GetTimeSeconds();
        ActiveGesture.Context = GestureEntry.Context;
        
        ActiveGestures.Add(ActiveGesture);
        
        UE_LOG(LogBehavioralAnimation, Verbose, TEXT("Executed gesture %s for character %s"), 
            *GestureEntry.Context, 
            *GestureEntry.Character->GetName());
    }
}

void UBehavioralAnimationSystem::RemoveActiveGesturesForCharacter(ACharacter* Character)
{
    for (int32 i = ActiveGestures.Num() - 1; i >= 0; --i)
    {
        if (ActiveGestures[i].Character == Character)
        {
            ActiveGestures.RemoveAt(i);
        }
    }
}