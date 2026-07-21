#include "Narr_SurvivalStoryFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UNarr_SurvivalStoryFramework::UNarr_SurvivalStoryFramework()
{
    CurrentChapter = ENarr_StoryChapter::Awakening;
    CurrentMood = ENarr_StoryMood::Desperate;
    PlayerReputation = 0;
    CurrentStoryBeatID = TEXT("awakening_alone");
}

void UNarr_SurvivalStoryFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeStoryFramework();
    UE_LOG(LogTemp, Warning, TEXT("Survival Story Framework initialized"));
}

void UNarr_SurvivalStoryFramework::Deinitialize()
{
    StoryBeats.Empty();
    CompletedQuests.Empty();
    Super::Deinitialize();
}

void UNarr_SurvivalStoryFramework::InitializeStoryFramework()
{
    CreateDefaultStoryBeats();
    
    // Initialize tribal lore with rich survival-focused content
    TribalLore.TribeName = TEXT("Stone-Walker Tribe");
    TribalLore.OriginStory = TEXT("When the great fire-mountain spoke, our ancestors fled the burning lands. They learned to walk with the stone-tooth beasts, to read the sky-signs, and to find water where others see only death.");
    
    TribalLore.SacredPlaces.Empty();
    TribalLore.SacredPlaces.Add(TEXT("The First Cave - Where our ancestors sheltered from the great cold"));
    TribalLore.SacredPlaces.Add(TEXT("Blood-Stone Circle - Where hunters prove their worth"));
    TribalLore.SacredPlaces.Add(TEXT("The Whispering Springs - Sacred water source"));
    
    TribalLore.TabooActions.Empty();
    TribalLore.TabooActions.Add(TEXT("Never hunt alone when shadow-tooth cats prowl"));
    TribalLore.TabooActions.Add(TEXT("Never waste water during the dry moons"));
    TribalLore.TabooActions.Add(TEXT("Never turn back on a wounded beast"));
    
    TribalLore.SurvivalWisdom.Empty();
    TribalLore.SurvivalWisdom.Add(TEXT("Sharp-claw tracks in mud mean death follows close"));
    TribalLore.SurvivalWisdom.Add(TEXT("Green water brings belly-sickness and fever"));
    TribalLore.SurvivalWisdom.Add(TEXT("When birds fly silent, great predators hunt"));
    TribalLore.SurvivalWisdom.Add(TEXT("Fire dies in wet wood, but lives in dry bone"));
    
    TribalLore.LeadershipTradition = TEXT("Only those who survive the Great Hunt and prove their wisdom in the Circle of Elders may lead our people");
    
    CurrentMood = ENarr_StoryMood::Desperate;
    UE_LOG(LogTemp, Warning, TEXT("Story framework initialized with %d story beats"), StoryBeats.Num());
}

