// SpiritualNPCBehavior.cpp
// Implementação do sistema de comportamento para NPCs espirituais

#include "SpiritualNPCBehavior.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

USpiritualNPCBehavior::USpiritualNPCBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default values
    NPCType = ESpiritualNPCType::Guide;
    CurrentState = ESpiritualNPCState::Dormant;
    ConsciousnessResonance = 0.5f;
    AwarenessRadius = 500.0f;
    bCanTranscend = true;
    
    StateTransitionTimer = 0.0f;
    BehaviorTimer = 0.0f;
    bPlayerInRange = false;
    LastPlayerState = EConsciousnessState::Ordinary;
    
    // Initialize default behavior pattern
    FSpiritualBehaviorPattern DefaultBehavior;
    DefaultBehavior.BehaviorName = "Contemplative Idle";
    DefaultBehavior.Duration = 10.0f;
    DefaultBehavior.bLooping = true;
    BehaviorPatterns.Add(DefaultBehavior);
    CurrentBehavior = DefaultBehavior;
}

void USpiritualNPCBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player consciousness system
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        PlayerConsciousness = PlayerPawn->FindComponentByClass<UConsciousnessSystem>();
    }
    
    // Initialize dialogue based on NPC type
    InitializeDialogueSystem();
}

void USpiritualNPCBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CheckPlayerProximity();
    ProcessConsciousnessResonance();
    UpdateBehaviorPattern(DeltaTime);
    
    StateTransitionTimer += DeltaTime;
}

void USpiritualNPCBehavior::UpdateNPCState(EConsciousnessState PlayerState)
{
    ESpiritualNPCState OptimalState = CalculateOptimalState(PlayerState);
    
    if (OptimalState != CurrentState)
    {
        ESpiritualNPCState PreviousState = CurrentState;
        CurrentState = OptimalState;
        
        // Trigger state change event
        OnStateChanged(CurrentState);
        
        // Update behavior pattern for new state
        ExecuteBehaviorPattern(CurrentState);
        
        UE_LOG(LogTemp, Log, TEXT("Spiritual NPC state changed from %d to %d"), 
               (int32)PreviousState, (int32)CurrentState);
    }
}

void USpiritualNPCBehavior::RespondToPlayerConsciousness(float PlayerConsciousnessLevel)
{
    // Calculate resonance based on consciousness alignment
    float ResonanceStrength = FMath::Abs(ConsciousnessResonance - PlayerConsciousnessLevel);
    ResonanceStrength = 1.0f - FMath::Clamp(ResonanceStrength, 0.0f, 1.0f);
    
    // Trigger resonance event
    OnSpiritualResonance(ResonanceStrength);
    
    // Adjust NPC behavior based on resonance
    if (ResonanceStrength > 0.7f)
    {
        // High resonance - NPC becomes more active and helpful
        if (CurrentState == ESpiritualNPCState::Dormant)
        {
            CurrentState = ESpiritualNPCState::Awakening;
        }
        else if (CurrentState == ESpiritualNPCState::Aware && bCanTranscend)
        {
            CurrentState = ESpiritualNPCState::Transcendent;
        }
    }
    else if (ResonanceStrength < 0.3f)
    {
        // Low resonance - NPC becomes more withdrawn
        if (CurrentState == ESpiritualNPCState::Transcendent)
        {
            CurrentState = ESpiritualNPCState::Aware;
        }
    }
}

