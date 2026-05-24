#include "Narr_VoiceActedCharacterSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogVoiceActedCharacter, Log, All);

UNarr_VoiceActedCharacterSystem::UNarr_VoiceActedCharacterSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;
    
    // Initialize voice system parameters
    MaxVoiceRange = 2000.0f;
    VoiceVolume = 1.0f;
    VoicePitch = 1.0f;
    bAutoPlayOnTrigger = true;
    bInterruptPreviousVoice = true;
    VoiceCooldownTime = 2.0f;
    
    // Character voice settings
    CharacterVoiceType = ENarr_CharacterVoiceType::Narrator;
    EmotionalState = ENarr_EmotionalState::Neutral;
    VoiceIntensity = 0.5f;
    
    // Internal state
    bIsPlayingVoice = false;
    bCanPlayVoice = true;
    CurrentVoicelineIndex = 0;
    LastVoicePlayTime = 0.0f;
}

void UNarr_VoiceActedCharacterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Create audio component for voice playback
    if (!VoiceAudioComponent && GetOwner())
    {
        VoiceAudioComponent = NewObject<UAudioComponent>(GetOwner());
        if (VoiceAudioComponent)
        {
            VoiceAudioComponent->AttachToComponent(GetOwner()->GetRootComponent(), 
                FAttachmentTransformRules::KeepWorldTransform);
            VoiceAudioComponent->SetVolumeMultiplier(VoiceVolume);
            VoiceAudioComponent->SetPitchMultiplier(VoicePitch);
            VoiceAudioComponent->bAutoActivate = false;
            
            UE_LOG(LogVoiceActedCharacter, Log, TEXT("Voice audio component created for %s"), 
                *GetOwner()->GetName());
        }
    }
    
    // Initialize voice library based on character type
    InitializeVoiceLibrary();
}

void UNarr_VoiceActedCharacterSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update voice cooldown
    if (!bCanPlayVoice)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastVoicePlayTime >= VoiceCooldownTime)
        {
            bCanPlayVoice = true;
        }
    }
    
    // Check if voice is still playing
    if (bIsPlayingVoice && VoiceAudioComponent && !VoiceAudioComponent->IsPlaying())
    {
        OnVoicelineFinished();
    }
}

void UNarr_VoiceActedCharacterSystem::InitializeVoiceLibrary()
{
    // Clear existing library
    VoiceLibrary.Empty();
    
    // Add voice lines based on character type
    switch (CharacterVoiceType)
    {
        case ENarr_CharacterVoiceType::Narrator:
            AddNarratorVoices();
            break;
            
        case ENarr_CharacterVoiceType::Scout:
            AddScoutVoices();
            break;
            
        case ENarr_CharacterVoiceType::TribalElder:
            AddTribalElderVoices();
            break;
            
        case ENarr_CharacterVoiceType::EmergencyAlert:
            AddEmergencyVoices();
            break;
            
        default:
            AddGenericVoices();
            break;
    }
    
    UE_LOG(LogVoiceActedCharacter, Log, TEXT("Voice library initialized with %d voice lines"), 
        VoiceLibrary.Num());
}

void UNarr_VoiceActedCharacterSystem::AddNarratorVoices()
{
    FNarr_VoiceLine VoiceLine;
    
    // Survival observations
    VoiceLine.VoicelineID = "narrator_survival_01";
    VoiceLine.DialogueText = "Day 236 in the Cretaceous wilderness. The morning reveals disturbing signs.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Tense;
    VoiceLine.Priority = ENarr_VoicePriority::Normal;
    VoiceLine.TriggerConditions.Add("morning");
    VoiceLine.TriggerConditions.Add("tracks_found");
    VoiceLibrary.Add(VoiceLine);
    
    // Environmental awareness
    VoiceLine.VoicelineID = "narrator_environment_01";
    VoiceLine.DialogueText = "The ancient forest holds secrets of survival written in every shadow.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Contemplative;
    VoiceLine.Priority = ENarr_VoicePriority::Low;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("forest_enter");
    VoiceLibrary.Add(VoiceLine);
    
    // Danger awareness
    VoiceLine.VoicelineID = "narrator_danger_01";
    VoiceLine.DialogueText = "Something moves in the undergrowth. Large. Predatory. Watching.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Fear;
    VoiceLine.Priority = ENarr_VoicePriority::High;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("predator_nearby");
    VoiceLibrary.Add(VoiceLine);
}

