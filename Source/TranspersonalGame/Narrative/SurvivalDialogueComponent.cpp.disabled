#include "SurvivalDialogueComponent.h"
#include "TranspersonalCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

USurvivalDialogueComponent::USurvivalDialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check twice per second
    
    bIsDialogueActive = false;
    CurrentSequenceIndex = -1;
    CurrentLineIndex = -1;
    DialogueCheckInterval = 1.0f;
    
    // Initialize default character voice URLs
    CharacterVoiceURLs.Add(TEXT("SurvivalNarrator"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777623025988_SurvivalNarrator.mp3"));
    CharacterVoiceURLs.Add(TEXT("PlayerCharacter"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777623028019_PlayerCharacter.mp3"));
    CharacterVoiceURLs.Add(TEXT("TRexEncounter"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777623030104_TRexEncounter.mp3"));
    CharacterVoiceURLs.Add(TEXT("CampfireGuide"), TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777623032267_CampfireGuide.mp3"));
}

void USurvivalDialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Start dialogue trigger checking
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DialogueTimerHandle,
            this,
            &USurvivalDialogueComponent::CheckDialogueTriggers,
            DialogueCheckInterval,
            true
        );
    }
    
    // Setup default dialogue sequences
    SetupDefaultDialogueSequences();
}

void USurvivalDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle active dialogue progression
    if (bIsDialogueActive && CurrentSequenceIndex >= 0 && CurrentLineIndex >= 0)
    {
        // Check if current line should end
        const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
        if (CurrentLineIndex < CurrentSequence.DialogueLines.Num())
        {
            const FNarr_DialogueLine& CurrentLine = CurrentSequence.DialogueLines[CurrentLineIndex];
            // Line progression is handled by timer, this is just for safety
        }
    }
}

void USurvivalDialogueComponent::StartDialogueSequence(const FString& SequenceName)
{
    // Find sequence by name
    for (int32 i = 0; i < DialogueSequences.Num(); i++)
    {
        if (DialogueSequences[i].SequenceName == SequenceName)
        {
            CurrentSequenceIndex = i;
            CurrentLineIndex = 0;
            bIsDialogueActive = true;
            
            const FNarr_DialogueSequence& Sequence = DialogueSequences[i];
            OnDialogueStarted(Sequence);
            
            if (Sequence.DialogueLines.Num() > 0)
            {
                const FNarr_DialogueLine& FirstLine = Sequence.DialogueLines[0];
                OnDialogueLineChanged(FirstLine);
                
                // Set timer for line progression
                if (GetWorld())
                {
                    GetWorld()->GetTimerManager().SetTimer(
                        DialogueTimerHandle,
                        this,
                        &USurvivalDialogueComponent::AdvanceDialogue,
                        FirstLine.DisplayDuration,
                        false
                    );
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Started dialogue sequence: %s"), *SequenceName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dialogue sequence not found: %s"), *SequenceName);
}

void USurvivalDialogueComponent::StopCurrentDialogue()
{
    if (bIsDialogueActive)
    {
        bIsDialogueActive = false;
        CurrentSequenceIndex = -1;
        CurrentLineIndex = -1;
        
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(DialogueTimerHandle);
        }
        
        OnDialogueEnded();
        UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
    }
}

void USurvivalDialogueComponent::AddDialogueSequence(const FNarr_DialogueSequence& NewSequence)
{
    DialogueSequences.Add(NewSequence);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue sequence: %s"), *NewSequence.SequenceName);
}

FNarr_DialogueLine USurvivalDialogueComponent::GetCurrentDialogueLine() const
{
    if (bIsDialogueActive && CurrentSequenceIndex >= 0 && CurrentLineIndex >= 0)
    {
        const FNarr_DialogueSequence& Sequence = DialogueSequences[CurrentSequenceIndex];
        if (CurrentLineIndex < Sequence.DialogueLines.Num())
        {
            return Sequence.DialogueLines[CurrentLineIndex];
        }
    }
    
    return FNarr_DialogueLine(); // Return empty line
}

void USurvivalDialogueComponent::SetCharacterVoiceURL(const FString& CharacterName, const FString& AudioURL)
{
    CharacterVoiceURLs.Add(CharacterName, AudioURL);
    UE_LOG(LogTemp, Log, TEXT("Set voice URL for %s: %s"), *CharacterName, *AudioURL);
}

void USurvivalDialogueComponent::CheckDialogueTriggers()
{
    if (bIsDialogueActive)
    {
        return; // Don't trigger new dialogue while one is active
    }
    
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        return;
    }
    
    // Check each dialogue sequence for trigger conditions
    for (const FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (ShouldTriggerSequence(Sequence))
        {
            StartDialogueSequence(Sequence.SequenceName);
            break; // Only trigger one sequence at a time
        }
    }
}

void USurvivalDialogueComponent::AdvanceDialogue()
{
    if (!bIsDialogueActive || CurrentSequenceIndex < 0)
    {
        return;
    }
    
    const FNarr_DialogueSequence& CurrentSequence = DialogueSequences[CurrentSequenceIndex];
    CurrentLineIndex++;
    
    if (CurrentLineIndex < CurrentSequence.DialogueLines.Num())
    {
        // Show next line
        const FNarr_DialogueLine& NextLine = CurrentSequence.DialogueLines[CurrentLineIndex];
        OnDialogueLineChanged(NextLine);
        
        // Set timer for next advancement
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                DialogueTimerHandle,
                this,
                &USurvivalDialogueComponent::AdvanceDialogue,
                NextLine.DisplayDuration,
                false
            );
        }
    }
    else
    {
        // Sequence finished
        StopCurrentDialogue();
    }
}

