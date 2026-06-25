#include "DialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANarr_DialogueActor::ANarr_DialogueActor()
{
    PrimaryActorTick.bCanEverTick = true;

    NPCName = TEXT("Unknown NPC");
    NPCRole = TEXT("Survivor");
    CurrentLineIndex = 0;
    bDialogueActive = false;
    TriggerRadius = 300.0f;
    DialogueTimer = 0.0f;

    // ── Pre-populate Elder Kael dialogue as default example ──
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = TEXT("Elder Kael");
    Line1.LineText = TEXT("The T-Rex moves at dawn. Three days ago it took two hunters near the northern river.");
    Line1.Tone = ENarr_DialogueTone::Warning;
    Line1.DisplayDuration = 5.0f;

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = TEXT("Elder Kael");
    Line2.LineText = TEXT("We tracked its path — it circles the valley before striking. If you go east, go in groups. Never alone.");
    Line2.Tone = ENarr_DialogueTone::Tactical;
    Line2.DisplayDuration = 6.0f;

    DialogueLines.Add(Line1);
    DialogueLines.Add(Line2);

    // Audio URLs from ElevenLabs TTS (generated this cycle)
    AudioURLs.Add(TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782387317497_Elder_Kael.mp3"));
}

void ANarr_DialogueActor::BeginPlay()
{
    Super::BeginPlay();
    CurrentLineIndex = 0;
    bDialogueActive = false;
    DialogueTimer = 0.0f;
}

void ANarr_DialogueActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDialogueActive) return;

    DialogueTimer += DeltaTime;

    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        const FNarr_DialogueLine& CurrentLine = DialogueLines[CurrentLineIndex];
        if (DialogueTimer >= CurrentLine.DisplayDuration)
        {
            DialogueTimer = 0.0f;
            AdvanceDialogue();
        }
    }
}

void ANarr_DialogueActor::StartDialogue()
{
    if (DialogueLines.Num() == 0) return;

    CurrentLineIndex = 0;
    bDialogueActive = true;
    DialogueTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue started: %s — %s"),
        *NPCName, *DialogueLines[0].LineText);
}

void ANarr_DialogueActor::AdvanceDialogue()
{
    if (!bDialogueActive) return;

    CurrentLineIndex++;

    if (!HasMoreLines())
    {
        EndDialogue();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Line %d: %s — %s"),
        CurrentLineIndex,
        *DialogueLines[CurrentLineIndex].SpeakerName,
        *DialogueLines[CurrentLineIndex].LineText);
}

void ANarr_DialogueActor::EndDialogue()
{
    bDialogueActive = false;
    CurrentLineIndex = 0;
    DialogueTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Dialogue ended: %s"), *NPCName);
}

FNarr_DialogueLine ANarr_DialogueActor::GetCurrentLine() const
{
    if (DialogueLines.IsValidIndex(CurrentLineIndex))
    {
        return DialogueLines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

bool ANarr_DialogueActor::HasMoreLines() const
{
    return DialogueLines.IsValidIndex(CurrentLineIndex);
}

void ANarr_DialogueActor::AddDialogueLine(const FString& Speaker, const FString& Text, ENarr_DialogueTone Tone, float Duration)
{
    FNarr_DialogueLine NewLine;
    NewLine.SpeakerName = Speaker;
    NewLine.LineText = Text;
    NewLine.Tone = Tone;
    NewLine.DisplayDuration = Duration;
    DialogueLines.Add(NewLine);
}
