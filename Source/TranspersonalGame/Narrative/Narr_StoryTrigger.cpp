#include "Narr_StoryTrigger.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ANarr_StoryTrigger::ANarr_StoryTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize trigger type
    TriggerType = ENarr_TriggerType::CaveDiscovery;
    
    // Initialize state
    bHasBeenTriggered = false;
    LastTriggerTime = 0.0f;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Set default collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &ANarr_StoryTrigger::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &ANarr_StoryTrigger::OnOverlapEnd);

    // Initialize default story data
    StoryData.StoryTitle = TEXT("Ancient Discovery");
    StoryData.NarrativeText = TEXT("You have discovered something ancient and mysterious...");
    StoryData.CharacterName = TEXT("Narrator");
    StoryData.TriggerRadius = 200.0f;
    StoryData.bOneTimeOnly = true;
    StoryData.bRequiresPlayerInput = false;
}

void ANarr_StoryTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Set up trigger based on type
    switch (TriggerType)
    {
        case ENarr_TriggerType::CaveDiscovery:
            StoryData.StoryTitle = TEXT("Cave Discovery");
            StoryData.NarrativeText = TEXT("The darkness ahead whispers of ancient secrets. Your ancestors once sheltered here from the great storms.");
            StoryData.CharacterName = TEXT("Cave Echo");
            break;

        case ENarr_TriggerType::HuntingGround:
            StoryData.StoryTitle = TEXT("Ancient Hunting Ground");
            StoryData.NarrativeText = TEXT("The bones scattered here tell of great hunts. Your people once brought down the mighty beasts in this very place.");
            StoryData.CharacterName = TEXT("Spirit of the Hunt");
            break;

        case ENarr_TriggerType::TribalMemory:
            StoryData.StoryTitle = TEXT("Tribal Memory Stone");
            StoryData.NarrativeText = TEXT("These stones remember the old ways. Touch them and feel the wisdom of those who came before.");
            StoryData.CharacterName = TEXT("Stone Memory");
            break;

        case ENarr_TriggerType::DangerWarning:
            StoryData.StoryTitle = TEXT("Danger Zone");
            StoryData.NarrativeText = TEXT("Beware! The great predators mark this territory. Tread carefully or become their next prey.");
            StoryData.CharacterName = TEXT("Warning Voice");
            break;

        case ENarr_TriggerType::SafeZone:
            StoryData.StoryTitle = TEXT("Safe Haven");
            StoryData.NarrativeText = TEXT("Here you may rest. The ancient protections still hold, keeping the dangers at bay.");
            StoryData.CharacterName = TEXT("Guardian Spirit");
            break;
    }
}

void ANarr_StoryTrigger::TriggerStoryEvent(AActor* TriggeringActor)
{
    if (!CanTrigger())
    {
        return;
    }

    // Check if triggering actor is a character
    ACharacter* Character = Cast<ACharacter>(TriggeringActor);
    if (!Character)
    {
        return;
    }

    // Mark as triggered
    bHasBeenTriggered = true;
    LastTriggerTime = GetWorld()->GetTimeSeconds();

    // Play narrative audio if available
    PlayNarrativeAudio();

    // Log the story event
    UE_LOG(LogTemp, Log, TEXT("Story Triggered: %s - %s"), *StoryData.StoryTitle, *StoryData.NarrativeText);

    // Display message to player (in a real game, this would go to UI)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), *StoryData.CharacterName, *StoryData.NarrativeText);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
    }

    // Call Blueprint event
    OnStoryTriggered(StoryData);
}

void ANarr_StoryTrigger::PlayNarrativeAudio()
{
    if (NarrativeSound && AudioComponent)
    {
        AudioComponent->SetSound(NarrativeSound);
        AudioComponent->Play();
    }
}

void ANarr_StoryTrigger::SetStoryData(const FNarr_StoryData& NewStoryData)
{
    StoryData = NewStoryData;
}

bool ANarr_StoryTrigger::CanTrigger() const
{
    if (StoryData.bOneTimeOnly && bHasBeenTriggered)
    {
        return false;
    }

    // Prevent rapid re-triggering
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastTriggerTime < 2.0f)
    {
        return false;
    }

    return true;
}

void ANarr_StoryTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    TriggerStoryEvent(OtherActor);
}

void ANarr_StoryTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Optional: Handle when player leaves trigger area
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && AudioComponent && AudioComponent->IsPlaying())
    {
        // Fade out audio when leaving trigger
        AudioComponent->FadeOut(1.0f, 0.0f);
    }
}