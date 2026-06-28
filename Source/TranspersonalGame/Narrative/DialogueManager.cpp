#include "Narrative/DialogueManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_DialogueManager::UNarr_DialogueManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    GlobalVoiceVolume = 1.0f;
    bIsPlayingDialogue = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
}

void UNarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    PopulateDefaultDialogue();
}

void UNarr_DialogueManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsPlayingDialogue) return;

    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        AdvanceToNextLine();
    }
}

void UNarr_DialogueManager::PlaySequence(FName SequenceID)
{
    if (!HasSequence(SequenceID))
    {
        UE_LOG(LogTemp, Warning, TEXT("DialogueManager: Sequence '%s' not found"), *SequenceID.ToString());
        return;
    }

    // Stop any active dialogue
    if (bIsPlayingDialogue)
    {
        StopDialogue();
    }

    ActiveSequence = GetSequence(SequenceID);
    ActiveSequenceID = SequenceID;
    CurrentLineIndex = 0;
    bIsPlayingDialogue = true;

    OnDialogueStarted.Broadcast(SequenceID, ActiveSequence.Lines.Num() > 0 ? ActiveSequence.Lines[0].Speaker : ENarr_SpeakerRole::Narrator);

    if (ActiveSequence.Lines.Num() > 0)
    {
        DisplayLine(ActiveSequence.Lines[0]);
    }
    else
    {
        StopDialogue();
    }
}

void UNarr_DialogueManager::StopDialogue()
{
    if (!bIsPlayingDialogue) return;

    FName EndedID = ActiveSequenceID;
    bIsPlayingDialogue = false;
    ActiveSequenceID = NAME_None;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;

    OnDialogueEnded.Broadcast(EndedID);
}

void UNarr_DialogueManager::SkipCurrentLine()
{
    if (!bIsPlayingDialogue) return;
    LineTimer = 0.0f;
    AdvanceToNextLine();
}

bool UNarr_DialogueManager::HasSequence(FName SequenceID) const
{
    for (const FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID) return true;
    }
    return false;
}

FNarr_DialogueSequence UNarr_DialogueManager::GetSequence(FName SequenceID) const
{
    for (const FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        if (Seq.SequenceID == SequenceID) return Seq;
    }
    return FNarr_DialogueSequence();
}

void UNarr_DialogueManager::TriggerByContext(ENarr_DialogueTrigger TriggerType)
{
    // Find first sequence that has lines matching this trigger
    for (const FNarr_DialogueSequence& Seq : DialogueLibrary)
    {
        for (const FNarr_DialogueLine& Line : Seq.Lines)
        {
            if (Line.Trigger == TriggerType)
            {
                PlaySequence(Seq.SequenceID);
                return;
            }
        }
    }
}

void UNarr_DialogueManager::RegisterSequence(const FNarr_DialogueSequence& Sequence)
{
    // Replace if exists
    for (int32 i = 0; i < DialogueLibrary.Num(); ++i)
    {
        if (DialogueLibrary[i].SequenceID == Sequence.SequenceID)
        {
            DialogueLibrary[i] = Sequence;
            return;
        }
    }
    DialogueLibrary.Add(Sequence);
}

void UNarr_DialogueManager::AdvanceToNextLine()
{
    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveSequence.Lines.Num())
    {
        if (ActiveSequence.bLoops && ActiveSequence.Lines.Num() > 0)
        {
            CurrentLineIndex = 0;
            DisplayLine(ActiveSequence.Lines[0]);
        }
        else
        {
            StopDialogue();
        }
        return;
    }

    // Pause between lines
    if (ActiveSequence.PauseBetweenLines > 0.0f)
    {
        LineTimer = ActiveSequence.PauseBetweenLines;
        // We'll display the next line after the pause
        // For simplicity, display immediately and let timer handle duration
    }

    DisplayLine(ActiveSequence.Lines[CurrentLineIndex]);
}

void UNarr_DialogueManager::DisplayLine(const FNarr_DialogueLine& Line)
{
    LineTimer = Line.DisplayDuration;
    OnLineDisplayed.Broadcast(Line, CurrentLineIndex);

    // Play voice audio if available
    if (!Line.VoiceAudio.IsNull())
    {
        USoundBase* Sound = Line.VoiceAudio.LoadSynchronous();
        if (Sound && GetOwner())
        {
            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                Sound,
                GetOwner()->GetActorLocation(),
                GlobalVoiceVolume
            );
        }
    }

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: [%s] %s"),
        *UEnum::GetValueAsString(Line.Speaker),
        *Line.LineText.ToString());
}