void UNarr_SurvivalStoryFramework::CreateDefaultStoryBeats()
{
    StoryBeats.Empty();
    
    // Chapter 1: Awakening
    FNarr_StoryBeat AwakeningBeat;
    AwakeningBeat.BeatID = TEXT("awakening_alone");
    AwakeningBeat.Chapter = ENarr_StoryChapter::Awakening;
    AwakeningBeat.Title = TEXT("Alone in the Wild");
    AwakeningBeat.Description = TEXT("You wake alone, injured, with no memory of how you came to be in this dangerous prehistoric world. Survival is your only priority.");
    AwakeningBeat.RequiredQuests.Empty();
    AwakeningBeat.UnlockedQuests.Add(TEXT("GatherWater"));
    AwakeningBeat.UnlockedQuests.Add(TEXT("FindFood"));
    AwakeningBeat.Mood = ENarr_StoryMood::Desperate;
    StoryBeats.Add(AwakeningBeat);
    
    FNarr_StoryBeat FirstShelterBeat;
    FirstShelterBeat.BeatID = TEXT("first_shelter");
    FirstShelterBeat.Chapter = ENarr_StoryChapter::Awakening;
    FirstShelterBeat.Title = TEXT("The First Shelter");
    FirstShelterBeat.Description = TEXT("Having secured water and food, you must now build shelter before the dangerous night falls.");
    FirstShelterBeat.RequiredQuests.Add(TEXT("GatherWater"));
    FirstShelterBeat.RequiredQuests.Add(TEXT("FindFood"));
    FirstShelterBeat.UnlockedQuests.Add(TEXT("BuildShelter"));
    FirstShelterBeat.UnlockedQuests.Add(TEXT("CraftTool"));
    FirstShelterBeat.Mood = ENarr_StoryMood::Cautious;
    StoryBeats.Add(FirstShelterBeat);
    
    // Chapter 2: First Contact
    FNarr_StoryBeat FirstContactBeat;
    FirstContactBeat.BeatID = TEXT("first_contact");
    FirstContactBeat.Chapter = ENarr_StoryChapter::FirstContact;
    FirstContactBeat.Title = TEXT("Voices in the Distance");
    FirstContactBeat.Description = TEXT("You hear human voices for the first time since awakening. Other survivors exist, but are they friend or foe?");
    FirstContactBeat.RequiredQuests.Add(TEXT("BuildShelter"));
    FirstContactBeat.UnlockedQuests.Add(TEXT("ExploreTerritory"));
    FirstContactBeat.Mood = ENarr_StoryMood::Cautious;
    StoryBeats.Add(FirstContactBeat);
    
    FNarr_StoryBeat TribalMeetingBeat;
    TribalMeetingBeat.BeatID = TEXT("tribal_meeting");
    TribalMeetingBeat.Chapter = ENarr_StoryChapter::FirstContact;
    TribalMeetingBeat.Title = TEXT("The Stone-Walker Tribe");
    TribalMeetingBeat.Description = TEXT("You encounter the Stone-Walker Tribe. Their elder speaks of ancient traditions and the harsh laws of survival.");
    TribalMeetingBeat.RequiredQuests.Add(TEXT("ExploreTerritory"));
    TribalMeetingBeat.UnlockedQuests.Add(TEXT("HuntPrey"));
    TribalMeetingBeat.Mood = ENarr_StoryMood::Hopeful;
    StoryBeats.Add(TribalMeetingBeat);
    
    // Chapter 3: Tribal Integration
    FNarr_StoryBeat ProvingWorthBeat;
    ProvingWorthBeat.BeatID = TEXT("proving_worth");
    ProvingWorthBeat.Chapter = ENarr_StoryChapter::TribalIntegration;
    ProvingWorthBeat.Title = TEXT("Proving Your Worth");
    ProvingWorthBeat.Description = TEXT("To join the tribe, you must prove you can contribute to their survival. The hunt will test your skills.");
    ProvingWorthBeat.RequiredQuests.Add(TEXT("HuntPrey"));
    ProvingWorthBeat.UnlockedQuests.Add(TEXT("DefendCamp"));
    ProvingWorthBeat.Mood = ENarr_StoryMood::Cautious;
    StoryBeats.Add(ProvingWorthBeat);
    
    // Chapter 4: Territory Expansion
    FNarr_StoryBeat NewLandsBeat;
    NewLandsBeat.BeatID = TEXT("new_lands");
    NewLandsBeat.Chapter = ENarr_StoryChapter::TerritoryExpansion;
    NewLandsBeat.Title = TEXT("Beyond the Known");
    NewLandsBeat.Description = TEXT("The tribe speaks of rich hunting grounds beyond the great river, but also of terrible predators that rule those lands.");
    NewLandsBeat.RequiredQuests.Add(TEXT("DefendCamp"));
    NewLandsBeat.UnlockedQuests.Add(TEXT("EscapePredator"));
    NewLandsBeat.Mood = ENarr_StoryMood::Contemplative;
    StoryBeats.Add(NewLandsBeat);
    
    UE_LOG(LogTemp, Warning, TEXT("Created %d default story beats"), StoryBeats.Num());
}

