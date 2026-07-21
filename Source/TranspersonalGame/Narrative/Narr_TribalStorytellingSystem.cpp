#include "Narr_TribalStorytellingSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"

UNarr_TribalStorytellingComponent::UNarr_TribalStorytellingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    StorytellingRange = 800.0f;
    bCanTellStories = true;
    bIsCurrentlyTellingStory = false;
    LastStoryTime = 0.0f;
    
    // Initialize with basic survival stories
    FNarr_StorySegment BasicSurvivalStory;
    BasicSurvivalStory.StoryTitle = TEXT("The First Hunt");
    BasicSurvivalStory.NarratorName = TEXT("Elder Kava");
    BasicSurvivalStory.StoryContent = TEXT("When the great beasts ruled the earth, our ancestors learned to hunt with cunning, not strength.");
    BasicSurvivalStory.EmotionalIntensity = 0.7f;
    BasicSurvivalStory.RequiredSurvivalSkills.Add(TEXT("Hunting"));
    BasicSurvivalStory.RequiredSurvivalSkills.Add(TEXT("Stealth"));
    BasicSurvivalStory.bIsLegendaryTale = false;
    
    AvailableStories.Add(BasicSurvivalStory);
}

void UNarr_TribalStorytellingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize character profile based on component owner
    if (AActor* Owner = GetOwner())
    {
        CharacterProfile.CharacterName = Owner->GetName();
        CharacterProfile.Age = FMath::RandRange(25, 60);
        CharacterProfile.SurvivalExperience = FMath::RandRange(5, 40);
        CharacterProfile.TrustLevel = 0.5f;
        
        // Add specializations based on character type
        if (Owner->GetName().Contains(TEXT("Elder")))
        {
            CharacterProfile.Specializations.Add(TEXT("Wisdom"));
            CharacterProfile.Specializations.Add(TEXT("Leadership"));
            CharacterProfile.bIsTribalElder = true;
        }
        else if (Owner->GetName().Contains(TEXT("Scout")))
        {
            CharacterProfile.Specializations.Add(TEXT("Tracking"));
            CharacterProfile.Specializations.Add(TEXT("Navigation"));
        }
        else if (Owner->GetName().Contains(TEXT("Craft")))
        {
            CharacterProfile.Specializations.Add(TEXT("Tool Making"));
            CharacterProfile.Specializations.Add(TEXT("Shelter Building"));
        }
    }
}

void UNarr_TribalStorytellingComponent::StartStorytelling()
{
    if (!bCanTellStories || bIsCurrentlyTellingStory)
    {
        return;
    }
    
    bIsCurrentlyTellingStory = true;
    LastStoryTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("%s begins telling a story"), *CharacterProfile.CharacterName);
}

void UNarr_TribalStorytellingComponent::EndStorytelling()
{
    bIsCurrentlyTellingStory = false;
    UE_LOG(LogTemp, Log, TEXT("%s finishes their story"), *CharacterProfile.CharacterName);
}

FNarr_StorySegment UNarr_TribalStorytellingComponent::GetRandomStory()
{
    if (AvailableStories.Num() == 0)
    {
        FNarr_StorySegment EmptyStory;
        EmptyStory.StoryTitle = TEXT("Silence");
        EmptyStory.StoryContent = TEXT("The elder sits in contemplative silence.");
        return EmptyStory;
    }
    
    int32 RandomIndex = FMath::RandRange(0, AvailableStories.Num() - 1);
    return AvailableStories[RandomIndex];
}

bool UNarr_TribalStorytellingComponent::CanPlayerHearStory(AActor* PlayerActor)
{
    if (!PlayerActor || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    return Distance <= StorytellingRange && bIsCurrentlyTellingStory;
}

ANarr_TribalStorytellingSystem::ANarr_TribalStorytellingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    CampfireLocation = FVector(500.0f, 500.0f, 100.0f);
    GatheringRadius = 1000.0f;
    bStorytellingActive = false;
    LastGatheringTime = 0.0f;
}

void ANarr_TribalStorytellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalCharacters();
    CreateTribalCharacters();
}

void ANarr_TribalStorytellingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateStorytellingState();
}

