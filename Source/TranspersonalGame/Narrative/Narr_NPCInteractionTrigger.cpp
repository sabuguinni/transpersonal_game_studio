#include "Narr_NPCInteractionTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ANarr_NPCInteractionTrigger::ANarr_NPCInteractionTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create interaction trigger
    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
    InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create NPC mesh
    NPCMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NPCMesh"));
    NPCMesh->SetupAttachment(RootComponent);
    NPCMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create voice audio component
    VoiceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudio"));
    VoiceAudio->SetupAttachment(RootComponent);
    VoiceAudio->bAutoActivate = false;

    // Initialize properties
    InteractionRange = 300.0f;
    bShowInteractionPrompt = true;
    bShouldAnimate = true;
    bPlayerInRange = false;
    CurrentPlayer = nullptr;
    AnimationTimer = 0.0f;

    // Initialize NPC profile with default values
    NPCProfile.NPCName = TEXT("Survivor");
    NPCProfile.Personality = ENarr_NPCPersonality::Cautious;
    NPCProfile.Role = ENarr_NPCRole::Hunter;
    NPCProfile.TrustLevel = 0.3f;
    NPCProfile.bIsAvailable = true;
    NPCProfile.BackgroundStory = TEXT("A weathered hunter who has survived many seasons in these dangerous lands.");

    // Initialize interaction data
    InteractionData.InteractionPrompt = FText::FromString(TEXT("Talk to Hunter"));
    InteractionData.InteractionType = ENarr_InteractionType::Talk;
    InteractionData.InteractionDuration = 3.0f;
    InteractionData.bRequiresItem = false;

    // Bind overlap events
    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &ANarr_NPCInteractionTrigger::OnTriggerBeginOverlap);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &ANarr_NPCInteractionTrigger::OnTriggerEndOverlap);
}

void ANarr_NPCInteractionTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateInteractionPrompt();
    
    // Log NPC spawn
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("NPC spawned: %s (%s)"), 
                                           *NPCProfile.NPCName, 
                                           *UEnum::GetValueAsString(NPCProfile.Role));
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, LogMessage);
    }
}

void ANarr_NPCInteractionTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bShouldAnimate)
    {
        UpdateNPCAnimation(DeltaTime);
    }
    
    // Update interaction prompt visibility
    if (bPlayerInRange && bShowInteractionPrompt)
    {
        UpdateInteractionPrompt();
    }
}

void ANarr_NPCInteractionTrigger::TriggerInteraction(AActor* InteractingActor)
{
    if (!CanInteract(InteractingActor))
    {
        return;
    }

    // Log interaction
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("%s interacting with %s"), 
                                           *InteractingActor->GetName(), 
                                           *NPCProfile.NPCName);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, LogMessage);
    }

    // Play personality-based greeting
    FString Greeting = GetPersonalityBasedGreeting();
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, Greeting);
    }

    // Play voice line if available
    if (VoiceLineURLs.Num() > 0)
    {
        PlayVoiceLine(0);
    }

    // Increase trust slightly on interaction
    NPCProfile.TrustLevel = FMath::Clamp(NPCProfile.TrustLevel + 0.1f, 0.0f, 1.0f);

    OnInteractionTriggered(InteractingActor);
}

bool ANarr_NPCInteractionTrigger::CanInteract(AActor* InteractingActor)
{
    if (!InteractingActor || !NPCProfile.bIsAvailable)
    {
        return false;
    }

    // Check distance
    float Distance = FVector::Dist(GetActorLocation(), InteractingActor->GetActorLocation());
    if (Distance > InteractionRange)
    {
        return false;
    }

    // Check if item is required
    if (InteractionData.bRequiresItem && !InteractionData.RequiredItemName.IsEmpty())
    {
        // TODO: Check player inventory for required item
        // For now, assume player has the item
        return true;
    }

    return true;
}

void ANarr_NPCInteractionTrigger::SetNPCProfile(const FNarr_NPCProfile& NewProfile)
{
    NPCProfile = NewProfile;
    UpdateInteractionPrompt();
    
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("NPC profile updated: %s"), *NewProfile.NPCName);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, LogMessage);
    }
}

FNarr_NPCProfile ANarr_NPCInteractionTrigger::GetNPCProfile() const
{
    return NPCProfile;
}

void ANarr_NPCInteractionTrigger::SetInteractionData(const FNarr_InteractionData& NewData)
{
    InteractionData = NewData;
    UpdateInteractionPrompt();
}