void UNarr_SurvivalStoryFramework::AdvanceStoryBeat(const FString& BeatID)
{
    for (FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == BeatID)
        {
            Beat.bIsCompleted = true;
            CurrentStoryBeatID = BeatID;
            CheckChapterProgression();
            UE_LOG(LogTemp, Warning, TEXT("Advanced to story beat: %s"), *BeatID);
            return;
        }
    }
    UE_LOG(LogTemp, Error, TEXT("Story beat not found: %s"), *BeatID);
}

ENarr_StoryChapter UNarr_SurvivalStoryFramework::GetCurrentChapter() const
{
    return CurrentChapter;
}

FNarr_StoryBeat UNarr_SurvivalStoryFramework::GetCurrentStoryBeat() const
{
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.BeatID == CurrentStoryBeatID)
        {
            return Beat;
        }
    }
    
    // Return first beat if current not found
    if (StoryBeats.Num() > 0)
    {
        return StoryBeats[0];
    }
    
    return FNarr_StoryBeat();
}

TArray<FNarr_StoryBeat> UNarr_SurvivalStoryFramework::GetAvailableStoryBeats() const
{
    TArray<FNarr_StoryBeat> AvailableBeats;
    
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (!Beat.bIsCompleted)
        {
            bool bRequirementsMet = true;
            for (const FString& RequiredQuest : Beat.RequiredQuests)
            {
                if (!CompletedQuests.Contains(RequiredQuest))
                {
                    bRequirementsMet = false;
                    break;
                }
            }
            
            if (bRequirementsMet)
            {
                AvailableBeats.Add(Beat);
            }
        }
    }
    
    return AvailableBeats;
}

void UNarr_SurvivalStoryFramework::OnQuestCompleted(const FString& QuestID)
{
    if (!CompletedQuests.Contains(QuestID))
    {
        CompletedQuests.Add(QuestID);
        UE_LOG(LogTemp, Warning, TEXT("Quest completed for story: %s"), *QuestID);
        
        // Update player reputation based on quest type
        if (QuestID.Contains(TEXT("Hunt")) || QuestID.Contains(TEXT("Defend")))
        {
            UpdatePlayerReputation(15); // Combat quests give more reputation
        }
        else if (QuestID.Contains(TEXT("Gather")) || QuestID.Contains(TEXT("Craft")))
        {
            UpdatePlayerReputation(10); // Survival quests give moderate reputation
        }
        else
        {
            UpdatePlayerReputation(5); // Other quests give basic reputation
        }
        
        CheckChapterProgression();
    }
}

bool UNarr_SurvivalStoryFramework::ShouldUnlockQuest(const FString& QuestID) const
{
    for (const FNarr_StoryBeat& Beat : StoryBeats)
    {
        if (Beat.UnlockedQuests.Contains(QuestID))
        {
            // Check if this beat's requirements are met
            bool bRequirementsMet = true;
            for (const FString& RequiredQuest : Beat.RequiredQuests)
            {
                if (!CompletedQuests.Contains(RequiredQuest))
                {
                    bRequirementsMet = false;
                    break;
                }
            }
            return bRequirementsMet;
        }
    }
    return false;
}

void UNarr_SurvivalStoryFramework::UpdatePlayerReputation(int32 ReputationChange)
{
    PlayerReputation += ReputationChange;
    PlayerReputation = FMath::Clamp(PlayerReputation, -100, 100);
    
    UE_LOG(LogTemp, Warning, TEXT("Player reputation updated: %d (change: %d)"), PlayerReputation, ReputationChange);
}

bool UNarr_SurvivalStoryFramework::CanBecomeTribalLeader() const
{
    return PlayerReputation >= 75 && CompletedQuests.Num() >= 8;
}

