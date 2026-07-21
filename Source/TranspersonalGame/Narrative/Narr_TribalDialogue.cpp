#include "Narr_TribalDialogue.h"

UNarr_TribalDialogue::UNarr_TribalDialogue()
{
    PrimaryComponentTick.bCanEverTick = false;
    AssignedRole = ENarr_TribalRole::None;
}

void UNarr_TribalDialogue::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultDialogues();
}

void UNarr_TribalDialogue::InitializeDefaultDialogues()
{
    TribalDialogues.Empty();

    // Elder dialogues
    FNarr_TribalDialogueLine ElderGreeting;
    ElderGreeting.SpeakerRole = ENarr_TribalRole::Elder;
    ElderGreeting.Context = ENarr_DialogueContext::Greeting;
    ElderGreeting.DialogueText = TEXT("Welcome, young one. The ancestors smile upon your return.");
    ElderGreeting.EmotionalIntensity = 0.8f;
    TribalDialogues.Add(ElderGreeting);

    FNarr_TribalDialogueLine ElderWarning;
    ElderWarning.SpeakerRole = ENarr_TribalRole::Elder;
    ElderWarning.Context = ENarr_DialogueContext::Warning;
    ElderWarning.DialogueText = TEXT("Beware the eastern valleys. The great beasts grow restless with the changing seasons.");
    ElderWarning.EmotionalIntensity = 1.2f;
    TribalDialogues.Add(ElderWarning);

    FNarr_TribalDialogueLine ElderTeaching;
    ElderTeaching.SpeakerRole = ENarr_TribalRole::Elder;
    ElderTeaching.Context = ENarr_DialogueContext::Teaching;
    ElderTeaching.DialogueText = TEXT("Listen well - fire is life, water is survival, but wisdom is what separates us from the beasts.");
    ElderTeaching.EmotionalIntensity = 1.0f;
    TribalDialogues.Add(ElderTeaching);

    // Hunter dialogues
    FNarr_TribalDialogueLine HunterGreeting;
    HunterGreeting.SpeakerRole = ENarr_TribalRole::Hunter;
    HunterGreeting.Context = ENarr_DialogueContext::Greeting;
    HunterGreeting.DialogueText = TEXT("The hunt calls, friend. Are your spears sharp and your courage steady?");
    HunterGreeting.EmotionalIntensity = 1.1f;
    TribalDialogues.Add(HunterGreeting);

    FNarr_TribalDialogueLine HunterWarning;
    HunterWarning.SpeakerRole = ENarr_TribalRole::Hunter;
    HunterWarning.Context = ENarr_DialogueContext::Warning;
    HunterWarning.DialogueText = TEXT("Pack hunters near the river bend. Three sets of tracks, fresh blood. Move with caution.");
    HunterWarning.EmotionalIntensity = 1.5f;
    TribalDialogues.Add(HunterWarning);

    FNarr_TribalDialogueLine HunterTeaching;
    HunterTeaching.SpeakerRole = ENarr_TribalRole::Hunter;
    HunterTeaching.Context = ENarr_DialogueContext::Teaching;
    HunterTeaching.DialogueText = TEXT("Strike fast, strike true. A wounded beast is more dangerous than a healthy one.");
    HunterTeaching.EmotionalIntensity = 1.0f;
    TribalDialogues.Add(HunterTeaching);

    // Scout dialogues
    FNarr_TribalDialogueLine ScoutGreeting;
    ScoutGreeting.SpeakerRole = ENarr_TribalRole::Scout;
    ScoutGreeting.Context = ENarr_DialogueContext::Greeting;
    ScoutGreeting.DialogueText = TEXT("Swift winds and clear paths to you, traveler.");
    ScoutGreeting.EmotionalIntensity = 0.9f;
    TribalDialogues.Add(ScoutGreeting);

    FNarr_TribalDialogueLine ScoutWarning;
    ScoutWarning.SpeakerRole = ENarr_TribalRole::Scout;
    ScoutWarning.Context = ENarr_DialogueContext::Warning;
    ScoutWarning.DialogueText = TEXT("Storm clouds gather to the north. Seek shelter before the sky opens its fury.");
    ScoutWarning.EmotionalIntensity = 1.3f;
    TribalDialogues.Add(ScoutWarning);

    // Gatherer dialogues
    FNarr_TribalDialogueLine GathererGreeting;
    GathererGreeting.SpeakerRole = ENarr_TribalRole::Gatherer;
    GathererGreeting.Context = ENarr_DialogueContext::Greeting;
    GathererGreeting.DialogueText = TEXT("The earth provides for those who know where to look. What do you seek?");
    GathererGreeting.EmotionalIntensity = 0.7f;
    TribalDialogues.Add(GathererGreeting);

    FNarr_TribalDialogueLine GathererTeaching;
    GathererTeaching.SpeakerRole = ENarr_TribalRole::Gatherer;
    GathererTeaching.Context = ENarr_DialogueContext::Teaching;
    GathererTeaching.DialogueText = TEXT("These berries heal wounds, but those leaves bring fever dreams. Know the difference or pay the price.");
    GathererTeaching.EmotionalIntensity = 1.0f;
    TribalDialogues.Add(GathererTeaching);
}

FNarr_TribalDialogueLine UNarr_TribalDialogue::GetDialogueForContext(ENarr_TribalRole Role, ENarr_DialogueContext Context)
{
    for (const FNarr_TribalDialogueLine& Line : TribalDialogues)
    {
        if (Line.SpeakerRole == Role && Line.Context == Context)
        {
            return Line;
        }
    }

    // Return default if not found
    FNarr_TribalDialogueLine DefaultLine;
    DefaultLine.SpeakerRole = Role;
    DefaultLine.Context = Context;
    DefaultLine.DialogueText = TEXT("...");
    return DefaultLine;
}

TArray<FNarr_TribalDialogueLine> UNarr_TribalDialogue::GetAllDialogueForRole(ENarr_TribalRole Role)
{
    TArray<FNarr_TribalDialogueLine> RoleDialogues;
    
    for (const FNarr_TribalDialogueLine& Line : TribalDialogues)
    {
        if (Line.SpeakerRole == Role)
        {
            RoleDialogues.Add(Line);
        }
    }
    
    return RoleDialogues;
}

void UNarr_TribalDialogue::AddDialogueLine(const FNarr_TribalDialogueLine& NewLine)
{
    TribalDialogues.Add(NewLine);
}

bool UNarr_TribalDialogue::HasDialogueForContext(ENarr_TribalRole Role, ENarr_DialogueContext Context) const
{
    for (const FNarr_TribalDialogueLine& Line : TribalDialogues)
    {
        if (Line.SpeakerRole == Role && Line.Context == Context)
        {
            return true;
        }
    }
    return false;
}