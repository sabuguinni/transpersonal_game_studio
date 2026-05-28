#include "Quest_DynamicEventSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

// UQuest_DynamicEventComponent Implementation
UQuest_DynamicEventComponent::UQuest_DynamicEventComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    bEventActive = false;
    EventTimeRemaining = 0.0f;
    EventGenerationInterval = 120.0f; // 2 minutes
    EventDetectionRange = 2000.0f; // 20 meters
    bAutoGenerateEvents = true;
}

void UQuest_DynamicEventComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Start automatic event generation if enabled
    if (bAutoGenerateEvents)
    {
        GetWorld()->GetTimerManager().SetTimer(
            EventGenerationTimer,
            this,
            &UQuest_DynamicEventComponent::GenerateRandomEventNearPlayer,
            EventGenerationInterval,
            true,
            FMath::RandRange(30.0f, 60.0f) // Initial delay
        );
    }
}

void UQuest_DynamicEventComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEventActive)
    {
        EventTimeRemaining -= DeltaTime;
        
        if (EventTimeRemaining <= 0.0f)
        {
            OnEventTimeout();
        }
    }
}

void UQuest_DynamicEventComponent::TriggerEvent(const FQuest_DynamicEventData& EventData)
{
    if (bEventActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger new event - another event is already active"));
        return;
    }
    
    CurrentEventData = EventData;
    bEventActive = true;
    EventTimeRemaining = EventData.EventDuration;
    
    // Set timeout timer
    GetWorld()->GetTimerManager().SetTimer(
        EventDurationTimer,
        this,
        &UQuest_DynamicEventComponent::OnEventTimeout,
        EventData.EventDuration,
        false
    );
    
    UE_LOG(LogTemp, Log, TEXT("Dynamic Event Triggered: %s"), *EventData.EventTitle);
}

void UQuest_DynamicEventComponent::CompleteEvent()
{
    if (!bEventActive)
    {
        return;
    }
    
    bEventActive = false;
    EventTimeRemaining = 0.0f;
    
    // Clear timeout timer
    GetWorld()->GetTimerManager().ClearTimer(EventDurationTimer);
    
    UE_LOG(LogTemp, Log, TEXT("Dynamic Event Completed: %s (Reward: %d points)"), 
           *CurrentEventData.EventTitle, CurrentEventData.RewardPoints);
}

FQuest_DynamicEventData UQuest_DynamicEventComponent::GenerateRandomEvent(const FVector& PlayerLocation)
{
    FQuest_DynamicEventData NewEvent;
    
    // Generate random event type and urgency
    NewEvent.EventType = GetRandomEventType();
    NewEvent.Urgency = GetRandomUrgency();
    
    // Set title and description based on type
    NewEvent.EventTitle = GetEventTitleForType(NewEvent.EventType);
    NewEvent.EventDescription = GetEventDescriptionForType(NewEvent.EventType);
    
    // Generate location near player
    FVector RandomOffset = FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        0.0f
    );
    NewEvent.EventLocation = PlayerLocation + RandomOffset;
    
    // Set properties based on urgency
    switch (NewEvent.Urgency)
    {
        case EQuest_EventUrgency::Low:
            NewEvent.EventRadius = 300.0f;
            NewEvent.EventDuration = 600.0f; // 10 minutes
            NewEvent.RewardPoints = 50;
            break;
        case EQuest_EventUrgency::Medium:
            NewEvent.EventRadius = 500.0f;
            NewEvent.EventDuration = 300.0f; // 5 minutes
            NewEvent.RewardPoints = 100;
            break;
        case EQuest_EventUrgency::High:
            NewEvent.EventRadius = 700.0f;
            NewEvent.EventDuration = 180.0f; // 3 minutes
            NewEvent.RewardPoints = 200;
            break;
        case EQuest_EventUrgency::Critical:
            NewEvent.EventRadius = 1000.0f;
            NewEvent.EventDuration = 120.0f; // 2 minutes
            NewEvent.RewardPoints = 300;
            break;
    }
    
    return NewEvent;
}

