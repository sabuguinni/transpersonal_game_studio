#include "NPCBehaviorSystem.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeNPCBehavior(NPCArchetype);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateBehaviorBasedOnConsciousness();
    CheckPlayerProximity();
    
    if (bIsTransforming)
    {
        TransformationTimer += DeltaTime;
        if (CurrentBehaviorData && TransformationTimer >= (1.0f / CurrentBehaviorData->TransformationSpeed))
        {
            bIsTransforming = false;
            TransformationTimer = 0.0f;
        }
    }
}

void UNPCBehaviorComponent::InitializeNPCBehavior(ENPCArchetype InArchetype)
{
    NPCArchetype = InArchetype;
    
    if (NPCBehaviorDataTable)
    {
        FString ArchetypeName = UEnum::GetValueAsString(InArchetype);
        CurrentBehaviorData = NPCBehaviorDataTable->FindRow<FNPCBehaviorData>(FName(*ArchetypeName), TEXT(""));
        
        if (CurrentBehaviorData)
        {
            CurrentConsciousnessState = CurrentBehaviorData->PreferredState;
            EmotionalState = CurrentBehaviorData->EmotionalResonance;
        }
    }
    
    // Set initial consciousness level based on archetype
    switch (InArchetype)
    {
        case ENPCArchetype::ShadowSelf:
            ConsciousnessLevel = 0.2f;
            CurrentConsciousnessState = EConsciousnessState::Shadow;
            break;
        case ENPCArchetype::InnerChild:
            ConsciousnessLevel = 0.4f;
            CurrentConsciousnessState = EConsciousnessState::Ego;
            break;
        case ENPCArchetype::WiseGuide:
            ConsciousnessLevel = 0.9f;
            CurrentConsciousnessState = EConsciousnessState::Wise;
            break;
        case ENPCArchetype::Anima:
        case ENPCArchetype::Animus:
            ConsciousnessLevel = 0.6f;
            CurrentConsciousnessState = EConsciousnessState::Anima;
            break;
        default:
            ConsciousnessLevel = 0.5f;
            break;
    }
}

void UNPCBehaviorComponent::UpdateConsciousnessState(EConsciousnessState NewState)
{
    EConsciousnessState OldState = CurrentConsciousnessState;
    CurrentConsciousnessState = NewState;
    
    // Update consciousness level based on state
    switch (NewState)
    {
        case EConsciousnessState::Ego:
            ConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.0f, 0.3f);
            break;
        case EConsciousnessState::Shadow:
            ConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.2f, 0.5f);
            break;
        case EConsciousnessState::Anima:
            ConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.4f, 0.7f);
            break;
        case EConsciousnessState::Wise:
            ConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.7f, 0.9f);
            break;
        case EConsciousnessState::Transcendent:
            ConsciousnessLevel = FMath::Clamp(ConsciousnessLevel, 0.9f, 1.0f);
            break;
    }
    
    OnConsciousnessStateChanged(OldState, NewState);
}

void UNPCBehaviorComponent::TriggerTransformation()
{
    if (CurrentBehaviorData && CurrentBehaviorData->bCanInitiateTransformation)
    {
        bIsTransforming = true;
        TransformationTimer = 0.0f;
        OnTransformationTriggered();
        
        // Trigger consciousness state evolution
        switch (CurrentConsciousnessState)
        {
            case EConsciousnessState::Ego:
                UpdateConsciousnessState(EConsciousnessState::Shadow);
                break;
            case EConsciousnessState::Shadow:
                UpdateConsciousnessState(EConsciousnessState::Anima);
                break;
            case EConsciousnessState::Anima:
                UpdateConsciousnessState(EConsciousnessState::Wise);
                break;
            case EConsciousnessState::Wise:
                UpdateConsciousnessState(EConsciousnessState::Transcendent);
                break;
            default:
                break;
        }
    }
}

