#include "Quest_InteractiveQuestMarker.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"

AQuest_InteractiveQuestMarker::AQuest_InteractiveQuestMarker()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create marker mesh
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    MarkerMesh->SetupAttachment(RootComponent);
    MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MarkerMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));

    // Create quest text
    QuestText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("QuestText"));
    QuestText->SetupAttachment(RootComponent);
    QuestText->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    QuestText->SetHorizontalAlignment(EHTA_Center);
    QuestText->SetVerticalAlignment(EVRTA_TextCenter);
    QuestText->SetWorldSize(100.0f);

    // Initialize default values
    MissionType = EQuest_MissionType::WaterGathering;
    QuestTitle = TEXT("Survival Mission");
    QuestDescription = TEXT("Complete this survival objective");
    QuestPriority = 1;
    InteractionRadius = 200.0f;
    bIsActive = true;
    bIsCompleted = false;

    // Initialize colors
    ActiveColor = FLinearColor::Yellow;
    CompletedColor = FLinearColor::Green;
    InactiveColor = FLinearColor::Gray;

    // Internal state
    bPlayerInRange = false;
    CurrentPlayer = nullptr;

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_InteractiveQuestMarker::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_InteractiveQuestMarker::OnInteractionSphereEndOverlap);
}

void AQuest_InteractiveQuestMarker::BeginPlay()
{
    Super::BeginPlay();
    
    // Update initial visual state
    UpdateVisualState();
    UpdateQuestText();
}

void AQuest_InteractiveQuestMarker::ActivateQuestMarker()
{
    bIsActive = true;
    bIsCompleted = false;
    UpdateVisualState();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Quest Marker Activated: %s"), *QuestTitle));
    }
}

void AQuest_InteractiveQuestMarker::DeactivateQuestMarker()
{
    bIsActive = false;
    UpdateVisualState();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Gray, 
            FString::Printf(TEXT("Quest Marker Deactivated: %s"), *QuestTitle));
    }
}

void AQuest_InteractiveQuestMarker::CompleteQuestMarker()
{
    bIsCompleted = true;
    bIsActive = false;
    UpdateVisualState();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Quest Completed: %s"), *QuestTitle));
    }
    
    // Broadcast completion event
    OnQuestMarkerActivated.Broadcast(this);
}

void AQuest_InteractiveQuestMarker::SetQuestInfo(EQuest_MissionType InMissionType, const FString& InTitle, const FString& InDescription, int32 InPriority)
{
    MissionType = InMissionType;
    QuestTitle = InTitle;
    QuestDescription = InDescription;
    QuestPriority = InPriority;
    
    UpdateQuestText();
}

bool AQuest_InteractiveQuestMarker::IsPlayerInRange(APawn* Player) const
{
    if (!Player || !bIsActive)
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRadius;
}

FString AQuest_InteractiveQuestMarker::GetQuestTypeString() const
{
    switch (MissionType)
    {
        case EQuest_MissionType::WaterGathering:
            return TEXT("Water Gathering");
        case EQuest_MissionType::FoodForaging:
            return TEXT("Food Foraging");
        case EQuest_MissionType::ShelterBuilding:
            return TEXT("Shelter Building");
        case EQuest_MissionType::ToolCrafting:
            return TEXT("Tool Crafting");
        case EQuest_MissionType::TerritoryMapping:
            return TEXT("Territory Mapping");
        case EQuest_MissionType::PredatorAvoidance:
            return TEXT("Predator Avoidance");
        default:
            return TEXT("Unknown Mission");
    }
}

void AQuest_InteractiveQuestMarker::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* Player = Cast<APawn>(OtherActor))
    {
        bPlayerInRange = true;
        CurrentPlayer = Player;
        
        if (bIsActive && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                FString::Printf(TEXT("Entered quest area: %s - %s"), *QuestTitle, *QuestDescription));
        }
    }
}

void AQuest_InteractiveQuestMarker::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (APawn* Player = Cast<APawn>(OtherActor))
    {
        if (CurrentPlayer == Player)
        {
            bPlayerInRange = false;
            CurrentPlayer = nullptr;
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
                    FString::Printf(TEXT("Left quest area: %s"), *QuestTitle));
            }
        }
    }
}

void AQuest_InteractiveQuestMarker::UpdateVisualState()
{
    if (!MarkerMesh)
    {
        return;
    }
    
    // Determine current color based on state
    FLinearColor CurrentColor;
    if (bIsCompleted)
    {
        CurrentColor = CompletedColor;
    }
    else if (bIsActive)
    {
        CurrentColor = ActiveColor;
    }
    else
    {
        CurrentColor = InactiveColor;
    }
    
    // Create dynamic material instance if needed
    if (UMaterialInterface* Material = MarkerMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("Color"), CurrentColor);
            MarkerMesh->SetMaterial(0, DynamicMaterial);
        }
    }
    
    // Update visibility
    MarkerMesh->SetVisibility(bIsActive || bIsCompleted);
    QuestText->SetVisibility(bIsActive || bIsCompleted);
}

void AQuest_InteractiveQuestMarker::UpdateQuestText()
{
    if (!QuestText)
    {
        return;
    }
    
    FString DisplayText = FString::Printf(TEXT("%s\n%s\nPriority: %d"), 
        *GetQuestTypeString(), *QuestTitle, QuestPriority);
    
    QuestText->SetText(FText::FromString(DisplayText));
    
    // Set text color based on state
    if (bIsCompleted)
    {
        QuestText->SetTextRenderColor(FColor::Green);
    }
    else if (bIsActive)
    {
        QuestText->SetTextRenderColor(FColor::Yellow);
    }
    else
    {
        QuestText->SetTextRenderColor(FColor::Gray);
    }
}