#include "Quest_InteractionSystem.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

AQuest_InteractionActor::AQuest_InteractionActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create interaction mesh
    InteractionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InteractionMesh"));
    InteractionMesh->SetupAttachment(RootComponent);
    InteractionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create interaction trigger
    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionTrigger->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));

    // Bind overlap events
    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AQuest_InteractionActor::OnTriggerBeginOverlap);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AQuest_InteractionActor::OnTriggerEndOverlap);

    // Initialize defaults
    bIsActive = true;
    CurrentUses = 0;
    bPlayerInRange = false;
    CurrentHoldTime = 0.0f;
    bIsInteracting = false;
    InteractingPlayer = nullptr;
}

void AQuest_InteractionActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Update trigger size based on interaction range
    if (InteractionTrigger)
    {
        float Range = InteractionData.InteractionRange;
        InteractionTrigger->SetBoxExtent(FVector(Range, Range, Range * 0.5f));
    }
}

void AQuest_InteractionActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle hold interaction
    if (bIsInteracting && InteractionData.bRequiresHold)
    {
        CurrentHoldTime += DeltaTime;
        if (CurrentHoldTime >= InteractionData.HoldDuration)
        {
            CompleteInteraction();
        }
    }
}

bool AQuest_InteractionActor::CanInteract() const
{
    if (!bIsActive || !bPlayerInRange)
    {
        return false;
    }

    if (InteractionData.MaxUses > 0 && CurrentUses >= InteractionData.MaxUses)
    {
        return false;
    }

    return true;
}

void AQuest_InteractionActor::StartInteraction()
{
    if (!CanInteract() || bIsInteracting)
    {
        return;
    }

    bIsInteracting = true;
    CurrentHoldTime = 0.0f;

    if (!InteractionData.bRequiresHold)
    {
        CompleteInteraction();
    }

    OnInteractionStarted();
}

void AQuest_InteractionActor::CompleteInteraction()
{
    if (!bIsInteracting)
    {
        return;
    }

    bIsInteracting = false;
    CurrentUses++;

    // Deactivate if max uses reached and not repeatable
    if (!InteractionData.bIsRepeatable && InteractionData.MaxUses > 0 && CurrentUses >= InteractionData.MaxUses)
    {
        SetInteractionActive(false);
    }

    OnInteractionCompleted();
}

void AQuest_InteractionActor::CancelInteraction()
{
    if (!bIsInteracting)
    {
        return;
    }

    bIsInteracting = false;
    CurrentHoldTime = 0.0f;
    OnInteractionCancelled();
}

void AQuest_InteractionActor::SetInteractionActive(bool bActive)
{
    bIsActive = bActive;
    
    if (InteractionMesh)
    {
        InteractionMesh->SetVisibility(bActive);
    }
    
    if (InteractionTrigger)
    {
        InteractionTrigger->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

void AQuest_InteractionActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacter* Character = Cast<ACharacter>(OtherActor))
    {
        bPlayerInRange = true;
        InteractingPlayer = Character;
        OnPlayerEnterRange();
    }
}

void AQuest_InteractionActor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (OtherActor == InteractingPlayer)
    {
        bPlayerInRange = false;
        InteractingPlayer = nullptr;
        
        if (bIsInteracting)
        {
            CancelInteraction();
        }
        
        OnPlayerExitRange();
    }
}

// Gatherable Resource Implementation
AQuest_GatherableResource::AQuest_GatherableResource()
{
    ResourceType = TEXT("Stone");
    ResourceAmount = 3;
    bDestroyOnGather = false;
    RespawnTime = 30.0f;
    OriginalResourceAmount = ResourceAmount;

    // Set interaction type
    InteractionData.InteractionType = EQuest_InteractionType::Gather;
    InteractionData.InteractionPrompt = FText::FromString(TEXT("Press E to gather"));
    InteractionData.bRequiresHold = true;
    InteractionData.HoldDuration = 2.0f;
}

void AQuest_GatherableResource::BeginPlay()
{
    Super::BeginPlay();
    OriginalResourceAmount = ResourceAmount;
}

void AQuest_GatherableResource::GatherResource(int32 Amount)
{
    ResourceAmount = FMath::Max(0, ResourceAmount - Amount);
    
    if (ResourceAmount <= 0)
    {
        if (bDestroyOnGather)
        {
            Destroy();
        }
        else
        {
            SetInteractionActive(false);
            
            if (RespawnTime > 0.0f)
            {
                GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AQuest_GatherableResource::RespawnResource, RespawnTime, false);
            }
        }
    }
}

void AQuest_GatherableResource::RespawnResource()
{
    ResourceAmount = OriginalResourceAmount;
    CurrentUses = 0;
    SetInteractionActive(true);
}

// Crafting Station Implementation
AQuest_CraftingStation::AQuest_CraftingStation()
{
    bRequiresTools = false;
    
    // Set interaction type
    InteractionData.InteractionType = EQuest_InteractionType::Craft;
    InteractionData.InteractionPrompt = FText::FromString(TEXT("Press E to craft"));
    InteractionData.bRequiresHold = false;
    
    // Default recipes
    AvailableRecipes.Add(TEXT("Stone Axe"));
    AvailableRecipes.Add(TEXT("Wooden Spear"));
    AvailableRecipes.Add(TEXT("Fire Starter"));
}

bool AQuest_CraftingStation::CanCraftRecipe(const FString& RecipeName) const
{
    return AvailableRecipes.Contains(RecipeName);
}

void AQuest_CraftingStation::StartCrafting(const FString& RecipeName)
{
    if (!CanCraftRecipe(RecipeName))
    {
        return;
    }

    OnCraftingStarted(RecipeName);
    
    // Simulate crafting time
    FTimerHandle CraftingTimer;
    FTimerDelegate CraftingDelegate;
    CraftingDelegate.BindLambda([this, RecipeName]()
    {
        OnCraftingCompleted(RecipeName);
    });
    
    GetWorldTimerManager().SetTimer(CraftingTimer, CraftingDelegate, 3.0f, false);
}