void ANarr_NPCInteractionTrigger::PlayVoiceLine(int32 LineIndex)
{
    if (VoiceLineURLs.IsValidIndex(LineIndex) && VoiceAudio)
    {
        // TODO: Load and play audio from URL
        // For now, just log the voice line
        if (GEngine)
        {
            FString LogMessage = FString::Printf(TEXT("Playing voice line %d: %s"), 
                                               LineIndex, 
                                               *VoiceLineURLs[LineIndex]);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta, LogMessage);
        }
        
        VoiceAudio->Play();
    }
}

void ANarr_NPCInteractionTrigger::AddVoiceLine(const FString& VoiceURL)
{
    VoiceLineURLs.Add(VoiceURL);
    
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("Added voice line to %s: %s"), 
                                           *NPCProfile.NPCName, 
                                           *VoiceURL);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, LogMessage);
    }
}

void ANarr_NPCInteractionTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        bPlayerInRange = true;
        CurrentPlayer = OtherActor;
        
        if (GEngine)
        {
            FString LogMessage = FString::Printf(TEXT("Player entered %s's interaction range"), *NPCProfile.NPCName);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, LogMessage);
        }
        
        OnPlayerEnterRange(OtherActor);
    }
}

void ANarr_NPCInteractionTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == CurrentPlayer)
    {
        bPlayerInRange = false;
        
        if (GEngine)
        {
            FString LogMessage = FString::Printf(TEXT("Player left %s's interaction range"), *NPCProfile.NPCName);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, LogMessage);
        }
        
        OnPlayerExitRange(OtherActor);
        CurrentPlayer = nullptr;
    }
}

void ANarr_NPCInteractionTrigger::UpdateNPCAnimation(float DeltaTime)
{
    AnimationTimer += DeltaTime;
    
    if (NPCMesh)
    {
        // Simple idle animation - slight bobbing
        float BobAmount = FMath::Sin(AnimationTimer * 2.0f) * 5.0f;
        FVector CurrentLocation = NPCMesh->GetRelativeLocation();
        NPCMesh->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, BobAmount));
        
        // Slight rotation based on personality
        float RotationSpeed = 0.5f;
        switch (NPCProfile.Personality)
        {
            case ENarr_NPCPersonality::Aggressive:
                RotationSpeed = 1.0f;
                break;
            case ENarr_NPCPersonality::Cautious:
                RotationSpeed = 0.3f;
                break;
            case ENarr_NPCPersonality::Wise:
                RotationSpeed = 0.2f;
                break;
            default:
                RotationSpeed = 0.5f;
                break;
        }
        
        float YawRotation = FMath::Sin(AnimationTimer * RotationSpeed) * 15.0f;
        NPCMesh->SetRelativeRotation(FRotator(0.0f, YawRotation, 0.0f));
    }
}

void ANarr_NPCInteractionTrigger::UpdateInteractionPrompt()
{
    if (bPlayerInRange && CanInteract(CurrentPlayer))
    {
        // Update prompt text based on NPC role and personality
        FString PromptText = FString::Printf(TEXT("Talk to %s (%s)"), 
                                           *NPCProfile.NPCName, 
                                           *UEnum::GetValueAsString(NPCProfile.Role));
        InteractionData.InteractionPrompt = FText::FromString(PromptText);
    }
}

FString ANarr_NPCInteractionTrigger::GetPersonalityBasedGreeting()
{
    FString Greeting;
    
    switch (NPCProfile.Personality)
    {
        case ENarr_NPCPersonality::Aggressive:
            Greeting = FString::Printf(TEXT("%s: What do you want? Make it quick."), *NPCProfile.NPCName);
            break;
        case ENarr_NPCPersonality::Cautious:
            Greeting = FString::Printf(TEXT("%s: Stay alert, stranger. These lands are dangerous."), *NPCProfile.NPCName);
            break;
        case ENarr_NPCPersonality::Wise:
            Greeting = FString::Printf(TEXT("%s: Welcome, young one. I have seen many seasons pass."), *NPCProfile.NPCName);
            break;
        case ENarr_NPCPersonality::Friendly:
            Greeting = FString::Printf(TEXT("%s: Greetings, friend! It's good to see another survivor."), *NPCProfile.NPCName);
            break;
        default:
            Greeting = FString::Printf(TEXT("%s: Hello there."), *NPCProfile.NPCName);
            break;
    }
    
    return Greeting;
}