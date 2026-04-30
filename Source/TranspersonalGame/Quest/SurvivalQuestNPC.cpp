#include "SurvivalQuestNPC.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

ASurvivalQuestNPC::ASurvivalQuestNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize state
    bPlayerInRange = false;
    bCurrentlyInteracting = false;
    InteractingPlayer = nullptr;
    LastInteractionTime = 0.0f;
    MoodUpdateTimer = 0.0f;

    // Setup collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    // Setup mesh
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

    // Setup movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(DEFAULT_INTERACTION_RANGE);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASurvivalQuestNPC::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ASurvivalQuestNPC::OnInteractionSphereEndOverlap);

    // Create dialogue widget
    DialogueWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DialogueWidget"));
    DialogueWidget->SetupAttachment(RootComponent);
    DialogueWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    DialogueWidget->SetWidgetSpace(EWidgetSpace::Screen);
    DialogueWidget->SetDrawSize(FVector2D(400.0f, 200.0f));
    DialogueWidget->SetVisibility(false);

    // Initialize defaults
    InitializeNPCDefaults();
}

void ASurvivalQuestNPC::BeginPlay()
{
    Super::BeginPlay();

    // Store initial location for patrol behavior
    InitialLocation = GetActorLocation();

    // Setup NPC based on type
    SetupDialogueForNPCType();
    SetupQuestsForNPCType();

    // Update mood based on initial stats
    UpdateMoodBasedOnStats();
}

void ASurvivalQuestNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update mood periodically
    MoodUpdateTimer += DeltaTime;
    if (MoodUpdateTimer >= MOOD_UPDATE_INTERVAL)
    {
        UpdateMoodBasedOnStats();
        MoodUpdateTimer = 0.0f;
    }

    // Handle player proximity behavior
    if (bPlayerInRange && !bCurrentlyInteracting)
    {
        HandlePlayerProximity(true);
    }
}

void ASurvivalQuestNPC::InitializeNPCDefaults()
{
    // Default values
    NPCType = EQuest_NPCType::Survivor;
    CurrentMood = EQuest_NPCMood::Calm;
    NPCName = TEXT("Unnamed Survivor");
    CurrentActiveQuestID = -1;
    bCanGiveQuests = true;
    bCanReceiveItems = true;
    InteractionRange = DEFAULT_INTERACTION_RANGE;
    bIsHostile = false;
    bFleeFromDinosaurs = true;
    FleeDistance = DEFAULT_FLEE_DISTANCE;
    PatrolRadius = DEFAULT_PATROL_RADIUS;

    // Initialize stats
    NPCStats = FQuest_NPCStats();

    // Initialize dialogue
    DialogueLines = FQuest_NPCDialogue();
}

