#include "Narr_EnvironmentalStorytelling.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UNarr_EnvironmentalStorytelling::UNarr_EnvironmentalStorytelling()
{
    PrimaryComponentTick.bCanEverTick = true;
    DiscoveryCheckInterval = 2.0f;
    PlayerDetectionRadius = 500.0f;
    LastDiscoveryCheck = 0.0f;
}

void UNarr_EnvironmentalStorytelling::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultStories();
    InitializeDefaultSites();
    
    UE_LOG(LogTemp, Log, TEXT("Environmental Storytelling Component initialized with %d stories and %d sites"), 
           EnvironmentalStories.Num(), AncientSites.Num());
}

void UNarr_EnvironmentalStorytelling::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastDiscoveryCheck += DeltaTime;
    if (LastDiscoveryCheck >= DiscoveryCheckInterval)
    {
        CheckForNearbyStories();
        LastDiscoveryCheck = 0.0f;
    }
}

void UNarr_EnvironmentalStorytelling::CheckForNearbyStories()
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Check environmental stories
    for (FNarr_EnvironmentalStory& Story : EnvironmentalStories)
    {
        if (!Story.bIsDiscovered)
        {
            float Distance = FVector::Dist(PlayerLocation, Story.TriggerLocation);
            if (Distance <= Story.TriggerRadius)
            {
                DiscoverStory(Story.StoryID);
            }
        }
    }

    // Check ancient sites
    for (const FNarr_AncientSite& Site : AncientSites)
    {
        float Distance = FVector::Dist(PlayerLocation, Site.SiteLocation);
        if (Distance <= Site.ExplorationRadius)
        {
            ExploreAncientSite(Site.SiteID);
        }
    }
}

void UNarr_EnvironmentalStorytelling::DiscoverStory(const FString& StoryID)
{
    if (DiscoveredStoryIDs.Contains(StoryID))
    {
        return;
    }

    for (FNarr_EnvironmentalStory& Story : EnvironmentalStories)
    {
        if (Story.StoryID == StoryID)
        {
            Story.bIsDiscovered = true;
            DiscoveredStoryIDs.Add(StoryID);
            
            UE_LOG(LogTemp, Warning, TEXT("Environmental Story Discovered: %s - %s"), 
                   *Story.Title, *Story.Description);
            
            // Trigger UI notification or audio cue here
            break;
        }
    }
}

void UNarr_EnvironmentalStorytelling::ExploreAncientSite(const FString& SiteID)
{
    for (const FNarr_AncientSite& Site : AncientSites)
    {
        if (Site.SiteID == SiteID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Exploring Ancient Site: %s"), *Site.SiteName);
            
            // Display cave paintings
            for (const FString& Painting : Site.CavePaintings)
            {
                UE_LOG(LogTemp, Log, TEXT("Cave Painting: %s"), *Painting);
            }
            
            // Display bone remains
            for (const FString& Bone : Site.BoneRemains)
            {
                UE_LOG(LogTemp, Log, TEXT("Bone Remains: %s"), *Bone);
            }
            
            // Display tool fragments
            for (const FString& Tool : Site.ToolFragments)
            {
                UE_LOG(LogTemp, Log, TEXT("Tool Fragment: %s"), *Tool);
            }
            
            break;
        }
    }
}

TArray<FString> UNarr_EnvironmentalStorytelling::GetDiscoveredStories() const
{
    return DiscoveredStoryIDs;
}

FNarr_EnvironmentalStory UNarr_EnvironmentalStorytelling::GetStoryByID(const FString& StoryID) const
{
    for (const FNarr_EnvironmentalStory& Story : EnvironmentalStories)
    {
        if (Story.StoryID == StoryID)
        {
            return Story;
        }
    }
    return FNarr_EnvironmentalStory();
}

void UNarr_EnvironmentalStorytelling::RegisterCavePainting(const FString& SiteID, const FString& PaintingDescription)
{
    for (FNarr_AncientSite& Site : AncientSites)
    {
        if (Site.SiteID == SiteID)
        {
            Site.CavePaintings.Add(PaintingDescription);
            UE_LOG(LogTemp, Log, TEXT("Cave painting registered at %s: %s"), *Site.SiteName, *PaintingDescription);
            break;
        }
    }
}

