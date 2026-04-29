#include "NarrativeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarrativeManager::UNarrativeManager()
{
    bIsDialogueActive = false;
    CurrentCharacterID = TEXT("");
    CurrentDialogueIndex = 0;
}

void UNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initializing narrative system..."));
    
    InitializeDefaultStory();
    InitializeDefaultCharacters();
    InitializeDefaultLore();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialization complete"));
}

void UNarrativeManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Shutting down narrative system"));
    
    StoryFlags.Empty();
    StoryBeats.Empty();
    Characters.Empty();
    LoreDatabase.Empty();
    CurrentDialogueSet.Empty();
    
    Super::Deinitialize();
}

void UNarrativeManager::StartDialogue(const FString& CharacterID, const FString& DialogueSetID)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Dialogue already active, ending current dialogue"));
        EndDialogue();
    }
    
    if (!Characters.Contains(CharacterID))
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeManager: Character %s not found"), *CharacterID);
        return;
    }
    
    // For now, create a simple dialogue set based on character
    CurrentDialogueSet.Empty();
    CurrentCharacterID = CharacterID;
    CurrentDialogueIndex = 0;
    
    // Create sample dialogue based on character type
    if (CharacterID == TEXT("tribal_elder"))
    {
        FNarr_DialogueEntry Entry1;
        Entry1.SpeakerName = TEXT("Tribal Elder");
        Entry1.DialogueText = FText::FromString(TEXT("Listen well, child. The great hunters have territories marked by scent and claw marks on trees."));
        Entry1.Duration = 5.0f;
        CurrentDialogueSet.Add(Entry1);
        
        FNarr_DialogueEntry Entry2;
        Entry2.SpeakerName = TEXT("Tribal Elder");
        Entry2.DialogueText = FText::FromString(TEXT("Never camp where you see deep scratches in bark, or where the ground is littered with bones."));
        Entry2.Duration = 4.0f;
        CurrentDialogueSet.Add(Entry2);
    }
    else if (CharacterID == TEXT("veteran_hunter"))
    {
        FNarr_DialogueEntry Entry1;
        Entry1.SpeakerName = TEXT("Veteran Hunter");
        Entry1.DialogueText = FText::FromString(TEXT("The meat-eaters have changed their hunting patterns. The three-claws now hunt in larger packs."));
        Entry1.Duration = 4.5f;
        CurrentDialogueSet.Add(Entry1);
    }
    else if (CharacterID == TEXT("young_scout"))
    {
        FNarr_DialogueEntry Entry1;
        Entry1.SpeakerName = TEXT("Young Scout");
        Entry1.DialogueText = FText::FromString(TEXT("The giant long-necks are moving south! Their herd stretches beyond the horizon!"));
        Entry1.Duration = 4.0f;
        CurrentDialogueSet.Add(Entry1);
    }
    
    if (CurrentDialogueSet.Num() > 0)
    {
        bIsDialogueActive = true;
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Started dialogue with %s (%d lines)"), *CharacterID, CurrentDialogueSet.Num());
    }
}

void UNarrativeManager::EndDialogue()
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Ending dialogue with %s"), *CurrentCharacterID);
        
        bIsDialogueActive = false;
        CurrentCharacterID = TEXT("");
        CurrentDialogueSet.Empty();
        CurrentDialogueIndex = 0;
    }
}

FNarr_DialogueEntry UNarrativeManager::GetNextDialogueLine()
{
    if (!bIsDialogueActive || CurrentDialogueIndex >= CurrentDialogueSet.Num())
    {
        return FNarr_DialogueEntry(); // Return default empty entry
    }
    
    FNarr_DialogueEntry CurrentEntry = CurrentDialogueSet[CurrentDialogueIndex];
    CurrentDialogueIndex++;
    
    // Check if we've reached the end
    if (CurrentDialogueIndex >= CurrentDialogueSet.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Dialogue sequence completed"));
    }
    
    return CurrentEntry;
}

void UNarrativeManager::TriggerStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID && !Beat.bIsCompleted)
        {
            // Check trigger conditions
            bool bCanTrigger = true;
            for (const FString& Condition : Beat.TriggerConditions)
            {
                if (!GetStoryFlag(Condition))
                {
                    bCanTrigger = false;
                    break;
                }
            }
            
            if (bCanTrigger)
            {
                Beat.bIsCompleted = true;
                
                // Set completion flags
                for (const FString& Flag : Beat.CompletionFlags)
                {
                    SetStoryFlag(Flag, true);
                }
                
                UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Story beat '%s' triggered"), *BeatID);
                return;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Story beat '%s' not found or already completed"), *BeatID);
}

bool UNarrativeManager::IsStoryBeatCompleted(const FString& BeatID) const
{
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            return Beat.bIsCompleted;
        }
    }
    return false;
}

