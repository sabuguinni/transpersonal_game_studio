#include "JurassicAnimationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IK/IKRigDefinition.h"
#include "ControlRig.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogJurassicAnimation, Log, All);

void UJurassicAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Initializing Jurassic Animation System"));
    
    // Inicializar configuração padrão
    CurrentConfig = FJurassicAnimationConfig();
    
    // Inicializar pools de objetos
    AnimationAgentPool.Reserve(50);
    MotionControllerPool.Reserve(50);
    IKComponentPool.Reserve(50);
    
    // Configurar timer para limpeza periódica
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UJurassicAnimationSystem::PerformCleanup,
            30.0f, // A cada 30 segundos
            true   // Repetir
        );
    }
    
    // Inicializar cache de dados de arquétipos
    InitializeArchetypeCache();
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Jurassic Animation System initialized successfully"));
}

void UJurassicAnimationSystem::Deinitialize()
{
    UE_LOG(LogJurassicAnimation, Log, TEXT("Deinitializing Jurassic Animation System"));
    
    // Limpar timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
    }
    
    // Limpar todos os agentes registrados
    for (auto& Pair : RegisteredAnimationAgents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->Shutdown();
        }
    }
    RegisteredAnimationAgents.Empty();
    
    // Limpar pools
    AnimationAgentPool.Empty();
    MotionControllerPool.Empty();
    IKComponentPool.Empty();
    
    // Limpar cache
    ArchetypeDataCache.Empty();
    LoadedAnimationAssets.Empty();
    
    Super::Deinitialize();
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Jurassic Animation System deinitialized"));
}

void UJurassicAnimationSystem::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_JurassicAnimationSystem_Tick);
    
    // Atualizar todos os agentes de animação registrados
    for (auto& Pair : RegisteredAnimationAgents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->UpdateAnimation(DeltaTime);
        }
    }
    
    // Processar fila de carregamento de assets
    ProcessAssetLoadingQueue();
    
    // Atualizar estatísticas de performance
    UpdatePerformanceStats(DeltaTime);
}

bool UJurassicAnimationSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Só criar em mundos de jogo
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UJurassicAnimationSystem::SetGlobalConfiguration(const FJurassicAnimationConfig& NewConfig)
{
    CurrentConfig = NewConfig;
    
    // Aplicar configuração a todos os agentes existentes
    for (auto& Pair : RegisteredAnimationAgents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->ApplyGlobalConfiguration(CurrentConfig);
        }
    }
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Global animation configuration updated"));
}

UJurassicAnimationAgent* UJurassicAnimationSystem::RegisterCharacter(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!IsValid(Character))
    {
        UE_LOG(LogJurassicAnimation, Warning, TEXT("Attempted to register invalid character"));
        return nullptr;
    }
    
    // Verificar se já está registrado
    if (RegisteredAnimationAgents.Contains(Character))
    {
        UE_LOG(LogJurassicAnimation, Warning, TEXT("Character %s is already registered"), *Character->GetName());
        return RegisteredAnimationAgents[Character];
    }
    
    // Criar novo agente de animação
    UJurassicAnimationAgent* AnimationAgent = CreateAnimationAgent(Character, Archetype);
    if (!AnimationAgent)
    {
        UE_LOG(LogJurassicAnimation, Error, TEXT("Failed to create animation agent for character %s"), *Character->GetName());
        return nullptr;
    }
    
    // Registrar o agente
    RegisteredAnimationAgents.Add(Character, AnimationAgent);
    
    // Aplicar configuração global
    AnimationAgent->ApplyGlobalConfiguration(CurrentConfig);
    
    // Inicializar o agente
    AnimationAgent->Initialize();
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Successfully registered character %s with archetype %s"), 
           *Character->GetName(), 
           *UEnum::GetValueAsString(Archetype));
    
    return AnimationAgent;
}

void UJurassicAnimationSystem::UnregisterCharacter(ACharacter* Character)
{
    if (!IsValid(Character))
    {
        return;
    }
    
    if (UJurassicAnimationAgent** AgentPtr = RegisteredAnimationAgents.Find(Character))
    {
        if (IsValid(*AgentPtr))
        {
            (*AgentPtr)->Shutdown();
            ReturnAnimationAgentToPool(*AgentPtr);
        }
        
        RegisteredAnimationAgents.Remove(Character);
        
        UE_LOG(LogJurassicAnimation, Log, TEXT("Unregistered character %s"), *Character->GetName());
    }
}