TArray<FString> UNarr_EnvironmentalStorytelling::GetCavePaintingsAtSite(const FString& SiteID) const
{
    for (const FNarr_AncientSite& Site : AncientSites)
    {
        if (Site.SiteID == SiteID)
        {
            return Site.CavePaintings;
        }
    }
    return TArray<FString>();
}

void UNarr_EnvironmentalStorytelling::AnalyzeBoneRemains(const FString& SiteID, const FString& BoneDescription)
{
    for (FNarr_AncientSite& Site : AncientSites)
    {
        if (Site.SiteID == SiteID)
        {
            Site.BoneRemains.Add(BoneDescription);
            UE_LOG(LogTemp, Log, TEXT("Bone remains analyzed at %s: %s"), *Site.SiteName, *BoneDescription);
            break;
        }
    }
}

TArray<FString> UNarr_EnvironmentalStorytelling::GetBoneRemainsAtSite(const FString& SiteID) const
{
    for (const FNarr_AncientSite& Site : AncientSites)
    {
        if (Site.SiteID == SiteID)
        {
            return Site.BoneRemains;
        }
    }
    return TArray<FString>();
}

void UNarr_EnvironmentalStorytelling::DiscoverToolFragment(const FString& SiteID, const FString& ToolDescription)
{
    for (FNarr_AncientSite& Site : AncientSites)
    {
        if (Site.SiteID == SiteID)
        {
            Site.ToolFragments.Add(ToolDescription);
            UE_LOG(LogTemp, Log, TEXT("Tool fragment discovered at %s: %s"), *Site.SiteName, *ToolDescription);
            break;
        }
    }
}

TArray<FString> UNarr_EnvironmentalStorytelling::GetToolFragmentsAtSite(const FString& SiteID) const
{
    for (const FNarr_AncientSite& Site : AncientSites)
    {
        if (Site.SiteID == SiteID)
        {
            return Site.ToolFragments;
        }
    }
    return TArray<FString>();
}

void UNarr_EnvironmentalStorytelling::InitializeDefaultStories()
{
    // Abandoned Campsite Story
    FNarr_EnvironmentalStory CampsiteStory;
    CampsiteStory.StoryID = TEXT("STORY_ABANDONED_CAMP");
    CampsiteStory.Title = TEXT("The Last Campfire");
    CampsiteStory.Description = TEXT("Cold ashes and scattered bones tell of a hunting party that never returned home. The arrangement of stones suggests they were tracking something massive.");
    CampsiteStory.VisualClues.Add(TEXT("Charred wood arranged in a circle"));
    CampsiteStory.VisualClues.Add(TEXT("Broken spear tips embedded in nearby trees"));
    CampsiteStory.VisualClues.Add(TEXT("Large claw marks on surrounding rocks"));
    CampsiteStory.TriggerLocation = FVector(5000.0f, 3000.0f, 100.0f);
    CampsiteStory.TriggerRadius = 800.0f;
    EnvironmentalStories.Add(CampsiteStory);

    // Dinosaur Graveyard Story
    FNarr_EnvironmentalStory GraveyardStory;
    GraveyardStory.StoryID = TEXT("STORY_BONE_VALLEY");
    GraveyardStory.Title = TEXT("Valley of Giants");
    GraveyardStory.Description = TEXT("Massive skeletons lie scattered across this valley. The bones are too large and numerous for natural death - something catastrophic happened here.");
    GraveyardStory.VisualClues.Add(TEXT("Enormous ribcages jutting from the earth"));
    GraveyardStory.VisualClues.Add(TEXT("Skulls the size of boulders"));
    GraveyardStory.VisualClues.Add(TEXT("Volcanic ash layers between bone deposits"));
    GraveyardStory.TriggerLocation = FVector(-2000.0f, 8000.0f, 50.0f);
    GraveyardStory.TriggerRadius = 1200.0f;
    EnvironmentalStories.Add(GraveyardStory);

    // Ancient Hunting Ground Story
    FNarr_EnvironmentalStory HuntingStory;
    HuntingStory.StoryID = TEXT("STORY_HUNTING_GROUND");
    HuntingStory.Title = TEXT("The Great Ambush");
    HuntingStory.Description = TEXT("Stone tools and dinosaur bones create a timeline of an epic hunt. Our ancestors were clever - they used the terrain to their advantage.");
    HuntingStory.VisualClues.Add(TEXT("Carefully placed stone barriers"));
    HuntingStory.VisualClues.Add(TEXT("Triceratops skull with spear damage"));
    HuntingStory.VisualClues.Add(TEXT("Tool-making debris scattered nearby"));
    HuntingStory.TriggerLocation = FVector(8000.0f, -1000.0f, 200.0f);
    HuntingStory.TriggerRadius = 600.0f;
    EnvironmentalStories.Add(HuntingStory);
}