void ASurvivalQuestNPC::SetupDialogueForNPCType()
{
    switch (NPCType)
    {
        case EQuest_NPCType::Elder:
            NPCName = TEXT("Tribal Elder");
            DialogueLines.GreetingText = TEXT("Young one, you carry the scent of distant lands. What brings you to our territory?");
            DialogueLines.QuestOfferText = TEXT("The tribe faces great danger. Will you help us survive these dark times?");
            DialogueLines.QuestCompleteText = TEXT("You have proven yourself worthy. The ancestors smile upon you.");
            DialogueLines.FarewellText = TEXT("May the spirits guide your path, brave traveler.");
            break;

        case EQuest_NPCType::Scout:
            NPCName = TEXT("Tribal Scout");
            DialogueLines.GreetingText = TEXT("Stranger! I've been watching you from the ridge. You move like prey, but fight like a predator.");
            DialogueLines.QuestOfferText = TEXT("I need someone fast and quiet for a dangerous reconnaissance mission.");
            DialogueLines.QuestCompleteText = TEXT("Excellent work! Your stealth skills rival those of our best hunters.");
            DialogueLines.FarewellText = TEXT("Stay low, stay quiet, stay alive.");
            break;

        case EQuest_NPCType::Hunter:
            NPCName = TEXT("Tribal Hunter");
            DialogueLines.GreetingText = TEXT("Another hunter, I can tell by your stance. These lands are rich with prey... and predators.");
            DialogueLines.QuestOfferText = TEXT("The great beasts grow bolder. We need skilled hunters to thin their numbers.");
            DialogueLines.QuestCompleteText = TEXT("A clean kill! You understand the balance between hunter and hunted.");
            DialogueLines.FarewellText = TEXT("Hunt well, and may your spear fly true.");
            break;

        case EQuest_NPCType::Gatherer:
            NPCName = TEXT("Tribal Gatherer");
            DialogueLines.GreetingText = TEXT("Welcome, traveler. I know every plant, every root that can sustain life in this harsh land.");
            DialogueLines.QuestOfferText = TEXT("The healing herbs grow scarce, and winter approaches. Will you help me gather what we need?");
            DialogueLines.QuestCompleteText = TEXT("These will save many lives. You have the eyes of a true gatherer.");
            DialogueLines.FarewellText = TEXT("Remember - the earth provides, but only for those who know how to look.");
            break;

        case EQuest_NPCType::Trader:
            NPCName = TEXT("Wandering Trader");
            DialogueLines.GreetingText = TEXT("Ah, a fellow wanderer! I carry goods from distant tribes. Perhaps we can make a deal?");
            DialogueLines.QuestOfferText = TEXT("I lost my caravan to raptors. Help me recover my goods, and I'll share the profits.");
            DialogueLines.QuestCompleteText = TEXT("Excellent! Your payment, as promised. May our paths cross again.");
            DialogueLines.FarewellText = TEXT("Safe travels, and remember - everything has a price, even survival.");
            break;

        case EQuest_NPCType::Survivor:
        default:
            NPCName = TEXT("Lost Survivor");
            DialogueLines.GreetingText = TEXT("Thank the spirits! Another human! I thought I was the only one left alive.");
            DialogueLines.QuestOfferText = TEXT("My family... they're trapped in the caves. Please, you're my only hope!");
            DialogueLines.QuestCompleteText = TEXT("You saved them! I... I don't know how to repay you.");
            DialogueLines.FarewellText = TEXT("I'll never forget what you've done for us.");
            break;
    }
}

void ASurvivalQuestNPC::SetupQuestsForNPCType()
{
    AvailableQuestIDs.Empty();
    CompletedQuestIDs.Empty();

    switch (NPCType)
    {
        case EQuest_NPCType::Elder:
            AvailableQuestIDs.Add(1001); // Protect the Sacred Grove
            AvailableQuestIDs.Add(1002); // Gather Ancient Wisdom
            break;

        case EQuest_NPCType::Scout:
            AvailableQuestIDs.Add(2001); // Scout the Raptor Territory
            AvailableQuestIDs.Add(2002); // Find Safe Passage
            break;

        case EQuest_NPCType::Hunter:
            AvailableQuestIDs.Add(3001); // Hunt the Alpha Predator
            AvailableQuestIDs.Add(3002); // Thin the Pack
            break;

        case EQuest_NPCType::Gatherer:
            AvailableQuestIDs.Add(4001); // Collect Healing Herbs
            AvailableQuestIDs.Add(4002); // Gather Winter Supplies
            break;

        case EQuest_NPCType::Trader:
            AvailableQuestIDs.Add(5001); // Recover Lost Caravan
            AvailableQuestIDs.Add(5002); // Establish Trade Route
            break;

        case EQuest_NPCType::Survivor:
        default:
            AvailableQuestIDs.Add(6001); // Rescue the Family
            AvailableQuestIDs.Add(6002); // Find Safe Shelter
            break;
    }
}

void ASurvivalQuestNPC::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
        return;

    APlayerController* PlayerController = Cast<APlayerController>(OtherActor->GetInstigatorController());
    if (!PlayerController)
        return;

    bPlayerInRange = true;
    
    // Show dialogue widget
    if (DialogueWidget)
    {
        DialogueWidget->SetVisibility(true);
    }

    // Broadcast interaction event
    OnPlayerInteract.Broadcast(PlayerController);
}