bool USurvivalDialogueComponent::ShouldTriggerSequence(const FNarr_DialogueSequence& Sequence) const
{
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        return false;
    }
    
    // Check distance
    float DistanceToPlayer = GetDistanceToPlayer();
    if (DistanceToPlayer > Sequence.TriggerRadius)
    {
        return false;
    }
    
    // Check line of sight if required
    if (Sequence.bRequiresLineOfSight && !HasLineOfSightToPlayer())
    {
        return false;
    }
    
    // Check player health requirement
    if (Player->GetHealth() < Sequence.MinPlayerHealth)
    {
        return false;
    }
    
    // Check threat context (simplified - in full implementation would check for nearby threats)
    if (Sequence.TriggerThreat != ESurvivalThreat::None)
    {
        // For now, always allow threat-based dialogue
        // In full implementation, would check for actual threats in area
    }
    
    return true;
}

ATranspersonalCharacter* USurvivalDialogueComponent::GetPlayerCharacter() const
{
    if (GetWorld())
    {
        return Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    }
    return nullptr;
}

float USurvivalDialogueComponent::GetDistanceToPlayer() const
{
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (Player && GetOwner())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    }
    return 99999.0f;
}

bool USurvivalDialogueComponent::HasLineOfSightToPlayer() const
{
    ATranspersonalCharacter* Player = GetPlayerCharacter();
    if (!Player || !GetOwner() || !GetWorld())
    {
        return false;
    }
    
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Player->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Player);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No hit means clear line of sight
}

void USurvivalDialogueComponent::SetupDefaultDialogueSequences()
{
    // Create default survival dialogue sequences
    
    // Intro sequence
    FNarr_DialogueSequence IntroSequence;
    IntroSequence.SequenceName = TEXT("PlayerIntro");
    IntroSequence.TriggerThreat = ESurvivalThreat::None;
    IntroSequence.TriggerRadius = 100.0f;
    IntroSequence.bRequiresLineOfSight = false;
    IntroSequence.MinPlayerHealth = 0.0f;
    
    FNarr_DialogueLine IntroLine;
    IntroLine.SpeakerName = TEXT("SurvivalNarrator");
    IntroLine.DialogueText = TEXT("The ancient world awakens. You are alone, vulnerable, with only primitive tools against creatures that ruled the earth for millions of years.");
    IntroLine.DisplayDuration = 8.0f;
    IntroLine.Context = ESurvivalContext::Exploration;
    IntroLine.AudioURL = CharacterVoiceURLs.FindRef(TEXT("SurvivalNarrator"));
    IntroSequence.DialogueLines.Add(IntroLine);
    
    DialogueSequences.Add(IntroSequence);
    
    // Danger sequence
    FNarr_DialogueSequence DangerSequence;
    DangerSequence.SequenceName = TEXT("TRexWarning");
    DangerSequence.TriggerThreat = ESurvivalThreat::Predator;
    DangerSequence.TriggerRadius = 800.0f;
    DangerSequence.bRequiresLineOfSight = true;
    DangerSequence.MinPlayerHealth = 50.0f;
    
    FNarr_DialogueLine DangerLine;
    DangerLine.SpeakerName = TEXT("TRexEncounter");
    DangerLine.DialogueText = TEXT("The great predator approaches. Its footsteps shake the earth. Hide among the rocks and pray it does not catch your scent.");
    DangerLine.DisplayDuration = 6.0f;
    DangerLine.Context = ESurvivalContext::Danger;
    DangerLine.AudioURL = CharacterVoiceURLs.FindRef(TEXT("TRexEncounter"));
    DangerSequence.DialogueLines.Add(DangerLine);
    
    DialogueSequences.Add(DangerSequence);
    
    UE_LOG(LogTemp, Log, TEXT("Setup %d default dialogue sequences"), DialogueSequences.Num());
}