void UJurassicAnimationSystem::UpdateCharacterEmotionalState(ACharacter* Character, const FEmotionalState& NewState)
{
    if (UJurassicAnimationAgent** AgentPtr = RegisteredAnimationAgents.Find(Character))
    {
        if (IsValid(*AgentPtr))
        {
            (*AgentPtr)->UpdateEmotionalState(NewState);
        }
    }
}

void UJurassicAnimationSystem::TriggerEmotionalAnimation(ACharacter* Character, ESurvivalState EmotionType, float Intensity)
{
    if (UJurassicAnimationAgent** AgentPtr = RegisteredAnimationAgents.Find(Character))
    {
        if (IsValid(*AgentPtr))
        {
            (*AgentPtr)->TriggerEmotionalResponse(EmotionType, Intensity);
        }
    }
}

void UJurassicAnimationSystem::SetCharacterBodyLanguageIntensity(ACharacter* Character, EBodyLanguageIntensity Intensity)
{
    if (UJurassicAnimationAgent** AgentPtr = RegisteredAnimationAgents.Find(Character))
    {
        if (IsValid(*AgentPtr))
        {
            (*AgentPtr)->SetBodyLanguageIntensity(Intensity);
        }
    }
}

UArchetypeAnimationData* UJurassicAnimationSystem::GetArchetypeData(ECharacterArchetype Archetype)
{
    if (UArchetypeAnimationData** CachedData = ArchetypeDataCache.Find(Archetype))
    {
        return *CachedData;
    }
    
    // Tentar carregar dados do arquétipo
    FString AssetPath = FString::Printf(TEXT("/Game/Animation/Archetypes/DA_%s_Animation"), 
                                       *UEnum::GetValueAsString(Archetype));
    
    UArchetypeAnimationData* ArchetypeData = LoadObject<UArchetypeAnimationData>(nullptr, *AssetPath);
    if (ArchetypeData)
    {
        ArchetypeDataCache.Add(Archetype, ArchetypeData);
        UE_LOG(LogJurassicAnimation, Log, TEXT("Loaded archetype data for %s"), *UEnum::GetValueAsString(Archetype));
    }
    else
    {
        UE_LOG(LogJurassicAnimation, Warning, TEXT("Failed to load archetype data for %s"), *UEnum::GetValueAsString(Archetype));
    }
    
    return ArchetypeData;
}

void UJurassicAnimationSystem::PreloadArchetypeAssets(ECharacterArchetype Archetype)
{
    UArchetypeAnimationData* ArchetypeData = GetArchetypeData(Archetype);
    if (!ArchetypeData)
    {
        return;
    }
    
    // Adicionar assets à fila de carregamento
    TArray<FSoftObjectPath> AssetsToLoad;
    
    // Motion Matching databases
    if (ArchetypeData->MotionData.LocomotionDatabase.IsValid())
    {
        AssetsToLoad.Add(ArchetypeData->MotionData.LocomotionDatabase.ToSoftObjectPath());
    }
    
    if (ArchetypeData->MotionData.InteractionDatabase.IsValid())
    {
        AssetsToLoad.Add(ArchetypeData->MotionData.InteractionDatabase.ToSoftObjectPath());
    }
    
    // Emotional animations
    for (auto& EmotionPair : ArchetypeData->EmotionalAnimations)
    {
        for (auto& MontageRef : EmotionPair.Value)
        {
            if (MontageRef.IsValid())
            {
                AssetsToLoad.Add(MontageRef.ToSoftObjectPath());
            }
        }
    }
    
    // Gestures
    for (auto& GestureRef : ArchetypeData->IdleGestures)
    {
        if (GestureRef.IsValid())
        {
            AssetsToLoad.Add(GestureRef.ToSoftObjectPath());
        }
    }
    
    // Adicionar à fila de carregamento
    for (const FSoftObjectPath& AssetPath : AssetsToLoad)
    {
        AssetLoadingQueue.AddUnique(AssetPath);
    }
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Queued %d assets for preloading for archetype %s"), 
           AssetsToLoad.Num(), *UEnum::GetValueAsString(Archetype));
}

void UJurassicAnimationSystem::EnableDebugMode(bool bEnable)
{
    CurrentConfig.bEnableDebugMode = bEnable;
    
    // Aplicar a todos os agentes
    for (auto& Pair : RegisteredAnimationAgents)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->SetDebugMode(bEnable);
        }
    }
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Debug mode %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

