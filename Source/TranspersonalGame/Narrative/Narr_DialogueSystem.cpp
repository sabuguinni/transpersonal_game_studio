#include "Narr_DialogueSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

// UNarr_DialogueComponent Implementation
UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    ProximityTriggerDistance = 500.0f;
    bAutoPlayOnTrigger = true;
    DialogueVolume = 1.0f;
    
    CurrentSequence = nullptr;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    bIsPlaying = false;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default dialogue sequences for prehistoric survival
    FNarr_DialogueSequence IntroSequence;
    IntroSequence.SequenceID = TEXT("PlayerIntro");
    IntroSequence.TriggerType = ENarr_DialogueTrigger::Proximity;
    IntroSequence.bRepeatable = false;
    IntroSequence.Priority = 10;
    
    FNarr_DialogueLine IntroLine;
    IntroLine.SpeakerName = TEXT("Narrator");
    IntroLine.DialogueText = FText::FromString(TEXT("You awaken in a world ruled by giants. Survival is your only goal."));
    IntroLine.DialogueType = ENarr_DialogueType::Narrative;
    IntroLine.Duration = 4.0f;
    
    IntroSequence.DialogueLines.Add(IntroLine);
    DialogueSequences.Add(IntroSequence);
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsPlaying && CurrentSequence)
    {
        CurrentLineTimer += DeltaTime;
        
        if (CurrentLineIndex < CurrentSequence->DialogueLines.Num())
        {
            const FNarr_DialogueLine& CurrentLine = CurrentSequence->DialogueLines[CurrentLineIndex];
            
            if (CurrentLineTimer >= CurrentLine.Duration)
            {
                OnDialogueLineComplete();
            }
        }
    }
    
    // Check for proximity triggers
    if (bAutoPlayOnTrigger && !bIsPlaying)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            CheckProximityTriggers(PlayerPawn);
        }
    }
}

void UNarr_DialogueComponent::StartDialogueSequence(const FString& SequenceID)
{
    if (bIsPlaying)
    {
        StopCurrentDialogue();
    }
    
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID && CanPlaySequence(Sequence))
        {
            CurrentSequence = &Sequence;
            CurrentLineIndex = 0;
            CurrentLineTimer = 0.0f;
            bIsPlaying = true;
            
            if (!Sequence.bRepeatable)
            {
                PlayedSequences.AddUnique(SequenceID);
            }
            
            PlayNextLine();
            break;
        }
    }
}

void UNarr_DialogueComponent::StopCurrentDialogue()
{
    bIsPlaying = false;
    CurrentSequence = nullptr;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
}

void UNarr_DialogueComponent::AddDialogueSequence(const FNarr_DialogueSequence& Sequence)
{
    DialogueSequences.Add(Sequence);
}

bool UNarr_DialogueComponent::IsDialogueActive() const
{
    return bIsPlaying && CurrentSequence != nullptr;
}

FString UNarr_DialogueComponent::GetCurrentSpeaker() const
{
    if (IsDialogueActive() && CurrentLineIndex < CurrentSequence->DialogueLines.Num())
    {
        return CurrentSequence->DialogueLines[CurrentLineIndex].SpeakerName;
    }
    return TEXT("");
}

FText UNarr_DialogueComponent::GetCurrentDialogueText() const
{
    if (IsDialogueActive() && CurrentLineIndex < CurrentSequence->DialogueLines.Num())
    {
        return CurrentSequence->DialogueLines[CurrentLineIndex].DialogueText;
    }
    return FText::GetEmpty();
}

void UNarr_DialogueComponent::CheckProximityTriggers(AActor* PlayerActor)
{
    if (!PlayerActor || bIsPlaying)
    {
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    
    if (DistanceToPlayer <= ProximityTriggerDistance)
    {
        TriggerDialogueByType(ENarr_DialogueTrigger::Proximity);
    }
}

void UNarr_DialogueComponent::TriggerDialogueByType(ENarr_DialogueTrigger TriggerType)
{
    if (bIsPlaying)
    {
        return;
    }
    
    // Find highest priority sequence of the specified trigger type
    FNarr_DialogueSequence* BestSequence = nullptr;
    int32 HighestPriority = -1;
    
    for (FNarr_DialogueSequence& Sequence : DialogueSequences)
    {
        if (Sequence.TriggerType == TriggerType && CanPlaySequence(Sequence))
        {
            if (Sequence.Priority > HighestPriority)
            {
                BestSequence = &Sequence;
                HighestPriority = Sequence.Priority;
            }
        }
    }
    
    if (BestSequence)
    {
        StartDialogueSequence(BestSequence->SequenceID);
    }
}

void UNarr_DialogueComponent::PlayNextLine()
{
    if (!CurrentSequence || CurrentLineIndex >= CurrentSequence->DialogueLines.Num())
    {
        StopCurrentDialogue();
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentSequence->DialogueLines[CurrentLineIndex];
    CurrentLineTimer = 0.0f;
    
    // Play voice clip if available
    if (CurrentLine.VoiceClip.IsValid())
    {
        USoundBase* SoundToPlay = CurrentLine.VoiceClip.LoadSynchronous();
        if (SoundToPlay)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, GetOwner()->GetActorLocation(), DialogueVolume);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue: [%s] %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText.ToString());
}

void UNarr_DialogueComponent::OnDialogueLineComplete()
{
    CurrentLineIndex++;
    
    if (CurrentLineIndex < CurrentSequence->DialogueLines.Num())
    {
        PlayNextLine();
    }
    else
    {
        StopCurrentDialogue();
    }
}

bool UNarr_DialogueComponent::CanPlaySequence(const FNarr_DialogueSequence& Sequence) const
{
    if (!Sequence.bRepeatable && PlayedSequences.Contains(Sequence.SequenceID))
    {
        return false;
    }
    
    return true;
}

// ANarr_DialogueActor Implementation
ANarr_DialogueActor::ANarr_DialogueActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create dialogue component
    DialogueComponent = CreateDefaultSubobject<UNarr_DialogueComponent>(TEXT("DialogueComponent"));
    
    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueActor::OnTriggerOverlap);
    
    DefaultSequenceID = TEXT("PlayerIntro");
}

