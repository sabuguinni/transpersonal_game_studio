#include "Narr_CharacterDatabase.h"
#include "Engine/Engine.h"

FNarr_CharacterProfile UNarr_CharacterDatabase::GetCharacterByName(const FString& Name) const
{
    for (const FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.CharacterName.Equals(Name, ESearchCase::IgnoreCase))
        {
            return Character;
        }
    }
    
    // Return empty profile if not found
    return FNarr_CharacterProfile();
}

TArray<FNarr_DialogueLine> UNarr_CharacterDatabase::GetDialogueForCharacter(const FString& CharacterName) const
{
    TArray<FNarr_DialogueLine> CharacterDialogue;
    
    for (const FNarr_DialogueLine& Dialogue : DialogueDatabase)
    {
        if (Dialogue.SpeakerName.Equals(CharacterName, ESearchCase::IgnoreCase))
        {
            CharacterDialogue.Add(Dialogue);
        }
    }
    
    return CharacterDialogue;
}

TArray<FNarr_CharacterProfile> UNarr_CharacterDatabase::GetQuestGivers() const
{
    TArray<FNarr_CharacterProfile> QuestGivers;
    
    for (const FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.bIsQuestGiver)
        {
            QuestGivers.Add(Character);
        }
    }
    
    return QuestGivers;
}

void UNarr_CharacterDatabase::AddCharacter(const FNarr_CharacterProfile& NewCharacter)
{
    // Check if character already exists
    for (int32 i = 0; i < Characters.Num(); i++)
    {
        if (Characters[i].CharacterName.Equals(NewCharacter.CharacterName, ESearchCase::IgnoreCase))
        {
            // Update existing character
            Characters[i] = NewCharacter;
            return;
        }
    }
    
    // Add new character
    Characters.Add(NewCharacter);
}

void UNarr_CharacterDatabase::AddDialogueLine(const FNarr_DialogueLine& NewDialogue)
{
    DialogueDatabase.Add(NewDialogue);
}

void UNarr_CharacterDatabase::InitializeDefaultCharacters()
{
    Characters.Empty();
    DialogueDatabase.Empty();
    
    // Elder Thok - Main Quest Giver
    FNarr_CharacterProfile ElderThok;
    ElderThok.CharacterName = TEXT("Elder Thok");
    ElderThok.CharacterRole = TEXT("Tribal Elder & Quest Giver");
    ElderThok.BackgroundStory = TEXT("Ancient keeper of hunting traditions. Has survived countless seasons and knows the secrets of the valley. Guides young hunters on their first trials.");
    ElderThok.bIsQuestGiver = true;
    ElderThok.bIsTrader = false;
    ElderThok.AssociatedQuests.Add(TEXT("FirstHunt"));
    ElderThok.AssociatedQuests.Add(TEXT("AncientSecrets"));
    ElderThok.VoiceAudioPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410559011_SpiritGuide.mp3");
    
    ElderThok.DialogueLines.Add(TEXT("Greetings, survivor! I am Elder Thok, keeper of the ancient hunting grounds."));
    ElderThok.DialogueLines.Add(TEXT("The valley holds many dangers, but also great rewards for those who prove themselves."));
    ElderThok.DialogueLines.Add(TEXT("Your first trial awaits - bring me proof of your hunting skills."));
    
    AddCharacter(ElderThok);
    
    // Tribal Scout - Warning NPC
    FNarr_CharacterProfile TribalScout;
    TribalScout.CharacterName = TEXT("Kael the Scout");
    TribalScout.CharacterRole = TEXT("Territory Scout & Warning Guide");
    TribalScout.BackgroundStory = TEXT("Swift and alert hunter who patrols the dangerous territories. Has witnessed the great predators and knows their hunting patterns.");
    TribalScout.bIsQuestGiver = false;
    TribalScout.bIsTrader = false;
    TribalScout.VoiceAudioPath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410561031_TribalScout.mp3");
    
    TribalScout.DialogueLines.Add(TEXT("Danger lurks in the shadow valleys."));
    TribalScout.DialogueLines.Add(TEXT("The great predators hunt in packs - stay alert!"));
    TribalScout.DialogueLines.Add(TEXT("Trust your instincts, and never venture alone into their territory."));
    
    AddCharacter(TribalScout);
    
    // Create dialogue entries
    FNarr_DialogueLine ElderGreeting;
    ElderGreeting.SpeakerName = TEXT("Elder Thok");
    ElderGreeting.DialogueText = TEXT("Greetings, survivor! I am Elder Thok, keeper of the ancient hunting grounds. The valley holds many dangers, but also great rewards for those who prove themselves.");
    ElderGreeting.AudioFilePath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410559011_SpiritGuide.mp3");
    ElderGreeting.TriggerCondition = TEXT("FirstMeeting");
    ElderGreeting.bIsQuestRelated = true;
    AddDialogueLine(ElderGreeting);
    
    FNarr_DialogueLine ScoutWarning;
    ScoutWarning.SpeakerName = TEXT("Kael the Scout");
    ScoutWarning.DialogueText = TEXT("Danger lurks in the shadow valleys. The great predators hunt in packs - stay alert, trust your instincts, and never venture alone into their territory.");
    ScoutWarning.AudioFilePath = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781410561031_TribalScout.mp3");
    ScoutWarning.TriggerCondition = TEXT("NearDangerZone");
    ScoutWarning.bIsQuestRelated = false;
    AddDialogueLine(ScoutWarning);
    
    UE_LOG(LogTemp, Log, TEXT("Character Database initialized with %d characters and %d dialogue lines"), Characters.Num(), DialogueDatabase.Num());
}