#include "Narr_EnvironmentalStoryteller.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Narr_DialogueManager.h"
#include "Kismet/GameplayStatics.h"

ANarr_EnvironmentalStoryteller::ANarr_EnvironmentalStoryteller()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create trigger box component
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize story data
    TriggerCooldown = 5.0f;
    LastTriggerTime = 0.0f;
}

void ANarr_EnvironmentalStoryteller::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap event
    if (TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANarr_EnvironmentalStoryteller::OnTriggerBoxBeginOverlap);
    }

    LoadEnvironmentalStories();
    UE_LOG(LogTemp, Log, TEXT("Environmental Storyteller initialized: %s"), *StoryData.StoryID);
}

void ANarr_EnvironmentalStoryteller::TriggerEnvironmentalStory()
{
    if (!CanTriggerStory())
    {
        return;
    }

    // Mark as triggered if one-time story
    if (StoryData.bIsOneTime)
    {
        StoryData.bHasBeenTriggered = true;
    }

    LastTriggerTime = GetWorld()->GetTimeSeconds();

    // Trigger dialogue system
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UNarr_DialogueManager* DialogueManager = GameInstance->GetSubsystem<UNarr_DialogueManager>())
        {
            FString ContextKey = FString::Printf(TEXT("Context_%s"), *UEnum::GetValueAsString(StoryData.EnvironmentType));
            DialogueManager->TriggerContextualDialogue(ContextKey, GetActorLocation());
        }
    }

    // Log the environmental story
    UE_LOG(LogTemp, Log, TEXT("Environmental Story Triggered: %s"), *StoryData.NarrationText);

    // Call Blueprint event
    OnStoryTriggered();
}

bool ANarr_EnvironmentalStoryteller::CanTriggerStory() const
{
    // Check if story has already been triggered (for one-time stories)
    if (StoryData.bIsOneTime && StoryData.bHasBeenTriggered)
    {
        return false;
    }

    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastTriggerTime < TriggerCooldown)
    {
        return false;
    }

    return true;
}

void ANarr_EnvironmentalStoryteller::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, 
    const FHitResult& SweepResult)
{
    // Check if the overlapping actor is the player character
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        if (PlayerCharacter->IsPlayerControlled())
        {
            TriggerEnvironmentalStory();
        }
    }
}

void ANarr_EnvironmentalStoryteller::LoadEnvironmentalStories()
{
    // Load environment-specific stories based on actor label or location
    FString ActorLabel = GetActorLabel();
    
    if (ActorLabel.Contains(TEXT("River")))
    {
        StoryData.StoryID = TEXT("RiverStory");
        StoryData.NarrationText = TEXT("The river runs red with the blood of recent kills. Something large has been hunting here.");
        StoryData.EnvironmentType = ENarr_EnvironmentType::River;
        StoryData.bIsOneTime = false;
    }
    else if (ActorLabel.Contains(TEXT("Camp")))
    {
        StoryData.StoryID = TEXT("HunterCampStory");
        StoryData.NarrationText = TEXT("Abandoned tools and cold ashes tell of hunters who never returned. Their fate serves as a warning.");
        StoryData.EnvironmentType = ENarr_EnvironmentType::HunterCamp;
        StoryData.bIsOneTime = true;
    }
    else if (ActorLabel.Contains(TEXT("Danger")))
    {
        StoryData.StoryID = TEXT("DangerZoneStory");
        StoryData.NarrationText = TEXT("The air grows thick with the scent of death. Massive claw marks scar the earth. You are in apex predator territory.");
        StoryData.EnvironmentType = ENarr_EnvironmentType::DangerZone;
        StoryData.bIsOneTime = false;
    }
    else
    {
        // Default story
        StoryData.StoryID = TEXT("GenericStory");
        StoryData.NarrationText = TEXT("This place holds secrets of the ancient world. Tread carefully.");
        StoryData.EnvironmentType = ENarr_EnvironmentType::SafeHaven;
        StoryData.bIsOneTime = true;
    }
}