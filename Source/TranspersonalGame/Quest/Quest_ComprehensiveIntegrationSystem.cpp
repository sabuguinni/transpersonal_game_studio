#include "Quest_ComprehensiveIntegrationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

AQuest_ComprehensiveIntegrationSystem::AQuest_ComprehensiveIntegrationSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default settings
    bEnableAdvancedIntegration = true;
    SystemUpdateInterval = 5.0f;
    MaxSimultaneousQuests = 10;
    bSystemInitialized = false;

    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AQuest_ComprehensiveIntegrationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the integration system
    InitializeIntegrationSystem();
    
    // Start system update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            SystemUpdateTimer,
            this,
            &AQuest_ComprehensiveIntegrationSystem::UpdateIntegrationSystem,
            SystemUpdateInterval,
            true
        );
    }
}

void AQuest_ComprehensiveIntegrationSystem::InitializeIntegrationSystem()
{
    if (bSystemInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Initializing Comprehensive Quest Integration System"));

    // Set up default integration zones
    SetupDefaultIntegrationZones();
    
    // Set up default environmental stories
    SetupDefaultEnvironmentalStories();
    
    // Initialize complexity layers
    ComplexityLayers.Empty();
    ComplexityLayers.Add(EQuestDifficulty::Easy, 1);
    ComplexityLayers.Add(EQuestDifficulty::Medium, 3);
    ComplexityLayers.Add(EQuestDifficulty::Hard, 5);
    ComplexityLayers.Add(EQuestDifficulty::Expert, 8);

    // Validate system integrity
    ValidateSystemIntegrity();
    
    bSystemInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Integration System initialized with %d zones and %d story elements"), 
           IntegrationZones.Num(), EnvironmentalStoryElements.Num());
}

void AQuest_ComprehensiveIntegrationSystem::SetupDefaultIntegrationZones()
{
    IntegrationZones.Empty();

    // Hunting grounds integration zone
    FQuest_IntegrationZoneData HuntingZone;
    HuntingZone.ZoneName = TEXT("HuntingGrounds");
    HuntingZone.ZoneLocation = FVector(4000, 3000, 200);
    HuntingZone.ZoneRadius = 1500.0f;
    HuntingZone.AssociatedQuestType = EQuestType::Combat;
    HuntingZone.MaxCrowdSize = 25;
    HuntingZone.bIsActive = true;
    IntegrationZones.Add(HuntingZone);

    // Gathering area integration zone
    FQuest_IntegrationZoneData GatheringZone;
    GatheringZone.ZoneName = TEXT("GatheringArea");
    GatheringZone.ZoneLocation = FVector(4000, -3000, 200);
    GatheringZone.ZoneRadius = 1200.0f;
    GatheringZone.AssociatedQuestType = EQuestType::Gathering;
    GatheringZone.MaxCrowdSize = 40;
    GatheringZone.bIsActive = true;
    IntegrationZones.Add(GatheringZone);

    // Social hub integration zone
    FQuest_IntegrationZoneData SocialZone;
    SocialZone.ZoneName = TEXT("SocialHub");
    SocialZone.ZoneLocation = FVector(6000, 0, 200);
    SocialZone.ZoneRadius = 800.0f;
    SocialZone.AssociatedQuestType = EQuestType::Social;
    SocialZone.MaxCrowdSize = 60;
    SocialZone.bIsActive = true;
    IntegrationZones.Add(SocialZone);

    // Observation post integration zone
    FQuest_IntegrationZoneData ObservationZone;
    ObservationZone.ZoneName = TEXT("ObservationPost");
    ObservationZone.ZoneLocation = FVector(2000, 0, 500);
    ObservationZone.ZoneRadius = 600.0f;
    ObservationZone.AssociatedQuestType = EQuestType::Exploration;
    ObservationZone.MaxCrowdSize = 15;
    ObservationZone.bIsActive = true;
    IntegrationZones.Add(ObservationZone);
}