FString USpiritualNPCBehavior::GetContextualDialogue(EConsciousnessState PlayerState)
{
    // Find appropriate dialogue for current states
    for (const FSpiritualDialogue& Dialogue : DialogueOptions)
    {
        if (Dialogue.RequiredPlayerState == PlayerState && 
            Dialogue.RequiredNPCState == CurrentState)
        {
            CurrentDialogue = Dialogue.DialogueText;
            return CurrentDialogue;
        }
    }
    
    // Fallback to generic dialogue based on NPC type
    switch (NPCType)
    {
        case ESpiritualNPCType::Guide:
            return "The path of awakening requires both courage and surrender.";
        case ESpiritualNPCType::Guardian:
            return "I sense the stirring of consciousness within you.";
        case ESpiritualNPCType::Seeker:
            return "We walk this path together, fellow traveler.";
        case ESpiritualNPCType::Shadow:
            return "Face what you fear to become whole.";
        case ESpiritualNPCType::Archetype:
            return "I am the eternal pattern that guides your becoming.";
        default:
            return "...";
    }
}

void USpiritualNPCBehavior::TriggerSpiritualInteraction()
{
    if (!PlayerConsciousness)
        return;
        
    // Get current player consciousness state
    EConsciousnessState PlayerState = PlayerConsciousness->GetCurrentState();
    
    // Update NPC state based on interaction
    UpdateNPCState(PlayerState);
    
    // Get contextual dialogue
    FString InteractionDialogue = GetContextualDialogue(PlayerState);
    
    // Apply consciousness impact if dialogue found
    for (const FSpiritualDialogue& Dialogue : DialogueOptions)
    {
        if (Dialogue.DialogueText == InteractionDialogue)
        {
            PlayerConsciousness->ModifyConsciousness(Dialogue.ConsciousnessImpact);
            break;
        }
    }
    
    // Trigger specific behaviors based on NPC type
    switch (NPCType)
    {
        case ESpiritualNPCType::Guide:
            GuidePlayerConsciousness(EConsciousnessState::Awakened);
            break;
        case ESpiritualNPCType::Guardian:
            ChannelArchetypalEnergy();
            break;
        case ESpiritualNPCType::Archetype:
            ManifestSpiritualVision();
            break;
    }
}

void USpiritualNPCBehavior::ExecuteBehaviorPattern(ESpiritualNPCState TargetState)
{
    // Find behavior pattern for target state
    for (const FSpiritualBehaviorPattern& Pattern : BehaviorPatterns)
    {
        if (Pattern.TriggerState == TargetState)
        {
            CurrentBehavior = Pattern;
            BehaviorTimer = 0.0f;
            
            UE_LOG(LogTemp, Log, TEXT("Executing behavior pattern: %s"), 
                   *Pattern.BehaviorName);
            break;
        }
    }
}

void USpiritualNPCBehavior::EnterMeditativeState()
{
    CurrentState = ESpiritualNPCState::Transcendent;
    
    // Create meditative behavior pattern
    FSpiritualBehaviorPattern MeditativeBehavior;
    MeditativeBehavior.BehaviorName = "Deep Meditation";
    MeditativeBehavior.Duration = 30.0f;
    MeditativeBehavior.bLooping = true;
    
    CurrentBehavior = MeditativeBehavior;
    BehaviorTimer = 0.0f;
    
    // Increase consciousness resonance during meditation
    ConsciousnessResonance = FMath::Min(ConsciousnessResonance + 0.2f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("NPC entered meditative state"));
}

void USpiritualNPCBehavior::ChannelArchetypalEnergy()
{
    if (NPCType == ESpiritualNPCType::Guardian || NPCType == ESpiritualNPCType::Archetype)
    {
        // Temporarily boost consciousness resonance
        float OriginalResonance = ConsciousnessResonance;
        ConsciousnessResonance = 1.0f;
        
        // Apply powerful consciousness boost to player
        if (PlayerConsciousness)
        {
            PlayerConsciousness->ModifyConsciousness(0.3f);
        }
        
        // Schedule resonance restoration
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this, OriginalResonance]()
            {
                ConsciousnessResonance = OriginalResonance;
            },
            10.0f,
            false
        );
        
        UE_LOG(LogTemp, Log, TEXT("Archetypal energy channeled"));
    }
}

