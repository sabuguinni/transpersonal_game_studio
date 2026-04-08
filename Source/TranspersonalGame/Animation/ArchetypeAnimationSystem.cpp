#include "ArchetypeAnimationSystem.h"
#include "BehavioralAnimationSystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Characters/CharacterArchetypes.h"
#include "MotionMatching/MotionMatchingSubsystem.h"
#include "IK/AdaptiveIKComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Configurações padrão
    CurrentArchetype = EBodyLanguageArchetype::ScientificCuriosity;
    CurrentMovementPersonality = EMovementPersonality::Cautious;
    CurrentSurvivalState = ESurvivalState::Fresh;
    CurrentStressLevel = EStressLevel::Calm;
    CurrentEmotionalState = EEmotionalState::Neutral;
    
    // Pesos de blending
    ArchetypeBlendWeight = 1.0f;
    PersonalityBlendWeight = 1.0f;
    EmotionalBlendWeight = 0.7f;
    StressBlendWeight = 0.8f;
    
    // Configurações de transição
    ArchetypeTransitionSpeed = 2.0f;
    PersonalityTransitionSpeed = 1.5f;
    EmotionalTransitionSpeed = 3.0f;
    StressTransitionSpeed = 4.0f;
    
    // Inicializar configurações padrão de arquétipos
    InitializeArchetypeConfigurations();
}

void UArchetypeAnimationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Obter referências necessárias
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogArchetypeAnimation, Error, TEXT("ArchetypeAnimationSystem: Owner is not a Character!"));
        return;
    }
    
    SkeletalMeshComponent = OwnerCharacter->GetMesh();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogArchetypeAnimation, Error, TEXT("ArchetypeAnimationSystem: No SkeletalMeshComponent found!"));
        return;
    }
    
    AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogArchetypeAnimation, Error, TEXT("ArchetypeAnimationSystem: No AnimInstance found!"));
        return;
    }
    
    // Obter subsistemas
    MotionMatchingSubsystem = GetWorld()->GetSubsystem<UMotionMatchingSubsystem>();
    BehavioralAnimationSubsystem = GetWorld()->GetSubsystem<UBehavioralAnimationSystem>();
    
    // Configurar IK adaptativo
    SetupAdaptiveIK();
    
    // Aplicar configuração inicial do arquétipo
    ApplyArchetypeConfiguration(CurrentArchetype);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem initialized for character: %s"), 
           *OwnerCharacter->GetName());
}

void UArchetypeAnimationSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!IsValid(OwnerCharacter) || !IsValid(AnimInstance))
    {
        return;
    }
    
    // Atualizar blending de arquétipos
    UpdateArchetypeBlending(DeltaTime);
    
    // Atualizar linguagem corporal procedural
    UpdateProceduralBodyLanguage(DeltaTime);
    
    // Atualizar micro-expressões
    UpdateMicroExpressions(DeltaTime);
    
    // Atualizar adaptação ao terreno
    UpdateTerrainAdaptation(DeltaTime);
    
    // Atualizar parâmetros de animação
    UpdateAnimationParameters(DeltaTime);
}

void UArchetypeAnimationSystem::SetArchetype(EBodyLanguageArchetype NewArchetype, float TransitionTime)
{
    if (NewArchetype == CurrentArchetype)
    {
        return;
    }
    
    PreviousArchetype = CurrentArchetype;
    CurrentArchetype = NewArchetype;
    ArchetypeTransitionAlpha = 0.0f;
    ArchetypeTransitionDuration = TransitionTime;
    
    // Aplicar nova configuração
    ApplyArchetypeConfiguration(NewArchetype);
    
    // Notificar subsistemas
    if (MotionMatchingSubsystem)
    {
        MotionMatchingSubsystem->OnArchetypeChanged(OwnerCharacter, NewArchetype);
    }
    
    if (BehavioralAnimationSubsystem)
    {
        BehavioralAnimationSubsystem->OnArchetypeChanged(OwnerCharacter, NewArchetype);
    }
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype changed from %d to %d for character: %s"), 
           (int32)PreviousArchetype, (int32)NewArchetype, *OwnerCharacter->GetName());
}

