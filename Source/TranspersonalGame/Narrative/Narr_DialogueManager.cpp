#include "Narr_DialogueManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    SetupInteractionComponents();
    
    // Initialize default values
    CurrentStoryArcIndex = 0;
    InteractionRange = 300.0f;
    bAutoStartDialogue = true;
    bInDialogue = false;
    CurrentPlayer = nullptr;
    NPCName = TEXT("Tribal Elder");
    NPCRole = TEXT("Village Leader");
}

void ANarr_DialogueManager::SetupInteractionComponents()
{
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(InteractionRange);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Create NPC mesh component
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    NPCMesh->SetupAttachment(RootComponent);
    NPCMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    NPCMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueManager::OnInteractionSphereBeginOverlap);
    }
    
    // Initialize default story arcs
    InitializeDefaultStoryArcs();
}

void ANarr_DialogueManager::InitializeDefaultStoryArcs()
{
    // Main Story Arc - Introduction
    FNarr_StoryArc IntroArc;
    IntroArc.ArcName = TEXT("Tribal Introduction");
    IntroArc.ArcDescription = TEXT("First meeting with the tribal elder");
    
    // Opening dialogue node
    FNarr_DialogueNode OpeningNode;
    OpeningNode.SpeakerName = NPCName;
    OpeningNode.DialogueText = TEXT("Stranger... you survived the night. Few do in their first week. The dinosaurs grow bolder as winter approaches.");
    OpeningNode.ResponseOptions.Add(TEXT("How do I survive here?"));
    OpeningNode.ResponseOptions.Add(TEXT("What happened to this place?"));
    OpeningNode.ResponseOptions.Add(TEXT("I need to find others like me."));
    OpeningNode.NextNodeIndices = {1, 2, 3};
    OpeningNode.bIsEndNode = false;
    IntroArc.DialogueNodes.Add(OpeningNode);
    
    // Survival advice node
    FNarr_DialogueNode SurvivalNode;
    SurvivalNode.SpeakerName = NPCName;
    SurvivalNode.DialogueText = TEXT("Listen well. Stay downwind of the large predators. The raptors hunt in packs - never face more than one. Water sources are death traps at dawn and dusk.");
    SurvivalNode.ResponseOptions.Add(TEXT("What about shelter?"));
    SurvivalNode.ResponseOptions.Add(TEXT("Where can I find food?"));
    SurvivalNode.NextNodeIndices = {4, 5};
    SurvivalNode.bIsEndNode = false;
    IntroArc.DialogueNodes.Add(SurvivalNode);
    
    // History node
    FNarr_DialogueNode HistoryNode;
    HistoryNode.SpeakerName = NPCName;
    HistoryNode.DialogueText = TEXT("This was once a thriving valley. Then the great migration began. Herds of giants trampled our crops. Predators followed. We adapted or died.");
    HistoryNode.ResponseOptions.Add(TEXT("How many survived?"));
    HistoryNode.NextNodeIndices = {6};
    HistoryNode.bIsEndNode = false;
    IntroArc.DialogueNodes.Add(HistoryNode);
    
    // Community node
    FNarr_DialogueNode CommunityNode;
    CommunityNode.SpeakerName = NPCName;
    CommunityNode.DialogueText = TEXT("There are others. Scattered camps across the valley. But trust is earned with blood here. Prove yourself first.");
    CommunityNode.ResponseOptions.Add(TEXT("How do I prove myself?"));
    CommunityNode.NextNodeIndices = {7};
    CommunityNode.bIsEndNode = false;
    IntroArc.DialogueNodes.Add(CommunityNode);
    
    // Shelter advice node
    FNarr_DialogueNode ShelterNode;
    ShelterNode.SpeakerName = NPCName;
    ShelterNode.DialogueText = TEXT("High ground, always. Caves seem safe but become traps. Build lean-tos against rock faces. Multiple escape routes.");
    SurvivalNode.bIsEndNode = true;
    IntroArc.DialogueNodes.Add(ShelterNode);
    
    // Food advice node
    FNarr_DialogueNode FoodNode;
    FoodNode.SpeakerName = NPCName;
    FoodNode.DialogueText = TEXT("Berries and roots sustain you, but meat makes you strong. Hunt the small ones first. Learn their patterns. Patience feeds you, haste feeds them.");
    FoodNode.bIsEndNode = true;
    IntroArc.DialogueNodes.Add(FoodNode);
    
    // Survivors node
    FNarr_DialogueNode SurvivorsNode;
    SurvivorsNode.SpeakerName = NPCName;
    SurvivorsNode.DialogueText = TEXT("Of fifty families, perhaps twelve remain. We are scattered like seeds on stone. But seeds can grow, given time.");
    SurvivorsNode.bIsEndNode = true;
    IntroArc.DialogueNodes.Add(SurvivorsNode);
    
    // Proving node
    FNarr_DialogueNode ProvingNode;
    ProvingNode.SpeakerName = NPCName;
    ProvingNode.DialogueText = TEXT("Bring us meat from a raptor kill. Not scraps - a clean kill. Show us you can hunt, not just scavenge. Then we talk of joining our fire.");
    ProvingNode.bIsEndNode = true;
    IntroArc.DialogueNodes.Add(ProvingNode);
    
    StoryArcs.Add(IntroArc);
    
    // Hunting Quest Arc
    FNarr_StoryArc HuntingArc;
    HuntingArc.ArcName = TEXT("The Raptor Hunt");
    HuntingArc.ArcDescription = TEXT("Prove hunting skills by taking down a raptor");
    
    FNarr_DialogueNode HuntingIntro;
    HuntingIntro.SpeakerName = NPCName;
    HuntingIntro.DialogueText = TEXT("You return. Good. The pack that killed our hunters nests in the eastern ravine. Three adults, two juveniles. Take one adult - clean kill, no scavenging.");
    HuntingIntro.ResponseOptions.Add(TEXT("I'll need better weapons."));
    HuntingIntro.ResponseOptions.Add(TEXT("Tell me about their hunting patterns."));
    HuntingIntro.NextNodeIndices = {1, 2};
    HuntingIntro.bIsEndNode = false;
    HuntingArc.DialogueNodes.Add(HuntingIntro);
    
    StoryArcs.Add(HuntingArc);
}

