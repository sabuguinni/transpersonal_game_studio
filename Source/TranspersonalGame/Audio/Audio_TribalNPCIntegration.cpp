#include "Audio_TribalNPCIntegration.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UAudio_TribalNPCIntegration::UAudio_TribalNPCIntegration()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    TribalRole = EAudio_TribalRole::Hunter;
    DialogueVolume = 0.8f;
    AmbientVolume = 0.4f;
    InteractionRange = 300.0f;
    DialogueCooldown = 5.0f;
    LastDialogueTime = 0.0f;
    bIsPlayingDialogue = false;
    
    // Initialize audio components
    DialogueAudioComponent = nullptr;
    AmbientAudioComponent = nullptr;
}

void UAudio_TribalNPCIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    LoadRoleSpecificAudio();
}

void UAudio_TribalNPCIntegration::InitializeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalNPCIntegration: No owner actor found"));
        return;
    }
    
    // Create dialogue audio component if not exists
    if (!DialogueAudioComponent)
    {
        DialogueAudioComponent = NewObject<UAudioComponent>(Owner);
        if (DialogueAudioComponent)
        {
            DialogueAudioComponent->SetupAttachment(Owner->GetRootComponent());
            DialogueAudioComponent->SetAutoActivate(false);
            DialogueAudioComponent->SetVolumeMultiplier(DialogueVolume);
            DialogueAudioComponent->bOverrideAttenuation = true;
            DialogueAudioComponent->AttenuationOverrides.bAttenuate = true;
            DialogueAudioComponent->AttenuationOverrides.FalloffDistance = InteractionRange;
            DialogueAudioComponent->RegisterComponent();
            
            // Bind to audio finished event
            DialogueAudioComponent->OnAudioFinished.AddDynamic(this, &UAudio_TribalNPCIntegration::OnDialogueFinished);
        }
    }
    
    // Create ambient audio component if not exists
    if (!AmbientAudioComponent)
    {
        AmbientAudioComponent = NewObject<UAudioComponent>(Owner);
        if (AmbientAudioComponent)
        {
            AmbientAudioComponent->SetupAttachment(Owner->GetRootComponent());
            AmbientAudioComponent->SetAutoActivate(true);
            AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
            AmbientAudioComponent->bOverrideAttenuation = true;
            AmbientAudioComponent->AttenuationOverrides.bAttenuate = true;
            AmbientAudioComponent->AttenuationOverrides.FalloffDistance = InteractionRange * 1.5f;
            AmbientAudioComponent->RegisterComponent();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Audio components initialized for %s"), *Owner->GetName());
}

void UAudio_TribalNPCIntegration::LoadRoleSpecificAudio()
{
    // Load audio assets based on tribal role
    switch (TribalRole)
    {
        case EAudio_TribalRole::Elder:
            // Load elder-specific audio (deeper, wiser voice)
            break;
            
        case EAudio_TribalRole::Hunter:
            // Load hunter-specific audio (alert, tactical voice)
            break;
            
        case EAudio_TribalRole::Scout:
            // Load scout-specific audio (quick, warning voice)
            break;
            
        case EAudio_TribalRole::Crafter:
            // Load crafter-specific audio (methodical, focused voice)
            break;
            
        case EAudio_TribalRole::Gatherer:
            // Load gatherer-specific audio (gentle, knowledgeable voice)
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Loaded audio for role %d"), (int32)TribalRole);
}

void UAudio_TribalNPCIntegration::PlayGreeting()
{
    if (!CanPlayDialogue())
    {
        return;
    }
    
    PlayDialogueSound(DialogueSet.GreetingSound);
    UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Playing greeting dialogue"));
}

void UAudio_TribalNPCIntegration::PlayWarning()
{
    if (!CanPlayDialogue())
    {
        return;
    }
    
    PlayDialogueSound(DialogueSet.WarningSound);
    UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Playing warning dialogue"));
}

void UAudio_TribalNPCIntegration::PlayFarewell()
{
    if (!CanPlayDialogue())
    {
        return;
    }
    
    PlayDialogueSound(DialogueSet.FarewellSound);
    UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Playing farewell dialogue"));
}

void UAudio_TribalNPCIntegration::PlayTradingDialogue()
{
    if (!CanPlayDialogue())
    {
        return;
    }
    
    PlayDialogueSound(DialogueSet.TradingSound);
    UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Playing trading dialogue"));
}

void UAudio_TribalNPCIntegration::StopCurrentDialogue()
{
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
        bIsPlayingDialogue = false;
        UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Stopped current dialogue"));
    }
}

bool UAudio_TribalNPCIntegration::CanPlayDialogue() const
{
    if (bIsPlayingDialogue)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    float CurrentTime = World->GetTimeSeconds();
    return (CurrentTime - LastDialogueTime) >= DialogueCooldown;
}

void UAudio_TribalNPCIntegration::SetTribalRole(EAudio_TribalRole NewRole)
{
    if (TribalRole != NewRole)
    {
        TribalRole = NewRole;
        LoadRoleSpecificAudio();
        UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Changed tribal role to %d"), (int32)NewRole);
    }
}

void UAudio_TribalNPCIntegration::PlayDialogueSound(TSoftObjectPtr<USoundBase> Sound)
{
    if (!DialogueAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalNPCIntegration: No dialogue audio component available"));
        return;
    }
    
    if (!Sound.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("TribalNPCIntegration: Invalid sound asset"));
        return;
    }
    
    // Stop any currently playing dialogue
    if (DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
    
    // Play the new dialogue
    DialogueAudioComponent->SetSound(Sound.Get());
    DialogueAudioComponent->Play();
    
    // Update state
    bIsPlayingDialogue = true;
    UWorld* World = GetWorld();
    if (World)
    {
        LastDialogueTime = World->GetTimeSeconds();
    }
}

void UAudio_TribalNPCIntegration::OnDialogueFinished()
{
    bIsPlayingDialogue = false;
    UE_LOG(LogTemp, Log, TEXT("TribalNPCIntegration: Dialogue finished"));
}