void USpiritualNPCBehavior::GuidePlayerConsciousness(EConsciousnessState TargetState)
{
    if (NPCType == ESpiritualNPCType::Guide && PlayerConsciousness)
    {
        EConsciousnessState CurrentPlayerState = PlayerConsciousness->GetCurrentState();
        
        // Calculate guidance strength based on state difference
        int32 StateDifference = (int32)TargetState - (int32)CurrentPlayerState;
        float GuidanceStrength = FMath::Clamp(StateDifference * 0.1f, 0.05f, 0.25f);
        
        // Apply gradual consciousness shift
        PlayerConsciousness->ModifyConsciousness(GuidanceStrength);
        
        UE_LOG(LogTemp, Log, TEXT("Guiding player consciousness towards state %d"), 
               (int32)TargetState);
    }
}

void USpiritualNPCBehavior::ManifestSpiritualVision()
{
    if (NPCType == ESpiritualNPCType::Archetype)
    {
        CurrentState = ESpiritualNPCState::Unity;
        
        // Create vision manifestation behavior
        FSpiritualBehaviorPattern VisionBehavior;
        VisionBehavior.BehaviorName = "Archetypal Manifestation";
        VisionBehavior.Duration = 15.0f;
        VisionBehavior.bLooping = false;
        
        CurrentBehavior = VisionBehavior;
        BehaviorTimer = 0.0f;
        
        // Trigger major consciousness expansion
        if (PlayerConsciousness)
        {
            PlayerConsciousness->ModifyConsciousness(0.5f);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Spiritual vision manifested"));
    }
}

void USpiritualNPCBehavior::CheckPlayerProximity()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
        
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                   PlayerPawn->GetActorLocation());
    
    bool bWasInRange = bPlayerInRange;
    bPlayerInRange = Distance <= AwarenessRadius;
    
    // Player entered awareness range
    if (bPlayerInRange && !bWasInRange)
    {
        if (PlayerConsciousness)
        {
            EConsciousnessState PlayerState = PlayerConsciousness->GetCurrentState();
            OnPlayerConsciousnessDetected(PlayerState);
            
            // Wake up if dormant
            if (CurrentState == ESpiritualNPCState::Dormant)
            {
                CurrentState = ESpiritualNPCState::Awakening;
            }
        }
    }
}

void USpiritualNPCBehavior::ProcessConsciousnessResonance()
{
    if (!bPlayerInRange || !PlayerConsciousness)
        return;
        
    EConsciousnessState CurrentPlayerState = PlayerConsciousness->GetCurrentState();
    
    // Check if player state changed
    if (CurrentPlayerState != LastPlayerState)
    {
        UpdateNPCState(CurrentPlayerState);
        LastPlayerState = CurrentPlayerState;
    }
    
    // Apply continuous resonance effect
    float PlayerConsciousnessLevel = PlayerConsciousness->GetConsciousnessLevel();
    RespondToPlayerConsciousness(PlayerConsciousnessLevel);
}

void USpiritualNPCBehavior::UpdateBehaviorPattern(float DeltaTime)
{
    BehaviorTimer += DeltaTime;
    
    // Check if current behavior should end
    if (!CurrentBehavior.bLooping && BehaviorTimer >= CurrentBehavior.Duration)
    {
        // Return to default state behavior
        ExecuteBehaviorPattern(CurrentState);
    }
    else if (CurrentBehavior.bLooping && BehaviorTimer >= CurrentBehavior.Duration)
    {
        // Reset looping behavior
        BehaviorTimer = 0.0f;
    }
}