void UArchetypeAnimationSystem::SetMovementPersonality(EMovementPersonality NewPersonality, float TransitionTime)
{
    if (NewPersonality == CurrentMovementPersonality)
    {
        return;
    }
    
    PreviousMovementPersonality = CurrentMovementPersonality;
    CurrentMovementPersonality = NewPersonality;
    PersonalityTransitionAlpha = 0.0f;
    PersonalityTransitionDuration = TransitionTime;
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Movement personality changed from %d to %d"), 
           (int32)PreviousMovementPersonality, (int32)NewPersonality);
}

void UArchetypeAnimationSystem::SetEmotionalState(EEmotionalState NewState, float TransitionTime)
{
    if (NewState == CurrentEmotionalState)
    {
        return;
    }
    
    PreviousEmotionalState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;
    EmotionalTransitionAlpha = 0.0f;
    EmotionalTransitionDuration = TransitionTime;
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Emotional state changed from %d to %d"), 
           (int32)PreviousEmotionalState, (int32)NewState);
}

void UArchetypeAnimationSystem::SetStressLevel(EStressLevel NewLevel, float TransitionTime)
{
    if (NewLevel == CurrentStressLevel)
    {
        return;
    }
    
    PreviousStressLevel = CurrentStressLevel;
    CurrentStressLevel = NewLevel;
    StressTransitionAlpha = 0.0f;
    StressTransitionDuration = TransitionTime;
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Stress level changed from %d to %d"), 
           (int32)PreviousStressLevel, (int32)NewLevel);
}

void UArchetypeAnimationSystem::InitializeArchetypeConfigurations()
{
    // Configuração para Paleontologista (Scientific Curiosity)
    FArchetypeBodyLanguage ScientificConfig;
    ScientificConfig.Archetype = EBodyLanguageArchetype::ScientificCuriosity;
    ScientificConfig.MovementPersonality = EMovementPersonality::Contemplative;
    ScientificConfig.PostureWeight = 0.8f;
    ScientificConfig.GestureFrequency = 0.3f;
    ScientificConfig.EyeContactLevel = 0.9f;
    ScientificConfig.PersonalSpaceRadius = 120.0f;
    ScientificConfig.MovementSpeed = 0.8f;
    ScientificConfig.TensionLevel = 0.2f;
    ScientificConfig.HeadTiltAngle = 5.0f;
    ScientificConfig.ShoulderTension = 0.1f;
    ScientificConfig.HandGestureStyle = 0.7f;
    ScientificConfig.BlinkRate = 0.8f;
    ScientificConfig.BreathingDepth = 0.9f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::ScientificCuriosity, ScientificConfig);
    
    // Configuração para Sobrevivente (Constant Vigilance)
    FArchetypeBodyLanguage VigilanceConfig;
    VigilanceConfig.Archetype = EBodyLanguageArchetype::ConstantVigilance;
    VigilanceConfig.MovementPersonality = EMovementPersonality::Hyperalert;
    VigilanceConfig.PostureWeight = 1.0f;
    VigilanceConfig.GestureFrequency = 0.1f;
    VigilanceConfig.EyeContactLevel = 0.4f;
    VigilanceConfig.PersonalSpaceRadius = 200.0f;
    VigilanceConfig.MovementSpeed = 0.9f;
    VigilanceConfig.TensionLevel = 0.8f;
    VigilanceConfig.HeadTiltAngle = 0.0f;
    VigilanceConfig.ShoulderTension = 0.7f;
    VigilanceConfig.HandGestureStyle = 0.2f;
    VigilanceConfig.BlinkRate = 1.5f;
    VigilanceConfig.BreathingDepth = 0.6f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::ConstantVigilance, VigilanceConfig);
    
    // Configuração para Explorador (Territorial Confidence)
    FArchetypeBodyLanguage ExplorerConfig;
    ExplorerConfig.Archetype = EBodyLanguageArchetype::TerritorialConfidence;
    ExplorerConfig.MovementPersonality = EMovementPersonality::Confident;
    ExplorerConfig.PostureWeight = 1.0f;
    ExplorerConfig.GestureFrequency = 0.6f;
    ExplorerConfig.EyeContactLevel = 0.8f;
    ExplorerConfig.PersonalSpaceRadius = 180.0f;
    ExplorerConfig.MovementSpeed = 1.1f;
    ExplorerConfig.TensionLevel = 0.3f;
    ExplorerConfig.HeadTiltAngle = -2.0f;
    ExplorerConfig.ShoulderTension = 0.2f;
    ExplorerConfig.HandGestureStyle = 0.8f;
    ExplorerConfig.BlinkRate = 0.9f;
    ExplorerConfig.BreathingDepth = 1.0f;
    ArchetypeConfigurations.Add(EBodyLanguageArchetype::TerritorialConfidence, ExplorerConfig);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype configurations initialized"));
}

