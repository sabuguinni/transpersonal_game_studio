#include "Narr_DialogueManager.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentState = ENarr_DialogueState::Inactive;
    CooldownDuration = 30.0f;
    TriggerRadius = 500.0f;
    VolumeMultiplier = 1.0f;
    PitchMultiplier = 1.0f;
    PlayerDetectionRange = 1000.0f;
    bAutoTriggerOnEnter = true;
    bRequireLineOfSight = false;
    bTrackPlayerMovement = true;
    
    // Internal state
    CooldownTimer = 0.0f;
    CurrentDialogueIndex = -1;
    bPlayerInRange = false;
    PlayerPawn = nullptr;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();

    // Bind trigger events
    if (TriggerSphere)
    {
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueManager::OnTriggerEnter);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueManager::OnTriggerExit);
    }

    // Find player pawn
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Set initial state
    SetDialogueState(ENarr_DialogueState::Ready);
}

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update cooldown timer
    if (CurrentState == ENarr_DialogueState::Cooldown)
    {
        CooldownTimer -= DeltaTime;
        if (CooldownTimer <= 0.0f)
        {
            SetDialogueState(ENarr_DialogueState::Ready);
        }
    }

    // Update player detection
    if (bTrackPlayerMovement)
    {
        UpdatePlayerDetection();
    }

    // Process dialogue triggers
    ProcessDialogueTriggers();

    // Check if current dialogue finished
    if (CurrentState == ENarr_DialogueState::Playing && AudioComponent && !AudioComponent->IsPlaying())
    {
        SetDialogueState(ENarr_DialogueState::Completed);
    }
}

void ANarr_DialogueManager::PlayDialogue(int32 DialogueIndex)
{
    if (!DialogueEntries.IsValidIndex(DialogueIndex) || CurrentState == ENarr_DialogueState::Playing)
    {
        return;
    }

    const FNarr_DialogueEntry& Entry = DialogueEntries[DialogueIndex];
    
    if (Entry.AudioClip && AudioComponent)
    {
        CurrentDialogueIndex = DialogueIndex;
        AudioComponent->SetSound(Entry.AudioClip);
        AudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        AudioComponent->SetPitchMultiplier(PitchMultiplier);
        AudioComponent->Play();
        
        SetDialogueState(ENarr_DialogueState::Playing);
        
        // Log dialogue for debugging
        UE_LOG(LogTemp, Warning, TEXT("Playing dialogue: %s - %s"), *Entry.CharacterName, *Entry.DialogueText);
    }
}

void ANarr_DialogueManager::StopDialogue()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
    
    SetDialogueState(ENarr_DialogueState::Ready);
    CurrentDialogueIndex = -1;
}

void ANarr_DialogueManager::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueEntries.Add(NewEntry);
}

bool ANarr_DialogueManager::IsDialoguePlaying() const
{
    return CurrentState == ENarr_DialogueState::Playing;
}

FNarr_DialogueEntry ANarr_DialogueManager::GetCurrentDialogue() const
{
    if (DialogueEntries.IsValidIndex(CurrentDialogueIndex))
    {
        return DialogueEntries[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueEntry();
}

void ANarr_DialogueManager::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA<APawn>())
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (Pawn && Pawn->IsPlayerControlled())
    {
        bPlayerInRange = true;
        
        if (bAutoTriggerOnEnter && CanTriggerDialogue())
        {
            // Find first available dialogue
            for (int32 i = 0; i < DialogueEntries.Num(); i++)
            {
                const FNarr_DialogueEntry& Entry = DialogueEntries[i];
                if (Entry.TriggerType == ENarr_DialogueTrigger::OnEnterZone)
                {
                    PlayDialogue(i);
                    break;
                }
            }
        }
    }
}

void ANarr_DialogueManager::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->IsA<APawn>())
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (Pawn && Pawn->IsPlayerControlled())
    {
        bPlayerInRange = false;
        
        // Trigger exit dialogues
        for (int32 i = 0; i < DialogueEntries.Num(); i++)
        {
            const FNarr_DialogueEntry& Entry = DialogueEntries[i];
            if (Entry.TriggerType == ENarr_DialogueTrigger::OnExitZone && CanTriggerDialogue())
            {
                PlayDialogue(i);
                break;
            }
        }
    }
}

void ANarr_DialogueManager::SetDialogueState(ENarr_DialogueState NewState)
{
    CurrentState = NewState;
    
    if (NewState == ENarr_DialogueState::Completed)
    {
        // Start cooldown if dialogue is not repeatable
        if (DialogueEntries.IsValidIndex(CurrentDialogueIndex))
        {
            const FNarr_DialogueEntry& Entry = DialogueEntries[CurrentDialogueIndex];
            if (!Entry.bIsRepeatable)
            {
                CooldownTimer = CooldownDuration;
                SetDialogueState(ENarr_DialogueState::Cooldown);
                return;
            }
        }
        
        SetDialogueState(ENarr_DialogueState::Ready);
    }
}

bool ANarr_DialogueManager::CanTriggerDialogue() const
{
    if (CurrentState != ENarr_DialogueState::Ready)
    {
        return false;
    }
    
    if (bRequireLineOfSight && !CheckLineOfSight())
    {
        return false;
    }
    
    return true;
}

void ANarr_DialogueManager::ResetCooldown()
{
    CooldownTimer = 0.0f;
    if (CurrentState == ENarr_DialogueState::Cooldown)
    {
        SetDialogueState(ENarr_DialogueState::Ready);
    }
}

void ANarr_DialogueManager::UpdatePlayerDetection()
{
    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        return;
    }
    
    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    bool bWasInRange = bPlayerInRange;
    bPlayerInRange = Distance <= PlayerDetectionRange;
    
    // Trigger range-based events
    if (bPlayerInRange && !bWasInRange)
    {
        // Player entered detection range
    }
    else if (!bPlayerInRange && bWasInRange)
    {
        // Player left detection range
    }
}

void ANarr_DialogueManager::ProcessDialogueTriggers()
{
    if (!CanTriggerDialogue() || !bPlayerInRange)
    {
        return;
    }
    
    // Process time-based triggers
    for (int32 i = 0; i < DialogueEntries.Num(); i++)
    {
        const FNarr_DialogueEntry& Entry = DialogueEntries[i];
        if (Entry.TriggerType == ENarr_DialogueTrigger::OnTimeElapsed)
        {
            // Time-based logic would go here
            // For now, we'll skip this type
        }
    }
}

bool ANarr_DialogueManager::CheckLineOfSight() const
{
    if (!PlayerPawn)
    {
        return false;
    }
    
    FVector Start = GetActorLocation();
    FVector End = PlayerPawn->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(PlayerPawn);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // No obstruction means clear line of sight
}