ESpiritualNPCState USpiritualNPCBehavior::CalculateOptimalState(EConsciousnessState PlayerState)
{
    // Calculate optimal NPC state based on player consciousness and NPC type
    switch (NPCType)
    {
        case ESpiritualNPCType::Guide:
            // Guides stay slightly ahead of player consciousness
            switch (PlayerState)
            {
                case EConsciousnessState::Ordinary:
                    return ESpiritualNPCState::Aware;
                case EConsciousnessState::Awakened:
                    return ESpiritualNPCState::Transcendent;
                case EConsciousnessState::Expanded:
                    return bCanTranscend ? ESpiritualNPCState::Unity : ESpiritualNPCState::Transcendent;
                default:
                    return ESpiritualNPCState::Aware;
            }
            
        case ESpiritualNPCType::Guardian:
            // Guardians match player consciousness level
            switch (PlayerState)
            {
                case EConsciousnessState::Ordinary:
                    return ESpiritualNPCState::Dormant;
                case EConsciousnessState::Awakened:
                    return ESpiritualNPCState::Aware;
                case EConsciousnessState::Expanded:
                    return ESpiritualNPCState::Transcendent;
                default:
                    return ESpiritualNPCState::Dormant;
            }
            
        case ESpiritualNPCType::Seeker:
            // Seekers evolve with player
            return (ESpiritualNPCState)FMath::Min((int32)PlayerState + 1, 
                                                  (int32)ESpiritualNPCState::Unity);
            
        case ESpiritualNPCType::Shadow:
            // Shadows appear when player consciousness is low
            return PlayerState == EConsciousnessState::Ordinary ? 
                   ESpiritualNPCState::Aware : ESpiritualNPCState::Dormant;
            
        case ESpiritualNPCType::Archetype:
            // Archetypes maintain high consciousness
            return bCanTranscend ? ESpiritualNPCState::Unity : ESpiritualNPCState::Transcendent;
            
        default:
            return ESpiritualNPCState::Dormant;
    }
}

void USpiritualNPCBehavior::InitializeDialogueSystem()
{
    DialogueOptions.Empty();
    
    // Add type-specific dialogues
    switch (NPCType)
    {
        case ESpiritualNPCType::Guide:
            {
                FSpiritualDialogue GuideDialogue1;
                GuideDialogue1.DialogueText = "Welcome, seeker. Your journey into consciousness begins with a single step inward.";
                GuideDialogue1.RequiredPlayerState = EConsciousnessState::Ordinary;
                GuideDialogue1.RequiredNPCState = ESpiritualNPCState::Aware;
                GuideDialogue1.ConsciousnessImpact = 0.1f;
                DialogueOptions.Add(GuideDialogue1);
                
                FSpiritualDialogue GuideDialogue2;
                GuideDialogue2.DialogueText = "I see the light of awareness growing within you. Trust the process.";
                GuideDialogue2.RequiredPlayerState = EConsciousnessState::Awakened;
                GuideDialogue2.RequiredNPCState = ESpiritualNPCState::Transcendent;
                GuideDialogue2.ConsciousnessImpact = 0.15f;
                DialogueOptions.Add(GuideDialogue2);
            }
            break;
            
        case ESpiritualNPCType::Guardian:
            {
                FSpiritualDialogue GuardianDialogue;
                GuardianDialogue.DialogueText = "I am the guardian of this sacred space. Your consciousness determines what you may perceive here.";
                GuardianDialogue.RequiredPlayerState = EConsciousnessState::Awakened;
                GuardianDialogue.RequiredNPCState = ESpiritualNPCState::Aware;
                GuardianDialogue.ConsciousnessImpact = 0.05f;
                DialogueOptions.Add(GuardianDialogue);
            }
            break;
            
        case ESpiritualNPCType::Archetype:
            {
                FSpiritualDialogue ArchetypeDialogue;
                ArchetypeDialogue.DialogueText = "I am the eternal pattern within you. In recognizing me, you recognize yourself.";
                ArchetypeDialogue.RequiredPlayerState = EConsciousnessState::Expanded;
                ArchetypeDialogue.RequiredNPCState = ESpiritualNPCState::Unity;
                ArchetypeDialogue.ConsciousnessImpact = 0.25f;
                DialogueOptions.Add(ArchetypeDialogue);
            }
            break;
    }
}