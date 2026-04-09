// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "DialogueComponent.h"
#include "NarrativeManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/DialogueWave.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

UDialogueComponent::UDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bIsPlayingDialogue = false;
    CurrentEmotionalTone = EEmotionalTone::Wonder;
    EmotionalIntensity = 1.0f;
    DialogueRange = 1000.0f;
    bAutoFaceListener = true;
    bUseSubtitles = true;
    DefaultEmotionalTone = EEmotionalTone::Wonder;
}

void UDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    SetupAudioComponent();
    CurrentEmotionalTone = DefaultEmotionalTone;
}

void UDialogueComponent::SetupAudioComponent()
{
    // Create audio component if it doesn't exist
    if (!AudioComponent)
    {
        AudioComponent = NewObject<UAudioComponent>(GetOwner());
        if (AudioComponent)
        {
            AudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
            AudioComponent->bAutoActivate = false;
            AudioComponent->SetVolumeMultiplier(1.0f);
            AudioComponent->SetPitchMultiplier(1.0f);
            
            // Set 3D audio properties for immersive dialogue
            AudioComponent->bAllowSpatialization = true;
            AudioComponent->bOverrideAttenuation = true;
            
            UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Audio component created for %s"), 
                   *GetOwner()->GetName());
        }
    }
}

void UDialogueComponent::PlayDialogue(const FString& LineID, AActor* Listener)
{
    if (bIsPlayingDialogue)
    {
        StopCurrentDialogue();
    }
    
    UNarrativeManager* NarrativeManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeManager>();
    if (!NarrativeManager)
    {
        UE_LOG(LogTemp, Error, TEXT("DialogueComponent: NarrativeManager not found"));
        return;
    }
    
    // Check if dialogue can be triggered
    if (!NarrativeManager->CanTriggerDialogue(LineID, GetOwner(), Listener))
    {
        UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Dialogue %s cannot be triggered"), *LineID);
        return;
    }
    
    // Play dialogue through narrative manager
    NarrativeManager->PlayDialogue(LineID, GetOwner(), Listener);
    
    // Set playing state
    bIsPlayingDialogue = true;
    
    // Auto-face listener if enabled
    if (bAutoFaceListener && Listener)
    {
        FVector DirectionToListener = (Listener->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = DirectionToListener.Rotation();
        NewRotation.Pitch = 0.0f; // Keep level
        GetOwner()->SetActorRotation(NewRotation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Playing dialogue %s from %s"), 
           *LineID, *GetOwner()->GetName());
}

void UDialogueComponent::PlayDialogueByContext(EDialogueContext Context, AActor* Listener)
{
    UNarrativeManager* NarrativeManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarrativeManager>();
    if (!NarrativeManager)
    {
        return;
    }
    
    // Get appropriate dialogue for context
    FDialogueLine ContextDialogue = NarrativeManager->GetDialogueForContext(CharacterID, Context);
    if (!ContextDialogue.LineID.IsEmpty())
    {
        PlayDialogue(ContextDialogue.LineID, Listener);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueComponent: No dialogue found for context %d"), (int32)Context);
    }
}

void UDialogueComponent::StopCurrentDialogue()
{
    if (!bIsPlayingDialogue)
    {
        return;
    }
    
    // Stop audio
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
    
    // Clear timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    // Update state
    bIsPlayingDialogue = false;
    
    // Broadcast finish event
    OnDialogueFinished.Broadcast(CurrentDialogueLine);
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Stopped dialogue for %s"), *GetOwner()->GetName());
}

void UDialogueComponent::SetCharacterVoice(UDialogueVoice* Voice)
{
    CharacterVoice = Voice;
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Character voice set for %s"), *GetOwner()->GetName());
}

void UDialogueComponent::SetEmotionalState(EEmotionalTone Tone, float Intensity)
{
    CurrentEmotionalTone = Tone;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
    
    UpdateEmotionalDisplay();
    
    UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Emotional state updated - Tone: %d, Intensity: %f"), 
           (int32)Tone, Intensity);
}

void UDialogueComponent::OnDialogueAudioFinished()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;
        OnDialogueFinished.Broadcast(CurrentDialogueLine);
        
        UE_LOG(LogTemp, Log, TEXT("DialogueComponent: Dialogue audio finished for %s"), *GetOwner()->GetName());
    }
}

FDialogueContext UDialogueComponent::CreateDialogueContext(AActor* Listener) const
{
    FDialogueContext Context;
    
    // Set speaker voice
    if (CharacterVoice)
    {
        Context.Speaker = CharacterVoice;
    }
    
    // Set listener voice if available
    if (Listener)
    {
        UDialogueComponent* ListenerDialogue = Listener->FindComponentByClass<UDialogueComponent>();
        if (ListenerDialogue && ListenerDialogue->CharacterVoice)
        {
            Context.Targets.Add(ListenerDialogue->CharacterVoice);
        }
    }
    
    return Context;
}

void UDialogueComponent::UpdateEmotionalDisplay()
{
    // Update audio component properties based on emotional state
    if (AudioComponent)
    {
        // Adjust pitch based on emotional tone
        float PitchMultiplier = 1.0f;
        switch (CurrentEmotionalTone)
        {
            case EEmotionalTone::Fear:
                PitchMultiplier = 1.2f; // Higher pitch for fear
                break;
            case EEmotionalTone::Desperation:
                PitchMultiplier = 1.1f;
                break;
            case EEmotionalTone::Wonder:
                PitchMultiplier = 1.05f; // Slightly higher for wonder
                break;
            case EEmotionalTone::Awe:
                PitchMultiplier = 0.95f; // Lower for awe
                break;
            case EEmotionalTone::Peace:
                PitchMultiplier = 0.9f; // Lower for peace
                break;
            default:
                PitchMultiplier = 1.0f;
                break;
        }
        
        AudioComponent->SetPitchMultiplier(PitchMultiplier * EmotionalIntensity);
        
        // Adjust volume based on emotional intensity
        float VolumeMultiplier = FMath::Lerp(0.7f, 1.3f, EmotionalIntensity);
        AudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    }
}