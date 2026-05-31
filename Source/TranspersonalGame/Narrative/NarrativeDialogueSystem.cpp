#include "NarrativeDialogueSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

void UNarrativeDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsPlayingDialogue = false;
    
    InitializeDialogueData();
    InitializeSurvivalTips();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueSystem initialized with %d intro dialogues and %d survival tips"), 
           IntroDialogues.Num(), SurvivalTips.Num());
}

void UNarrativeDialogueSystem::Deinitialize()
{
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    Super::Deinitialize();
}

void UNarrativeDialogueSystem::PlayDialogueLine(const FNarr_DialogueLine& DialogueLine)
{
    if (bIsPlayingDialogue)
    {
        StopCurrentDialogue();
    }
    
    CurrentDialogue = DialogueLine;
    bIsPlayingDialogue = true;
    
    // Display text on screen
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                            *DialogueLine.SpeakerName, 
                                            *DialogueLine.DialogueText.ToString());
        GEngine->AddOnScreenDebugMessage(-1, DialogueLine.DisplayDuration, FColor::Yellow, DisplayText);
    }
    
    // Play audio if available
    if (DialogueLine.VoiceAudio.IsValid())
    {
        USoundCue* SoundCue = DialogueLine.VoiceAudio.LoadSynchronous();
        if (SoundCue && GetWorld())
        {
            UGameplayStatics::PlaySound2D(GetWorld(), SoundCue);
        }
    }
    
    // Set timer to finish dialogue
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(DialogueTimerHandle, 
                                             this, 
                                             &UNarrativeDialogueSystem::OnDialogueFinished, 
                                             DialogueLine.DisplayDuration, 
                                             false);
    }
}

void UNarrativeDialogueSystem::TriggerSurvivalTip(ENarr_DinosaurSpecies Species, float Distance)
{
    for (const FNarr_SurvivalTip& Tip : SurvivalTips)
    {
        if (Tip.TriggerSpecies == Species && Distance <= Tip.TriggerDistance)
        {
            FNarr_DialogueLine TipDialogue;
            TipDialogue.SpeakerName = TEXT("Survival Guide");
            TipDialogue.DialogueText = Tip.TipText;
            TipDialogue.DisplayDuration = 4.0f;
            TipDialogue.bIsNarration = true;
            TipDialogue.VoiceAudio = Tip.WarningAudio;
            
            PlayDialogueLine(TipDialogue);
            break;
        }
    }
}

void UNarrativeDialogueSystem::PlayNarration(const FText& NarrationText, float Duration)
{
    FNarr_DialogueLine NarrationDialogue;
    NarrationDialogue.SpeakerName = TEXT("Narrator");
    NarrationDialogue.DialogueText = NarrationText;
    NarrationDialogue.DisplayDuration = Duration;
    NarrationDialogue.bIsNarration = true;
    
    PlayDialogueLine(NarrationDialogue);
}

void UNarrativeDialogueSystem::StopCurrentDialogue()
{
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(DialogueTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
    }
    
    bIsPlayingDialogue = false;
    CurrentDialogue = FNarr_DialogueLine();
}

bool UNarrativeDialogueSystem::IsDialoguePlaying() const
{
    return bIsPlayingDialogue;
}

void UNarrativeDialogueSystem::OnDialogueFinished()
{
    bIsPlayingDialogue = false;
    CurrentDialogue = FNarr_DialogueLine();
}

void UNarrativeDialogueSystem::InitializeDialogueData()
{
    IntroDialogues.Empty();
    
    // Intro sequence dialogues
    FNarr_DialogueLine Intro1;
    Intro1.SpeakerName = TEXT("Narrator");
    Intro1.DialogueText = FText::FromString(TEXT("You wake in a world where giants roam. The age of dinosaurs has returned."));
    Intro1.DisplayDuration = 5.0f;
    Intro1.bIsNarration = true;
    IntroDialogues.Add(Intro1);
    
    FNarr_DialogueLine Intro2;
    Intro2.SpeakerName = TEXT("Narrator");
    Intro2.DialogueText = FText::FromString(TEXT("Your survival depends on understanding these ancient predators and their territories."));
    Intro2.DisplayDuration = 5.0f;
    Intro2.bIsNarration = true;
    IntroDialogues.Add(Intro2);
    
    FNarr_DialogueLine Intro3;
    Intro3.SpeakerName = TEXT("Narrator");
    Intro3.DialogueText = FText::FromString(TEXT("Learn their patterns. Respect their power. Adapt or perish."));
    Intro3.DisplayDuration = 4.0f;
    Intro3.bIsNarration = true;
    IntroDialogues.Add(Intro3);
}

void UNarrativeDialogueSystem::InitializeSurvivalTips()
{
    SurvivalTips.Empty();
    
    // T-Rex survival tip
    FNarr_SurvivalTip TRexTip;
    TRexTip.TipText = FText::FromString(TEXT("T-Rex detected! Stay perfectly still. Their vision is based on movement."));
    TRexTip.TriggerSpecies = ENarr_DinosaurSpecies::TRex;
    TRexTip.TriggerDistance = 2000.0f;
    SurvivalTips.Add(TRexTip);
    
    // Velociraptor survival tip
    FNarr_SurvivalTip RaptorTip;
    RaptorTip.TipText = FText::FromString(TEXT("Velociraptors hunt in packs. Watch for flanking maneuvers."));
    RaptorTip.TriggerSpecies = ENarr_DinosaurSpecies::Velociraptor;
    RaptorTip.TriggerDistance = 1500.0f;
    SurvivalTips.Add(RaptorTip);
    
    // Brachiosaurus survival tip
    FNarr_SurvivalTip BrachioTip;
    BrachioTip.TipText = FText::FromString(TEXT("Brachiosaurus herds indicate fresh water nearby. Follow them to safety."));
    BrachioTip.TriggerSpecies = ENarr_DinosaurSpecies::Brachiosaurus;
    BrachioTip.TriggerDistance = 3000.0f;
    SurvivalTips.Add(BrachioTip);
    
    // Triceratops survival tip
    FNarr_SurvivalTip TriceratopsTip;
    TriceratopsTip.TipText = FText::FromString(TEXT("Triceratops are territorial but predictable. Avoid their charging lanes."));
    TriceratopsTip.TriggerSpecies = ENarr_DinosaurSpecies::Triceratops;
    TriceratopsTip.TriggerDistance = 1200.0f;
    SurvivalTips.Add(TriceratopsTip);
}