#include "Quest_QuestGiver.h"
#include "Quest_QuestManager.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AQuest_QuestGiver::AQuest_QuestGiver()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize data
    GiverData = FQuest_QuestGiverData();
    bPlayerInRange = false;
    CurrentPlayer = nullptr;
    QuestManager = nullptr;

    // Set default quests
    GiverData.AvailableQuests.Add(TEXT("tutorial_survival"));
    GiverData.AvailableQuests.Add(TEXT("first_hunt"));
}

void AQuest_QuestGiver::BeginPlay()
{
    Super::BeginPlay();

    InitializeQuestGiver();

    // Bind overlap events
    if (InteractionSphere)
    {
        InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_QuestGiver::OnInteractionSphereBeginOverlap);
        InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_QuestGiver::OnInteractionSphereEndOverlap);
    }

    // Get quest manager
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        QuestManager = GameInstance->GetSubsystem<UQuest_QuestManager>();
    }

    UpdateVisualState();
}

void AQuest_QuestGiver::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update visual state based on available quests
    if (bPlayerInRange && CurrentPlayer)
    {
        UpdateVisualState();
    }
}

void AQuest_QuestGiver::InteractWithPlayer(AActor* Player)
{
    if (!Player || !GiverData.bIsActive)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Quest Giver %s interacting with player"), *GiverData.GiverName);

    // Show greeting dialogue
    if (GEngine)
    {
        FString DialogueText = FString::Printf(TEXT("%s: %s"), 
            *GiverData.GiverName, *GiverData.GreetingDialogue);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DialogueText);
    }

    // Check for available quests
    TArray<FString> AvailableQuests = GetAvailableQuests();
    if (AvailableQuests.Num() > 0)
    {
        // Give first available quest
        GiveQuestToPlayer(AvailableQuests[0], Player);
    }
}

bool AQuest_QuestGiver::CanGiveQuest(const FString& QuestID) const
{
    if (!GiverData.bIsActive)
    {
        return false;
    }

    return GiverData.AvailableQuests.Contains(QuestID);
}

void AQuest_QuestGiver::GiveQuestToPlayer(const FString& QuestID, AActor* Player)
{
    if (!CanGiveQuest(QuestID) || !QuestManager)
    {
        return;
    }

    // Start the quest through quest manager
    bool bQuestStarted = QuestManager->StartQuest(QuestID);
    
    if (bQuestStarted)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest Giver %s gave quest %s to player"), 
            *GiverData.GiverName, *QuestID);

        // Remove quest from available list (one-time quests)
        GiverData.AvailableQuests.Remove(QuestID);

        // Show quest start message
        if (GEngine)
        {
            FString QuestMessage = FString::Printf(TEXT("Quest Started: %s"), *QuestID);
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, QuestMessage);
        }
    }
}

TArray<FString> AQuest_QuestGiver::GetAvailableQuests() const
{
    if (!GiverData.bIsActive)
    {
        return TArray<FString>();
    }

    return GiverData.AvailableQuests;
}

FString AQuest_QuestGiver::GetGreetingDialogue() const
{
    return GiverData.GreetingDialogue;
}

void AQuest_QuestGiver::SetActive(bool bNewActive)
{
    GiverData.bIsActive = bNewActive;
    UpdateVisualState();
}

void AQuest_QuestGiver::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        bPlayerInRange = true;
        CurrentPlayer = OtherActor;

        UE_LOG(LogTemp, Log, TEXT("Player entered interaction range of Quest Giver %s"), 
            *GiverData.GiverName);

        // Show interaction prompt
        if (GEngine && GiverData.bIsActive)
        {
            FString InteractionText = FString::Printf(TEXT("Press E to talk to %s"), 
                *GiverData.GiverName);
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::White, InteractionText);
        }
    }
}

void AQuest_QuestGiver::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == CurrentPlayer)
    {
        bPlayerInRange = false;
        CurrentPlayer = nullptr;

        UE_LOG(LogTemp, Log, TEXT("Player left interaction range of Quest Giver %s"), 
            *GiverData.GiverName);
    }
}

void AQuest_QuestGiver::InitializeQuestGiver()
{
    // Set interaction sphere radius
    if (InteractionSphere)
    {
        InteractionSphere->SetSphereRadius(GiverData.InteractionRadius);
    }

    // Set actor label for identification
    SetActorLabel(GiverData.GiverName);

    UE_LOG(LogTemp, Log, TEXT("Quest Giver %s initialized with %d available quests"), 
        *GiverData.GiverName, GiverData.AvailableQuests.Num());
}

void AQuest_QuestGiver::UpdateVisualState()
{
    if (!MeshComponent)
    {
        return;
    }

    // Change material/color based on quest availability
    if (GiverData.bIsActive && GiverData.AvailableQuests.Num() > 0)
    {
        // Has quests - make visible/highlighted
        MeshComponent->SetVisibility(true);
    }
    else
    {
        // No quests or inactive - dim or hide
        MeshComponent->SetVisibility(GiverData.bIsActive);
    }
}