void ASurvivalQuestNPC::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor)
        return;

    APlayerController* PlayerController = Cast<APlayerController>(OtherActor->GetInstigatorController());
    if (!PlayerController)
        return;

    bPlayerInRange = false;

    // Hide dialogue widget
    if (DialogueWidget)
    {
        DialogueWidget->SetVisibility(false);
    }

    // End interaction if currently interacting
    if (bCurrentlyInteracting && InteractingPlayer == PlayerController)
    {
        EndInteraction();
    }
}

void ASurvivalQuestNPC::StartInteraction(APlayerController* PlayerController)
{
    if (!PlayerController || !CanInteractWithPlayer(PlayerController))
        return;

    bCurrentlyInteracting = true;
    InteractingPlayer = PlayerController;
    LastInteractionTime = GetWorld()->GetTimeSeconds();

    // Face the player
    if (APawn* PlayerPawn = PlayerController->GetPawn())
    {
        FVector DirectionToPlayer = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = DirectionToPlayer.Rotation();
        NewRotation.Pitch = 0.0f;
        NewRotation.Roll = 0.0f;
        SetActorRotation(NewRotation);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC %s started interaction with player"), *NPCName);
}

void ASurvivalQuestNPC::EndInteraction()
{
    bCurrentlyInteracting = false;
    InteractingPlayer = nullptr;

    UE_LOG(LogTemp, Log, TEXT("NPC %s ended interaction"), *NPCName);
}

bool ASurvivalQuestNPC::CanInteractWithPlayer(APlayerController* PlayerController) const
{
    if (!PlayerController || !bCanGiveQuests)
        return false;

    // Check interaction cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastInteractionTime < INTERACTION_COOLDOWN)
        return false;

    // Check if player is in range
    return IsPlayerInRange(PlayerController);
}

bool ASurvivalQuestNPC::HasAvailableQuests() const
{
    return AvailableQuestIDs.Num() > 0;
}

TArray<int32> ASurvivalQuestNPC::GetAvailableQuests() const
{
    return AvailableQuestIDs;
}

bool ASurvivalQuestNPC::GiveQuestToPlayer(int32 QuestID, APlayerController* PlayerController)
{
    if (!PlayerController || !AvailableQuestIDs.Contains(QuestID))
        return false;

    // Remove from available quests
    AvailableQuestIDs.Remove(QuestID);
    CurrentActiveQuestID = QuestID;

    UE_LOG(LogTemp, Log, TEXT("NPC %s gave quest %d to player"), *NPCName, QuestID);
    return true;
}

bool ASurvivalQuestNPC::CompleteQuest(int32 QuestID, APlayerController* PlayerController)
{
    if (!PlayerController || CurrentActiveQuestID != QuestID)
        return false;

    // Move to completed quests
    CompletedQuestIDs.AddUnique(QuestID);
    CurrentActiveQuestID = -1;

    // Increase trust and reduce desperation
    ModifyTrust(10.0f);
    ModifyDesperation(-15.0f);

    // Broadcast quest completion
    OnQuestStatusChanged.Broadcast(QuestID, true);

    UE_LOG(LogTemp, Log, TEXT("NPC %s completed quest %d with player"), *NPCName, QuestID);
    return true;
}

void ASurvivalQuestNPC::UpdateQuestProgress(int32 QuestID, int32 ObjectiveIndex, bool bCompleted)
{
    if (CurrentActiveQuestID == QuestID)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC %s updated quest %d objective %d: %s"), 
            *NPCName, QuestID, ObjectiveIndex, bCompleted ? TEXT("Completed") : TEXT("Failed"));
    }
}

FString ASurvivalQuestNPC::GetGreetingText() const
{
    return DialogueLines.GreetingText;
}

FString ASurvivalQuestNPC::GetQuestOfferText() const
{
    return DialogueLines.QuestOfferText;
}

FString ASurvivalQuestNPC::GetContextualDialogue() const
{
    return GenerateContextualDialogue();
}

void ASurvivalQuestNPC::ModifyTrust(float Amount)
{
    NPCStats.Trust = FMath::Clamp(NPCStats.Trust + Amount, 0.0f, 100.0f);
}

