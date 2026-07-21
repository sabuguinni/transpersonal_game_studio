#include "Quest_VisualMarkerSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"

UQuest_VisualMarkerSystem* UQuest_VisualMarkerSystem::Instance = nullptr;

AQuest_VisualMarker::AQuest_VisualMarker()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create marker light component
    MarkerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MarkerLight"));
    MarkerLight->SetupAttachment(RootComponent);
    MarkerLight->SetIntensity(2000.0f);
    MarkerLight->SetAttenuationRadius(5000.0f);
    MarkerLight->SetCastShadows(false);
    MarkerLight->SetLightColor(FLinearColor::White);

    // Create marker mesh component
    MarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
    MarkerMesh->SetupAttachment(RootComponent);
    MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Load default sphere mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        MarkerMesh->SetStaticMesh(SphereMeshAsset.Object);
        MarkerMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Initialize pulse parameters
    PulseSpeed = 2.0f;
    PulseIntensityMin = 1000.0f;
    PulseIntensityMax = 3000.0f;
    CurrentPulseTime = 0.0f;

    // Initialize marker data
    MarkerData.MarkerType = EQuest_MarkerType::SideQuest;
    MarkerData.MarkerState = EQuest_MarkerState::Available;
    MarkerData.MarkerColor = FLinearColor::White;
    MarkerData.InteractionRadius = 500.0f;
    MarkerData.bIsVisible = true;
}

void AQuest_VisualMarker::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyStateVisuals();
    SetMarkerColor(MarkerData.MarkerColor);
    SetMarkerVisibility(MarkerData.bIsVisible);
}

void AQuest_VisualMarker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (MarkerData.bIsVisible && MarkerData.MarkerState == EQuest_MarkerState::Available)
    {
        UpdatePulseEffect(DeltaTime);
    }
}

void AQuest_VisualMarker::UpdatePulseEffect(float DeltaTime)
{
    CurrentPulseTime += DeltaTime * PulseSpeed;
    
    float PulseValue = FMath::Sin(CurrentPulseTime) * 0.5f + 0.5f;
    float CurrentIntensity = FMath::Lerp(PulseIntensityMin, PulseIntensityMax, PulseValue);
    
    if (MarkerLight)
    {
        MarkerLight->SetIntensity(CurrentIntensity);
    }
}

void AQuest_VisualMarker::UpdateMarkerState(EQuest_MarkerState NewState)
{
    MarkerData.MarkerState = NewState;
    ApplyStateVisuals();
}

void AQuest_VisualMarker::ApplyStateVisuals()
{
    if (!MarkerLight) return;

    switch (MarkerData.MarkerState)
    {
        case EQuest_MarkerState::Available:
            MarkerLight->SetLightColor(FLinearColor::White);
            PulseSpeed = 2.0f;
            break;
            
        case EQuest_MarkerState::Active:
            MarkerLight->SetLightColor(FLinearColor::Yellow);
            PulseSpeed = 4.0f;
            break;
            
        case EQuest_MarkerState::Completed:
            MarkerLight->SetLightColor(FLinearColor::Green);
            MarkerLight->SetIntensity(PulseIntensityMin);
            break;
            
        case EQuest_MarkerState::Failed:
            MarkerLight->SetLightColor(FLinearColor::Red);
            MarkerLight->SetIntensity(PulseIntensityMin);
            break;
            
        case EQuest_MarkerState::Locked:
            MarkerLight->SetLightColor(FLinearColor::Gray);
            MarkerLight->SetIntensity(PulseIntensityMin * 0.5f);
            break;
    }
}

void AQuest_VisualMarker::SetMarkerVisibility(bool bVisible)
{
    MarkerData.bIsVisible = bVisible;
    
    if (MarkerLight)
    {
        MarkerLight->SetVisibility(bVisible);
    }
    
    if (MarkerMesh)
    {
        MarkerMesh->SetVisibility(bVisible);
    }
}

void AQuest_VisualMarker::SetMarkerColor(FLinearColor NewColor)
{
    MarkerData.MarkerColor = NewColor;
    
    if (MarkerLight)
    {
        MarkerLight->SetLightColor(NewColor);
    }
}

bool AQuest_VisualMarker::IsPlayerInRange(AActor* Player)
{
    if (!Player) return false;
    
    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= MarkerData.InteractionRadius;
}

void AQuest_VisualMarker::SetMarkerData(const FQuest_MarkerData& NewData)
{
    MarkerData = NewData;
    SetActorLocation(MarkerData.WorldLocation);
    ApplyStateVisuals();
    SetMarkerColor(MarkerData.MarkerColor);
    SetMarkerVisibility(MarkerData.bIsVisible);
}

// UQuest_VisualMarkerSystem Implementation

UQuest_VisualMarkerSystem::UQuest_VisualMarkerSystem()
{
    ActiveMarkers.Empty();
    MarkerRegistry.Empty();
}