void ANarr_TribalStorytellingSystem::InitializeTribalCharacters()
{
    TribalMembers.Empty();
    
    // Elder Kava Stormwatcher
    FNarr_TribalCharacterProfile ElderKava;
    ElderKava.CharacterName = TEXT("Elder Kava Stormwatcher");
    ElderKava.Age = 58;
    ElderKava.SurvivalExperience = 40;
    ElderKava.Specializations.Add(TEXT("Weather Prediction"));
    ElderKava.Specializations.Add(TEXT("Tribal Leadership"));
    ElderKava.Specializations.Add(TEXT("Ancient Knowledge"));
    ElderKava.PersonalityTraits = TEXT("Wise, Patient, Protective of tribe");
    ElderKava.TrustLevel = 0.9f;
    ElderKava.bIsTribalElder = true;
    
    // Add Elder Kava's stories
    FNarr_StorySegment KavaStory;
    KavaStory.StoryTitle = TEXT("The Great Storm Migration");
    KavaStory.NarratorName = TEXT("Elder Kava Stormwatcher");
    KavaStory.StoryContent = TEXT("When the storm clouds gather like angry beasts, follow the ancient paths. The ancestors carved these routes through seasons of survival.");
    KavaStory.EmotionalIntensity = 0.8f;
    KavaStory.RequiredSurvivalSkills.Add(TEXT("Weather Reading"));
    KavaStory.RequiredSurvivalSkills.Add(TEXT("Navigation"));
    KavaStory.bIsLegendaryTale = true;
    ElderKava.KnownStories.Add(KavaStory);
    
    TribalMembers.Add(ElderKava);
    
    // Scout Thane Pathfinder
    FNarr_TribalCharacterProfile ScoutThane;
    ScoutThane.CharacterName = TEXT("Scout Thane Pathfinder");
    ScoutThane.Age = 32;
    ScoutThane.SurvivalExperience = 18;
    ScoutThane.Specializations.Add(TEXT("Tracking"));
    ScoutThane.Specializations.Add(TEXT("Route Finding"));
    ScoutThane.Specializations.Add(TEXT("Danger Assessment"));
    ScoutThane.PersonalityTraits = TEXT("Alert, Cautious, Quick-thinking");
    ScoutThane.TrustLevel = 0.8f;
    ScoutThane.bIsTribalElder = false;
    
    // Add Scout Thane's stories
    FNarr_StorySegment ThaneStory;
    ThaneStory.StoryTitle = TEXT("The Hidden Watering Holes");
    ThaneStory.NarratorName = TEXT("Scout Thane Pathfinder");
    ThaneStory.StoryContent = TEXT("The great beasts know where water flows. Watch their tracks, but never follow too close. Death walks behind the thirsty.");
    ThaneStory.EmotionalIntensity = 0.7f;
    ThaneStory.RequiredSurvivalSkills.Add(TEXT("Tracking"));
    ThaneStory.RequiredSurvivalSkills.Add(TEXT("Water Finding"));
    ThaneStory.bIsLegendaryTale = false;
    ScoutThane.KnownStories.Add(ThaneStory);
    
    TribalMembers.Add(ScoutThane);
    
    // Craft Master Nira Stonehand
    FNarr_TribalCharacterProfile CraftMasterNira;
    CraftMasterNira.CharacterName = TEXT("Craft Master Nira Stonehand");
    CraftMasterNira.Age = 45;
    CraftMasterNira.SurvivalExperience = 30;
    CraftMasterNira.Specializations.Add(TEXT("Stone Tool Crafting"));
    CraftMasterNira.Specializations.Add(TEXT("Shelter Construction"));
    CraftMasterNira.Specializations.Add(TEXT("Fire Making"));
    CraftMasterNira.PersonalityTraits = TEXT("Methodical, Patient, Perfectionist");
    CraftMasterNira.TrustLevel = 0.85f;
    CraftMasterNira.bIsTribalElder = false;
    
    // Add Craft Master Nira's stories
    FNarr_StorySegment NiraStory;
    NiraStory.StoryTitle = TEXT("The First Blade");
    NiraStory.NarratorName = TEXT("Craft Master Nira Stonehand");
    NiraStory.StoryContent = TEXT("Stone holds memory of the earth. Shape it with respect, not force. A blade made in anger will shatter when hope is needed most.");
    NiraStory.EmotionalIntensity = 0.6f;
    NiraStory.RequiredSurvivalSkills.Add(TEXT("Tool Crafting"));
    NiraStory.RequiredSurvivalSkills.Add(TEXT("Resource Gathering"));
    NiraStory.bIsLegendaryTale = false;
    CraftMasterNira.KnownStories.Add(NiraStory);
    
    TribalMembers.Add(CraftMasterNira);
}

void ANarr_TribalStorytellingSystem::StartTribalGathering()
{
    if (bStorytellingActive)
    {
        return;
    }
    
    bStorytellingActive = true;
    LastGatheringTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Tribal gathering begins around the campfire"));
    
    // Notify all tribal members to start storytelling
    for (const FNarr_TribalCharacterProfile& Member : TribalMembers)
    {
        UE_LOG(LogTemp, Log, TEXT("%s joins the gathering"), *Member.CharacterName);
    }
}

void ANarr_TribalStorytellingSystem::EndTribalGathering()
{
    bStorytellingActive = false;
    UE_LOG(LogTemp, Log, TEXT("Tribal gathering ends, members return to their duties"));
}

FNarr_TribalCharacterProfile ANarr_TribalStorytellingSystem::GetElderKava()
{
    for (const FNarr_TribalCharacterProfile& Member : TribalMembers)
    {
        if (Member.CharacterName.Contains(TEXT("Elder Kava")))
        {
            return Member;
        }
    }
    
    return FNarr_TribalCharacterProfile();
}

FNarr_TribalCharacterProfile ANarr_TribalStorytellingSystem::GetScoutThane()
{
    for (const FNarr_TribalCharacterProfile& Member : TribalMembers)
    {
        if (Member.CharacterName.Contains(TEXT("Scout Thane")))
        {
            return Member;
        }
    }
    
    return FNarr_TribalCharacterProfile();
}

FNarr_TribalCharacterProfile ANarr_TribalStorytellingSystem::GetCraftMasterNira()
{
    for (const FNarr_TribalCharacterProfile& Member : TribalMembers)
    {
        if (Member.CharacterName.Contains(TEXT("Craft Master Nira")))
        {
            return Member;
        }
    }
    
    return FNarr_TribalCharacterProfile();
}

void ANarr_TribalStorytellingSystem::CreateTribalCharacters()
{
    UE_LOG(LogTemp, Log, TEXT("Tribal storytelling system initialized with %d characters"), TribalMembers.Num());
}

void ANarr_TribalStorytellingSystem::UpdateStorytellingState()
{
    if (!bStorytellingActive)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float GatheringDuration = CurrentTime - LastGatheringTime;
    
    // Auto-end gathering after 10 minutes of game time
    if (GatheringDuration > 600.0f)
    {
        EndTribalGathering();
    }
}