void UArchetypeAnimationSystem::ApplyArchetypeConfiguration(EBodyLanguageArchetype Archetype)
{
    if (!ArchetypeConfigurations.Contains(Archetype))
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("No configuration found for archetype: %d"), (int32)Archetype);
        return;
    }
    
    const FArchetypeBodyLanguage& Config = ArchetypeConfigurations[Archetype];
    CurrentBodyLanguageConfig = Config;
    
    // Aplicar configuração ao Motion Matching
    if (MotionMatchingSubsystem)
    {
        MotionMatchingSubsystem->SetArchetypeConfiguration(OwnerCharacter, Config);
    }
    
    // Aplicar configuração ao IK
    if (AdaptiveIKComponent)
    {
        AdaptiveIKComponent->SetArchetypeConfiguration(Config);
    }
}

void UArchetypeAnimationSystem::UpdateArchetypeBlending(float DeltaTime)
{
    // Atualizar transição de arquétipo
    if (ArchetypeTransitionAlpha < 1.0f)
    {
        ArchetypeTransitionAlpha += DeltaTime / FMath::Max(ArchetypeTransitionDuration, 0.1f);
        ArchetypeTransitionAlpha = FMath::Clamp(ArchetypeTransitionAlpha, 0.0f, 1.0f);
    }
    
    // Atualizar transição de personalidade
    if (PersonalityTransitionAlpha < 1.0f)
    {
        PersonalityTransitionAlpha += DeltaTime / FMath::Max(PersonalityTransitionDuration, 0.1f);
        PersonalityTransitionAlpha = FMath::Clamp(PersonalityTransitionAlpha, 0.0f, 1.0f);
    }
    
    // Atualizar transição emocional
    if (EmotionalTransitionAlpha < 1.0f)
    {
        EmotionalTransitionAlpha += DeltaTime / FMath::Max(EmotionalTransitionDuration, 0.1f);
        EmotionalTransitionAlpha = FMath::Clamp(EmotionalTransitionAlpha, 0.0f, 1.0f);
    }
    
    // Atualizar transição de stress
    if (StressTransitionAlpha < 1.0f)
    {
        StressTransitionAlpha += DeltaTime / FMath::Max(StressTransitionDuration, 0.1f);
        StressTransitionAlpha = FMath::Clamp(StressTransitionAlpha, 0.0f, 1.0f);
    }
}

void UArchetypeAnimationSystem::UpdateProceduralBodyLanguage(float DeltaTime)
{
    if (!IsValid(AnimInstance))
    {
        return;
    }
    
    // Calcular parâmetros de linguagem corporal baseados no estado atual
    float PostureInfluence = CalculatePostureInfluence();
    float GestureInfluence = CalculateGestureInfluence();
    float TensionInfluence = CalculateTensionInfluence();
    
    // Aplicar aos parâmetros de animação
    AnimInstance->SetCurveValue(TEXT("PostureWeight"), PostureInfluence);
    AnimInstance->SetCurveValue(TEXT("GestureFrequency"), GestureInfluence);
    AnimInstance->SetCurveValue(TEXT("TensionLevel"), TensionInfluence);
    AnimInstance->SetCurveValue(TEXT("MovementSpeed"), CurrentBodyLanguageConfig.MovementSpeed);
}