void ANarr_DialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ANarr_DialogueManager::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
    {
        if (PlayerPawn->IsPlayerControlled() && bAutoStartDialogue && !bInDialogue)
        {
            StartDialogue(PlayerPawn);
        }
    }
}

void ANarr_DialogueManager::StartDialogue(APawn* PlayerPawn)
{
    if (!PlayerPawn || bInDialogue) return;
    
    CurrentPlayer = PlayerPawn;
    bInDialogue = true;
    
    if (StoryArcs.IsValidIndex(CurrentStoryArcIndex))
    {
        FNarr_StoryArc& CurrentArc = StoryArcs[CurrentStoryArcIndex];
        if (CurrentArc.DialogueNodes.IsValidIndex(CurrentArc.CurrentNodeIndex))
        {
            FNarr_DialogueNode& CurrentNode = CurrentArc.DialogueNodes[CurrentArc.CurrentNodeIndex];
            OnDialogueStarted(CurrentNode.SpeakerName, CurrentNode.DialogueText);
        }
    }
}

void ANarr_DialogueManager::ProcessPlayerResponse(int32 ResponseIndex)
{
    if (!bInDialogue || !StoryArcs.IsValidIndex(CurrentStoryArcIndex)) return;
    
    FNarr_StoryArc& CurrentArc = StoryArcs[CurrentStoryArcIndex];
    if (!CurrentArc.DialogueNodes.IsValidIndex(CurrentArc.CurrentNodeIndex)) return;
    
    FNarr_DialogueNode& CurrentNode = CurrentArc.DialogueNodes[CurrentArc.CurrentNodeIndex];
    
    if (CurrentNode.NextNodeIndices.IsValidIndex(ResponseIndex))
    {
        int32 NextNodeIndex = CurrentNode.NextNodeIndices[ResponseIndex];
        if (CurrentArc.DialogueNodes.IsValidIndex(NextNodeIndex))
        {
            CurrentArc.CurrentNodeIndex = NextNodeIndex;
            FNarr_DialogueNode& NextNode = CurrentArc.DialogueNodes[NextNodeIndex];
            
            if (NextNode.bIsEndNode)
            {
                EndDialogue();
            }
        }
    }
}

void ANarr_DialogueManager::EndDialogue()
{
    if (!bInDialogue) return;
    
    bInDialogue = false;
    CurrentPlayer = nullptr;
    OnDialogueEnded();
}

FNarr_DialogueNode ANarr_DialogueManager::GetCurrentDialogueNode()
{
    if (StoryArcs.IsValidIndex(CurrentStoryArcIndex))
    {
        FNarr_StoryArc& CurrentArc = StoryArcs[CurrentStoryArcIndex];
        if (CurrentArc.DialogueNodes.IsValidIndex(CurrentArc.CurrentNodeIndex))
        {
            return CurrentArc.DialogueNodes[CurrentArc.CurrentNodeIndex];
        }
    }
    
    return FNarr_DialogueNode();
}

void ANarr_DialogueManager::AdvanceStoryArc()
{
    if (StoryArcs.IsValidIndex(CurrentStoryArcIndex))
    {
        StoryArcs[CurrentStoryArcIndex].bArcCompleted = true;
        OnStoryArcCompleted(StoryArcs[CurrentStoryArcIndex].ArcName);
        
        if (StoryArcs.IsValidIndex(CurrentStoryArcIndex + 1))
        {
            CurrentStoryArcIndex++;
        }
    }
}

void ANarr_DialogueManager::SetStoryArc(int32 ArcIndex)
{
    if (StoryArcs.IsValidIndex(ArcIndex))
    {
        CurrentStoryArcIndex = ArcIndex;
    }
}

bool ANarr_DialogueManager::IsStoryArcCompleted(int32 ArcIndex)
{
    if (StoryArcs.IsValidIndex(ArcIndex))
    {
        return StoryArcs[ArcIndex].bArcCompleted;
    }
    return false;
}