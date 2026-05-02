#include "Narr_SurvivalDialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"

UNarr_SurvivalDialogueSystem::UNarr_SurvivalDialogueSystem()
{
    bIsPlaying = false;
    DialogueStartTime = 0.0f;
    DialogueAudioComponent = nullptr;
}

void UNarr_SurvivalDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Initializing survival dialogue system"));
    
    SetupAudioComponent();
    InitializeDefaultDialogues();
    
    // Start proximity checking timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ProximityCheckTimerHandle,
            [this]()
            {
                if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
                {
                    if (APawn* PlayerPawn = PC->GetPawn())
                    {
                        CheckProximityTriggers(PlayerPawn->GetActorLocation());
                    }
                }
            },
            2.0f, // Check every 2 seconds
            true
        );
    }
}

void UNarr_SurvivalDialogueSystem::Deinitialize()
{
    StopCurrentDialogue();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
        World->GetTimerManager().ClearTimer(ProximityCheckTimerHandle);
    }
    
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->Stop();
        DialogueAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void UNarr_SurvivalDialogueSystem::RegisterDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    RegisteredDialogues.Add(DialogueLine);
    
    // Add to contextual map
    if (!ContextualDialogues.Contains(DialogueLine.Context))
    {
        ContextualDialogues.Add(DialogueLine.Context, TArray<FNarr_DialogueLine>());
    }
    ContextualDialogues[DialogueLine.Context].Add(DialogueLine);
    
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Registered dialogue line for context %d"), (int32)DialogueLine.Context);
}

void UNarr_SurvivalDialogueSystem::TriggerContextualDialogue(ENarr_DialogueContext Context, const FVector& PlayerLocation)
{
    if (bIsPlaying)
    {
        UE_LOG(LogTemp, Warning, TEXT("UNarr_SurvivalDialogueSystem: Cannot trigger dialogue - already playing"));
        return;
    }
    
    if (!ContextualDialogues.Contains(Context))
    {
        UE_LOG(LogTemp, Warning, TEXT("UNarr_SurvivalDialogueSystem: No dialogues found for context %d"), (int32)Context);
        return;
    }
    
    const TArray<FNarr_DialogueLine>& Candidates = ContextualDialogues[Context];
    if (Candidates.Num() == 0)
    {
        return;
    }
    
    // Select best dialogue based on conditions
    FNarr_DialogueLine SelectedDialogue = SelectBestDialogue(Candidates, PlayerLocation);
    PlayDialogueLine(SelectedDialogue);
}

void UNarr_SurvivalDialogueSystem::PlayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    if (bIsPlaying)
    {
        StopCurrentDialogue();
    }
    
    CurrentDialogue = DialogueLine;
    bIsPlaying = true;
    DialogueStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Playing dialogue: %s"), *DialogueLine.DialogueText);
    
    // Display text (in a real implementation, this would trigger UI)
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            DialogueLine.Duration,
            FColor::Yellow,
            FString::Printf(TEXT("%s: %s"), *DialogueLine.SpeakerName, *DialogueLine.DialogueText)
        );
    }
    
    // Play audio if available
    if (!DialogueLine.AudioURL.IsEmpty())
    {
        PlayAudioClip(DialogueLine.AudioURL);
    }
    
    // Set timer to finish dialogue
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &UNarr_SurvivalDialogueSystem::OnDialogueFinished,
            DialogueLine.Duration,
            false
        );
    }
}

void UNarr_SurvivalDialogueSystem::StopCurrentDialogue()
{
    if (!bIsPlaying)
    {
        return;
    }
    
    bIsPlaying = false;
    
    if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
    {
        DialogueAudioComponent->Stop();
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Stopped current dialogue"));
}

bool UNarr_SurvivalDialogueSystem::IsDialoguePlaying() const
{
    return bIsPlaying;
}

void UNarr_SurvivalDialogueSystem::LoadAudioFromURL(const FString& AudioURL, const FString& DialogueID)
{
    // In a real implementation, this would download and cache audio from URL
    // For now, we'll just log the request
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Audio load requested - ID: %s, URL: %s"), *DialogueID, *AudioURL);
}

void UNarr_SurvivalDialogueSystem::PlayAudioClip(const FString& DialogueID)
{
    if (!DialogueAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UNarr_SurvivalDialogueSystem: No audio component available"));
        return;
    }
    
    // In a real implementation, this would play the loaded audio clip
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Playing audio clip: %s"), *DialogueID);
}

void UNarr_SurvivalDialogueSystem::CheckProximityTriggers(const FVector& PlayerLocation)
{
    for (const auto& Trigger : ProximityTriggers)
    {
        const FVector& TriggerLocation = Trigger.Key;
        const FNarr_DialogueLine& DialogueLine = Trigger.Value;
        
        // Skip if already triggered
        if (TriggeredLocations.Contains(TriggerLocation))
        {
            continue;
        }
        
        // Check distance
        float Distance = FVector::Dist(PlayerLocation, TriggerLocation);
        if (Distance <= DialogueLine.TriggerDistance)
        {
            if (CanTriggerDialogue(DialogueLine, PlayerLocation))
            {
                PlayDialogueLine(DialogueLine);
                TriggeredLocations.Add(TriggerLocation);
                break; // Only trigger one at a time
            }
        }
    }
}

void UNarr_SurvivalDialogueSystem::RegisterProximityDialogue(const FNarr_DialogueLine& DialogueLine, const FVector& TriggerLocation)
{
    ProximityTriggers.Add(TriggerLocation, DialogueLine);
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Registered proximity dialogue at location (%f, %f, %f)"), 
           TriggerLocation.X, TriggerLocation.Y, TriggerLocation.Z);
}