void AQuest_ComprehensiveIntegrationSystem::SetupDefaultEnvironmentalStories()
{
    EnvironmentalStoryElements.Empty();

    // Abandoned camp story
    FQuest_EnvironmentalStoryData AbandonedCamp;
    AbandonedCamp.StoryElementName = TEXT("AbandonedCamp");
    AbandonedCamp.StoryDescription = TEXT("The remains of a hastily abandoned campsite. Scattered tools and cold ashes suggest a sudden departure. What drove them away?");
    AbandonedCamp.ElementLocation = FVector(5000, 2000, 150);
    AbandonedCamp.InteractionRadius = 400.0f;
    AbandonedCamp.bRequiresSpecificQuest = false;
    EnvironmentalStoryElements.Add(AbandonedCamp);

    // Dinosaur bones story
    FQuest_EnvironmentalStoryData DinosaurBones;
    DinosaurBones.StoryElementName = TEXT("DinosaurBones");
    DinosaurBones.StoryDescription = TEXT("Massive fossilized bones of an ancient predator. Deep gouges in the bones tell of epic battles fought long ago.");
    DinosaurBones.ElementLocation = FVector(5000, -2000, 150);
    DinosaurBones.InteractionRadius = 500.0f;
    DinosaurBones.bRequiresSpecificQuest = true;
    DinosaurBones.RequiredQuestID = TEXT("PaleontologyBasics");
    EnvironmentalStoryElements.Add(DinosaurBones);

    // Ancient tool story
    FQuest_EnvironmentalStoryData AncientTool;
    AncientTool.StoryElementName = TEXT("AncientTool");
    AncientTool.StoryDescription = TEXT("A primitive but expertly crafted stone tool. Its design shows remarkable ingenuity for such an ancient artifact.");
    AncientTool.ElementLocation = FVector(7000, 1000, 150);
    AncientTool.InteractionRadius = 300.0f;
    AncientTool.bRequiresSpecificQuest = false;
    EnvironmentalStoryElements.Add(AncientTool);

    // Weather shelter story
    FQuest_EnvironmentalStoryData WeatherShelter;
    WeatherShelter.StoryElementName = TEXT("WeatherShelter");
    WeatherShelter.StoryDescription = TEXT("A natural rock formation modified for shelter. Scratch marks on the walls suggest it was used by both humans and animals.");
    WeatherShelter.ElementLocation = FVector(7000, -1000, 150);
    WeatherShelter.InteractionRadius = 600.0f;
    WeatherShelter.bRequiresSpecificQuest = true;
    WeatherShelter.RequiredQuestID = TEXT("ShelterConstruction");
    EnvironmentalStoryElements.Add(WeatherShelter);
}

void AQuest_ComprehensiveIntegrationSystem::RegisterIntegrationZone(const FQuest_IntegrationZoneData& ZoneData)
{
    // Check if zone already exists
    for (int32 i = 0; i < IntegrationZones.Num(); i++)
    {
        if (IntegrationZones[i].ZoneName == ZoneData.ZoneName)
        {
            IntegrationZones[i] = ZoneData;
            UE_LOG(LogTemp, Warning, TEXT("Updated existing integration zone: %s"), *ZoneData.ZoneName);
            return;
        }
    }
    
    // Add new zone
    IntegrationZones.Add(ZoneData);
    UE_LOG(LogTemp, Warning, TEXT("Registered new integration zone: %s"), *ZoneData.ZoneName);
}

void AQuest_ComprehensiveIntegrationSystem::RegisterEnvironmentalStory(const FQuest_EnvironmentalStoryData& StoryData)
{
    // Check if story element already exists
    for (int32 i = 0; i < EnvironmentalStoryElements.Num(); i++)
    {
        if (EnvironmentalStoryElements[i].StoryElementName == StoryData.StoryElementName)
        {
            EnvironmentalStoryElements[i] = StoryData;
            UE_LOG(LogTemp, Warning, TEXT("Updated existing story element: %s"), *StoryData.StoryElementName);
            return;
        }
    }
    
    // Add new story element
    EnvironmentalStoryElements.Add(StoryData);
    UE_LOG(LogTemp, Warning, TEXT("Registered new environmental story: %s"), *StoryData.StoryElementName);
}

