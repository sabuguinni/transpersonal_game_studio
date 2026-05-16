#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger mesh (invisible sphere for proximity detection)
    TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
    TriggerMesh->SetupAttachment(RootComponent);
    TriggerMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create audio component for dialogue playback
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize default values
    AssociatedBiome = EBiomeType::Savana;
    bAutoPlayOnTrigger = true;
    DialogueVolume = 1.0f;
    bHasTriggered = false;
    LastTriggerTime = 0.0f;
    CurrentDialogueIndex = 0;
    bIsPlayingDialogue = false;
    CurrentDialogueTimer = 0.0f;

    // Set default trigger settings
    TriggerSettings.TriggerType = ENarr_TriggerCondition::ProximityEnter;
    TriggerSettings.TriggerRadius = 500.0f;
    TriggerSettings.bOnlyTriggerOnce = false;
    TriggerSettings.CooldownTime = 10.0f;
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDialogueSystem();
    LoadBiomeSpecificDialogue();

    // Set trigger mesh scale based on trigger radius
    if (TriggerMesh)
    {
        float Scale = TriggerSettings.TriggerRadius / 100.0f; // Assuming 100 unit base radius
        TriggerMesh->SetWorldScale3D(FVector(Scale, Scale, Scale));
    }

    UE_LOG(LogTemp, Log, TEXT("Dialogue System initialized with %d dialogue lines"), DialogueLines.Num());
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle dialogue playback timing
    if (bIsPlayingDialogue && CurrentDialogueTimer > 0.0f)
    {
        CurrentDialogueTimer -= DeltaTime;
        if (CurrentDialogueTimer <= 0.0f)
        {
            PlayNextDialogueLine();
        }
    }

    // Check proximity trigger if enabled
    if (TriggerSettings.TriggerType == ENarr_TriggerCondition::ProximityEnter)
    {
        CheckProximityTrigger();
    }
}

void ANarr_DialogueSystem::InitializeDialogueSystem()
{
    // Initialize with default survival dialogue lines
    if (DialogueLines.Num() == 0)
    {
        FNarr_DialogueLine SurvivalTip;
        SurvivalTip.DialogueText = TEXT("Stay alert, survivor. The ancient valley holds many dangers.");
        SurvivalTip.CharacterName = TEXT("Survival Guide");
        SurvivalTip.Duration = 4.0f;
        SurvivalTip.DialogueType = ENarr_DialogueType::SurvivalTip;
        SurvivalTip.bIsNarration = true;
        DialogueLines.Add(SurvivalTip);

        FNarr_DialogueLine DangerWarning;
        DangerWarning.DialogueText = TEXT("Warning! Predators hunt in packs. Listen for their calls.");
        DangerWarning.CharacterName = TEXT("Danger Alert");
        DangerWarning.Duration = 3.5f;
        DangerWarning.DialogueType = ENarr_DialogueType::DangerWarning;
        DangerWarning.bIsNarration = true;
        DialogueLines.Add(DangerWarning);
    }
}