void ANarr_DialogueActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with dialogue manager
    UNarr_DialogueManager* DialogueManager = GetGameInstance()->GetSubsystem<UNarr_DialogueManager>();
    if (DialogueManager)
    {
        DialogueManager->RegisterDialogueActor(this);
    }
}

void ANarr_DialogueActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueActor::TriggerDialogue(AActor* PlayerActor)
{
    if (DialogueComponent && !DefaultSequenceID.IsEmpty())
    {
        DialogueComponent->StartDialogueSequence(DefaultSequenceID);
    }
}

void ANarr_DialogueActor::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (PlayerPawn && PlayerPawn->IsPlayerControlled())
    {
        TriggerDialogue(PlayerPawn);
    }
}

// UNarr_DialogueManager Implementation
void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Setup global narrative sequences for prehistoric survival
    FNarr_DialogueSequence WelcomeSequence;
    WelcomeSequence.SequenceID = TEXT("GlobalWelcome");
    WelcomeSequence.TriggerType = ENarr_DialogueTrigger::Environmental;
    WelcomeSequence.bRepeatable = false;
    WelcomeSequence.Priority = 100;
    
    FNarr_DialogueLine WelcomeLine;
    WelcomeLine.SpeakerName = TEXT("Ancient Voice");
    WelcomeLine.DialogueText = FText::FromString(TEXT("Welcome to the age of giants, where only the clever survive."));
    WelcomeLine.DialogueType = ENarr_DialogueType::Narrative;
    WelcomeLine.Duration = 5.0f;
    
    WelcomeSequence.DialogueLines.Add(WelcomeLine);
    GlobalDialogueSequences.Add(WelcomeSequence);
}

void UNarr_DialogueManager::RegisterDialogueActor(ANarr_DialogueActor* DialogueActor)
{
    if (DialogueActor && !RegisteredDialogueActors.Contains(DialogueActor))
    {
        RegisteredDialogueActors.Add(DialogueActor);
    }
}

void UNarr_DialogueManager::UnregisterDialogueActor(ANarr_DialogueActor* DialogueActor)
{
    RegisteredDialogueActors.Remove(DialogueActor);
}

void UNarr_DialogueManager::PlayGlobalDialogue(const FString& SequenceID)
{
    for (const FNarr_DialogueSequence& Sequence : GlobalDialogueSequences)
    {
        if (Sequence.SequenceID == SequenceID)
        {
            // Play through first available dialogue actor
            if (RegisteredDialogueActors.Num() > 0 && RegisteredDialogueActors[0]->DialogueComponent)
            {
                RegisteredDialogueActors[0]->DialogueComponent->StartDialogueSequence(SequenceID);
            }
            break;
        }
    }
}

void UNarr_DialogueManager::StopAllDialogue()
{
    for (ANarr_DialogueActor* DialogueActor : RegisteredDialogueActors)
    {
        if (DialogueActor && DialogueActor->DialogueComponent)
        {
            DialogueActor->DialogueComponent->StopCurrentDialogue();
        }
    }
}

TArray<ANarr_DialogueActor*> UNarr_DialogueManager::GetNearbyDialogueActors(const FVector& Location, float Radius) const
{
    TArray<ANarr_DialogueActor*> NearbyActors;
    
    for (ANarr_DialogueActor* DialogueActor : RegisteredDialogueActors)
    {
        if (DialogueActor)
        {
            float Distance = FVector::Dist(DialogueActor->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                NearbyActors.Add(DialogueActor);
            }
        }
    }
    
    return NearbyActors;
}