void UNarr_DialogueManager::PopulateDefaultDialogue()
{
    // Danger sequence — raptors spotted
    FNarr_DialogueSequence DangerSeq;
    DangerSeq.SequenceID = FName("SEQ_RaptorDanger");
    DangerSeq.bLoops = false;
    DangerSeq.PauseBetweenLines = 1.5f;

    FNarr_DialogueLine Line1;
    Line1.LineID = FName("LINE_RaptorDanger_01");
    Line1.Speaker = ENarr_SpeakerRole::TrailReader;
    Line1.LineText = FText::FromString(TEXT("Three raptors — circling from the east. Stay low."));
    Line1.Trigger = ENarr_DialogueTrigger::DangerNear;
    Line1.DisplayDuration = 4.0f;
    DangerSeq.Lines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.LineID = FName("LINE_RaptorDanger_02");
    Line2.Speaker = ENarr_SpeakerRole::ScoutLeader;
    Line2.LineText = FText::FromString(TEXT("River crossing — north. Move now, do not run."));
    Line2.Trigger = ENarr_DialogueTrigger::DangerNear;
    Line2.DisplayDuration = 4.0f;
    DangerSeq.Lines.Add(Line2);

    RegisterSequence(DangerSeq);

    // Stampede warning sequence
    FNarr_DialogueSequence StampedeSeq;
    StampedeSeq.SequenceID = FName("SEQ_StampedeWarning");
    StampedeSeq.bLoops = false;
    StampedeSeq.PauseBetweenLines = 1.0f;

    FNarr_DialogueLine SLine1;
    SLine1.LineID = FName("LINE_Stampede_01");
    SLine1.Speaker = ENarr_SpeakerRole::HunterElder;
    SLine1.LineText = FText::FromString(TEXT("The ground shakes. Hundreds of them — from the north ridge. Climb!"));
    SLine1.Trigger = ENarr_DialogueTrigger::DangerNear;
    SLine1.DisplayDuration = 5.0f;
    StampedeSeq.Lines.Add(SLine1);

    RegisterSequence(StampedeSeq);

    // Discovery sequence — nest found
    FNarr_DialogueSequence NestSeq;
    NestSeq.SequenceID = FName("SEQ_NestDiscovery");
    NestSeq.bLoops = false;
    NestSeq.PauseBetweenLines = 2.0f;

    FNarr_DialogueLine NLine1;
    NLine1.LineID = FName("LINE_Nest_01");
    NLine1.Speaker = ENarr_SpeakerRole::ScoutLeader;
    NLine1.LineText = FText::FromString(TEXT("Forty eggs. The mother is close. We take nothing. We leave no trace."));
    NLine1.Trigger = ENarr_DialogueTrigger::Discovery;
    NLine1.DisplayDuration = 5.0f;
    NestSeq.Lines.Add(NLine1);

    FNarr_DialogueLine NLine2;
    NLine2.LineID = FName("LINE_Nest_02");
    NLine2.Speaker = ENarr_SpeakerRole::HunterElder;
    NLine2.LineText = FText::FromString(TEXT("Mark this place. We return when the season turns."));
    NLine2.Trigger = ENarr_DialogueTrigger::Discovery;
    NLine2.DisplayDuration = 4.0f;
    NestSeq.Lines.Add(NLine2);

    RegisterSequence(NestSeq);

    // Survivor backstory — idle campfire
    FNarr_DialogueSequence SurvivorSeq;
    SurvivorSeq.SequenceID = FName("SEQ_SurvivorMemory");
    SurvivorSeq.bLoops = false;
    SurvivorSeq.PauseBetweenLines = 2.5f;

    FNarr_DialogueLine SVLine1;
    SVLine1.LineID = FName("LINE_Survivor_01");
    SVLine1.Speaker = ENarr_SpeakerRole::Survivor;
    SVLine1.LineText = FText::FromString(TEXT("Three winters ago, my father's tribe crossed the northern plains during migration. Only four survived. I was one of them."));
    SVLine1.Trigger = ENarr_DialogueTrigger::Idle;
    SVLine1.DisplayDuration = 7.0f;
    SurvivorSeq.Lines.Add(SVLine1);

    FNarr_DialogueLine SVLine2;
    SVLine2.LineID = FName("LINE_Survivor_02");
    SVLine2.Speaker = ENarr_SpeakerRole::Survivor;
    SVLine2.LineText = FText::FromString(TEXT("That is why we wait. We always wait for the herd to pass."));
    SVLine2.Trigger = ENarr_DialogueTrigger::Idle;
    SVLine2.DisplayDuration = 5.0f;
    SurvivorSeq.Lines.Add(SVLine2);

    RegisterSequence(SurvivorSeq);

    UE_LOG(LogTemp, Log, TEXT("DialogueManager: Populated %d default sequences"), DialogueLibrary.Num());
}
