#include "Narr_StorytellingSystem.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalCharacter.h"
#include "Engine/Engine.h"

ANarr_StorytellingSystem::ANarr_StorytellingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    PlayerDetectionRadius = 1000.0f;
    StoryTriggerCooldown = 30.0f;
    LastStoryTime = -1000.0f;
    PlayerCharacter = nullptr;
}

void ANarr_StorytellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize story database with default stories
    InitializeStoryDatabase();
    
    // Find player character
    PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    
    if (PlayerCharacter)
    {
        UE_LOG(LogTemp, Log, TEXT("StorytellingSystem: Found player character"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("StorytellingSystem: Player character not found"));
    }
}

void ANarr_StorytellingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check for player proximity to trigger contextual stories
    CheckPlayerProximity();
}

void ANarr_StorytellingSystem::InitializeStoryDatabase()
{
    StoryDatabase.Empty();
    
    // Warning stories
    FNarr_StoryData WarningStory;
    WarningStory.StoryID = TEXT("Warning_Predator");
    WarningStory.StoryType = ENarr_StoryType::Warning;
    WarningStory.DialogueText = TEXT("The winds carry whispers of danger, wanderer. The great predator stalks these hunting grounds.");
    WarningStory.TriggerRadius = 800.0f;
    WarningStory.bPlayOnce = false;
    StoryDatabase.Add(WarningStory);
    
    // Wisdom stories
    FNarr_StoryData WisdomStory;
    WisdomStory.StoryID = TEXT("Wisdom_Ancient");
    WisdomStory.StoryType = ENarr_StoryType::Wisdom;
    WisdomStory.DialogueText = TEXT("The bones of the earth speak of ancient hunts. Listen to the wind, survivor.");
    WisdomStory.TriggerRadius = 600.0f;
    WisdomStory.bPlayOnce = true;
    StoryDatabase.Add(WisdomStory);
    
    // Aftermath stories
    FNarr_StoryData AftermathStory;
    AftermathStory.StoryID = TEXT("Aftermath_Battle");
    AftermathStory.StoryType = ENarr_StoryType::Aftermath;
    AftermathStory.DialogueText = TEXT("The river runs red with the blood of the fallen. Seek shelter in the high caves.");
    AftermathStory.TriggerRadius = 700.0f;
    AftermathStory.bPlayOnce = false;
    StoryDatabase.Add(AftermathStory);
    
    // Migration stories
    FNarr_StoryData MigrationStory;
    MigrationStory.StoryID = TEXT("Migration_Herds");
    MigrationStory.StoryType = ENarr_StoryType::Migration;
    MigrationStory.DialogueText = TEXT("The thunder of hooves shakes the ground. The great migration begins.");
    MigrationStory.TriggerRadius = 1000.0f;
    MigrationStory.bPlayOnce = false;
    StoryDatabase.Add(MigrationStory);
    
    UE_LOG(LogTemp, Log, TEXT("StorytellingSystem: Initialized %d stories"), StoryDatabase.Num());
}

void ANarr_StorytellingSystem::CheckPlayerProximity()
{
    if (!PlayerCharacter || !CanTriggerStory())
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    FVector MyLocation = GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, MyLocation);
    
    if (Distance <= PlayerDetectionRadius)
    {
        // Determine story type based on context
        ENarr_StoryType ContextualType = ENarr_StoryType::Lore;
        
        // Simple context detection based on player state
        if (PlayerCharacter->GetHealthPercentage() < 0.3f)
        {
            ContextualType = ENarr_StoryType::Warning;
        }
        else if (Distance < 300.0f)
        {
            ContextualType = ENarr_StoryType::Wisdom;
        }
        
        TriggerStoryByLocation(PlayerLocation, ContextualType);
    }
}

void ANarr_StorytellingSystem::TriggerStoryByLocation(FVector Location, ENarr_StoryType StoryType)
{
    if (!CanTriggerStory())
    {
        return;
    }
    
    FNarr_StoryData StoryToPlay = GetRandomStoryByType(StoryType);
    
    if (!StoryToPlay.StoryID.IsEmpty())
    {
        PlayStoryAudio(StoryToPlay);
        LastStoryTime = GetWorld()->GetTimeSeconds();
        
        // Mark as triggered if play once
        if (StoryToPlay.bPlayOnce)
        {
            FNarr_StoryData* FoundStory = FindStoryByID(StoryToPlay.StoryID);
            if (FoundStory)
            {
                FoundStory->bHasBeenTriggered = true;
            }
        }
        
        OnStoryTriggered(StoryToPlay);
        UE_LOG(LogTemp, Log, TEXT("StorytellingSystem: Triggered story %s"), *StoryToPlay.StoryID);
    }
}

void ANarr_StorytellingSystem::TriggerStoryByID(const FString& StoryID)
{
    FNarr_StoryData* FoundStory = FindStoryByID(StoryID);
    
    if (FoundStory && (!FoundStory->bPlayOnce || !FoundStory->bHasBeenTriggered))
    {
        PlayStoryAudio(*FoundStory);
        LastStoryTime = GetWorld()->GetTimeSeconds();
        
        if (FoundStory->bPlayOnce)
        {
            FoundStory->bHasBeenTriggered = true;
        }
        
        OnStoryTriggered(*FoundStory);
        UE_LOG(LogTemp, Log, TEXT("StorytellingSystem: Triggered story by ID %s"), *StoryID);
    }
}

void ANarr_StorytellingSystem::AddStoryToDatabase(const FNarr_StoryData& NewStory)
{
    StoryDatabase.Add(NewStory);
    UE_LOG(LogTemp, Log, TEXT("StorytellingSystem: Added story %s to database"), *NewStory.StoryID);
}

bool ANarr_StorytellingSystem::CanTriggerStory() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastStoryTime) >= StoryTriggerCooldown;
}

FNarr_StoryData ANarr_StorytellingSystem::GetRandomStoryByType(ENarr_StoryType StoryType)
{
    TArray<FNarr_StoryData> MatchingStories;
    
    for (const FNarr_StoryData& Story : StoryDatabase)
    {
        if (Story.StoryType == StoryType && (!Story.bPlayOnce || !Story.bHasBeenTriggered))
        {
            MatchingStories.Add(Story);
        }
    }
    
    if (MatchingStories.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, MatchingStories.Num() - 1);
        return MatchingStories[RandomIndex];
    }
    
    return FNarr_StoryData();
}

void ANarr_StorytellingSystem::PlayStoryAudio(const FNarr_StoryData& StoryData)
{
    if (AudioComponent && StoryData.AudioClip)
    {
        AudioComponent->SetSound(StoryData.AudioClip);
        AudioComponent->Play();
        UE_LOG(LogTemp, Log, TEXT("StorytellingSystem: Playing audio for story %s"), *StoryData.StoryID);
    }
    else
    {
        // Display text if no audio
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Narrator: %s"), *StoryData.DialogueText));
        }
        UE_LOG(LogTemp, Log, TEXT("StorytellingSystem: Displaying text for story %s"), *StoryData.StoryID);
    }
}

FNarr_StoryData* ANarr_StorytellingSystem::FindStoryByID(const FString& StoryID)
{
    for (FNarr_StoryData& Story : StoryDatabase)
    {
        if (Story.StoryID == StoryID)
        {
            return &Story;
        }
    }
    return nullptr;
}