void UNarr_VoiceActedCharacterSystem::AddScoutVoices()
{
    FNarr_VoiceLine VoiceLine;
    
    // Resource discovery
    VoiceLine.VoicelineID = "scout_water_01";
    VoiceLine.DialogueText = "Fresh water source located downstream, approximately two kilometers southeast.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Focused;
    VoiceLine.Priority = ENarr_VoicePriority::High;
    VoiceLine.TriggerConditions.Add("water_found");
    VoiceLibrary.Add(VoiceLine);
    
    // Threat assessment
    VoiceLine.VoicelineID = "scout_threat_01";
    VoiceLine.DialogueText = "Large crocodilian tracks detected along the banks. Approach with extreme caution.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Alert;
    VoiceLine.Priority = ENarr_VoicePriority::High;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("danger_spotted");
    VoiceLibrary.Add(VoiceLine);
    
    // Navigation
    VoiceLine.VoicelineID = "scout_navigation_01";
    VoiceLine.DialogueText = "Path ahead is clear, but watch for loose rocks on the eastern slope.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Neutral;
    VoiceLine.Priority = ENarr_VoicePriority::Normal;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("path_clear");
    VoiceLibrary.Add(VoiceLine);
}

void UNarr_VoiceActedCharacterSystem::AddTribalElderVoices()
{
    FNarr_VoiceLine VoiceLine;
    
    // Wisdom sharing
    VoiceLine.VoicelineID = "elder_wisdom_01";
    VoiceLine.DialogueText = "The ancient forest whispers secrets of survival to those who listen.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Wise;
    VoiceLine.Priority = ENarr_VoicePriority::Normal;
    VoiceLine.TriggerConditions.Add("teaching_moment");
    VoiceLibrary.Add(VoiceLine);
    
    // Behavioral lessons
    VoiceLine.VoicelineID = "elder_behavior_01";
    VoiceLine.DialogueText = "Watch how the Triceratops herd moves - young in center, adults protecting.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Teaching;
    VoiceLine.Priority = ENarr_VoicePriority::Normal;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("herd_observed");
    VoiceLibrary.Add(VoiceLine);
    
    // Survival advice
    VoiceLine.VoicelineID = "elder_survival_01";
    VoiceLine.DialogueText = "Never travel alone. The predators are always watching for the weak and isolated.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Serious;
    VoiceLine.Priority = ENarr_VoicePriority::High;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("solo_warning");
    VoiceLibrary.Add(VoiceLine);
}

void UNarr_VoiceActedCharacterSystem::AddEmergencyVoices()
{
    FNarr_VoiceLine VoiceLine;
    
    // Critical threat
    VoiceLine.VoicelineID = "emergency_threat_01";
    VoiceLine.DialogueText = "Critical threat assessment! Massive Carnotaurus pack detected approaching!";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Panic;
    VoiceLine.Priority = ENarr_VoicePriority::Critical;
    VoiceLine.TriggerConditions.Add("pack_threat");
    VoiceLibrary.Add(VoiceLine);
    
    // Evacuation protocol
    VoiceLine.VoicelineID = "emergency_evacuate_01";
    VoiceLine.DialogueText = "Immediate evacuation protocol initiated. Grab essential supplies only!";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Urgent;
    VoiceLine.Priority = ENarr_VoicePriority::Critical;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("evacuate");
    VoiceLibrary.Add(VoiceLine);
    
    // Escape route
    VoiceLine.VoicelineID = "emergency_escape_01";
    VoiceLine.DialogueText = "Move to the river crossing! They won't follow through deep water!";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Desperate;
    VoiceLine.Priority = ENarr_VoicePriority::Critical;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("escape_route");
    VoiceLibrary.Add(VoiceLine);
}

void UNarr_VoiceActedCharacterSystem::AddGenericVoices()
{
    FNarr_VoiceLine VoiceLine;
    
    // Generic acknowledgment
    VoiceLine.VoicelineID = "generic_acknowledge_01";
    VoiceLine.DialogueText = "Understood. Proceeding with caution.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Neutral;
    VoiceLine.Priority = ENarr_VoicePriority::Low;
    VoiceLine.TriggerConditions.Add("acknowledge");
    VoiceLibrary.Add(VoiceLine);
    
    // Generic warning
    VoiceLine.VoicelineID = "generic_warning_01";
    VoiceLine.DialogueText = "Something doesn't feel right. Stay alert.";
    VoiceLine.EmotionalTone = ENarr_EmotionalState::Cautious;
    VoiceLine.Priority = ENarr_VoicePriority::Normal;
    VoiceLine.TriggerConditions.Empty();
    VoiceLine.TriggerConditions.Add("general_warning");
    VoiceLibrary.Add(VoiceLine);
}