void UNarr_SurvivalStoryFramework::CheckChapterProgression()
{
    int32 CompletedQuestCount = CompletedQuests.Num();
    
    if (CompletedQuestCount >= 2 && CurrentChapter == ENarr_StoryChapter::Awakening)
    {
        CurrentChapter = ENarr_StoryChapter::FirstContact;
        CurrentMood = ENarr_StoryMood::Cautious;
        UE_LOG(LogTemp, Warning, TEXT("Advanced to Chapter: First Contact"));
    }
    else if (CompletedQuestCount >= 4 && CurrentChapter == ENarr_StoryChapter::FirstContact)
    {
        CurrentChapter = ENarr_StoryChapter::TribalIntegration;
        CurrentMood = ENarr_StoryMood::Hopeful;
        UE_LOG(LogTemp, Warning, TEXT("Advanced to Chapter: Tribal Integration"));
    }
    else if (CompletedQuestCount >= 6 && CurrentChapter == ENarr_StoryChapter::TribalIntegration)
    {
        CurrentChapter = ENarr_StoryChapter::TerritoryExpansion;
        CurrentMood = ENarr_StoryMood::Contemplative;
        UE_LOG(LogTemp, Warning, TEXT("Advanced to Chapter: Territory Expansion"));
    }
    else if (CanBecomeTribalLeader() && CurrentChapter == ENarr_StoryChapter::TerritoryExpansion)
    {
        CurrentChapter = ENarr_StoryChapter::AlphaChallenge;
        CurrentMood = ENarr_StoryMood::Triumphant;
        UE_LOG(LogTemp, Warning, TEXT("Advanced to Chapter: Alpha Challenge"));
    }
}

FString UNarr_SurvivalStoryFramework::GetContextualNarration(const FString& Context) const
{
    return GenerateContextualDialogue(Context, CurrentMood);
}

ENarr_StoryMood UNarr_SurvivalStoryFramework::GetCurrentMood() const
{
    return CurrentMood;
}

void UNarr_SurvivalStoryFramework::SetStoryMood(ENarr_StoryMood NewMood)
{
    CurrentMood = NewMood;
    UE_LOG(LogTemp, Warning, TEXT("Story mood changed to: %d"), (int32)NewMood);
}

FString UNarr_SurvivalStoryFramework::GenerateContextualDialogue(const FString& Context, ENarr_StoryMood Mood) const
{
    FString BaseDialogue;
    
    if (Context.Contains(TEXT("water")))
    {
        switch (Mood)
        {
        case ENarr_StoryMood::Desperate:
            BaseDialogue = TEXT("The water grows scarce. Without it, we are nothing but bones in the sun.");
            break;
        case ENarr_StoryMood::Cautious:
            BaseDialogue = TEXT("This water runs clear, but taste it slowly. Strange waters have killed stronger warriors than you.");
            break;
        case ENarr_StoryMood::Hopeful:
            BaseDialogue = TEXT("Fresh water! The spirits smile upon us today. Fill every vessel we have.");
            break;
        default:
            BaseDialogue = TEXT("Water is life. Guard it as you would your own blood.");
        }
    }
    else if (Context.Contains(TEXT("hunt")))
    {
        switch (Mood)
        {
        case ENarr_StoryMood::Desperate:
            BaseDialogue = TEXT("The hunt must succeed. Our children grow thin, and winter approaches with hungry jaws.");
            break;
        case ENarr_StoryMood::Cautious:
            BaseDialogue = TEXT("Move silent as shadow. The prey has sharp ears, and predators sharper teeth.");
            break;
        case ENarr_StoryMood::Triumphant:
            BaseDialogue = TEXT("A successful hunt! Tonight we feast, and tomorrow we are stronger.");
            break;
        default:
            BaseDialogue = TEXT("The hunt teaches patience, courage, and respect for all life.");
        }
    }
    else if (Context.Contains(TEXT("danger")))
    {
        switch (Mood)
        {
        case ENarr_StoryMood::Desperate:
            BaseDialogue = TEXT("Death stalks us with every breath. Stay close, stay alert, stay alive.");
            break;
        case ENarr_StoryMood::Cautious:
            BaseDialogue = TEXT("Something moves in the shadows. Trust your instincts - they have kept our people alive.");
            break;
        default:
            BaseDialogue = TEXT("Danger is the price of survival. Face it with courage, but never with foolishness.");
        }
    }
    else
    {
        BaseDialogue = TEXT("The old ways teach us: survive today, plan for tomorrow, remember yesterday.");
    }
    
    return BaseDialogue;
}