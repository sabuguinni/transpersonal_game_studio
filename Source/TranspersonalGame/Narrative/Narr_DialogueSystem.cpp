#include "Narr_DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Characters/TranspersonalCharacter.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Create visual marker
    VisualMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMarker"));
    VisualMarker->SetupAttachment(RootComponent);
    VisualMarker->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));

    // Initialize properties
    CurrentDialogueIndex = 0;
    bIsPlayingDialogue = false;
    InteractionRange = 300.0f;
    bPlayerInRange = false;
    PlayerCharacter = nullptr;
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnInteractionSphereBeginOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnInteractionSphereEndOverlap);
    }

    // Bind audio finished event
    if (AudioComponent)
    {
        AudioComponent->OnAudioFinished.AddDynamic(this, &ANarr_DialogueSystem::OnAudioFinished);
    }

    // Initialize character profiles
    CreateCharacterProfiles();
    InitializeCharacters();
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check for player input when in range
    if (bPlayerInRange && PlayerCharacter && !bIsPlayingDialogue)
    {
        // Check for interaction input (E key or gamepad button)
        if (PlayerCharacter->GetInputComponent())
        {
            // Auto-play dialogue when player enters range for now
            // In a full implementation, this would check for input
            if (CurrentCharacter.DialogueLines.Num() > 0)
            {
                PlayDialogueLine(CurrentDialogueIndex);
            }
        }
    }
}

void ANarr_DialogueSystem::InitializeCharacters()
{
    if (Characters.Num() == 0)
    {
        CreateCharacterProfiles();
    }

    // Set first available character as current
    for (const FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.bIsAvailable)
        {
            CurrentCharacter = Character;
            break;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue System initialized with %d characters"), Characters.Num());
}

void ANarr_DialogueSystem::SetCurrentCharacter(const FString& CharacterName)
{
    for (const FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.CharacterName == CharacterName && Character.bIsAvailable)
        {
            CurrentCharacter = Character;
            CurrentDialogueIndex = 0;
            UE_LOG(LogTemp, Warning, TEXT("Set current character to: %s"), *CharacterName);
            return;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Character not found or not available: %s"), *CharacterName);
}

void ANarr_DialogueSystem::PlayDialogueLine(int32 LineIndex)
{
    if (bIsPlayingDialogue)
    {
        return;
    }

    if (CurrentCharacter.DialogueLines.IsValidIndex(LineIndex))
    {
        const FNarr_DialogueLine& Line = CurrentCharacter.DialogueLines[LineIndex];
        
        bIsPlayingDialogue = true;
        CurrentDialogueIndex = LineIndex;

        // Display dialogue text on screen
        if (GEngine)
        {
            FString DisplayText = FString::Printf(TEXT("%s: %s"), *Line.CharacterName, *Line.DialogueText);
            GEngine->AddOnScreenDebugMessage(-1, Line.Duration, FColor::Yellow, DisplayText);
        }

        // Play audio if available
        if (!Line.AudioURL.IsEmpty() && AudioComponent)
        {
            LoadAudioFromURL(Line.AudioURL);
        }
        else
        {
            // If no audio, use timer to simulate duration
            GetWorld()->GetTimerManager().SetTimer(
                FTimerHandle(),
                this,
                &ANarr_DialogueSystem::OnAudioFinished,
                Line.Duration,
                false
            );
        }

        UE_LOG(LogTemp, Warning, TEXT("Playing dialogue: %s - %s"), *Line.CharacterName, *Line.DialogueText);
    }
}

void ANarr_DialogueSystem::PlayNextDialogue()
{
    if (CurrentCharacter.DialogueLines.IsValidIndex(CurrentDialogueIndex + 1))
    {
        PlayDialogueLine(CurrentDialogueIndex + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No more dialogue lines for character: %s"), *CurrentCharacter.CharacterName);
        StopDialogue();
    }
}

void ANarr_DialogueSystem::StopDialogue()
{
    bIsPlayingDialogue = false;
    CurrentDialogueIndex = 0;

    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue stopped"));
}

FNarr_DialogueLine ANarr_DialogueSystem::GetDialogueByContext(ESurvivalContext Context)
{
    for (const FNarr_DialogueLine& Line : CurrentCharacter.DialogueLines)
    {
        if (Line.Context == Context)
        {
            return Line;
        }
    }

    // Return first line if no context match
    if (CurrentCharacter.DialogueLines.Num() > 0)
    {
        return CurrentCharacter.DialogueLines[0];
    }

    return FNarr_DialogueLine();
}

TArray<FString> ANarr_DialogueSystem::GetAvailableCharacters()
{
    TArray<FString> AvailableNames;
    
    for (const FNarr_CharacterProfile& Character : Characters)
    {
        if (Character.bIsAvailable)
        {
            AvailableNames.Add(Character.CharacterName);
        }
    }

    return AvailableNames;
}

void ANarr_DialogueSystem::TestDialogueSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing Dialogue System..."));
    
    if (Characters.Num() > 0)
    {
        SetCurrentCharacter(Characters[0].CharacterName);
        PlayDialogueLine(0);
    }
}

void ANarr_DialogueSystem::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(OtherActor))
    {
        PlayerCharacter = Character;
        bPlayerInRange = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Player entered dialogue range"));
        
        // Visual feedback
        if (VisualMarker)
        {
            VisualMarker->SetVisibility(true);
        }
    }
}