bool UNarr_VoiceActedCharacterSystem::PlayVoiceLine(const FString& VoicelineID, bool bForcePlay)
{
    if (!bCanPlayVoice && !bForcePlay)
    {
        UE_LOG(LogVoiceActedCharacter, Warning, TEXT("Voice system on cooldown, cannot play: %s"), 
            *VoicelineID);
        return false;
    }
    
    // Find the voice line
    FNarr_VoiceLine* FoundVoiceLine = VoiceLibrary.FindByPredicate(
        [VoicelineID](const FNarr_VoiceLine& VoiceLine)
        {
            return VoiceLine.VoicelineID == VoicelineID;
        });
    
    if (!FoundVoiceLine)
    {
        UE_LOG(LogVoiceActedCharacter, Warning, TEXT("Voice line not found: %s"), *VoicelineID);
        return false;
    }
    
    return PlayVoiceLine(*FoundVoiceLine, bForcePlay);
}

bool UNarr_VoiceActedCharacterSystem::PlayVoiceLine(const FNarr_VoiceLine& VoiceLine, bool bForcePlay)
{
    if (!VoiceAudioComponent)
    {
        UE_LOG(LogVoiceActedCharacter, Error, TEXT("No voice audio component available"));
        return false;
    }
    
    if (!bCanPlayVoice && !bForcePlay)
    {
        return false;
    }
    
    // Stop current voice if interruption is allowed
    if (bIsPlayingVoice && bInterruptPreviousVoice)
    {
        StopCurrentVoice();
    }
    else if (bIsPlayingVoice)
    {
        // Queue the voice line for later
        QueuedVoiceLines.Add(VoiceLine);
        return true;
    }
    
    // Update emotional state and voice parameters
    EmotionalState = VoiceLine.EmotionalTone;
    UpdateVoiceParameters();
    
    // For now, we'll use text-to-speech or log the dialogue
    // In a full implementation, this would load and play the actual audio asset
    UE_LOG(LogVoiceActedCharacter, Log, TEXT("Playing voice line [%s]: %s"), 
        *VoiceLine.VoicelineID, *VoiceLine.DialogueText);
    
    // Simulate voice playback
    bIsPlayingVoice = true;
    bCanPlayVoice = false;
    LastVoicePlayTime = GetWorld()->GetTimeSeconds();
    CurrentVoiceLine = VoiceLine;
    
    // Set a timer to simulate voice duration (estimate based on text length)
    float EstimatedDuration = FMath::Max(2.0f, VoiceLine.DialogueText.Len() * 0.1f);
    GetWorld()->GetTimerManager().SetTimer(VoicePlaybackTimer, this, 
        &UNarr_VoiceActedCharacterSystem::OnVoicelineFinished, EstimatedDuration, false);
    
    // Broadcast voice started event
    OnVoiceLineStarted.Broadcast(VoiceLine);
    
    return true;
}

bool UNarr_VoiceActedCharacterSystem::PlayRandomVoiceLine(const TArray<FString>& TriggerConditions)
{
    // Find voice lines that match the trigger conditions
    TArray<FNarr_VoiceLine> MatchingVoiceLines;
    
    for (const FNarr_VoiceLine& VoiceLine : VoiceLibrary)
    {
        bool bMatches = false;
        
        if (TriggerConditions.Num() == 0)
        {
            // If no conditions specified, all voice lines match
            bMatches = true;
        }
        else
        {
            // Check if any trigger condition matches
            for (const FString& Condition : TriggerConditions)
            {
                if (VoiceLine.TriggerConditions.Contains(Condition))
                {
                    bMatches = true;
                    break;
                }
            }
        }
        
        if (bMatches)
        {
            MatchingVoiceLines.Add(VoiceLine);
        }
    }
    
    if (MatchingVoiceLines.Num() == 0)
    {
        UE_LOG(LogVoiceActedCharacter, Warning, TEXT("No matching voice lines found for conditions"));
        return false;
    }
    
    // Select random voice line
    int32 RandomIndex = FMath::RandRange(0, MatchingVoiceLines.Num() - 1);
    return PlayVoiceLine(MatchingVoiceLines[RandomIndex]);
}