void UNarr_EnvironmentalStorytelling::InitializeDefaultSites()
{
    // Sacred Cave Site
    FNarr_AncientSite SacredCave;
    SacredCave.SiteID = TEXT("SITE_SACRED_CAVE");
    SacredCave.SiteName = TEXT("The Painted Depths");
    SacredCave.CavePaintings.Add(TEXT("Red ochre handprints covering the cave entrance"));
    SacredCave.CavePaintings.Add(TEXT("Hunting scenes showing humans working together against massive predators"));
    SacredCave.CavePaintings.Add(TEXT("Star patterns that might represent seasonal migration routes"));
    SacredCave.BoneRemains.Add(TEXT("Human skull with unusual tool marks - possible ritual significance"));
    SacredCave.BoneRemains.Add(TEXT("Carefully arranged Raptor claws in circular pattern"));
    SacredCave.ToolFragments.Add(TEXT("Obsidian blade with intricate knapping work"));
    SacredCave.ToolFragments.Add(TEXT("Bone needle with decorative carvings"));
    SacredCave.SiteLocation = FVector(-5000.0f, 5000.0f, 300.0f);
    SacredCave.ExplorationRadius = 1000.0f;
    AncientSites.Add(SacredCave);

    // Toolmaker's Workshop Site
    FNarr_AncientSite Workshop;
    Workshop.SiteID = TEXT("SITE_TOOLMAKER_WORKSHOP");
    Workshop.SiteName = TEXT("The Ancient Workshop");
    Workshop.CavePaintings.Add(TEXT("Diagrams showing tool construction techniques"));
    Workshop.CavePaintings.Add(TEXT("Images of different stone types and their uses"));
    Workshop.BoneRemains.Add(TEXT("Mammoth bones used as anvils for tool making"));
    Workshop.BoneRemains.Add(TEXT("Fish bones suggesting a varied diet"));
    Workshop.ToolFragments.Add(TEXT("Half-finished spear points in various stages"));
    Workshop.ToolFragments.Add(TEXT("Hammer stones worn smooth from use"));
    Workshop.ToolFragments.Add(TEXT("Antler pressure flakers for fine knapping"));
    Workshop.SiteLocation = FVector(3000.0f, -6000.0f, 150.0f);
    Workshop.ExplorationRadius = 800.0f;
    AncientSites.Add(Workshop);

    // Tribal Gathering Site
    FNarr_AncientSite GatheringSite;
    GatheringSite.SiteID = TEXT("SITE_TRIBAL_GATHERING");
    GatheringSite.SiteName = TEXT("The Great Circle");
    GatheringSite.CavePaintings.Add(TEXT("Large group scenes showing multiple families together"));
    GatheringSite.CavePaintings.Add(TEXT("Seasonal calendar markings on cave walls"));
    GatheringSite.BoneRemains.Add(TEXT("Communal fire pit with bones from many different hunts"));
    GatheringSite.BoneRemains.Add(TEXT("Children's bones suggesting this was a safe gathering place"));
    GatheringSite.ToolFragments.Add(TEXT("Decorative beads made from small bones"));
    GatheringSite.ToolFragments.Add(TEXT("Musical instruments carved from hollow bones"));
    GatheringSite.SiteLocation = FVector(0.0f, 10000.0f, 100.0f);
    GatheringSite.ExplorationRadius = 1500.0f;
    AncientSites.Add(GatheringSite);
}

bool UNarr_EnvironmentalStorytelling::IsPlayerNearLocation(const FVector& Location, float Radius) const
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
    return Distance <= Radius;
}

APawn* UNarr_EnvironmentalStorytelling::GetPlayerPawn() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return nullptr;
    }

    return PlayerController->GetPawn();
}