void ANarr_DialogueSystem::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(OtherActor))
    {
        PlayerCharacter = nullptr;
        bPlayerInRange = false;
        
        UE_LOG(LogTemp, Warning, TEXT("Player left dialogue range"));
        
        // Stop current dialogue
        StopDialogue();
        
        // Hide visual feedback
        if (VisualMarker)
        {
            VisualMarker->SetVisibility(false);
        }
    }
}

void ANarr_DialogueSystem::OnAudioFinished()
{
    bIsPlayingDialogue = false;
    UE_LOG(LogTemp, Warning, TEXT("Audio finished, dialogue complete"));
}

void ANarr_DialogueSystem::CreateCharacterProfiles()
{
    Characters.Empty();

    // Predator Stalker Character
    FNarr_CharacterProfile PredatorStalker;
    PredatorStalker.CharacterName = TEXT("Predator Stalker");
    PredatorStalker.BackgroundStory = TEXT("A seasoned tracker who has survived countless predator encounters. Knows the hunting patterns of the apex predators and teaches others how to avoid becoming prey.");
    PredatorStalker.Expertise = ESurvivalSkill::Combat;
    PredatorStalker.bIsAvailable = true;

    FNarr_DialogueLine PredatorLine;
    PredatorLine.CharacterName = PredatorStalker.CharacterName;
    PredatorLine.DialogueText = TEXT("The ancient predator circles your camp, its massive footsteps shaking the ground beneath you. This is no ordinary hunt - it has been tracking you for hours, waiting for the perfect moment to strike.");
    PredatorLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777714373019_PredatorStalker.mp3");
    PredatorLine.Duration = 18.0f;
    PredatorLine.Context = ESurvivalContext::Combat;
    PredatorStalker.DialogueLines.Add(PredatorLine);

    Characters.Add(PredatorStalker);

    // Tribal Elder Character
    FNarr_CharacterProfile TribalElder;
    TribalElder.CharacterName = TEXT("Tribal Elder");
    TribalElder.BackgroundStory = TEXT("The oldest and wisest member of the tribe. Has observed the migration patterns of herbivores for decades and understands the ancient rhythms of the prehistoric world.");
    TribalElder.Expertise = ESurvivalSkill::Tracking;
    TribalElder.bIsAvailable = true;

    FNarr_DialogueLine ElderLine;
    ElderLine.CharacterName = TribalElder.CharacterName;
    ElderLine.DialogueText = TEXT("Listen carefully, young survivor. The herbivores migrate through this valley every season, following the same ancient paths their ancestors used. If you learn to read their signs, you can predict where they'll be.");
    ElderLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777714376046_TribalElder.mp3");
    ElderLine.Duration = 18.0f;
    ElderLine.Context = ESurvivalContext::Exploration;
    TribalElder.DialogueLines.Add(ElderLine);

    Characters.Add(TribalElder);

    // Water Scout Character
    FNarr_CharacterProfile WaterScout;
    WaterScout.CharacterName = TEXT("Water Scout");
    WaterScout.BackgroundStory = TEXT("A specialist in finding and securing water sources. Understands the dangers that lurk around watering holes and has developed strategies for safe water collection.");
    WaterScout.Expertise = ESurvivalSkill::Gathering;
    WaterScout.bIsAvailable = true;

    FNarr_DialogueLine WaterLine;
    WaterLine.CharacterName = WaterScout.CharacterName;
    WaterLine.DialogueText = TEXT("Water means life, but it also means danger. Every watering hole is a battlefield where predators wait in ambush. Approach slowly, watch for ripples that shouldn't be there, and always have an escape route planned.");
    WaterLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777714378354_WaterScout.mp3");
    WaterLine.Duration = 15.0f;
    WaterLine.Context = ESurvivalContext::ResourceGathering;
    WaterScout.DialogueLines.Add(WaterLine);

    Characters.Add(WaterScout);

    // Pack Hunter Expert Character
    FNarr_CharacterProfile PackExpert;
    PackExpert.CharacterName = TEXT("Pack Hunter Expert");
    PackExpert.BackgroundStory = TEXT("A survivor who has studied the pack hunting behavior of raptors and other intelligent predators. Knows their tactics and can teach others how to counter their coordinated attacks.");
    PackExpert.Expertise = ESurvivalSkill::Combat;
    PackExpert.bIsAvailable = true;

    FNarr_DialogueLine PackLine;
    PackLine.CharacterName = PackExpert.CharacterName;
    PackLine.DialogueText = TEXT("The pack hunters work together with deadly precision. When you see one raptor, know that two others are already flanking you. Their intelligence rivals our own - never underestimate their ability to learn and adapt.");
    PackLine.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777714380429_PackHunterExpert.mp3");
    PackLine.Duration = 15.0f;
    PackLine.Context = ESurvivalContext::Combat;
    PackExpert.DialogueLines.Add(PackLine);

    Characters.Add(PackExpert);

    UE_LOG(LogTemp, Warning, TEXT("Created %d character profiles"), Characters.Num());
}

void ANarr_DialogueSystem::LoadAudioFromURL(const FString& AudioURL)
{
    // For now, just log the audio URL
    // In a full implementation, this would download and play the audio
    UE_LOG(LogTemp, Warning, TEXT("Loading audio from URL: %s"), *AudioURL);
    
    // Simulate audio playback with timer
    if (!AudioURL.IsEmpty())
    {
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &ANarr_DialogueSystem::OnAudioFinished,
            CurrentCharacter.DialogueLines[CurrentDialogueIndex].Duration,
            false
        );
    }
}