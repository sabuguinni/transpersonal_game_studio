#include "Narr_EnvironmentalStoryTrigger.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

ANarr_EnvironmentalStoryTrigger::ANarr_EnvironmentalStoryTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Default trigger settings
    bRequirePlayerCharacter = true;
    CooldownTime = 30.0f;
    MaxActivations = -1; // Unlimited by default
    bShowDebugInfo = false;
    DebugColor = FLinearColor::Green;

    // Initialize state
    bHasBeenTriggered = false;
    ActivationCount = 0;
    LastActivationTime = 0.0f;

    // Default story data
    StoryData.StoryID = TEXT("DefaultStory");
    StoryData.NarrativeText = FText::FromString(TEXT("You discover something interesting..."));
    StoryData.TriggerType = ENarr_StoryTriggerType::FossilDiscovery;
    StoryData.bOneTimeOnly = true;
    StoryData.DelayBeforeNarration = 1.0f;

    // Bind overlap events
    if (GetCollisionComponent())
    {
        GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ANarr_EnvironmentalStoryTrigger::OnTriggerBeginOverlap);
        GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &ANarr_EnvironmentalStoryTrigger::OnTriggerEndOverlap);
    }
}

void ANarr_EnvironmentalStoryTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Setup debug visualization
    if (bShowDebugInfo && GetCollisionComponent())
    {
        GetCollisionComponent()->SetHiddenInGame(false);
        GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }

    UE_LOG(LogTemp, Log, TEXT("Environmental Story Trigger '%s' initialized with story: %s"), 
        *GetName(), *StoryData.StoryID);
}

void ANarr_EnvironmentalStoryTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
    const FHitResult& SweepResult)
{
    // Check if we can trigger
    if (!CanTrigger())
    {
        return;
    }

    // Check if it's a player character (if required)
    if (bRequirePlayerCharacter)
    {
        ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
        if (!PlayerCharacter || !PlayerCharacter->IsPlayerControlled())
        {
            return;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Story trigger '%s' activated by %s"), 
        *GetName(), OtherActor ? *OtherActor->GetName() : TEXT("Unknown"));

    // Trigger the story narration
    TriggerStoryNarration();
}

void ANarr_EnvironmentalStoryTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Optional: Handle exit behavior
    if (bShowDebugInfo)
    {
        UE_LOG(LogTemp, Log, TEXT("Actor %s left story trigger '%s'"), 
            OtherActor ? *OtherActor->GetName() : TEXT("Unknown"), *GetName());
    }
}

void ANarr_EnvironmentalStoryTrigger::TriggerStoryNarration()
{
    if (!CanTrigger())
    {
        return;
    }

    // Update trigger state
    bHasBeenTriggered = true;
    ActivationCount++;
    LastActivationTime = GetWorld()->GetTimeSeconds();

    // Select appropriate story data
    FNarr_StoryTriggerData SelectedStory = SelectStoryData();

    // Handle delay before narration
    if (SelectedStory.DelayBeforeNarration > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(NarrationDelayTimer, 
            this, &ANarr_EnvironmentalStoryTrigger::PlayNarrationAudio, 
            SelectedStory.DelayBeforeNarration, false);
    }
    else
    {
        PlayNarrationAudio();
    }

    // Handle story progression
    HandleStoryProgression();

    UE_LOG(LogTemp, Log, TEXT("Story narration triggered: %s (Type: %d)"), 
        *SelectedStory.StoryID, (int32)SelectedStory.TriggerType);
}

void ANarr_EnvironmentalStoryTrigger::PlayNarrationAudio()
{
    FNarr_StoryTriggerData SelectedStory = SelectStoryData();

    // Play audio if available
    if (SelectedStory.VoicelineAsset.IsValid() && AudioComponent)
    {
        USoundCue* SoundCue = SelectedStory.VoicelineAsset.LoadSynchronous();
        if (SoundCue)
        {
            AudioComponent->SetSound(SoundCue);
            AudioComponent->Play();
            
            UE_LOG(LogTemp, Log, TEXT("Playing narration audio for story: %s"), *SelectedStory.StoryID);
        }
    }

    // Display narrative text (could be sent to UI system)
    if (!SelectedStory.NarrativeText.IsEmpty())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                FString::Printf(TEXT("Story: %s"), *SelectedStory.NarrativeText.ToString()));
        }
    }
}

void ANarr_EnvironmentalStoryTrigger::HandleStoryProgression()
{
    // This could integrate with a larger narrative system
    // For now, just log the story progression
    FNarr_StoryTriggerData SelectedStory = SelectStoryData();
    
    UE_LOG(LogTemp, Log, TEXT("Story progression: %s triggered %d times"), 
        *SelectedStory.StoryID, ActivationCount);

    // Could send events to quest system, achievement system, etc.
}

FNarr_StoryTriggerData ANarr_EnvironmentalStoryTrigger::SelectStoryData() const
{
    // For now, return the main story data
    // Could implement logic to select from AlternativeStories based on game state
    return StoryData;
}

void ANarr_EnvironmentalStoryTrigger::SetStoryData(const FNarr_StoryTriggerData& NewStoryData)
{
    StoryData = NewStoryData;
    UE_LOG(LogTemp, Log, TEXT("Story data updated for trigger '%s': %s"), 
        *GetName(), *NewStoryData.StoryID);
}

void ANarr_EnvironmentalStoryTrigger::ResetTrigger()
{
    bHasBeenTriggered = false;
    ActivationCount = 0;
    LastActivationTime = 0.0f;
    
    // Clear any pending timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(NarrationDelayTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("Story trigger '%s' has been reset"), *GetName());
}

bool ANarr_EnvironmentalStoryTrigger::CanTrigger() const
{
    // Check if it's a one-time trigger that has already been activated
    if (StoryData.bOneTimeOnly && bHasBeenTriggered)
    {
        return false;
    }

    // Check max activations limit
    if (MaxActivations > 0 && ActivationCount >= MaxActivations)
    {
        return false;
    }

    // Check cooldown
    if (CooldownTime > 0.0f && GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastActivationTime < CooldownTime)
        {
            return false;
        }
    }

    return true;
}