void ASurvivalQuestNPC::ModifyFear(float Amount)
{
    NPCStats.Fear = FMath::Clamp(NPCStats.Fear + Amount, 0.0f, 100.0f);
}

void ASurvivalQuestNPC::ModifyDesperation(float Amount)
{
    NPCStats.Desperation = FMath::Clamp(NPCStats.Desperation + Amount, 0.0f, 100.0f);
}

void ASurvivalQuestNPC::UpdateMoodBasedOnStats()
{
    if (NPCStats.Fear > 70.0f)
    {
        CurrentMood = EQuest_NPCMood::Desperate;
    }
    else if (NPCStats.Desperation > 60.0f)
    {
        CurrentMood = EQuest_NPCMood::Worried;
    }
    else if (NPCStats.Trust < 30.0f)
    {
        CurrentMood = EQuest_NPCMood::Suspicious;
    }
    else if (NPCStats.Trust > 70.0f && NPCStats.Fear < 30.0f)
    {
        CurrentMood = EQuest_NPCMood::Grateful;
    }
    else
    {
        CurrentMood = EQuest_NPCMood::Calm;
    }
}

void ASurvivalQuestNPC::FleeFromThreat(AActor* ThreatActor)
{
    if (!ThreatActor || !bFleeFromDinosaurs)
        return;

    // Calculate flee direction
    FVector FleeDirection = (GetActorLocation() - ThreatActor->GetActorLocation()).GetSafeNormal();
    FVector FleeTarget = GetActorLocation() + (FleeDirection * FleeDistance);

    // Increase fear
    ModifyFear(20.0f);

    UE_LOG(LogTemp, Log, TEXT("NPC %s fleeing from threat"), *NPCName);
}

void ASurvivalQuestNPC::ReturnToPatrol()
{
    // Return to initial location for patrol behavior
    FVector DirectionToHome = (InitialLocation - GetActorLocation()).GetSafeNormal();
    
    UE_LOG(LogTemp, Log, TEXT("NPC %s returning to patrol"), *NPCName);
}

bool ASurvivalQuestNPC::IsPlayerInRange(APlayerController* PlayerController) const
{
    if (!PlayerController)
        return false;

    APawn* PlayerPawn = PlayerController->GetPawn();
    if (!PlayerPawn)
        return false;

    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= InteractionRange;
}

FString ASurvivalQuestNPC::GenerateContextualDialogue() const
{
    FString ContextDialogue;

    switch (CurrentMood)
    {
        case EQuest_NPCMood::Desperate:
            ContextDialogue = TEXT("Please, you must help us! Time is running out!");
            break;
        case EQuest_NPCMood::Worried:
            ContextDialogue = TEXT("I fear for our safety. The beasts grow bolder each day.");
            break;
        case EQuest_NPCMood::Suspicious:
            ContextDialogue = TEXT("I don't know if I can trust you yet, stranger.");
            break;
        case EQuest_NPCMood::Grateful:
            ContextDialogue = TEXT("Your help has meant everything to us. Thank you.");
            break;
        case EQuest_NPCMood::Hostile:
            ContextDialogue = TEXT("Stay back! I won't let you harm my people!");
            break;
        case EQuest_NPCMood::Calm:
        default:
            ContextDialogue = DialogueLines.GreetingText;
            break;
    }

    return ContextDialogue;
}

void ASurvivalQuestNPC::HandlePlayerProximity(bool bPlayerNearby)
{
    if (bPlayerNearby && !bCurrentlyInteracting)
    {
        // Look at player occasionally
        if (InteractingPlayer)
        {
            if (APawn* PlayerPawn = InteractingPlayer->GetPawn())
            {
                FVector DirectionToPlayer = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                FRotator TargetRotation = DirectionToPlayer.Rotation();
                TargetRotation.Pitch = 0.0f;
                TargetRotation.Roll = 0.0f;
                
                FRotator CurrentRotation = GetActorRotation();
                FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
                SetActorRotation(NewRotation);
            }
        }
    }
}