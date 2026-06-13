#include "Quest_DialogueManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AQuest_DialogueManager::AQuest_DialogueManager()
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

    // Create dialogue marker mesh
    DialogueMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DialogueMarker"));
    DialogueMarker->SetupAttachment(RootComponent);

    // Initialize default values
    InteractionRange = 300.0f;
    bIsInConversation = false;
    bPlayerInRange = false;
    NPCName = FText::FromString("Survivor");
    CurrentConversationID = "";
    CurrentDialogueID = "";

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_DialogueManager::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_DialogueManager::OnInteractionSphereEndOverlap);
}

void AQuest_DialogueManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize default conversation trees if none exist
    if (ConversationTrees.Num() == 0)
    {
        // Create tutorial conversation
        FQuest_ConversationTree TutorialTree;
        TutorialTree.ConversationID = "tutorial_basic";
        TutorialTree.StartDialogueID = "tutorial_start";

        FQuest_DialogueEntry StartEntry;
        StartEntry.DialogueID = "tutorial_start";
        StartEntry.DialogueText = FText::FromString("Welcome, survivor. This world is harsh and unforgiving. The dinosaurs rule here, not us.");
        StartEntry.ResponseOptions.Add(FText::FromString("Tell me what I need to know."));
        StartEntry.ResponseOptions.Add(FText::FromString("I can handle myself."));
        StartEntry.NextDialogueIDs.Add("tutorial_advice");
        StartEntry.NextDialogueIDs.Add("tutorial_warning");
        StartEntry.bIsQuestStart = false;

        FQuest_DialogueEntry AdviceEntry;
        AdviceEntry.DialogueID = "tutorial_advice";
        AdviceEntry.DialogueText = FText::FromString("Stay quiet, move carefully. Hunt the small ones first. Build shelter before nightfall.");
        AdviceEntry.ResponseOptions.Add(FText::FromString("Any other advice?"));
        AdviceEntry.ResponseOptions.Add(FText::FromString("I understand."));
        AdviceEntry.NextDialogueIDs.Add("tutorial_resources");
        AdviceEntry.NextDialogueIDs.Add("tutorial_end");

        FQuest_DialogueEntry WarningEntry;
        WarningEntry.DialogueID = "tutorial_warning";
        WarningEntry.DialogueText = FText::FromString("Overconfidence will get you killed. The raptors hunt in packs. The T-Rex fears nothing.");
        AdviceEntry.ResponseOptions.Add(FText::FromString("I'll be careful."));
        AdviceEntry.NextDialogueIDs.Add("tutorial_end");

        FQuest_DialogueEntry ResourcesEntry;
        ResourcesEntry.DialogueID = "tutorial_resources";
        ResourcesEntry.DialogueText = FText::FromString("Gather stones for tools, sticks for spears. Fresh water is life. Cooked meat lasts longer.");
        ResourcesEntry.ResponseOptions.Add(FText::FromString("Thank you for the guidance."));
        ResourcesEntry.NextDialogueIDs.Add("tutorial_end");

        FQuest_DialogueEntry EndEntry;
        EndEntry.DialogueID = "tutorial_end";
        EndEntry.DialogueText = FText::FromString("Survive another day, and maybe we'll talk again.");
        EndEntry.bIsQuestEnd = true;

        TutorialTree.DialogueEntries.Add(StartEntry);
        TutorialTree.DialogueEntries.Add(AdviceEntry);
        TutorialTree.DialogueEntries.Add(WarningEntry);
        TutorialTree.DialogueEntries.Add(ResourcesEntry);
        TutorialTree.DialogueEntries.Add(EndEntry);

        ConversationTrees.Add(TutorialTree);

        // Create hunting quest conversation
        FQuest_ConversationTree HuntingTree;
        HuntingTree.ConversationID = "hunting_raptors";
        HuntingTree.StartDialogueID = "hunting_start";

        FQuest_DialogueEntry HuntStart;
        HuntStart.DialogueID = "hunting_start";
        HuntStart.DialogueText = FText::FromString("The raptor pack grows bolder. They killed two of ours yesterday.");
        HuntStart.ResponseOptions.Add(FText::FromString("I'll hunt them down."));
        HuntStart.ResponseOptions.Add(FText::FromString("We should defend instead."));
        HuntStart.NextDialogueIDs.Add("hunting_accept");
        HuntStart.NextDialogueIDs.Add("hunting_decline");
        HuntStart.QuestID = "hunt_raptor_pack";
        HuntStart.bIsQuestStart = true;

        FQuest_DialogueEntry HuntAccept;
        HuntAccept.DialogueID = "hunting_accept";
        HuntAccept.DialogueText = FText::FromString("Good. They nest in the eastern ravine. Three adults, maybe more. Bring back their claws as proof.");
        HuntAccept.ResponseOptions.Add(FText::FromString("I'll return with their claws."));
        HuntAccept.NextDialogueIDs.Add("hunting_end");

        FQuest_DialogueEntry HuntDecline;
        HuntDecline.DialogueID = "hunting_decline";
        HuntDecline.DialogueText = FText::FromString("Defense won't work forever. They're learning our patterns.");
        HuntDecline.ResponseOptions.Add(FText::FromString("Maybe you're right."));
        HuntDecline.NextDialogueIDs.Add("hunting_accept");

        FQuest_DialogueEntry HuntEnd;
        HuntEnd.DialogueID = "hunting_end";
        HuntEnd.DialogueText = FText::FromString("Be careful out there. Raptors are smarter than they look.");
        HuntEnd.bIsQuestEnd = true;

        HuntingTree.DialogueEntries.Add(HuntStart);
        HuntingTree.DialogueEntries.Add(HuntAccept);
        HuntingTree.DialogueEntries.Add(HuntDecline);
        HuntingTree.DialogueEntries.Add(HuntEnd);

        ConversationTrees.Add(HuntingTree);
    }
}