void UArchetypeAnimationSystem::UpdateMicroExpressions(float DeltaTime)
{
    if (!IsValid(AnimInstance))
    {
        return;
    }
    
    // Calcular micro-expressões baseadas no estado emocional e stress
    float BlinkRate = CalculateBlinkRate();
    float HeadTilt = CalculateHeadTilt();
    float ShoulderTension = CalculateShoulderTension();
    float BreathingDepth = CalculateBreathingDepth();
    
    // Aplicar micro-expressões
    AnimInstance->SetCurveValue(TEXT("BlinkRate"), BlinkRate);
    AnimInstance->SetCurveValue(TEXT("HeadTilt"), HeadTilt);
    AnimInstance->SetCurveValue(TEXT("ShoulderTension"), ShoulderTension);
    AnimInstance->SetCurveValue(TEXT("BreathingDepth"), BreathingDepth);
}

void UArchetypeAnimationSystem::UpdateTerrainAdaptation(float DeltaTime)
{
    if (!AdaptiveIKComponent)
    {
        return;
    }
    
    // Atualizar IK baseado no arquétipo atual
    AdaptiveIKComponent->UpdateArchetypeAdaptation(CurrentArchetype, CurrentStressLevel);
}

void UArchetypeAnimationSystem::UpdateAnimationParameters(float DeltaTime)
{
    if (!IsValid(AnimInstance))
    {
        return;
    }
    
    // Parâmetros gerais de estado
    AnimInstance->SetCurveValue(TEXT("ArchetypeBlend"), ArchetypeTransitionAlpha);
    AnimInstance->SetCurveValue(TEXT("PersonalityBlend"), PersonalityTransitionAlpha);
    AnimInstance->SetCurveValue(TEXT("EmotionalBlend"), EmotionalTransitionAlpha);
    AnimInstance->SetCurveValue(TEXT("StressBlend"), StressTransitionAlpha);
    
    // Estados enumerados
    AnimInstance->SetCurveValue(TEXT("CurrentArchetype"), (float)(int32)CurrentArchetype);
    AnimInstance->SetCurveValue(TEXT("CurrentPersonality"), (float)(int32)CurrentMovementPersonality);
    AnimInstance->SetCurveValue(TEXT("CurrentEmotion"), (float)(int32)CurrentEmotionalState);
    AnimInstance->SetCurveValue(TEXT("CurrentStress"), (float)(int32)CurrentStressLevel);
}

void UArchetypeAnimationSystem::SetupAdaptiveIK()
{
    if (!IsValid(OwnerCharacter))
    {
        return;
    }
    
    // Procurar componente IK existente
    AdaptiveIKComponent = OwnerCharacter->FindComponentByClass<UAdaptiveIKComponent>();
    
    // Se não existir, criar um
    if (!AdaptiveIKComponent)
    {
        AdaptiveIKComponent = NewObject<UAdaptiveIKComponent>(OwnerCharacter);
        OwnerCharacter->AddInstanceComponent(AdaptiveIKComponent);
        AdaptiveIKComponent->RegisterComponent();
    }
    
    if (AdaptiveIKComponent)
    {
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Adaptive IK component setup complete"));
    }
}

float UArchetypeAnimationSystem::CalculatePostureInfluence() const
{
    float BasePosture = CurrentBodyLanguageConfig.PostureWeight;
    float StressModifier = GetStressModifier();
    float EmotionalModifier = GetEmotionalModifier();
    
    return FMath::Clamp(BasePosture * StressModifier * EmotionalModifier, 0.0f, 1.0f);
}

