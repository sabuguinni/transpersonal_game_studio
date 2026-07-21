#include "Narr_TribalNPC.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"

ANarr_TribalNPC::ANarr_TribalNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mesh component
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    NPCMesh->SetupAttachment(RootComponent);
    
    // Use basic cube mesh for now
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        NPCMesh->SetStaticMesh(CubeMeshAsset.Object);
        NPCMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 2.0f)); // Make it taller like a person
    }

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(300.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create interaction manager
    InteractionManager = CreateDefaultSubobject<UNarr_NPCInteractionManager>(TEXT("InteractionManager"));

    // Initialize default values
    TribalRole = ENarr_TribalRole::Elder;
    NPCName = TEXT("Tribal Member");
    bIsAvailable = true;
    TrustLevel = 0.5f;
    TimesInteracted = 0;

    // Set default personality
    Personality.Friendliness = 0.7f;
    Personality.Caution = 0.6f;
    Personality.Knowledge = 0.5f;
    Personality.Aggression = 0.2f;
}

void ANarr_TribalNPC::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_TribalNPC::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_TribalNPC::OnInteractionSphereEndOverlap);

    // Setup role-based interactions
    SetupInteractions();
}

void ANarr_TribalNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update trust level over time
    UpdateTrustLevel(DeltaTime);
}

void ANarr_TribalNPC::InitializeAsRole(ENarr_TribalRole Role)
{
    TribalRole = Role;

    switch (Role)
    {
        case ENarr_TribalRole::Elder:
            NPCName = TEXT("Tribal Elder");
            Personality.Knowledge = 0.9f;
            Personality.Friendliness = 0.8f;
            Personality.Caution = 0.7f;
            Personality.Aggression = 0.1f;
            break;

        case ENarr_TribalRole::Hunter:
            NPCName = TEXT("Tribal Hunter");
            Personality.Knowledge = 0.7f;
            Personality.Friendliness = 0.6f;
            Personality.Caution = 0.8f;
            Personality.Aggression = 0.5f;
            break;

        case ENarr_TribalRole::Scout:
            NPCName = TEXT("Tribal Scout");
            Personality.Knowledge = 0.8f;
            Personality.Friendliness = 0.5f;
            Personality.Caution = 0.9f;
            Personality.Aggression = 0.3f;
            break;

        case ENarr_TribalRole::Crafter:
            NPCName = TEXT("Tribal Crafter");
            Personality.Knowledge = 0.6f;
            Personality.Friendliness = 0.7f;
            Personality.Caution = 0.4f;
            Personality.Aggression = 0.2f;
            break;

        case ENarr_TribalRole::Gatherer:
            NPCName = TEXT("Tribal Gatherer");
            Personality.Knowledge = 0.5f;
            Personality.Friendliness = 0.8f;
            Personality.Caution = 0.6f;
            Personality.Aggression = 0.1f;
            break;
    }

    SetupInteractions();
}

void ANarr_TribalNPC::SetupInteractions()
{
    if (!InteractionManager)
    {
        return;
    }

    // Clear existing interactions
    InteractionManager->AvailableInteractions.Empty();

    // Add role-based interactions
    FNarr_NPCInteraction Greeting;
    Greeting.InteractionText = GetRoleBasedGreeting();
    Greeting.InteractionRange = 250.0f;
    Greeting.bRequiresItem = false;
    InteractionManager->AddInteraction(Greeting);

    // Add additional role-specific interactions
    switch (TribalRole)
    {
        case ENarr_TribalRole::Elder:
        {
            FNarr_NPCInteraction Wisdom;
            Wisdom.InteractionText = TEXT("The old ways teach us patience. The beasts respect those who understand their nature.");
            Wisdom.InteractionRange = 250.0f;
            Wisdom.bRequiresItem = false;
            InteractionManager->AddInteraction(Wisdom);
            break;
        }

        case ENarr_TribalRole::Hunter:
        {
            FNarr_NPCInteraction HuntingTip;
            HuntingTip.InteractionText = TEXT("The thunder lizard's weak spot is behind the neck. But approach from downwind - their hearing is sharp.");
            HuntingTip.InteractionRange = 250.0f;
            HuntingTip.bRequiresItem = false;
            InteractionManager->AddInteraction(HuntingTip);
            break;
        }

        case ENarr_TribalRole::Scout:
        {
            FNarr_NPCInteraction Warning;
            Warning.InteractionText = TEXT("I've seen fresh tracks by the river. Large predator, moving north. Avoid that area after sunset.");
            Warning.InteractionRange = 250.0f;
            Warning.bRequiresItem = false;
            InteractionManager->AddInteraction(Warning);
            break;
        }
    }
}

FString ANarr_TribalNPC::GetRoleBasedGreeting()
{
    switch (TribalRole)
    {
        case ENarr_TribalRole::Elder:
            return TEXT("Welcome, young hunter. The valley holds many secrets for those wise enough to listen.");

        case ENarr_TribalRole::Hunter:
            return TEXT("Greetings, fellow hunter. The beasts grow restless - we must be vigilant.");

        case ENarr_TribalRole::Scout:
            return TEXT("Traveler, heed my words. Danger lurks in the shadows beyond our camp.");

        case ENarr_TribalRole::Crafter:
            return TEXT("Ah, a visitor! Perhaps you need tools for the hunt? I craft the finest spears in the valley.");

        case ENarr_TribalRole::Gatherer:
            return TEXT("Hello there! I know where the sweetest fruits grow, away from the hunting grounds.");

        default:
            return TEXT("Greetings, stranger. What brings you to our territory?");
    }
}

void ANarr_TribalNPC::OnPlayerInteract(AActor* PlayerActor)
{
    if (!bIsAvailable || !InteractionManager)
    {
        return;
    }

    // Trigger interaction through manager
    InteractionManager->TriggerInteraction(PlayerActor);

    // Update interaction count and trust
    TimesInteracted++;
    TrustLevel = FMath::Clamp(TrustLevel + 0.1f, 0.0f, 1.0f);

    UE_LOG(LogTemp, Warning, TEXT("%s interacted with player. Trust level: %f"), *NPCName, TrustLevel);
}

void ANarr_TribalNPC::UpdateTrustLevel(float DeltaTime)
{
    // Slowly decrease trust over time if not interacting
    if (TimesInteracted > 0)
    {
        TrustLevel = FMath::Clamp(TrustLevel - (DeltaTime * 0.01f), 0.0f, 1.0f);
    }
}

bool ANarr_TribalNPC::CanProvideInformation()
{
    return bIsAvailable && TrustLevel > 0.3f;
}

void ANarr_TribalNPC::SetAvailability(bool bAvailable)
{
    bIsAvailable = bAvailable;
    
    if (InteractionManager)
    {
        InteractionManager->SetInteractionEnabled(bAvailable);
    }
}

void ANarr_TribalNPC::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Player entered interaction range
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        UE_LOG(LogTemp, Log, TEXT("Player entered interaction range of %s"), *NPCName);
    }
}

void ANarr_TribalNPC::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Player left interaction range
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        UE_LOG(LogTemp, Log, TEXT("Player left interaction range of %s"), *NPCName);
    }
}