UQuest_VisualMarkerSystem* UQuest_VisualMarkerSystem::GetInstance(UWorld* World)
{
    if (!Instance)
    {
        Instance = NewObject<UQuest_VisualMarkerSystem>();
        Instance->AddToRoot();
    }
    return Instance;
}

AQuest_VisualMarker* UQuest_VisualMarkerSystem::CreateQuestMarker(const FQuest_MarkerData& MarkerData, UWorld* World)
{
    if (!World) return nullptr;

    // Check if marker already exists
    if (MarkerRegistry.Contains(MarkerData.QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest marker with ID %s already exists"), *MarkerData.QuestID);
        return MarkerRegistry[MarkerData.QuestID];
    }

    // Spawn new marker
    AQuest_VisualMarker* NewMarker = World->SpawnActor<AQuest_VisualMarker>(
        AQuest_VisualMarker::StaticClass(),
        MarkerData.WorldLocation,
        FRotator::ZeroRotator
    );

    if (NewMarker)
    {
        NewMarker->SetMarkerData(MarkerData);
        ActiveMarkers.Add(NewMarker);
        MarkerRegistry.Add(MarkerData.QuestID, NewMarker);
        
        UE_LOG(LogTemp, Log, TEXT("Created quest marker: %s at location %s"), 
               *MarkerData.QuestID, *MarkerData.WorldLocation.ToString());
    }

    return NewMarker;
}

void UQuest_VisualMarkerSystem::RemoveQuestMarker(const FString& QuestID)
{
    if (AQuest_VisualMarker** FoundMarker = MarkerRegistry.Find(QuestID))
    {
        if (*FoundMarker)
        {
            ActiveMarkers.Remove(*FoundMarker);
            (*FoundMarker)->Destroy();
        }
        MarkerRegistry.Remove(QuestID);
    }
}

AQuest_VisualMarker* UQuest_VisualMarkerSystem::FindQuestMarker(const FString& QuestID)
{
    if (AQuest_VisualMarker** FoundMarker = MarkerRegistry.Find(QuestID))
    {
        return *FoundMarker;
    }
    return nullptr;
}

TArray<AQuest_VisualMarker*> UQuest_VisualMarkerSystem::GetAllQuestMarkers()
{
    return ActiveMarkers;
}

TArray<AQuest_VisualMarker*> UQuest_VisualMarkerSystem::GetQuestMarkersByType(EQuest_MarkerType MarkerType)
{
    TArray<AQuest_VisualMarker*> FilteredMarkers;
    
    for (AQuest_VisualMarker* Marker : ActiveMarkers)
    {
        if (Marker && Marker->GetMarkerData().MarkerType == MarkerType)
        {
            FilteredMarkers.Add(Marker);
        }
    }
    
    return FilteredMarkers;
}

TArray<AQuest_VisualMarker*> UQuest_VisualMarkerSystem::GetQuestMarkersInRange(FVector Location, float Range)
{
    TArray<AQuest_VisualMarker*> MarkersInRange;
    
    for (AQuest_VisualMarker* Marker : ActiveMarkers)
    {
        if (Marker)
        {
            float Distance = FVector::Dist(Location, Marker->GetActorLocation());
            if (Distance <= Range)
            {
                MarkersInRange.Add(Marker);
            }
        }
    }
    
    return MarkersInRange;
}

void UQuest_VisualMarkerSystem::UpdateAllMarkersVisibility(bool bVisible)
{
    for (AQuest_VisualMarker* Marker : ActiveMarkers)
    {
        if (Marker)
        {
            Marker->SetMarkerVisibility(bVisible);
        }
    }
}

void UQuest_VisualMarkerSystem::RefreshMarkerStates()
{
    // Remove destroyed markers
    ActiveMarkers.RemoveAll([](AQuest_VisualMarker* Marker) {
        return !IsValid(Marker);
    });
    
    // Clean up registry
    TArray<FString> KeysToRemove;
    for (auto& Pair : MarkerRegistry)
    {
        if (!IsValid(Pair.Value))
        {
            KeysToRemove.Add(Pair.Key);
        }
    }
    
    for (const FString& Key : KeysToRemove)
    {
        MarkerRegistry.Remove(Key);
    }
}

void UQuest_VisualMarkerSystem::CreateBiomeQuestMarkers(UWorld* World)
{
    if (!World) return;

    // Savana - Main Hub Quest
    FQuest_MarkerData SavanaQuest;
    SavanaQuest.QuestID = TEXT("SAVANA_MAIN_HUB");
    SavanaQuest.QuestTitle = TEXT("Survival Central");
    SavanaQuest.QuestDescription = TEXT("Learn basic survival skills in the central savana");
    SavanaQuest.MarkerType = EQuest_MarkerType::MainQuest;
    SavanaQuest.WorldLocation = FVector(0, 0, 300);
    SavanaQuest.MarkerColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);
    CreateQuestMarker(SavanaQuest, World);

    // Floresta - Exploration Quest
    FQuest_MarkerData FlorestaQuest;
    FlorestaQuest.QuestID = TEXT("FLORESTA_EXPLORATION");
    FlorestaQuest.QuestTitle = TEXT("Forest Explorer");
    FlorestaQuest.QuestDescription = TEXT("Explore the dense forest and discover its secrets");
    FlorestaQuest.MarkerType = EQuest_MarkerType::ExplorationQuest;
    FlorestaQuest.WorldLocation = FVector(-45000, 40000, 300);
    FlorestaQuest.MarkerColor = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f);
    CreateQuestMarker(FlorestaQuest, World);

    // Pantano - Survival Quest
    FQuest_MarkerData PantanoQuest;
    PantanoQuest.QuestID = TEXT("PANTANO_SURVIVAL");
    PantanoQuest.QuestTitle = TEXT("Swamp Survivor");
    PantanoQuest.QuestDescription = TEXT("Survive the dangerous swamplands");
    PantanoQuest.MarkerType = EQuest_MarkerType::SurvivalQuest;
    PantanoQuest.WorldLocation = FVector(-50000, -45000, 300);
    PantanoQuest.MarkerColor = FLinearColor(0.5f, 0.3f, 0.8f, 1.0f);
    CreateQuestMarker(PantanoQuest, World);

    // Deserto - Crafting Quest
    FQuest_MarkerData DesertoQuest;
    DesertoQuest.QuestID = TEXT("DESERTO_CRAFTING");
    DesertoQuest.QuestTitle = TEXT("Desert Craftsman");
    DesertoQuest.QuestDescription = TEXT("Master advanced crafting in harsh desert conditions");
    DesertoQuest.MarkerType = EQuest_MarkerType::CraftingQuest;
    DesertoQuest.WorldLocation = FVector(55000, 0, 300);
    DesertoQuest.MarkerColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
    CreateQuestMarker(DesertoQuest, World);

    // Montanha - Hunting Quest
    FQuest_MarkerData MontanhaQuest;
    MontanhaQuest.QuestID = TEXT("MONTANHA_HUNTING");
    MontanhaQuest.QuestTitle = TEXT("Mountain Hunter");
    MontanhaQuest.QuestDescription = TEXT("Hunt dangerous predators in the snowy mountains");
    MontanhaQuest.MarkerType = EQuest_MarkerType::HuntingQuest;
    MontanhaQuest.WorldLocation = FVector(40000, 50000, 800);
    MontanhaQuest.MarkerColor = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f);
    CreateQuestMarker(MontanhaQuest, World);
}

