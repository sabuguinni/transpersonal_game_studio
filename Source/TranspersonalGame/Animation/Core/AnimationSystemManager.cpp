#include "AnimationSystemManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnimationSystemManager::UAnimationSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Valores padrão
    CharacterType = ECharacterAnimationType::Player;
    CurrentEmotionalState = EEmotionalState::Calm;
    PreviousEmotionalState = EEmotionalState::Calm;
    
    StateTransitionTime = 0.0f;
    PersonalityUpdateTimer = 0.0f;
    NaturalVariationTimer = 0.0f;
}

void UAnimationSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar personalidade com variações aleatórias subtis
    if (Personality.Nervousness == 0.5f) // Se ainda está no valor padrão
    {
        // Gerar personalidade única baseada no tipo de personagem
        switch (CharacterType)
        {
            case ECharacterAnimationType::Player:
                Personality.Nervousness = FMath::RandRange(0.6f, 0.8f); // Jogador sempre nervoso
                Personality.Confidence = FMath::RandRange(0.3f, 0.5f);
                Personality.Aggression = FMath::RandRange(0.2f, 0.4f);
                Personality.Curiosity = FMath::RandRange(0.7f, 0.9f);
                break;
                
            case ECharacterAnimationType::SmallHerbivore:
                Personality.Nervousness = FMath::RandRange(0.7f, 0.9f);
                Personality.Confidence = FMath::RandRange(0.2f, 0.4f);
                Personality.Aggression = FMath::RandRange(0.1f, 0.3f);
                Personality.Curiosity = FMath::RandRange(0.5f, 0.8f);
                break;
                
            case ECharacterAnimationType::LargeHerbivore:
                Personality.Nervousness = FMath::RandRange(0.4f, 0.6f);
                Personality.Confidence = FMath::RandRange(0.6f, 0.8f);
                Personality.Aggression = FMath::RandRange(0.3f, 0.5f);
                Personality.Curiosity = FMath::RandRange(0.3f, 0.6f);
                break;
                
            case ECharacterAnimationType::SmallCarnivore:
                Personality.Nervousness = FMath::RandRange(0.3f, 0.5f);
                Personality.Confidence = FMath::RandRange(0.6f, 0.8f);
                Personality.Aggression = FMath::RandRange(0.7f, 0.9f);
                Personality.Curiosity = FMath::RandRange(0.8f, 1.0f);
                break;
                
            case ECharacterAnimationType::LargeCarnivore:
                Personality.Nervousness = FMath::RandRange(0.1f, 0.3f);
                Personality.Confidence = FMath::RandRange(0.8f, 1.0f);
                Personality.Aggression = FMath::RandRange(0.6f, 0.8f);
                Personality.Curiosity = FMath::RandRange(0.4f, 0.7f);
                break;
                
            case ECharacterAnimationType::Apex:
                Personality.Nervousness = FMath::RandRange(0.0f, 0.2f);
                Personality.Confidence = FMath::RandRange(0.9f, 1.0f);
                Personality.Aggression = FMath::RandRange(0.8f, 1.0f);
                Personality.Curiosity = FMath::RandRange(0.2f, 0.5f);
                break;
        }
        
        // Variações físicas únicas
        Personality.MovementSpeed = FMath::RandRange(0.85f, 1.15f);
        Personality.StepLength = FMath::RandRange(0.9f, 1.1f);
        Personality.BodySway = FMath::RandRange(0.95f, 1.05f);
        Personality.HeadMovement = FMath::RandRange(0.8f, 1.2f);
    }
}

void UAnimationSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessEmotionalTransition(DeltaTime);
    UpdatePersonalityInfluence(DeltaTime);
    ApplyNaturalVariations(DeltaTime);
}

void UAnimationSystemManager::SetEmotionalState(EEmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        PreviousEmotionalState = CurrentEmotionalState;
        CurrentEmotionalState = NewState;
        StateTransitionTime = 0.0f;
        
        OnEmotionalStateChanged.Broadcast(PreviousEmotionalState, CurrentEmotionalState);
    }
}