void UNarr_VoiceActedCharacterSystem::StopCurrentVoice()
{
    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }
    
    if (bIsPlayingVoice)
    {
        bIsPlayingVoice = false;
        GetWorld()->GetTimerManager().ClearTimer(VoicePlaybackTimer);
        
        // Broadcast voice stopped event
        OnVoiceLineStopped.Broadcast(CurrentVoiceLine);
    }
}

void UNarr_VoiceActedCharacterSystem::SetVoiceParameters(float NewVolume, float NewPitch, float NewRange)
{
    VoiceVolume = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    VoicePitch = FMath::Clamp(NewPitch, 0.5f, 2.0f);
    MaxVoiceRange = FMath::Max(100.0f, NewRange);
    
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->SetVolumeMultiplier(VoiceVolume);
        VoiceAudioComponent->SetPitchMultiplier(VoicePitch);
    }
}

void UNarr_VoiceActedCharacterSystem::UpdateVoiceParameters()
{
    if (!VoiceAudioComponent)
        return;
    
    // Adjust voice parameters based on emotional state
    float EmotionalVolumeModifier = 1.0f;
    float EmotionalPitchModifier = 1.0f;
    
    switch (EmotionalState)
    {
        case ENarr_EmotionalState::Fear:
        case ENarr_EmotionalState::Panic:
            EmotionalVolumeModifier = 1.2f;
            EmotionalPitchModifier = 1.1f;
            break;
            
        case ENarr_EmotionalState::Urgent:
        case ENarr_EmotionalState::Alert:
            EmotionalVolumeModifier = 1.1f;
            EmotionalPitchModifier = 1.05f;
            break;
            
        case ENarr_EmotionalState::Wise:
        case ENarr_EmotionalState::Contemplative:
            EmotionalVolumeModifier = 0.9f;
            EmotionalPitchModifier = 0.95f;
            break;
            
        case ENarr_EmotionalState::Desperate:
            EmotionalVolumeModifier = 1.3f;
            EmotionalPitchModifier = 1.15f;
            break;
            
        default:
            // Neutral state - no modification
            break;
    }
    
    // Apply intensity scaling
    EmotionalVolumeModifier = FMath::Lerp(1.0f, EmotionalVolumeModifier, VoiceIntensity);
    EmotionalPitchModifier = FMath::Lerp(1.0f, EmotionalPitchModifier, VoiceIntensity);
    
    // Update audio component
    VoiceAudioComponent->SetVolumeMultiplier(VoiceVolume * EmotionalVolumeModifier);
    VoiceAudioComponent->SetPitchMultiplier(VoicePitch * EmotionalPitchModifier);
}

void UNarr_VoiceActedCharacterSystem::OnVoicelineFinished()
{
    bIsPlayingVoice = false;
    
    // Broadcast completion event
    OnVoiceLineCompleted.Broadcast(CurrentVoiceLine);
    
    // Process queued voice lines
    if (QueuedVoiceLines.Num() > 0)
    {
        FNarr_VoiceLine NextVoiceLine = QueuedVoiceLines[0];
        QueuedVoiceLines.RemoveAt(0);
        
        // Small delay before next voice line
        GetWorld()->GetTimerManager().SetTimer(VoiceQueueTimer, 
            FTimerDelegate::CreateLambda([this, NextVoiceLine]()
            {
                PlayVoiceLine(NextVoiceLine);
            }), 0.5f, false);
    }
}

TArray<FNarr_VoiceLine> UNarr_VoiceActedCharacterSystem::GetVoiceLinesForConditions(const TArray<FString>& Conditions) const
{
    TArray<FNarr_VoiceLine> MatchingLines;
    
    for (const FNarr_VoiceLine& VoiceLine : VoiceLibrary)
    {
        for (const FString& Condition : Conditions)
        {
            if (VoiceLine.TriggerConditions.Contains(Condition))
            {
                MatchingLines.Add(VoiceLine);
                break;
            }
        }
    }
    
    return MatchingLines;
}

bool UNarr_VoiceActedCharacterSystem::IsVoiceSystemReady() const
{
    return VoiceAudioComponent != nullptr && VoiceLibrary.Num() > 0;
}

float UNarr_VoiceActedCharacterSystem::GetVoiceSystemStatus() const
{
    if (!IsVoiceSystemReady())
        return 0.0f;
    
    if (bIsPlayingVoice)
        return 1.0f;
    
    if (!bCanPlayVoice)
        return 0.5f;
    
    return 0.75f; // Ready to play
}