void UQuest_DynamicEventComponent::GenerateRandomEventNearPlayer()
{
    if (bEventActive)
    {
        return; // Don't generate new event if one is already active
    }
    
    // Find player location
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    FQuest_DynamicEventData NewEvent = GenerateRandomEvent(PlayerLocation);
    
    // Spawn event actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AQuest_DynamicEventActor* EventActor = World->SpawnActor<AQuest_DynamicEventActor>(
        AQuest_DynamicEventActor::StaticClass(),
        NewEvent.EventLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (EventActor)
    {
        EventActor->SetupEvent(NewEvent);
        EventActor->ActivateEvent();
        TriggerEvent(NewEvent);
    }
}

void UQuest_DynamicEventComponent::OnEventTimeout()
{
    UE_LOG(LogTemp, Warning, TEXT("Dynamic Event Timed Out: %s"), *CurrentEventData.EventTitle);
    CompleteEvent();
}

EQuest_DynamicEventType UQuest_DynamicEventComponent::GetRandomEventType() const
{
    int32 RandomValue = FMath::RandRange(0, 5);
    return static_cast<EQuest_DynamicEventType>(RandomValue);
}

EQuest_EventUrgency UQuest_DynamicEventComponent::GetRandomUrgency() const
{
    // Weighted random for urgency (more common events are less urgent)
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    
    if (RandomValue < 0.5f)
        return EQuest_EventUrgency::Low;
    else if (RandomValue < 0.8f)
        return EQuest_EventUrgency::Medium;
    else if (RandomValue < 0.95f)
        return EQuest_EventUrgency::High;
    else
        return EQuest_EventUrgency::Critical;
}

FString UQuest_DynamicEventComponent::GetEventTitleForType(EQuest_DynamicEventType Type) const
{
    switch (Type)
    {
        case EQuest_DynamicEventType::DinosaurAttack:
            return TEXT("Dinosaur Threat");
        case EQuest_DynamicEventType::ResourceDepletion:
            return TEXT("Resource Shortage");
        case EQuest_DynamicEventType::WeatherStorm:
            return TEXT("Incoming Storm");
        case EQuest_DynamicEventType::TerritorialDispute:
            return TEXT("Territory Conflict");
        case EQuest_DynamicEventType::InjuredTribeMember:
            return TEXT("Medical Emergency");
        case EQuest_DynamicEventType::FoodShortage:
            return TEXT("Food Crisis");
        default:
            return TEXT("Unknown Event");
    }
}

FString UQuest_DynamicEventComponent::GetEventDescriptionForType(EQuest_DynamicEventType Type) const
{
    switch (Type)
    {
        case EQuest_DynamicEventType::DinosaurAttack:
            return TEXT("A predator has been spotted in the area. Investigate and neutralize the threat.");
        case EQuest_DynamicEventType::ResourceDepletion:
            return TEXT("Essential resources are running low. Find alternative sources quickly.");
        case EQuest_DynamicEventType::WeatherStorm:
            return TEXT("A dangerous storm approaches. Secure shelter and supplies.");
        case EQuest_DynamicEventType::TerritorialDispute:
            return TEXT("Competing groups threaten your territory. Resolve the conflict.");
        case EQuest_DynamicEventType::InjuredTribeMember:
            return TEXT("A tribe member needs immediate medical attention. Gather healing supplies.");
        case EQuest_DynamicEventType::FoodShortage:
            return TEXT("Food stores are critically low. Hunt or forage for sustenance.");
        default:
            return TEXT("An unknown situation requires your attention.");
    }
}

// AQuest_DynamicEventActor Implementation
AQuest_DynamicEventActor::AQuest_DynamicEventActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    
    // Create trigger sphere
    EventTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("EventTrigger"));
    EventTrigger->SetupAttachment(RootComponent);
    EventTrigger->SetSphereRadius(500.0f);
    EventTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    EventTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    EventTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Create visual mesh
    EventMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EventMesh"));
    EventMesh->SetupAttachment(RootComponent);
    EventMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Create text component
    EventText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("EventText"));
    EventText->SetupAttachment(RootComponent);
    EventText->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    EventText->SetHorizontalAlignment(EHTA_Center);
    EventText->SetVerticalAlignment(EVRTA_TextCenter);
    EventText->SetWorldSize(100.0f);
    
    // Create dynamic event component
    DynamicEventComponent = CreateDefaultSubobject<UQuest_DynamicEventComponent>(TEXT("DynamicEventComponent"));
    
    // Initialize state
    bPlayerInRange = false;
    bEventCompleted = false;
    PulseSpeed = 2.0f;
    PulseIntensity = 0.5f;
    PulseTimer = 0.0f;
    
    // Load default mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded())
    {
        EventMesh->SetStaticMesh(SphereMesh.Object);
    }
}