void ANarr_DialogueSystem::LoadBiomeSpecificDialogue()
{
    // Clear existing dialogue and load biome-specific content
    DialogueLines.Empty();

    switch (AssociatedBiome)
    {
        case EBiomeType::Savana:
        {
            FNarr_DialogueLine SavanaLine;
            SavanaLine.DialogueText = TEXT("The open savana offers little shelter. Watch the horizon for movement.");
            SavanaLine.CharacterName = TEXT("Plains Guide");
            SavanaLine.Duration = 4.0f;
            SavanaLine.DialogueType = ENarr_DialogueType::Environmental;
            DialogueLines.Add(SavanaLine);
            break;
        }
        case EBiomeType::Floresta:
        {
            FNarr_DialogueLine ForestLine;
            ForestLine.DialogueText = TEXT("The forest provides cover, but predators lurk in the shadows between trees.");
            ForestLine.CharacterName = TEXT("Forest Watcher");
            ForestLine.Duration = 4.5f;
            ForestLine.DialogueType = ENarr_DialogueType::Environmental;
            DialogueLines.Add(ForestLine);
            break;
        }
        case EBiomeType::Pantano:
        {
            FNarr_DialogueLine SwampLine;
            SwampLine.DialogueText = TEXT("Beware the swamp waters. What lurks beneath may be more dangerous than what stalks above.");
            SwampLine.CharacterName = TEXT("Swamp Survivor");
            SwampLine.Duration = 5.0f;
            SwampLine.DialogueType = ENarr_DialogueType::DangerWarning;
            DialogueLines.Add(SwampLine);
            break;
        }
        case EBiomeType::Deserto:
        {
            FNarr_DialogueLine DesertLine;
            DesertLine.DialogueText = TEXT("The desert sun is merciless. Seek shade and conserve your water.");
            DesertLine.CharacterName = TEXT("Desert Wanderer");
            DesertLine.Duration = 4.0f;
            DesertLine.DialogueType = ENarr_DialogueType::SurvivalTip;
            DialogueLines.Add(DesertLine);
            break;
        }
        case EBiomeType::Montanha:
        {
            FNarr_DialogueLine MountainLine;
            MountainLine.DialogueText = TEXT("High ground offers advantage, but the climb leaves you exposed. Choose your path wisely.");
            MountainLine.CharacterName = TEXT("Mountain Guide");
            MountainLine.Duration = 4.5f;
            MountainLine.DialogueType = ENarr_DialogueType::SurvivalTip;
            DialogueLines.Add(MountainLine);
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded %d biome-specific dialogue lines for %s"), 
           DialogueLines.Num(), 
           *UEnum::GetValueAsString(AssociatedBiome));
}

void ANarr_DialogueSystem::TriggerDialogue()
{
    if (bIsPlayingDialogue)
    {
        return; // Already playing dialogue
    }

    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastTriggerTime < TriggerSettings.CooldownTime)
    {
        return;
    }

    // Check if should only trigger once
    if (TriggerSettings.bOnlyTriggerOnce && bHasTriggered)
    {
        return;
    }

    if (DialogueLines.Num() > 0)
    {
        bIsPlayingDialogue = true;
        CurrentDialogueIndex = 0;
        bHasTriggered = true;
        LastTriggerTime = CurrentTime;
        
        PlayNextDialogueLine();
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue triggered: %s"), *DialogueLines[0].DialogueText);
    }
}

void ANarr_DialogueSystem::PlayNextDialogueLine()
{
    if (CurrentDialogueIndex >= DialogueLines.Num())
    {
        OnDialogueComplete();
        return;
    }

    const FNarr_DialogueLine& CurrentLine = DialogueLines[CurrentDialogueIndex];
    
    // Display dialogue text (in a real game, this would go to UI)
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                            *CurrentLine.CharacterName, 
                                            *CurrentLine.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, CurrentLine.Duration, FColor::Yellow, DisplayText);
    }

    // Set timer for next line
    CurrentDialogueTimer = CurrentLine.Duration;
    CurrentDialogueIndex++;

    UE_LOG(LogTemp, Log, TEXT("Playing dialogue line %d: %s"), 
           CurrentDialogueIndex, *CurrentLine.DialogueText);
}

void ANarr_DialogueSystem::StopDialogue()
{
    bIsPlayingDialogue = false;
    CurrentDialogueTimer = 0.0f;
    CurrentDialogueIndex = 0;

    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }

    UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
}

void ANarr_DialogueSystem::AddDialogueLine(const FNarr_DialogueLine& NewLine)
{
    DialogueLines.Add(NewLine);
    UE_LOG(LogTemp, Log, TEXT("Added dialogue line: %s"), *NewLine.DialogueText);
}

bool ANarr_DialogueSystem::CheckTriggerConditions(AActor* TriggeringActor)
{
    if (!TriggeringActor)
    {
        return false;
    }

    // Check if triggering actor is a player character
    if (!TriggeringActor->IsA<ACharacter>())
    {
        return false;
    }

    // Check tag requirements
    if (!TriggerSettings.RequiredTag.IsEmpty())
    {
        if (!TriggeringActor->ActorHasTag(FName(*TriggerSettings.RequiredTag)))
        {
            return false;
        }
    }

    return true;
}

void ANarr_DialogueSystem::SetBiomeContext(EBiomeType NewBiome)
{
    if (AssociatedBiome != NewBiome)
    {
        AssociatedBiome = NewBiome;
        LoadBiomeSpecificDialogue();
    }
}

void ANarr_DialogueSystem::OnDialogueComplete()
{
    bIsPlayingDialogue = false;
    CurrentDialogueTimer = 0.0f;
    CurrentDialogueIndex = 0;

    UE_LOG(LogTemp, Log, TEXT("Dialogue sequence completed"));

    // Broadcast completion event (in a real game, this would trigger delegates)
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Dialogue Complete"));
    }
}

void ANarr_DialogueSystem::CheckProximityTrigger()
{
    if (!GetWorld())
    {
        return;
    }

    // Find nearby player characters
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor && CheckTriggerConditions(Actor))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= TriggerSettings.TriggerRadius)
            {
                if (bAutoPlayOnTrigger)
                {
                    TriggerDialogue();
                }
                break;
            }
        }
    }
}