void UQuest_VisualMarkerSystem::CreateTutorialMarkers(UWorld* World)
{
    if (!World) return;

    // Tutorial markers near spawn point
    FQuest_MarkerData TutorialMovement;
    TutorialMovement.QuestID = TEXT("TUTORIAL_MOVEMENT");
    TutorialMovement.QuestTitle = TEXT("First Steps");
    TutorialMovement.QuestDescription = TEXT("Learn to move and navigate the world");
    TutorialMovement.MarkerType = EQuest_MarkerType::MainQuest;
    TutorialMovement.WorldLocation = FVector(1000, 1000, 200);
    TutorialMovement.MarkerColor = FLinearColor::White;
    CreateQuestMarker(TutorialMovement, World);

    FQuest_MarkerData TutorialCrafting;
    TutorialCrafting.QuestID = TEXT("TUTORIAL_CRAFTING");
    TutorialCrafting.QuestTitle = TEXT("Basic Crafting");
    TutorialCrafting.QuestDescription = TEXT("Craft your first tools");
    TutorialCrafting.MarkerType = EQuest_MarkerType::CraftingQuest;
    TutorialCrafting.WorldLocation = FVector(-1000, 1000, 200);
    TutorialCrafting.MarkerColor = FLinearColor::Yellow;
    CreateQuestMarker(TutorialCrafting, World);
}

void UQuest_VisualMarkerSystem::CreateSurvivalQuestMarkers(UWorld* World)
{
    if (!World) return;

    // Emergency survival markers
    FQuest_MarkerData WaterSource;
    WaterSource.QuestID = TEXT("FIND_WATER");
    WaterSource.QuestTitle = TEXT("Find Water");
    WaterSource.QuestDescription = TEXT("Locate a clean water source");
    WaterSource.MarkerType = EQuest_MarkerType::SurvivalQuest;
    WaterSource.WorldLocation = FVector(2000, -2000, 150);
    WaterSource.MarkerColor = FLinearColor::Blue;
    CreateQuestMarker(WaterSource, World);

    FQuest_MarkerData Shelter;
    Shelter.QuestID = TEXT("BUILD_SHELTER");
    Shelter.QuestTitle = TEXT("Build Shelter");
    Shelter.QuestDescription = TEXT("Construct a basic shelter for protection");
    Shelter.MarkerType = EQuest_MarkerType::CraftingQuest;
    Shelter.WorldLocation = FVector(-2000, -2000, 150);
    Shelter.MarkerColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    CreateQuestMarker(Shelter, World);
}