FJurassicAnimationStats UJurassicAnimationSystem::GetPerformanceStats() const
{
    return CurrentStats;
}

void UJurassicAnimationSystem::InitializeArchetypeCache()
{
    // Pre-cache dados dos arquétipos mais comuns
    TArray<ECharacterArchetype> CommonArchetypes = {
        ECharacterArchetype::Survivor,
        ECharacterArchetype::Leader,
        ECharacterArchetype::Scout
    };
    
    for (ECharacterArchetype Archetype : CommonArchetypes)
    {
        GetArchetypeData(Archetype);
    }
}

UJurassicAnimationAgent* UJurassicAnimationSystem::CreateAnimationAgent(ACharacter* Character, ECharacterArchetype Archetype)
{
    // Tentar reutilizar do pool
    UJurassicAnimationAgent* Agent = GetAnimationAgentFromPool();
    
    if (!Agent)
    {
        // Criar novo se pool estiver vazio
        Agent = NewObject<UJurassicAnimationAgent>(this);
    }
    
    if (Agent)
    {
        Agent->SetupForCharacter(Character, Archetype);
    }
    
    return Agent;
}

UJurassicAnimationAgent* UJurassicAnimationSystem::GetAnimationAgentFromPool()
{
    if (AnimationAgentPool.Num() > 0)
    {
        return AnimationAgentPool.Pop();
    }
    return nullptr;
}

void UJurassicAnimationSystem::ReturnAnimationAgentToPool(UJurassicAnimationAgent* Agent)
{
    if (IsValid(Agent))
    {
        Agent->Reset();
        AnimationAgentPool.Add(Agent);
    }
}

void UJurassicAnimationSystem::ProcessAssetLoadingQueue()
{
    const int32 MaxAssetsPerFrame = 2; // Limitar para manter performance
    int32 ProcessedThisFrame = 0;
    
    while (AssetLoadingQueue.Num() > 0 && ProcessedThisFrame < MaxAssetsPerFrame)
    {
        FSoftObjectPath AssetPath = AssetLoadingQueue[0];
        AssetLoadingQueue.RemoveAt(0);
        
        if (!LoadedAnimationAssets.Contains(AssetPath))
        {
            if (UObject* LoadedAsset = AssetPath.TryLoad())
            {
                LoadedAnimationAssets.Add(AssetPath, LoadedAsset);
                UE_LOG(LogJurassicAnimation, VeryVerbose, TEXT("Loaded animation asset: %s"), *AssetPath.ToString());
            }
        }
        
        ProcessedThisFrame++;
    }
}

void UJurassicAnimationSystem::UpdatePerformanceStats(float DeltaTime)
{
    CurrentStats.ActiveAnimationAgents = RegisteredAnimationAgents.Num();
    CurrentStats.PooledAgents = AnimationAgentPool.Num();
    CurrentStats.LoadedAssets = LoadedAnimationAssets.Num();
    CurrentStats.QueuedAssets = AssetLoadingQueue.Num();
    
    // Calcular FPS médio
    FrameTimeAccumulator += DeltaTime;
    FrameCount++;
    
    if (FrameTimeAccumulator >= 1.0f)
    {
        CurrentStats.AverageFrameTime = FrameTimeAccumulator / FrameCount;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
    
    // Calcular uso de memória (estimativa)
    CurrentStats.MemoryUsageMB = (RegisteredAnimationAgents.Num() * sizeof(UJurassicAnimationAgent) + 
                                 LoadedAnimationAssets.Num() * 1024) / (1024 * 1024);
}

void UJurassicAnimationSystem::PerformCleanup()
{
    // Remover referências inválidas
    TArray<ACharacter*> InvalidCharacters;
    
    for (auto& Pair : RegisteredAnimationAgents)
    {
        if (!IsValid(Pair.Key) || !IsValid(Pair.Value))
        {
            InvalidCharacters.Add(Pair.Key);
        }
    }
    
    for (ACharacter* InvalidCharacter : InvalidCharacters)
    {
        UnregisterCharacter(InvalidCharacter);
    }
    
    // Limpar assets não utilizados há muito tempo
    // (Implementação mais complexa seria necessária para tracking de uso)
    
    UE_LOG(LogJurassicAnimation, VeryVerbose, TEXT("Performed cleanup, removed %d invalid references"), InvalidCharacters.Num());
}