float UArchetypeAnimationSystem::CalculateGestureInfluence() const
{
    float BaseGesture = CurrentBodyLanguageConfig.GestureFrequency;
    float PersonalityModifier = GetPersonalityModifier();
    float EmotionalModifier = GetEmotionalModifier();
    
    return FMath::Clamp(BaseGesture * PersonalityModifier * EmotionalModifier, 0.0f, 1.0f);
}

float UArchetypeAnimationSystem::CalculateTensionInfluence() const
{
    float BaseTension = CurrentBodyLanguageConfig.TensionLevel;
    float StressModifier = GetStressModifier();
    
    return FMath::Clamp(BaseTension + (StressModifier - 1.0f) * 0.5f, 0.0f, 1.0f);
}

float UArchetypeAnimationSystem::CalculateBlinkRate() const
{
    float BaseRate = CurrentBodyLanguageConfig.BlinkRate;
    float StressMultiplier = 1.0f;
    
    switch (CurrentStressLevel)
    {
        case EStressLevel::Calm: StressMultiplier = 0.8f; break;
        case EStressLevel::Aware: StressMultiplier = 1.0f; break;
        case EStressLevel::Alert: StressMultiplier = 1.3f; break;
        case EStressLevel::Tense: StressMultiplier = 1.6f; break;
        case EStressLevel::Panicked: StressMultiplier = 2.0f; break;
    }
    
    return BaseRate * StressMultiplier;
}

float UArchetypeAnimationSystem::CalculateHeadTilt() const
{
    float BaseTilt = CurrentBodyLanguageConfig.HeadTiltAngle;
    float EmotionalModifier = 1.0f;
    
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Curious: EmotionalModifier = 1.5f; break;
        case EEmotionalState::Contemplative: EmotionalModifier = 1.2f; break;
        case EEmotionalState::Sad: EmotionalModifier = 0.5f; break;
        case EEmotionalState::Confident: EmotionalModifier = 0.8f; break;
        default: EmotionalModifier = 1.0f; break;
    }
    
    return BaseTilt * EmotionalModifier;
}

float UArchetypeAnimationSystem::CalculateShoulderTension() const
{
    float BaseTension = CurrentBodyLanguageConfig.ShoulderTension;
    float StressModifier = GetStressModifier();
    
    return FMath::Clamp(BaseTension * StressModifier, 0.0f, 1.0f);
}

float UArchetypeAnimationSystem::CalculateBreathingDepth() const
{
    float BaseDepth = CurrentBodyLanguageConfig.BreathingDepth;
    float StressModifier = 2.0f - GetStressModifier(); // Stress reduz profundidade
    
    return FMath::Clamp(BaseDepth * StressModifier, 0.1f, 2.0f);
}

float UArchetypeAnimationSystem::GetStressModifier() const
{
    switch (CurrentStressLevel)
    {
        case EStressLevel::Calm: return 0.7f;
        case EStressLevel::Aware: return 1.0f;
        case EStressLevel::Alert: return 1.3f;
        case EStressLevel::Tense: return 1.6f;
        case EStressLevel::Panicked: return 2.0f;
        default: return 1.0f;
    }
}

float UArchetypeAnimationSystem::GetEmotionalModifier() const
{
    switch (CurrentEmotionalState)
    {
        case EEmotionalState::Excited: return 1.3f;
        case EEmotionalState::Angry: return 1.5f;
        case EEmotionalState::Fearful: return 0.7f;
        case EEmotionalState::Sad: return 0.6f;
        case EEmotionalState::Determined: return 1.2f;
        default: return 1.0f;
    }
}

float UArchetypeAnimationSystem::GetPersonalityModifier() const
{
    switch (CurrentMovementPersonality)
    {
        case EMovementPersonality::Confident: return 1.2f;
        case EMovementPersonality::Nervous: return 1.5f;
        case EMovementPersonality::Aggressive: return 1.4f;
        case EMovementPersonality::Graceful: return 0.8f;
        case EMovementPersonality::Exhausted: return 0.5f;
        default: return 1.0f;
    }
}