bool AQuest_ComprehensiveIntegrationSystem::ValidateQuestIntegration(const FString& QuestID)
{
    if (QuestID.IsEmpty())
    {
        return false;
    }

    // Check if quest integrates with any zones
    for (const FQuest_IntegrationZoneData& Zone : IntegrationZones)
    {
        if (Zone.bIsActive)
        {
            UE_LOG(LogTemp, Log, TEXT("Quest %s validated against zone %s"), *QuestID, *Zone.ZoneName);
            return true;
        }
    }

    return false;
}

TArray<FQuest_IntegrationZoneData> AQuest_ComprehensiveIntegrationSystem::GetActiveIntegrationZones()
{
    TArray<FQuest_IntegrationZoneData> ActiveZones;
    
    for (const FQuest_IntegrationZoneData& Zone : IntegrationZones)
    {
        if (Zone.bIsActive)
        {
            ActiveZones.Add(Zone);
        }
    }
    
    return ActiveZones;
}

FQuest_EnvironmentalStoryData AQuest_ComprehensiveIntegrationSystem::GetNearestStoryElement(const FVector& PlayerLocation)
{
    FQuest_EnvironmentalStoryData NearestElement;
    float NearestDistance = FLT_MAX;
    
    for (const FQuest_EnvironmentalStoryData& Element : EnvironmentalStoryElements)
    {
        float Distance = FVector::Dist(PlayerLocation, Element.ElementLocation);
        if (Distance < NearestDistance && Distance <= Element.InteractionRadius)
        {
            NearestDistance = Distance;
            NearestElement = Element;
        }
    }
    
    return NearestElement;
}

void AQuest_ComprehensiveIntegrationSystem::UpdateIntegrationSystem()
{
    if (!bSystemInitialized)
    {
        return;
    }

    // Update integration zones
    int32 ActiveZoneCount = 0;
    for (const FQuest_IntegrationZoneData& Zone : IntegrationZones)
    {
        if (Zone.bIsActive)
        {
            ActiveZoneCount++;
        }
    }

    // Log system status
    UE_LOG(LogTemp, Log, TEXT("Quest Integration System Update: %d active zones, %d story elements"), 
           ActiveZoneCount, EnvironmentalStoryElements.Num());
}

void AQuest_ComprehensiveIntegrationSystem::ActivateIntegrationZone(const FString& ZoneName)
{
    for (FQuest_IntegrationZoneData& Zone : IntegrationZones)
    {
        if (Zone.ZoneName == ZoneName)
        {
            Zone.bIsActive = true;
            UE_LOG(LogTemp, Warning, TEXT("Activated integration zone: %s"), *ZoneName);
            return;
        }
    }
}

void AQuest_ComprehensiveIntegrationSystem::DeactivateIntegrationZone(const FString& ZoneName)
{
    for (FQuest_IntegrationZoneData& Zone : IntegrationZones)
    {
        if (Zone.ZoneName == ZoneName)
        {
            Zone.bIsActive = false;
            UE_LOG(LogTemp, Warning, TEXT("Deactivated integration zone: %s"), *ZoneName);
            return;
        }
    }
}

bool AQuest_ComprehensiveIntegrationSystem::IsPlayerInIntegrationZone(const FVector& PlayerLocation, FString& OutZoneName)
{
    for (const FQuest_IntegrationZoneData& Zone : IntegrationZones)
    {
        if (Zone.bIsActive)
        {
            float Distance = FVector::Dist(PlayerLocation, Zone.ZoneLocation);
            if (Distance <= Zone.ZoneRadius)
            {
                OutZoneName = Zone.ZoneName;
                return true;
            }
        }
    }
    
    OutZoneName = TEXT("");
    return false;
}

void AQuest_ComprehensiveIntegrationSystem::ValidateSystemIntegrity()
{
    // Validate integration zones
    for (const FQuest_IntegrationZoneData& Zone : IntegrationZones)
    {
        if (Zone.ZoneName.IsEmpty() || Zone.ZoneRadius <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid integration zone detected"));
        }
    }

    // Validate environmental stories
    for (const FQuest_EnvironmentalStoryData& Story : EnvironmentalStoryElements)
    {
        if (Story.StoryElementName.IsEmpty() || Story.InteractionRadius <= 0.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid environmental story detected"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Quest Integration System validation complete"));
}