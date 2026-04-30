#include "SurvivalStoryTrigger.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

ASurvivalStoryTrigger::ASurvivalStoryTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize state
    bHasTriggered = false;
    TriggerCount = 0;
    bUseRandomMessages = false;

    // Setup collision
    GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCollisionComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCollisionComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio components
    VoiceOverComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceOverComponent"));
    VoiceOverComponent->SetupAttachment(RootComponent);
    VoiceOverComponent->bAutoActivate = false;
    VoiceOverComponent->SetVolumeMultiplier(0.8f);

    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.5f);

    // Setup default story moment
    StoryMoment.TriggerMessage = TEXT("You sense something important about this place...");
    StoryMoment.DelayBeforeMessage = 2.0f;
    StoryMoment.bOnlyTriggerOnce = true;
    StoryMoment.TriggerType = ENarr_StoryTriggerType::SafeZone;

    // Setup random messages for variety
    RandomMessages.Add(TEXT("The wind carries whispers of ancient dangers..."));
    RandomMessages.Add(TEXT("This place holds memories of those who came before..."));
    RandomMessages.Add(TEXT("Something stirs in the shadows beyond your sight..."));
    RandomMessages.Add(TEXT("The earth beneath your feet tells stories of survival..."));
}

void ASurvivalStoryTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    OnActorBeginOverlap.AddDynamic(this, &ASurvivalStoryTrigger::OnPlayerEnterTrigger);
    OnActorEndOverlap.AddDynamic(this, &ASurvivalStoryTrigger::OnPlayerExitTrigger);

    UE_LOG(LogTemp, Log, TEXT("SurvivalStoryTrigger '%s' initialized with type: %d"), 
           *GetName(), (int32)StoryMoment.TriggerType);
}

void ASurvivalStoryTrigger::OnPlayerEnterTrigger(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!IsPlayerCharacter(OtherActor) || !CanTrigger())
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Player entered story trigger: %s"), *GetName());

    // Schedule the story moment with delay
    if (StoryMoment.DelayBeforeMessage > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            MessageDelayTimer,
            this,
            &ASurvivalStoryTrigger::TriggerStoryMoment,
            StoryMoment.DelayBeforeMessage,
            false
        );
    }
    else
    {
        TriggerStoryMoment();
    }
}

void ASurvivalStoryTrigger::OnPlayerExitTrigger(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!IsPlayerCharacter(OtherActor))
    {
        return;
    }

    // Clear any pending timer
    if (MessageDelayTimer.IsValid())
    {
        GetWorldTimerManager().ClearTimer(MessageDelayTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("Player exited story trigger: %s"), *GetName());
}

void ASurvivalStoryTrigger::TriggerStoryMoment()
{
    if (!CanTrigger())
    {
        return;
    }

    // Update state
    bHasTriggered = true;
    TriggerCount++;

    // Get the message to display
    FString MessageToShow = GetContextualMessage();

    // Display message to player
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Yellow,
                MessageToShow
            );
        }
    }

    // Play voice over if available
    PlayVoiceOver();

    // Log the trigger event
    LogStoryTrigger();

    UE_LOG(LogTemp, Log, TEXT("Story moment triggered: %s"), *MessageToShow);
}

void ASurvivalStoryTrigger::PlayVoiceOver()
{
    if (!VoiceOverComponent)
    {
        return;
    }

    // Load and play voice over cue if specified
    if (StoryMoment.VoiceOverCue.IsValid())
    {
        USoundCue* SoundCue = StoryMoment.VoiceOverCue.LoadSynchronous();
        if (SoundCue)
        {
            VoiceOverComponent->SetSound(SoundCue);
            VoiceOverComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("Playing voice over for story trigger: %s"), *GetName());
        }
    }
}

bool ASurvivalStoryTrigger::IsPlayerCharacter(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    // Check if it's a player-controlled character
    ACharacter* Character = Cast<ACharacter>(Actor);
    if (!Character)
    {
        return false;
    }

    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    return PC != nullptr;
}

FString ASurvivalStoryTrigger::GetContextualMessage() const
{
    if (bUseRandomMessages && RandomMessages.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, RandomMessages.Num() - 1);
        return RandomMessages[RandomIndex];
    }

    return StoryMoment.TriggerMessage;
}

void ASurvivalStoryTrigger::LogStoryTrigger() const
{
    FString TriggerTypeName;
    switch (StoryMoment.TriggerType)
    {
        case ENarr_StoryTriggerType::WaterSource:
            TriggerTypeName = TEXT("Water Source");
            break;
        case ENarr_StoryTriggerType::DangerZone:
            TriggerTypeName = TEXT("Danger Zone");
            break;
        case ENarr_StoryTriggerType::SafeZone:
            TriggerTypeName = TEXT("Safe Zone");
            break;
        case ENarr_StoryTriggerType::HuntingGrounds:
            TriggerTypeName = TEXT("Hunting Grounds");
            break;
        case ENarr_StoryTriggerType::FirstContact:
            TriggerTypeName = TEXT("First Contact");
            break;
        case ENarr_StoryTriggerType::NightFall:
            TriggerTypeName = TEXT("Night Fall");
            break;
        default:
            TriggerTypeName = TEXT("Unknown");
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("=== NARRATIVE EVENT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Trigger: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("Type: %s"), *TriggerTypeName);
    UE_LOG(LogTemp, Warning, TEXT("Count: %d"), TriggerCount);
    UE_LOG(LogTemp, Warning, TEXT("Message: %s"), *GetContextualMessage());
}

void ASurvivalStoryTrigger::SetStoryMoment(const FNarr_StoryMoment& NewStoryMoment)
{
    StoryMoment = NewStoryMoment;
    UE_LOG(LogTemp, Log, TEXT("Story moment updated for trigger: %s"), *GetName());
}

void ASurvivalStoryTrigger::ResetTrigger()
{
    bHasTriggered = false;
    TriggerCount = 0;

    // Clear any pending timer
    if (MessageDelayTimer.IsValid())
    {
        GetWorldTimerManager().ClearTimer(MessageDelayTimer);
    }

    UE_LOG(LogTemp, Log, TEXT("Story trigger reset: %s"), *GetName());
}

bool ASurvivalStoryTrigger::CanTrigger() const
{
    if (StoryMoment.bOnlyTriggerOnce && bHasTriggered)
    {
        return false;
    }

    return true;
}