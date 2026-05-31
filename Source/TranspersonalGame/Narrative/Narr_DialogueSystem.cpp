#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// UNarr_DialogueComponent Implementation
UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    TriggerRadius = 500.0f;
    bIsActive = true;
    bIsPlaying = false;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    PlayerActor = nullptr;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player actor
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerActor = PC->GetPawn();
        }
    }
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsActive && !bIsPlaying)
    {
        CheckPlayerProximity();
    }
    
    if (bIsPlaying)
    {
        UpdateDialogueDisplay();
        
        if (DialogueSequence.bAutoAdvance)
        {
            CurrentLineTimer += DeltaTime;
            if (CurrentLineTimer >= DialogueSequence.AutoAdvanceDelay)
            {
                NextLine();
            }
        }
    }
}

void UNarr_DialogueComponent::StartDialogue()
{
    if (DialogueSequence.DialogueLines.Num() > 0)
    {
        bIsPlaying = true;
        CurrentLineIndex = 0;
        CurrentLineTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Dialogue started with %d lines"), DialogueSequence.DialogueLines.Num());
    }
}

void UNarr_DialogueComponent::StopDialogue()
{
    bIsPlaying = false;
    CurrentLineIndex = 0;
    CurrentLineTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue stopped"));
}

void UNarr_DialogueComponent::NextLine()
{
    if (HasMoreLines())
    {
        CurrentLineIndex++;
        CurrentLineTimer = 0.0f;
        
        if (!HasMoreLines())
        {
            StopDialogue();
        }
    }
}

void UNarr_DialogueComponent::AddDialogueLine(const FString& Speaker, const FString& Text, ENarr_DialogueType Type)
{
    FNarr_DialogueLine NewLine;
    NewLine.SpeakerName = Speaker;
    NewLine.DialogueText = Text;
    NewLine.DialogueType = Type;
    NewLine.DisplayDuration = 3.0f;
    
    DialogueSequence.DialogueLines.Add(NewLine);
}

FNarr_DialogueLine UNarr_DialogueComponent::GetCurrentLine() const
{
    if (CurrentLineIndex >= 0 && CurrentLineIndex < DialogueSequence.DialogueLines.Num())
    {
        return DialogueSequence.DialogueLines[CurrentLineIndex];
    }
    
    return FNarr_DialogueLine();
}

bool UNarr_DialogueComponent::HasMoreLines() const
{
    return CurrentLineIndex < DialogueSequence.DialogueLines.Num() - 1;
}

void UNarr_DialogueComponent::CheckPlayerProximity()
{
    if (!PlayerActor || !GetOwner())
        return;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    
    if (Distance <= TriggerRadius)
    {
        StartDialogue();
    }
}

void UNarr_DialogueComponent::UpdateDialogueDisplay()
{
    if (bIsPlaying && CurrentLineIndex < DialogueSequence.DialogueLines.Num())
    {
        FNarr_DialogueLine CurrentLine = GetCurrentLine();
        
        // Display dialogue on screen
        if (GEngine)
        {
            FString DisplayText = FString::Printf(TEXT("%s: %s"), *CurrentLine.SpeakerName, *CurrentLine.DialogueText);
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::White, DisplayText);
        }
    }
}

// ANarr_DialogueTrigger Implementation
ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bTriggerOnce = true;
    bRequirePlayerInput = false;
    bHasTriggered = false;
    
    OnActorBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnActorBeginOverlap);
    OnActorEndOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnActorEndOverlap);
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
}

void ANarr_DialogueTrigger::OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!bHasTriggered || !bTriggerOnce)
    {
        if (OtherActor && OtherActor->IsA<APawn>())
        {
            TriggerDialogue(OtherActor);
        }
    }
}

void ANarr_DialogueTrigger::OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    // Handle overlap end if needed
}