void UAnimationSystemManager::UpdatePersonalityInfluence(float DeltaTime)
{
    PersonalityUpdateTimer += DeltaTime;
    
    // Atualizar influência da personalidade a cada 0.1 segundos
    if (PersonalityUpdateTimer >= 0.1f)
    {
        PersonalityUpdateTimer = 0.0f;
        
        // A personalidade influencia como as transições emocionais acontecem
        // Personagens mais nervosos mudam de estado mais rapidamente
        float nervousnessMultiplier = 1.0f + (Personality.Nervousness * 0.5f);
        StateTransitionTime *= nervousnessMultiplier;
    }
}

float UAnimationSystemManager::GetPersonalityModifier(const FString& ModifierName) const
{
    if (ModifierName == "MovementSpeed")
    {
        float baseSpeed = Personality.MovementSpeed;
        
        // Ajustar velocidade baseado no estado emocional
        switch (CurrentEmotionalState)
        {
            case EEmotionalState::Fearful:
                return baseSpeed * 1.3f; // Mais rápido quando com medo
            case EEmotionalState::Nervous:
                return baseSpeed * 1.1f;
            case EEmotionalState::Resting:
                return baseSpeed * 0.6f; // Mais lento quando descansando
            case EEmotionalState::Hunting:
                return baseSpeed * 1.2f;
            default:
                return baseSpeed;
        }
    }
    else if (ModifierName == "HeadMovement")
    {
        float baseMovement = Personality.HeadMovement;
        
        // Mais movimento de cabeça quando alerta ou nervoso
        switch (CurrentEmotionalState)
        {
            case EEmotionalState::Alert:
                return baseMovement * 1.4f;
            case EEmotionalState::Nervous:
                return baseMovement * 1.6f;
            case EEmotionalState::Fearful:
                return baseMovement * 2.0f;
            case EEmotionalState::Resting:
                return baseMovement * 0.3f;
            default:
                return baseMovement;
        }
    }
    else if (ModifierName == "BodySway")
    {
        float baseSway = Personality.BodySway;
        
        // Mais balanço corporal quando nervoso
        switch (CurrentEmotionalState)
        {
            case EEmotionalState::Nervous:
                return baseSway * 1.3f;
            case EEmotionalState::Fearful:
                return baseSway * 1.5f;
            case EEmotionalState::Confident:
                return baseSway * 0.8f;
            default:
                return baseSway;
        }
    }
    
    return 1.0f; // Valor padrão
}

void UAnimationSystemManager::ProcessEmotionalTransition(float DeltaTime)
{
    StateTransitionTime += DeltaTime;
    
    // Transições emocionais levam tempo baseado na personalidade
    float transitionDuration = 2.0f; // Base de 2 segundos
    
    // Personagens mais nervosos fazem transições mais rápidas
    transitionDuration *= (1.0f - (Personality.Nervousness * 0.5f));
    
    // Personagens mais confiantes fazem transições mais lentas
    transitionDuration *= (1.0f + (Personality.Confidence * 0.3f));
}

void UAnimationSystemManager::ApplyNaturalVariations(float DeltaTime)
{
    NaturalVariationTimer += DeltaTime;
    
    // Aplicar micro-variações naturais a cada 2-5 segundos
    float variationInterval = FMath::RandRange(2.0f, 5.0f);
    
    if (NaturalVariationTimer >= variationInterval)
    {
        NaturalVariationTimer = 0.0f;
        
        // Pequenas variações baseadas na personalidade
        // Estas serão usadas pelos Animation Blueprints para criar movimento orgânico
        
        // Personagens mais nervosos têm mais variações
        if (Personality.Nervousness > 0.6f)
        {
            // Trigger para pequenos movimentos nervosos
            // Será captado pelo Animation Blueprint
        }
        
        // Personagens curiosos ocasionalmente olham em volta
        if (Personality.Curiosity > 0.7f && FMath::RandRange(0.0f, 1.0f) < 0.3f)
        {
            // Trigger para movimento de curiosidade
        }
    }
}