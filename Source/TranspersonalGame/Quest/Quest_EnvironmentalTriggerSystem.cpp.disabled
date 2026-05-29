#include "Quest_EnvironmentalTriggerSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UQuest_EnvironmentalTriggerComponent::UQuest_EnvironmentalTriggerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    ActivationCooldown = 5.0f;
    bRequiresSpecificItem = false;
    RequiredItemName = TEXT("");
    LastActivationTime = 0.0f;
}

void UQuest_EnvironmentalTriggerComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize trigger data with default values if not set
    if (TriggerData.TriggerMessage.IsEmpty())
    {
        switch (TriggerData.TriggerType)
        {
            case EQuest_EnvironmentalTriggerType::WaterSource:
                TriggerData.TriggerMessage = TEXT("Fresh water source detected. Approach to gather water.");
                break;
            case EQuest_EnvironmentalTriggerType::ForagingArea:
                TriggerData.TriggerMessage = TEXT("Rich foraging area. Search for edible plants and berries.");
                break;
            case EQuest_EnvironmentalTriggerType::CraftingSpot:
                TriggerData.TriggerMessage = TEXT("Suitable crafting location. Gather materials to create tools.");
                break;
            case EQuest_EnvironmentalTriggerType::ShelterZone:
                TriggerData.TriggerMessage = TEXT("Protected area ideal for shelter construction.");
                break;
            case EQuest_EnvironmentalTriggerType::DangerZone:
                TriggerData.TriggerMessage = TEXT("Dangerous territory. Proceed with extreme caution.");
                break;
            case EQuest_EnvironmentalTriggerType::RestArea:
                TriggerData.TriggerMessage = TEXT("Safe resting spot. Recover stamina and health here.");
                break;
        }
    }
}

void UQuest_EnvironmentalTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (TriggerData.bIsActive)
    {
        CheckForPlayersInRange();
    }
}

void UQuest_EnvironmentalTriggerComponent::ActivateTrigger(AActor* TriggeringActor)
{
    if (!TriggerData.bIsActive)
        return;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastActivationTime < ActivationCooldown)
        return;
        
    if (!ValidatePlayerRequirements(TriggeringActor))
        return;
        
    LastActivationTime = CurrentTime;
    
    // Add player to trigger list if not already present
    if (!PlayersInTrigger.Contains(TriggeringActor))
    {
        PlayersInTrigger.Add(TriggeringActor);
    }
    
    // Log trigger activation
    UE_LOG(LogTemp, Warning, TEXT("Environmental trigger activated: %s by %s"), 
           *TriggerData.TriggerMessage, *TriggeringActor->GetName());
    
    // Call Blueprint event
    OnTriggerActivated(TriggeringActor);
    
    // Display message to player
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TriggerData.TriggerMessage);
    }
}

void UQuest_EnvironmentalTriggerComponent::DeactivateTrigger()
{
    TriggerData.bIsActive = false;
    PlayersInTrigger.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Environmental trigger deactivated"));
    OnTriggerDeactivated();
}

bool UQuest_EnvironmentalTriggerComponent::IsPlayerInRange(AActor* Player) const
{
    if (!Player || !TriggerData.bIsActive)
        return false;
        
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= TriggerData.Radius;
}

void UQuest_EnvironmentalTriggerComponent::SetTriggerData(const FQuest_EnvironmentalTriggerData& NewTriggerData)
{
    TriggerData = NewTriggerData;
}

void UQuest_EnvironmentalTriggerComponent::CheckForPlayersInRange()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (IsPlayerInRange(Actor) && !PlayersInTrigger.Contains(Actor))
        {
            ActivateTrigger(Actor);
        }
        else if (!IsPlayerInRange(Actor) && PlayersInTrigger.Contains(Actor))
        {
            PlayersInTrigger.Remove(Actor);
        }
    }
}

bool UQuest_EnvironmentalTriggerComponent::ValidatePlayerRequirements(AActor* Player) const
{
    if (!bRequiresSpecificItem || RequiredItemName.IsEmpty())
        return true;
        
    // TODO: Implement inventory check when inventory system is available
    // For now, always return true
    return true;
}

// Environmental Trigger Actor Implementation

AQuest_EnvironmentalTriggerActor::AQuest_EnvironmentalTriggerActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create trigger collision
    TriggerCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerCollision"));
    TriggerCollision->SetupAttachment(RootComponent);
    TriggerCollision->SetSphereRadius(200.0f);
    TriggerCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Create visual mesh
    TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
    TriggerMesh->SetupAttachment(RootComponent);
    TriggerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Create environmental trigger component
    EnvironmentalTrigger = CreateDefaultSubobject<UQuest_EnvironmentalTriggerComponent>(TEXT("EnvironmentalTrigger"));
    
    // Bind overlap events
    TriggerCollision->OnComponentBeginOverlap.AddDynamic(this, &AQuest_EnvironmentalTriggerActor::OnTriggerBeginOverlap);
    TriggerCollision->OnComponentEndOverlap.AddDynamic(this, &AQuest_EnvironmentalTriggerActor::OnTriggerEndOverlap);
}

void AQuest_EnvironmentalTriggerActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial visual state
    UpdateVisualState();
    
    // Sync collision radius with trigger data
    if (EnvironmentalTrigger)
    {
        FQuest_EnvironmentalTriggerData TriggerData = EnvironmentalTrigger->GetTriggerData();
        TriggerCollision->SetSphereRadius(TriggerData.Radius);
    }
}

void AQuest_EnvironmentalTriggerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update visual effects based on trigger state
    UpdateVisualState();
}

void AQuest_EnvironmentalTriggerActor::SetTriggerActive(bool bActive)
{
    if (EnvironmentalTrigger)
    {
        FQuest_EnvironmentalTriggerData TriggerData = EnvironmentalTrigger->GetTriggerData();
        TriggerData.bIsActive = bActive;
        EnvironmentalTrigger->SetTriggerData(TriggerData);
        
        UpdateVisualState();
    }
}

void AQuest_EnvironmentalTriggerActor::UpdateVisualState()
{
    if (!TriggerMesh || !EnvironmentalTrigger)
        return;
        
    FQuest_EnvironmentalTriggerData TriggerData = EnvironmentalTrigger->GetTriggerData();
    
    if (TriggerData.bIsActive && ActiveMaterial)
    {
        TriggerMesh->SetMaterial(0, ActiveMaterial);
        TriggerMesh->SetVisibility(true);
    }
    else if (!TriggerData.bIsActive && InactiveMaterial)
    {
        TriggerMesh->SetMaterial(0, InactiveMaterial);
        TriggerMesh->SetVisibility(false);
    }
}

void AQuest_EnvironmentalTriggerActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                                           bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>() && EnvironmentalTrigger)
    {
        EnvironmentalTrigger->ActivateTrigger(OtherActor);
    }
}

void AQuest_EnvironmentalTriggerActor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Handle player leaving trigger area
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        UE_LOG(LogTemp, Log, TEXT("Player left environmental trigger area: %s"), *GetName());
    }
}