void UNarr_SurvivalDialogueSystem::InitializeDefaultDialogues()
{
    // Predator Encounter
    FNarr_DialogueLine PredatorDialogue;
    PredatorDialogue.SpeakerName = TEXT("Survival Instinct");
    PredatorDialogue.DialogueText = TEXT("The massive predator's eyes lock onto yours through the thick undergrowth. Every instinct screams at you to run, but sudden movement will trigger its hunting response. Stay perfectly still.");
    PredatorDialogue.Context = ENarr_DialogueContext::PredatorEncounter;
    PredatorDialogue.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777708328590_PredatorEncounter.mp3");
    PredatorDialogue.Duration = 16.0f;
    PredatorDialogue.TriggerDistance = 1000.0f;
    RegisterDialogueLine(PredatorDialogue);
    
    // Scout Report
    FNarr_DialogueLine ScoutDialogue;
    ScoutDialogue.SpeakerName = TEXT("Scout");
    ScoutDialogue.DialogueText = TEXT("I've found fresh water upstream, but the tracks around it... they're massive. Three-toed prints, deeper than my hand. Whatever drinks here is bigger than anything I've seen.");
    ScoutDialogue.Context = ENarr_DialogueContext::ScoutReport;
    ScoutDialogue.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777708335639_ScoutReport.mp3");
    ScoutDialogue.Duration = 14.0f;
    ScoutDialogue.TriggerDistance = 800.0f;
    RegisterDialogueLine(ScoutDialogue);
    
    // Herbalist Wisdom
    FNarr_DialogueLine HerbalistDialogue;
    HerbalistDialogue.SpeakerName = TEXT("Herbalist");
    HerbalistDialogue.DialogueText = TEXT("The herd moves at dawn. They follow the same path every three days - down to the river, then back to the high ground. If we time it right, we can gather the medicinal plants they leave behind.");
    HerbalistDialogue.Context = ENarr_DialogueContext::HerbalistWisdom;
    HerbalistDialogue.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777708341599_HerbalistWisdom.mp3");
    HerbalistDialogue.Duration = 15.0f;
    HerbalistDialogue.TriggerDistance = 600.0f;
    RegisterDialogueLine(HerbalistDialogue);
    
    // Night Survival
    FNarr_DialogueLine NightDialogue;
    NightDialogue.SpeakerName = TEXT("Elder");
    NightDialogue.DialogueText = TEXT("Night is falling, and the forest grows quiet. Too quiet. When the usual sounds stop, it means something big is hunting. Find shelter now, and don't light a fire.");
    NightDialogue.Context = ENarr_DialogueContext::NightSurvival;
    NightDialogue.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777708346976_NightSurvival.mp3");
    NightDialogue.Duration = 14.0f;
    NightDialogue.TriggerDistance = 500.0f;
    RegisterDialogueLine(NightDialogue);
    
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Initialized %d default dialogue lines"), RegisteredDialogues.Num());
}

void UNarr_SurvivalDialogueSystem::SetupAudioComponent()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            DialogueAudioComponent = NewObject<UAudioComponent>(GameInstance);
            if (DialogueAudioComponent)
            {
                DialogueAudioComponent->SetVolumeMultiplier(0.8f);
                DialogueAudioComponent->bAutoActivate = false;
                UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Audio component created"));
            }
        }
    }
}

bool UNarr_SurvivalDialogueSystem::CanTriggerDialogue(const FNarr_DialogueLine& DialogueLine, const FVector& PlayerLocation)
{
    // Don't interrupt current dialogue
    if (bIsPlaying)
    {
        return false;
    }
    
    // Check line of sight if required
    if (DialogueLine.bRequiresLineOfSight)
    {
        // In a real implementation, this would do a line trace
        // For now, we'll just return true
    }
    
    return true;
}

FNarr_DialogueLine UNarr_SurvivalDialogueSystem::SelectBestDialogue(const TArray<FNarr_DialogueLine>& Candidates, const FVector& PlayerLocation)
{
    if (Candidates.Num() == 0)
    {
        return FNarr_DialogueLine();
    }
    
    // For now, just return a random candidate
    // In a real implementation, this would consider player state, recent dialogues, etc.
    int32 RandomIndex = FMath::RandRange(0, Candidates.Num() - 1);
    return Candidates[RandomIndex];
}

void UNarr_SurvivalDialogueSystem::OnDialogueFinished()
{
    bIsPlaying = false;
    UE_LOG(LogTemp, Log, TEXT("UNarr_SurvivalDialogueSystem: Dialogue finished"));
}