void AQuest_DynamicEventActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind overlap events
    EventTrigger->OnComponentBeginOverlap.AddDynamic(this, &AQuest_DynamicEventActor::OnPlayerEnterEvent);
    EventTrigger->OnComponentEndOverlap.AddDynamic(this, &AQuest_DynamicEventActor::OnPlayerExitEvent);
}

void AQuest_DynamicEventActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateVisualEffects(DeltaTime);
}

void AQuest_DynamicEventActor::SetupEvent(const FQuest_DynamicEventData& InEventData)
{
    EventData = InEventData;
    
    // Update trigger radius
    EventTrigger->SetSphereRadius(EventData.EventRadius);
    
    // Update text
    UpdateEventText();
    
    // Set mesh color based on urgency
    FLinearColor EventColor = GetColorForUrgency(EventData.Urgency);
    if (UMaterialInterface* Material = EventMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = EventMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), EventColor);
        }
    }
}

void AQuest_DynamicEventActor::ActivateEvent()
{
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    
    UE_LOG(LogTemp, Log, TEXT("Dynamic Event Actor Activated: %s"), *EventData.EventTitle);
}

void AQuest_DynamicEventActor::DeactivateEvent()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    bEventCompleted = true;
    
    // Destroy actor after a delay
    FTimerHandle DestroyTimer;
    GetWorld()->GetTimerManager().SetTimer(DestroyTimer, [this]()
    {
        Destroy();
    }, 2.0f, false);
}

void AQuest_DynamicEventActor::OnPlayerEnterEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bEventCompleted)
    {
        return;
    }
    
    // Check if it's the player
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (OtherActor == PlayerPawn)
    {
        bPlayerInRange = true;
        UE_LOG(LogTemp, Log, TEXT("Player entered event area: %s"), *EventData.EventTitle);
        
        // Show interaction prompt
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("Press E to interact with: %s"), *EventData.EventTitle));
        }
    }
}

void AQuest_DynamicEventActor::OnPlayerExitEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (OtherActor == PlayerPawn)
    {
        bPlayerInRange = false;
        UE_LOG(LogTemp, Log, TEXT("Player left event area: %s"), *EventData.EventTitle);
    }
}

void AQuest_DynamicEventActor::UpdateVisualEffects(float DeltaTime)
{
    PulseTimer += DeltaTime * PulseSpeed;
    
    // Pulse effect
    float PulseValue = (FMath::Sin(PulseTimer) + 1.0f) * 0.5f;
    float ScaleMultiplier = 1.0f + (PulseValue * PulseIntensity);
    
    EventMesh->SetRelativeScale3D(FVector(ScaleMultiplier));
    
    // Urgency-based pulse speed
    switch (EventData.Urgency)
    {
        case EQuest_EventUrgency::Critical:
            PulseSpeed = 4.0f;
            break;
        case EQuest_EventUrgency::High:
            PulseSpeed = 3.0f;
            break;
        case EQuest_EventUrgency::Medium:
            PulseSpeed = 2.0f;
            break;
        case EQuest_EventUrgency::Low:
            PulseSpeed = 1.0f;
            break;
    }
}

FLinearColor AQuest_DynamicEventActor::GetColorForUrgency(EQuest_EventUrgency Urgency) const
{
    switch (Urgency)
    {
        case EQuest_EventUrgency::Critical:
            return FLinearColor::Red;
        case EQuest_EventUrgency::High:
            return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
        case EQuest_EventUrgency::Medium:
            return FLinearColor::Yellow;
        case EQuest_EventUrgency::Low:
            return FLinearColor::Green;
        default:
            return FLinearColor::White;
    }
}

void AQuest_DynamicEventActor::UpdateEventText()
{
    FString DisplayText = FString::Printf(TEXT("%s\n%s"), 
                                         *EventData.EventTitle,
                                         *EventData.EventDescription);
    EventText->SetText(FText::FromString(DisplayText));
    EventText->SetTextRenderColor(GetColorForUrgency(EventData.Urgency).ToFColor(true));
}