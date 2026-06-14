#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentDialogueID = -1;
    bIsDialogueActive = false;
    DialogueRange = 500.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    LoadDefaultDialogues();
    LoadDefaultCharacters();
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Auto-advance dialogue based on duration if no player input
    if (bIsDialogueActive && CurrentDialogueID >= 0)
    {
        int32 DialogueIndex = FindDialogueIndex(CurrentDialogueID);
        if (DialogueIndex >= 0)
        {
            // Handle auto-advance logic here if needed
        }
    }
}

bool UNarr_DialogueSystem::StartDialogue(const FString& CharacterName, int32 StartingDialogueID)
{
    if (bIsDialogueActive)
    {
        return false;
    }

    int32 DialogueIndex = FindDialogueIndex(StartingDialogueID);
    if (DialogueIndex < 0)
    {
        return false;
    }

    CurrentDialogueID = StartingDialogueID;
    bIsDialogueActive = true;

    UE_LOG(LogTemp, Warning, TEXT("Started dialogue with %s, ID: %d"), *CharacterName, StartingDialogueID);
    return true;
}

void UNarr_DialogueSystem::EndDialogue()
{
    CurrentDialogueID = -1;
    bIsDialogueActive = false;
    UE_LOG(LogTemp, Warning, TEXT("Dialogue ended"));
}

bool UNarr_DialogueSystem::AdvanceDialogue(int32 ResponseChoice)
{
    if (!bIsDialogueActive || CurrentDialogueID < 0)
    {
        return false;
    }

    int32 CurrentIndex = FindDialogueIndex(CurrentDialogueID);
    if (CurrentIndex < 0)
    {
        return false;
    }

    const FNarr_DialogueEntry& CurrentEntry = DialogueDatabase[CurrentIndex];
    
    // Handle response choice and advance to next dialogue
    if (CurrentEntry.NextDialogueID >= 0)
    {
        CurrentDialogueID = CurrentEntry.NextDialogueID;
        return true;
    }
    else
    {
        EndDialogue();
        return false;
    }
}

FNarr_DialogueEntry UNarr_DialogueSystem::GetCurrentDialogue() const
{
    if (CurrentDialogueID >= 0)
    {
        int32 Index = FindDialogueIndex(CurrentDialogueID);
        if (Index >= 0)
        {
            return DialogueDatabase[Index];
        }
    }
    return FNarr_DialogueEntry();
}

TArray<FString> UNarr_DialogueSystem::GetCurrentResponses() const
{
    if (CurrentDialogueID >= 0)
    {
        int32 Index = FindDialogueIndex(CurrentDialogueID);
        if (Index >= 0)
        {
            return DialogueDatabase[Index].ResponseOptions;
        }
    }
    return TArray<FString>();
}

void UNarr_DialogueSystem::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueDatabase.Add(NewEntry);
}

void UNarr_DialogueSystem::AddCharacterProfile(const FNarr_CharacterProfile& NewProfile)
{
    CharacterProfiles.Add(NewProfile);
}

FNarr_CharacterProfile UNarr_DialogueSystem::GetCharacterProfile(const FString& CharacterName) const
{
    int32 Index = FindCharacterIndex(CharacterName);
    if (Index >= 0)
    {
        return CharacterProfiles[Index];
    }
    return FNarr_CharacterProfile();
}

void UNarr_DialogueSystem::UpdateTrustLevel(const FString& CharacterName, float DeltaTrust)
{
    int32 Index = FindCharacterIndex(CharacterName);
    if (Index >= 0)
    {
        CharacterProfiles[Index].TrustLevel = FMath::Clamp(CharacterProfiles[Index].TrustLevel + DeltaTrust, 0.0f, 1.0f);
        UE_LOG(LogTemp, Warning, TEXT("Updated trust for %s: %f"), *CharacterName, CharacterProfiles[Index].TrustLevel);
    }
}

void UNarr_DialogueSystem::InitializeTribalDialogues()
{
    // Tribal Leader dialogues
    FNarr_DialogueEntry TribalGreeting;
    TribalGreeting.SpeakerName = TEXT("Tribal Leader");
    TribalGreeting.DialogueText = TEXT("Welcome, hunter. The elders speak of your courage against the great beasts.");
    TribalGreeting.Duration = 4.0f;
    TribalGreeting.ResponseOptions.Add(TEXT("I seek to prove myself worthy."));
    TribalGreeting.ResponseOptions.Add(TEXT("What dangers threaten the tribe?"));
    TribalGreeting.NextDialogueID = 1;
    DialogueDatabase.Add(TribalGreeting);

    FNarr_DialogueEntry TribalWisdom;
    TribalWisdom.SpeakerName = TEXT("Tribal Leader");
    TribalWisdom.DialogueText = TEXT("The ancient ways teach us - respect the land, fear the predators, but never surrender to them.");
    TribalWisdom.Duration = 5.0f;
    TribalWisdom.NextDialogueID = -1;
    DialogueDatabase.Add(TribalWisdom);
}