void AQuest_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update interaction availability based on player proximity
    bool bCurrentlyInRange = IsPlayerInRange();
    if (bCurrentlyInRange != bPlayerInRange)
    {
        bPlayerInRange = bCurrentlyInRange;
        // Could trigger UI updates here
    }
}

bool AQuest_DialogueManager::StartConversation(const FString& ConversationID)
{
    if (bIsInConversation)
    {
        return false;
    }

    FQuest_ConversationTree* Tree = FindConversationTree(ConversationID);
    if (!Tree || Tree->StartDialogueID.IsEmpty())
    {
        return false;
    }

    CurrentConversationID = ConversationID;
    CurrentDialogueID = Tree->StartDialogueID;
    bIsInConversation = true;

    OnConversationStarted();
    
    FQuest_DialogueEntry CurrentEntry = GetCurrentDialogue();
    OnDialogueChanged(CurrentEntry);

    return true;
}

void AQuest_DialogueManager::EndConversation()
{
    if (!bIsInConversation)
    {
        return;
    }

    bIsInConversation = false;
    CurrentConversationID = "";
    CurrentDialogueID = "";

    OnConversationEnded();
}

FQuest_DialogueEntry AQuest_DialogueManager::GetCurrentDialogue()
{
    FQuest_DialogueEntry EmptyEntry;
    
    if (!bIsInConversation || CurrentDialogueID.IsEmpty())
    {
        return EmptyEntry;
    }

    FQuest_DialogueEntry* Entry = FindDialogueEntry(CurrentDialogueID);
    return Entry ? *Entry : EmptyEntry;
}

bool AQuest_DialogueManager::SelectResponse(int32 ResponseIndex)
{
    if (!bIsInConversation)
    {
        return false;
    }

    FQuest_DialogueEntry* CurrentEntry = FindDialogueEntry(CurrentDialogueID);
    if (!CurrentEntry || ResponseIndex < 0 || ResponseIndex >= CurrentEntry->NextDialogueIDs.Num())
    {
        return false;
    }

    FString NextDialogueID = CurrentEntry->NextDialogueIDs[ResponseIndex];
    
    // Check if this ends the conversation
    FQuest_DialogueEntry* NextEntry = FindDialogueEntry(NextDialogueID);
    if (NextEntry && NextEntry->bIsQuestEnd)
    {
        CurrentDialogueID = NextDialogueID;
        OnDialogueChanged(*NextEntry);
        EndConversation();
        return true;
    }

    // Continue to next dialogue
    CurrentDialogueID = NextDialogueID;
    if (NextEntry)
    {
        OnDialogueChanged(*NextEntry);
    }

    return true;
}

TArray<FText> AQuest_DialogueManager::GetCurrentResponseOptions()
{
    TArray<FText> EmptyOptions;
    
    if (!bIsInConversation)
    {
        return EmptyOptions;
    }

    FQuest_DialogueEntry* Entry = FindDialogueEntry(CurrentDialogueID);
    return Entry ? Entry->ResponseOptions : EmptyOptions;
}

bool AQuest_DialogueManager::IsPlayerInRange()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= InteractionRange;
}

void AQuest_DialogueManager::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInRange = true;
    }
}

void AQuest_DialogueManager::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInRange = false;
        if (bIsInConversation)
        {
            EndConversation();
        }
    }
}

FQuest_DialogueEntry* AQuest_DialogueManager::FindDialogueEntry(const FString& DialogueID)
{
    if (CurrentConversationID.IsEmpty())
    {
        return nullptr;
    }

    FQuest_ConversationTree* Tree = FindConversationTree(CurrentConversationID);
    if (!Tree)
    {
        return nullptr;
    }

    for (FQuest_DialogueEntry& Entry : Tree->DialogueEntries)
    {
        if (Entry.DialogueID == DialogueID)
        {
            return &Entry;
        }
    }

    return nullptr;
}

FQuest_ConversationTree* AQuest_DialogueManager::FindConversationTree(const FString& ConversationID)
{
    for (FQuest_ConversationTree& Tree : ConversationTrees)
    {
        if (Tree.ConversationID == ConversationID)
        {
            return &Tree;
        }
    }

    return nullptr;
}