void UNPCBehaviorComponent::RespondToPlayerConsciousness(float PlayerConsciousnessLevel)
{
    if (!CurrentBehaviorData) return;
    
    float ResonanceStrength = CalculateEmotionalResonance(nullptr);
    
    // NPCs respond differently based on their archetype
    switch (NPCArchetype)
    {
        case ENPCArchetype::ShadowSelf:
            // Shadow becomes more active when player consciousness is low
            EmotionalState = FMath::Lerp(EmotionalState, 1.0f - PlayerConsciousnessLevel, 0.1f);
            break;
            
        case ENPCArchetype::WiseGuide:
            // Wise Guide becomes more present when player is ready
            EmotionalState = FMath::Lerp(EmotionalState, PlayerConsciousnessLevel, 0.1f);
            break;
            
        case ENPCArchetype::InnerChild:
            // Inner Child responds to emotional openness
            EmotionalState = FMath::Lerp(EmotionalState, PlayerConsciousnessLevel * 0.8f, 0.15f);
            break;
            
        default:
            EmotionalState = FMath::Lerp(EmotionalState, PlayerConsciousnessLevel, 0.05f);
            break;
    }
    
    OnEmotionalResonance(ResonanceStrength);
    
    // Trigger transformation if conditions are met
    if (PlayerConsciousnessLevel > 0.7f && CurrentBehaviorData->bCanInitiateTransformation)
    {
        TriggerTransformation();
    }
}

void UNPCBehaviorComponent::UpdateBehaviorBasedOnConsciousness()
{
    if (!CurrentBehaviorData) return;
    
    // Update emotional state based on consciousness level
    float TargetEmotion = ConsciousnessLevel * CurrentBehaviorData->EmotionalResonance;
    EmotionalState = FMath::FInterpTo(EmotionalState, TargetEmotion, GetWorld()->GetDeltaSeconds(), 2.0f);
    
    // Dynamic behavior changes based on consciousness state
    if (ConsciousnessLevel > 0.8f && CurrentConsciousnessState != EConsciousnessState::Transcendent)
    {
        UpdateConsciousnessState(EConsciousnessState::Transcendent);
    }
    else if (ConsciousnessLevel > 0.6f && CurrentConsciousnessState == EConsciousnessState::Shadow)
    {
        UpdateConsciousnessState(EConsciousnessState::Anima);
    }
}

void UNPCBehaviorComponent::CheckPlayerProximity()
{
    if (!CurrentBehaviorData) return;
    
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!Player) return;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    
    if (Distance <= CurrentBehaviorData->ConsciousnessInfluenceRadius)
    {
        // Player is within influence range - calculate resonance
        float ResonanceLevel = CalculateEmotionalResonance(Player);
        
        // Influence player's consciousness based on NPC's state
        // This would interface with the player's consciousness system
        OnEmotionalResonance(ResonanceLevel);
    }
}

float UNPCBehaviorComponent::CalculateEmotionalResonance(ACharacter* Player)
{
    if (!CurrentBehaviorData || !Player) return 0.0f;
    
    float BaseResonance = CurrentBehaviorData->EmotionalResonance;
    float ConsciousnessModifier = ConsciousnessLevel;
    float EmotionalModifier = EmotionalState;
    
    // Calculate resonance based on NPC archetype and current state
    float Resonance = BaseResonance * ConsciousnessModifier * EmotionalModifier;
    
    // Apply archetype-specific modifiers
    switch (NPCArchetype)
    {
        case ENPCArchetype::ShadowSelf:
            Resonance *= (bIsTransforming ? 2.0f : 1.0f);
            break;
        case ENPCArchetype::WiseGuide:
            Resonance *= (CurrentConsciousnessState == EConsciousnessState::Wise ? 1.5f : 1.0f);
            break;
        case ENPCArchetype::InnerChild:
            Resonance *= (EmotionalState > 0.7f ? 1.3f : 0.8f);
            break;
        default:
            break;
    }
    
    return FMath::Clamp(Resonance, 0.0f, 1.0f);
}