void UNarr_DialogueSystem::InitializeSurvivalDialogues()
{
    // Scout mentor dialogues
    FNarr_DialogueEntry ScoutAdvice;
    ScoutAdvice.SpeakerName = TEXT("Scout Mentor");
    ScoutAdvice.DialogueText = TEXT("Read the signs carefully. Broken branches, claw marks, fresh dung - all tell the story of what passed here.");
    ScoutAdvice.Duration = 6.0f;
    ScoutAdvice.ResponseOptions.Add(TEXT("Teach me to track the great beasts."));
    ScoutAdvice.ResponseOptions.Add(TEXT("How do I avoid becoming prey?"));
    ScoutAdvice.NextDialogueID = 10;
    DialogueDatabase.Add(ScoutAdvice);
}

void UNarr_DialogueSystem::InitializeHuntingDialogues()
{
    // Hunter veteran dialogues
    FNarr_DialogueEntry HunterTactics;
    HunterTactics.SpeakerName = TEXT("Hunter Veteran");
    HunterTactics.DialogueText = TEXT("Never face a raptor pack alone. They coordinate like we do, but faster. Use terrain, use fire, use your brain.");
    HunterTactics.Duration = 7.0f;
    HunterTactics.ResponseOptions.Add(TEXT("What weapons work best?"));
    HunterTactics.ResponseOptions.Add(TEXT("How many hunters for a T-Rex?"));
    HunterTactics.NextDialogueID = 20;
    DialogueDatabase.Add(HunterTactics);
}

void UNarr_DialogueSystem::LoadDefaultDialogues()
{
    DialogueDatabase.Empty();
    InitializeTribalDialogues();
    InitializeSurvivalDialogues();
    InitializeHuntingDialogues();
    
    UE_LOG(LogTemp, Warning, TEXT("Loaded %d dialogue entries"), DialogueDatabase.Num());
}

void UNarr_DialogueSystem::LoadDefaultCharacters()
{
    CharacterProfiles.Empty();

    // Tribal Leader
    FNarr_CharacterProfile TribalLeader;
    TribalLeader.CharacterName = TEXT("Tribal Leader");
    TribalLeader.VoiceType = TEXT("Deep_Authoritative");
    TribalLeader.Personality = TEXT("Wise, Protective, Traditional");
    TribalLeader.KnownTopics.Add(TEXT("Tribal History"));
    TribalLeader.KnownTopics.Add(TEXT("Leadership"));
    TribalLeader.KnownTopics.Add(TEXT("Ancient Wisdom"));
    TribalLeader.TrustLevel = 0.7f;
    CharacterProfiles.Add(TribalLeader);

    // Scout Mentor
    FNarr_CharacterProfile ScoutMentor;
    ScoutMentor.CharacterName = TEXT("Scout Mentor");
    ScoutMentor.VoiceType = TEXT("Experienced_Calm");
    ScoutMentor.Personality = TEXT("Patient, Observant, Practical");
    ScoutMentor.KnownTopics.Add(TEXT("Tracking"));
    ScoutMentor.KnownTopics.Add(TEXT("Survival"));
    ScoutMentor.KnownTopics.Add(TEXT("Animal Behavior"));
    ScoutMentor.TrustLevel = 0.8f;
    CharacterProfiles.Add(ScoutMentor);

    // Hunter Veteran
    FNarr_CharacterProfile HunterVeteran;
    HunterVeteran.CharacterName = TEXT("Hunter Veteran");
    HunterVeteran.VoiceType = TEXT("Gruff_Experienced");
    HunterVeteran.Personality = TEXT("Tough, Direct, Battle-hardened");
    HunterVeteran.KnownTopics.Add(TEXT("Combat Tactics"));
    HunterVeteran.KnownTopics.Add(TEXT("Weapon Crafting"));
    HunterVeteran.KnownTopics.Add(TEXT("Dinosaur Hunting"));
    HunterVeteran.TrustLevel = 0.6f;
    CharacterProfiles.Add(HunterVeteran);

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d character profiles"), CharacterProfiles.Num());
}

int32 UNarr_DialogueSystem::FindDialogueIndex(int32 DialogueID) const
{
    for (int32 i = 0; i < DialogueDatabase.Num(); i++)
    {
        if (i == DialogueID) // Using array index as ID for simplicity
        {
            return i;
        }
    }
    return -1;
}

int32 UNarr_DialogueSystem::FindCharacterIndex(const FString& CharacterName) const
{
    for (int32 i = 0; i < CharacterProfiles.Num(); i++)
    {
        if (CharacterProfiles[i].CharacterName == CharacterName)
        {
            return i;
        }
    }
    return -1;
}