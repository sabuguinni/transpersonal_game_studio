#include "Narr_ContextualDialogue.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/TriggerBox.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "TranspersonalGameState.h"

UNarr_ContextualDialogue::UNarr_ContextualDialogue()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check context twice per second
    
    ContextCheckInterval = 2.0f;
    LastContextCheckTime = 0.0f;
    CurrentContext = ENarr_DialogueContext::None;
    bIsPlayingDialogue = false;
    DialogueCooldown = 5.0f; // 5 seconds between dialogue lines
    LastDialogueTime = 0.0f;
}

void UNarr_ContextualDialogue::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultDialogues();
}

void UNarr_ContextualDialogue::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check for context changes periodically
    if (CurrentTime - LastContextCheckTime >= ContextCheckInterval)
    {
        CheckPlayerContext();
        LastContextCheckTime = CurrentTime;
    }
}

void UNarr_ContextualDialogue::TriggerContextualDialogue(ENarr_DialogueContext Context)
{
    if (!CanPlayDialogue())
    {
        return;
    }
    
    FNarr_ContextualLine* BestLine = FindBestDialogueForContext(Context);
    if (BestLine)
    {
        PlayDialogueLine(*BestLine);
        
        if (BestLine->bPlayOnce)
        {
            BestLine->bHasBeenPlayed = true;
        }
        
        LastDialogueTime = GetWorld()->GetTimeSeconds();
    }
}

void UNarr_ContextualDialogue::AddContextualLine(const FNarr_ContextualLine& NewLine)
{
    ContextualLines.Add(NewLine);
}

bool UNarr_ContextualDialogue::HasDialogueForContext(ENarr_DialogueContext Context) const
{
    for (const FNarr_ContextualLine& Line : ContextualLines)
    {
        if (Line.Context == Context && (!Line.bPlayOnce || !Line.bHasBeenPlayed))
        {
            return true;
        }
    }
    return false;
}

void UNarr_ContextualDialogue::PlayDialogueLine(const FNarr_ContextualLine& Line)
{
    bIsPlayingDialogue = true;
    
    // Log the dialogue for debugging
    UE_LOG(LogTemp, Warning, TEXT("NARRATIVE: %s"), *Line.DialogueText);
    
    // Here we would trigger audio playback if we had the audio system set up
    // For now, we'll just log and set a timer to end the dialogue
    
    FTimerHandle DialogueTimer;
    GetWorld()->GetTimerManager().SetTimer(DialogueTimer, [this]()
    {
        bIsPlayingDialogue = false;
    }, 3.0f, false); // Assume 3 seconds per dialogue line
}

void UNarr_ContextualDialogue::CheckPlayerContext()
{
    CheckProximityToNarrativeTriggers();
    CheckPlayerSurvivalStats();
}

ENarr_DialogueContext UNarr_ContextualDialogue::GetCurrentPlayerContext() const
{
    return CurrentContext;
}

void UNarr_ContextualDialogue::InitializeDefaultDialogues()
{
    // Initialize survival context dialogues
    FNarr_ContextualLine FootprintLine;
    FootprintLine.DialogueText = TEXT("These footprints... they're massive. Whatever made these is still out there.");
    FootprintLine.VoicelineFile = TEXT("footprint_discovery.wav");
    FootprintLine.Context = ENarr_DialogueContext::FirstFootprints;
    FootprintLine.Priority = 10.0f;
    FootprintLine.bPlayOnce = true;
    AddContextualLine(FootprintLine);
    
    FNarr_ContextualLine HerdLine;
    HerdLine.DialogueText = TEXT("A herd in the distance. They're nervous about something. I should stay downwind.");
    HerdLine.VoicelineFile = TEXT("herd_sighting.wav");
    HerdLine.Context = ENarr_DialogueContext::HerdSighting;
    HerdLine.Priority = 8.0f;
    HerdLine.bPlayOnce = false;
    AddContextualLine(HerdLine);
    
    FNarr_ContextualLine PredatorLine;
    PredatorLine.DialogueText = TEXT("Something's watching me. Every instinct tells me to run, but sudden movement might trigger an attack.");
    PredatorLine.VoicelineFile = TEXT("predator_nearby.wav");
    PredatorLine.Context = ENarr_DialogueContext::PredatorNearby;
    PredatorLine.Priority = 15.0f;
    PredatorLine.bPlayOnce = false;
    AddContextualLine(PredatorLine);
    
    FNarr_ContextualLine CampLine;
    CampLine.DialogueText = TEXT("Someone was here before me. The ashes are cold, but the fear in this place... it lingers.");
    CampLine.VoicelineFile = TEXT("abandoned_camp.wav");
    CampLine.Context = ENarr_DialogueContext::AbandonedCamp;
    CampLine.Priority = 7.0f;
    CampLine.bPlayOnce = true;
    AddContextualLine(CampLine);
    
    FNarr_ContextualLine HungerLine;
    HungerLine.DialogueText = TEXT("My stomach aches with hunger. I need to find food soon, but hunting means exposure.");
    HungerLine.VoicelineFile = TEXT("hunger_warning.wav");
    HungerLine.Context = ENarr_DialogueContext::Hungry;
    HungerLine.Priority = 6.0f;
    HungerLine.bPlayOnce = false;
    AddContextualLine(HungerLine);
    
    FNarr_ContextualLine ThirstLine;
    ThirstLine.DialogueText = TEXT("Water. I need water. But rivers and lakes are where the big predators come to drink.");
    ThirstLine.VoicelineFile = TEXT("thirst_warning.wav");
    ThirstLine.Context = ENarr_DialogueContext::Thirsty;
    ThirstLine.Priority = 8.0f;
    ThirstLine.bPlayOnce = false;
    AddContextualLine(ThirstLine);
}

