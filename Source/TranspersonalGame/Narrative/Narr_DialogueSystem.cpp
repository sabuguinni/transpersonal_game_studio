#include "Narr_DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(300.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create visual mesh
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initialize properties
    NPCName = TEXT("Tribal Elder");
    NPCType = ENarr_DialogueType::Tribal;
    bCanRepeatDialogue = true;
    InteractionRange = 300.0f;
    bHasBeenTriggered = false;
    CurrentDialogueIndex = 0;

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnTriggerEnter);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnTriggerExit);
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDialogueLines();
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueSystem::InitializeDialogueLines()
{
    DialogueLines.Empty();

    switch (NPCType)
    {
        case ENarr_DialogueType::Tribal:
            SetupTribalDialogue();
            break;
        case ENarr_DialogueType::Warning:
            SetupSurvivalWarnings();
            break;
        default:
            SetupTribalDialogue();
            break;
    }
}

void ANarr_DialogueSystem::SetupTribalDialogue()
{
    FNarr_DialogueLine Line1;
    Line1.SpeakerName = NPCName;
    Line1.DialogueText = TEXT("Greetings, survivor. The ancient ways teach us to read the signs of the great beasts.");
    Line1.DialogueType = ENarr_DialogueType::Tribal;
    Line1.Duration = 4.0f;
    Line1.bRequiresTrigger = false;
    DialogueLines.Add(Line1);

    FNarr_DialogueLine Line2;
    Line2.SpeakerName = NPCName;
    Line2.DialogueText = TEXT("The thunder lizard marks its territory with deep footprints. Follow them to find water, but beware its wrath.");
    Line2.DialogueType = ENarr_DialogueType::Lore;
    Line2.Duration = 5.0f;
    Line2.bRequiresTrigger = false;
    DialogueLines.Add(Line2);

    FNarr_DialogueLine Line3;
    Line3.SpeakerName = NPCName;
    Line3.DialogueText = TEXT("The pack hunters work together. If you see one, there are always more watching from the shadows.");
    Line3.DialogueType = ENarr_DialogueType::Warning;
    Line3.Duration = 4.5f;
    Line3.bRequiresTrigger = false;
    DialogueLines.Add(Line3);

    FNarr_DialogueLine Line4;
    Line4.SpeakerName = NPCName;
    Line4.DialogueText = TEXT("Our ancestors left markings on the sacred stones. Learn their wisdom, and you may yet survive this harsh world.");
    Line4.DialogueType = ENarr_DialogueType::Lore;
    Line4.Duration = 5.5f;
    Line4.bRequiresTrigger = false;
    DialogueLines.Add(Line4);
}

void ANarr_DialogueSystem::SetupSurvivalWarnings()
{
    FNarr_DialogueLine Warning1;
    Warning1.SpeakerName = TEXT("Survival Instinct");
    Warning1.DialogueText = TEXT("Danger detected. Large predator approaching from the north. Seek high ground immediately.");
    Warning1.DialogueType = ENarr_DialogueType::Warning;
    Warning1.Duration = 3.5f;
    Warning1.bRequiresTrigger = true;
    DialogueLines.Add(Warning1);

    FNarr_DialogueLine Warning2;
    Warning2.SpeakerName = TEXT("Survival Instinct");
    Warning2.DialogueText = TEXT("Pack hunters detected. Multiple threats converging on your position. Find cover or prepare to fight.");
    Warning2.DialogueType = ENarr_DialogueType::Warning;
    Warning2.Duration = 4.0f;
    Warning2.bRequiresTrigger = true;
    DialogueLines.Add(Warning2);

    FNarr_DialogueLine Warning3;
    Warning3.SpeakerName = TEXT("Survival Instinct");
    Warning3.DialogueText = TEXT("Safe zone identified. Large herbivore presence indicates no immediate predator threat in this area.");
    Warning3.DialogueType = ENarr_DialogueType::Tutorial;
    Warning3.Duration = 4.5f;
    Warning3.bRequiresTrigger = true;
    DialogueLines.Add(Warning3);
}

void ANarr_DialogueSystem::TriggerDialogue()
{
    if (DialogueLines.Num() == 0)
    {
        return;
    }

    if (!bHasBeenTriggered || bCanRepeatDialogue)
    {
        bHasBeenTriggered = true;
        CurrentDialogueIndex = 0;

        FNarr_DialogueLine CurrentLine = GetCurrentDialogue();
        OnDialogueTriggered(CurrentLine);

        // Log dialogue for debugging
        if (GEngine)
        {
            FString LogMessage = FString::Printf(TEXT("[%s]: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
            GEngine->AddOnScreenDebugMessage(-1, CurrentLine.Duration, FColor::Yellow, LogMessage);
        }
    }
}

void ANarr_DialogueSystem::NextDialogueLine()
{
    if (HasMoreDialogue())
    {
        CurrentDialogueIndex++;
        if (CurrentDialogueIndex < DialogueLines.Num())
        {
            FNarr_DialogueLine CurrentLine = GetCurrentDialogue();
            OnDialogueTriggered(CurrentLine);

            // Log dialogue for debugging
            if (GEngine)
            {
                FString LogMessage = FString::Printf(TEXT("[%s]: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
                GEngine->AddOnScreenDebugMessage(-1, CurrentLine.Duration, FColor::Yellow, LogMessage);
            }
        }
    }
    else
    {
        OnDialogueCompleted();
        if (bCanRepeatDialogue)
        {
            CurrentDialogueIndex = 0;
        }
    }
}

FNarr_DialogueLine ANarr_DialogueSystem::GetCurrentDialogue() const
{
    if (DialogueLines.IsValidIndex(CurrentDialogueIndex))
    {
        return DialogueLines[CurrentDialogueIndex];
    }
    
    return FNarr_DialogueLine();
}

bool ANarr_DialogueSystem::HasMoreDialogue() const
{
    return CurrentDialogueIndex < DialogueLines.Num() - 1;
}

void ANarr_DialogueSystem::ResetDialogue()
{
    bHasBeenTriggered = false;
    CurrentDialogueIndex = 0;
}

void ANarr_DialogueSystem::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        // Check if it's the player character
        ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (OtherActor == PlayerCharacter)
        {
            TriggerDialogue();
        }
    }
}

void ANarr_DialogueSystem::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (OtherActor == PlayerCharacter)
        {
            // Player left trigger area - could implement dialogue interruption here
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Left dialogue area"));
            }
        }
    }
}