void ANarr_DialogueTrigger::TriggerDialogue(AActor* TriggeringActor)
{
    if (TriggerDialogue.DialogueLines.Num() > 0)
    {
        bHasTriggered = true;
        
        // Display dialogue lines
        for (int32 i = 0; i < TriggerDialogue.DialogueLines.Num(); i++)
        {
            FNarr_DialogueLine Line = TriggerDialogue.DialogueLines[i];
            
            if (GEngine)
            {
                FString DisplayText = FString::Printf(TEXT("%s: %s"), *Line.SpeakerName, *Line.DialogueText);
                GEngine->AddOnScreenDebugMessage(-1, Line.DisplayDuration, FColor::Yellow, DisplayText);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Dialogue Trigger: %s - %s"), *Line.SpeakerName, *Line.DialogueText);
        }
    }
}

void ANarr_DialogueTrigger::ResetTrigger()
{
    bHasTriggered = false;
}

// ANarr_NarrativeNPC Implementation
ANarr_NarrativeNPC::ANarr_NarrativeNPC()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    
    // Create dialogue component
    DialogueComponent = CreateDefaultSubobject<UNarr_DialogueComponent>(TEXT("DialogueComponent"));
    
    // Set default values
    NPCName = TEXT("Tribal Member");
    NPCRole = TEXT("Survivor");
    InteractionRange = 300.0f;
    bPlayerInRange = false;
    
    // Load default mesh (cube for placeholder)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(CubeMeshAsset.Object);
        MeshComponent->SetWorldScale3D(FVector(1.0f, 1.0f, 2.0f)); // Make it person-sized
    }
}

void ANarr_NarrativeNPC::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultDialogue();
}

void ANarr_NarrativeNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CheckPlayerDistance();
}

void ANarr_NarrativeNPC::StartConversation(AActor* Player)
{
    if (DialogueComponent)
    {
        DialogueComponent->StartDialogue();
        UE_LOG(LogTemp, Log, TEXT("NPC %s started conversation"), *NPCName);
    }
}

void ANarr_NarrativeNPC::EndConversation()
{
    if (DialogueComponent)
    {
        DialogueComponent->StopDialogue();
        UE_LOG(LogTemp, Log, TEXT("NPC %s ended conversation"), *NPCName);
    }
}

void ANarr_NarrativeNPC::AddGreeting(const FString& Greeting)
{
    GreetingLines.Add(Greeting);
}

void ANarr_NarrativeNPC::AddInformation(const FString& Information)
{
    InformationLines.Add(Information);
}

void ANarr_NarrativeNPC::CheckPlayerDistance()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* Player = PC->GetPawn())
            {
                float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
                bool bWasInRange = bPlayerInRange;
                bPlayerInRange = Distance <= InteractionRange;
                
                // Player just entered range
                if (bPlayerInRange && !bWasInRange)
                {
                    StartConversation(Player);
                }
                // Player just left range
                else if (!bPlayerInRange && bWasInRange)
                {
                    EndConversation();
                }
            }
        }
    }
}

void ANarr_NarrativeNPC::SetupDefaultDialogue()
{
    if (DialogueComponent)
    {
        // Add default greetings
        if (GreetingLines.Num() == 0)
        {
            GreetingLines.Add(TEXT("Greetings, stranger. These lands are dangerous."));
            GreetingLines.Add(TEXT("Stay alert. The predators hunt at dusk."));
            GreetingLines.Add(TEXT("Welcome to our territory, traveler."));
        }
        
        // Add default information
        if (InformationLines.Num() == 0)
        {
            InformationLines.Add(TEXT("The T-Rex roams the eastern plains."));
            InformationLines.Add(TEXT("Fresh water can be found near the great trees."));
            InformationLines.Add(TEXT("Avoid the swamplands after dark."));
        }
        
        // Setup dialogue sequence
        for (const FString& Greeting : GreetingLines)
        {
            DialogueComponent->AddDialogueLine(NPCName, Greeting, ENarr_DialogueType::Information);
        }
        
        for (const FString& Info : InformationLines)
        {
            DialogueComponent->AddDialogueLine(NPCName, Info, ENarr_DialogueType::Information);
        }
    }
}