void UNarrativeManager::SetStoryFlag(const FString& FlagName, bool bValue)
{
    StoryFlags.Add(FlagName, bValue);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Set story flag '%s' to %s"), *FlagName, bValue ? TEXT("true") : TEXT("false"));
}

bool UNarrativeManager::GetStoryFlag(const FString& FlagName) const
{
    if (const bool* Flag = StoryFlags.Find(FlagName))
    {
        return *Flag;
    }
    return false; // Default to false for unknown flags
}

void UNarrativeManager::RegisterCharacter(const FNarr_CharacterProfile& Character)
{
    Characters.Add(Character.CharacterID, Character);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Registered character '%s'"), *Character.CharacterID);
}

FNarr_CharacterProfile UNarrativeManager::GetCharacterProfile(const FString& CharacterID) const
{
    if (const FNarr_CharacterProfile* Profile = Characters.Find(CharacterID))
    {
        return *Profile;
    }
    return FNarr_CharacterProfile(); // Return default profile
}

void UNarrativeManager::UpdateCharacterTrust(const FString& CharacterID, float DeltaTrust)
{
    if (FNarr_CharacterProfile* Profile = Characters.Find(CharacterID))
    {
        Profile->TrustLevel = FMath::Clamp(Profile->TrustLevel + DeltaTrust, 0.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Updated trust for '%s' to %.2f"), *CharacterID, Profile->TrustLevel);
    }
}

void UNarrativeManager::AddLoreEntry(const FString& Category, const FString& Title, const FText& Content)
{
    if (!LoreDatabase.Contains(Category))
    {
        LoreDatabase.Add(Category, TMap<FString, FText>());
    }
    
    LoreDatabase[Category].Add(Title, Content);
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Added lore entry '%s' to category '%s'"), *Title, *Category);
}

TArray<FString> UNarrativeManager::GetLoreCategories() const
{
    TArray<FString> Categories;
    LoreDatabase.GetKeys(Categories);
    return Categories;
}

FText UNarrativeManager::GetLoreEntry(const FString& Category, const FString& Title) const
{
    if (const TMap<FString, FText>* CategoryMap = LoreDatabase.Find(Category))
    {
        if (const FText* Entry = CategoryMap->Find(Title))
        {
            return *Entry;
        }
    }
    return FText::FromString(TEXT("Lore entry not found"));
}

void UNarrativeManager::InitializeDefaultStory()
{
    // Create initial story beats
    FNarr_StoryBeat Beat1;
    Beat1.BeatID = TEXT("first_awakening");
    Beat1.Title = FText::FromString(TEXT("The Awakening"));
    Beat1.Description = FText::FromString(TEXT("You awaken in an unknown prehistoric world"));
    Beat1.CompletionFlags.Add(TEXT("player_awakened"));
    StoryBeats.Add(Beat1);
    
    FNarr_StoryBeat Beat2;
    Beat2.BeatID = TEXT("first_dinosaur_encounter");
    Beat2.Title = FText::FromString(TEXT("First Contact"));
    Beat2.Description = FText::FromString(TEXT("Your first encounter with the ancient rulers of this world"));
    Beat2.TriggerConditions.Add(TEXT("player_awakened"));
    Beat2.CompletionFlags.Add(TEXT("seen_dinosaur"));
    StoryBeats.Add(Beat2);
    
    FNarr_StoryBeat Beat3;
    Beat3.BeatID = TEXT("tribal_contact");
    Beat3.Title = FText::FromString(TEXT("The Tribe"));
    Beat3.Description = FText::FromString(TEXT("You discover you are not alone in this world"));
    Beat3.TriggerConditions.Add(TEXT("seen_dinosaur"));
    Beat3.CompletionFlags.Add(TEXT("met_tribe"));
    StoryBeats.Add(Beat3);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized %d story beats"), StoryBeats.Num());
}

void UNarrativeManager::InitializeDefaultCharacters()
{
    // Tribal Elder
    FNarr_CharacterProfile Elder;
    Elder.CharacterID = TEXT("tribal_elder");
    Elder.CharacterName = FText::FromString(TEXT("Korah the Wise"));
    Elder.BackgroundStory = FText::FromString(TEXT("An ancient storyteller who has survived countless seasons and knows the ways of the great beasts"));
    Elder.PersonalityTraits.Add(TEXT("wise"));
    Elder.PersonalityTraits.Add(TEXT("cautious"));
    Elder.PersonalityTraits.Add(TEXT("protective"));
    Elder.TrustLevel = 0.7f;
    RegisterCharacter(Elder);
    
    // Veteran Hunter
    FNarr_CharacterProfile Hunter;
    Hunter.CharacterID = TEXT("veteran_hunter");
    Hunter.CharacterName = FText::FromString(TEXT("Thane the Tracker"));
    Hunter.BackgroundStory = FText::FromString(TEXT("A grizzled hunter who has faced the great predators and lived to tell the tale"));
    Hunter.PersonalityTraits.Add(TEXT("brave"));
    Hunter.PersonalityTraits.Add(TEXT("practical"));
    Hunter.PersonalityTraits.Add(TEXT("direct"));
    Hunter.TrustLevel = 0.5f;
    RegisterCharacter(Hunter);
    
    // Young Scout
    FNarr_CharacterProfile Scout;
    Scout.CharacterID = TEXT("young_scout");
    Scout.CharacterName = FText::FromString(TEXT("Aya the Swift"));
    Scout.BackgroundStory = FText::FromString(TEXT("A young and eager scout who serves as the tribe's eyes and ears"));
    Scout.PersonalityTraits.Add(TEXT("energetic"));
    Scout.PersonalityTraits.Add(TEXT("curious"));
    Scout.PersonalityTraits.Add(TEXT("loyal"));
    Scout.TrustLevel = 0.6f;
    RegisterCharacter(Scout);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized %d characters"), Characters.Num());
}

void UNarrativeManager::InitializeDefaultLore()
{
    // Dinosaur Lore
    AddLoreEntry(TEXT("Dinosaurs"), TEXT("Thunder Walker"), 
        FText::FromString(TEXT("The massive long-necked giants whose footsteps shake the earth. They travel in great herds and are generally peaceful, but their sheer size makes them dangerous.")));
    
    AddLoreEntry(TEXT("Dinosaurs"), TEXT("Three-Claws"), 
        FText::FromString(TEXT("Swift and intelligent pack hunters with razor-sharp claws. They hunt in coordinated groups and have been observed using complex strategies.")));
    
    AddLoreEntry(TEXT("Dinosaurs"), TEXT("King Predator"), 
        FText::FromString(TEXT("The apex predator of this world. A massive carnivore with bone-crushing jaws and an intelligence that rivals the greatest hunters.")));
    
    // Survival Lore
    AddLoreEntry(TEXT("Survival"), TEXT("Territory Markers"), 
        FText::FromString(TEXT("Great predators mark their territory with claw marks on trees and scent markings. Learning to read these signs can save your life.")));
    
    AddLoreEntry(TEXT("Survival"), TEXT("Safe Camping"), 
        FText::FromString(TEXT("Never camp in areas with scattered bones, deep claw marks, or strong predator scents. High ground with multiple escape routes is always preferable.")));
    
    AddLoreEntry(TEXT("Survival"), TEXT("Water Sources"), 
        FText::FromString(TEXT("All creatures need water, making water sources both vital and dangerous. Approach with extreme caution, especially during dawn and dusk.")));
    
    // Tribal Lore
    AddLoreEntry(TEXT("Tribe"), TEXT("The Ancient Pact"), 
        FText::FromString(TEXT("Long ago, the first humans learned to coexist with the great beasts through respect, wisdom, and careful observation of their ways.")));
    
    AddLoreEntry(TEXT("Tribe"), TEXT("The Great Migration"), 
        FText::FromString(TEXT("Every few seasons, the great herds migrate across the land, followed by predators. These times are both dangerous and full of opportunity.")));
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeManager: Initialized lore database with %d categories"), LoreDatabase.Num());
}

bool UNarrativeManager::CheckDialogueConditions(const FNarr_DialogueEntry& Entry) const
{
    for (const FString& Flag : Entry.RequiredFlags)
    {
        if (!GetStoryFlag(Flag))
        {
            return false;
        }
    }
    return true;
}

void UNarrativeManager::ApplyDialogueEffects(const FNarr_DialogueEntry& Entry)
{
    for (const FString& Flag : Entry.SetFlags)
    {
        SetStoryFlag(Flag, true);
    }
}