bool UNarr_ContextualDialogue::CanPlayDialogue() const
{
    if (bIsPlayingDialogue)
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastDialogueTime) >= DialogueCooldown;
}

FNarr_ContextualLine* UNarr_ContextualDialogue::FindBestDialogueForContext(ENarr_DialogueContext Context)
{
    FNarr_ContextualLine* BestLine = nullptr;
    float HighestPriority = 0.0f;
    
    for (FNarr_ContextualLine& Line : ContextualLines)
    {
        if (Line.Context == Context && Line.Priority > HighestPriority)
        {
            if (!Line.bPlayOnce || !Line.bHasBeenPlayed)
            {
                BestLine = &Line;
                HighestPriority = Line.Priority;
            }
        }
    }
    
    return BestLine;
}

void UNarr_ContextualDialogue::CheckProximityToNarrativeTriggers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Find all trigger boxes in the level
    TArray<AActor*> TriggerBoxes;
    UGameplayStatics::GetAllActorsOfClass(World, ATriggerBox::StaticClass(), TriggerBoxes);
    
    for (AActor* TriggerActor : TriggerBoxes)
    {
        ATriggerBox* TriggerBox = Cast<ATriggerBox>(TriggerActor);
        if (!TriggerBox)
        {
            continue;
        }
        
        FString TriggerName = TriggerBox->GetActorLabel();
        float Distance = FVector::Dist(PlayerLocation, TriggerBox->GetActorLocation());
        
        // Check if player is close to narrative triggers
        if (Distance < 500.0f) // Within 5 meters
        {
            if (TriggerName.Contains(TEXT("Footprints")))
            {
                if (CurrentContext != ENarr_DialogueContext::FirstFootprints)
                {
                    CurrentContext = ENarr_DialogueContext::FirstFootprints;
                    TriggerContextualDialogue(CurrentContext);
                }
            }
            else if (TriggerName.Contains(TEXT("Herd")))
            {
                if (CurrentContext != ENarr_DialogueContext::HerdSighting)
                {
                    CurrentContext = ENarr_DialogueContext::HerdSighting;
                    TriggerContextualDialogue(CurrentContext);
                }
            }
            else if (TriggerName.Contains(TEXT("Predator")) || TriggerName.Contains(TEXT("Danger")))
            {
                if (CurrentContext != ENarr_DialogueContext::PredatorNearby)
                {
                    CurrentContext = ENarr_DialogueContext::PredatorNearby;
                    TriggerContextualDialogue(CurrentContext);
                }
            }
            else if (TriggerName.Contains(TEXT("Camp")))
            {
                if (CurrentContext != ENarr_DialogueContext::AbandonedCamp)
                {
                    CurrentContext = ENarr_DialogueContext::AbandonedCamp;
                    TriggerContextualDialogue(CurrentContext);
                }
            }
        }
    }
}

void UNarr_ContextualDialogue::CheckPlayerSurvivalStats()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    ATranspersonalGameState* GameState = World->GetGameState<ATranspersonalGameState>();
    if (!GameState)
    {
        return;
    }
    
    // Check hunger
    if (GameState->GetHunger() < 30.0f)
    {
        if (CurrentContext != ENarr_DialogueContext::Hungry)
        {
            CurrentContext = ENarr_DialogueContext::Hungry;
            TriggerContextualDialogue(CurrentContext);
        }
    }
    
    // Check thirst
    if (GameState->GetThirst() < 25.0f)
    {
        if (CurrentContext != ENarr_DialogueContext::Thirsty)
        {
            CurrentContext = ENarr_DialogueContext::Thirsty;
            TriggerContextualDialogue(CurrentContext);
        }
    }
    
    // Check health
    if (GameState->GetHealth() < 40.0f)
    {
        if (CurrentContext != ENarr_DialogueContext::LowHealth)
        {
            CurrentContext = ENarr_DialogueContext::LowHealth;
            TriggerContextualDialogue(CurrentContext);
        }
    }
    
    // Check stamina
    if (GameState->GetStamina() < 20.0f)
    {
        if (CurrentContext != ENarr_DialogueContext::Exhausted)
        {
            CurrentContext = ENarr_DialogueContext::Exhausted;
            TriggerContextualDialogue(CurrentContext);
        }
    }
}