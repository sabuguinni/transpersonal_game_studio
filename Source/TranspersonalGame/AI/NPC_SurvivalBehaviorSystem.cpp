#include "NPC_SurvivalBehaviorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_SurvivalBehaviorSystem::UNPC_SurvivalBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default values
    ResourceSearchRadius = 2000.0f;
    SurvivalUpdateInterval = 1.0f;
    ResourceMemoryDuration = 300.0f;
    bEnableAdvancedSurvival = true;
    CurrentPriorityNeed = ENPC_SurvivalNeed::None;
    CurrentTargetLocation = FVector::ZeroVector;
    bIsSeekingResource = false;
    LastSurvivalUpdate = 0.0f;
    LastResourceUpdate = 0.0f;
}

void UNPC_SurvivalBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSurvivalNeeds();
    
    UE_LOG(LogTemp, Log, TEXT("NPC Survival Behavior System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_SurvivalBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableAdvancedSurvival || !GetOwner())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update survival needs periodically
    if (CurrentTime - LastSurvivalUpdate >= SurvivalUpdateInterval)
    {
        UpdateSurvivalNeeds(DeltaTime);
        EvaluateSurvivalPriorities();
        LastSurvivalUpdate = CurrentTime;
    }
    
    // Update resource memory less frequently
    if (CurrentTime - LastResourceUpdate >= 5.0f)
    {
        UpdateResourceMemory(DeltaTime);
        LastResourceUpdate = CurrentTime;
    }
    
    // Execute current survival behavior
    ExecuteSurvivalBehavior();
}

void UNPC_SurvivalBehaviorSystem::InitializeSurvivalNeeds()
{
    // Initialize all survival needs with default values
    TArray<ENPC_SurvivalNeed> AllNeeds = {
        ENPC_SurvivalNeed::Water,
        ENPC_SurvivalNeed::Food,
        ENPC_SurvivalNeed::Shelter,
        ENPC_SurvivalNeed::Safety,
        ENPC_SurvivalNeed::Social,
        ENPC_SurvivalNeed::Rest,
        ENPC_SurvivalNeed::Warmth
    };
    
    for (ENPC_SurvivalNeed Need : AllNeeds)
    {
        FNPC_SurvivalNeedData NeedData;
        NeedData.NeedType = Need;
        NeedData.CurrentLevel = FMath::RandRange(60.0f, 90.0f);
        NeedData.MaxLevel = 100.0f;
        NeedData.UrgencyLevel = ENPC_SurvivalUrgency::Low;
        NeedData.bIsBeingSatisfied = false;
        NeedData.TimeSinceLastSatisfied = 0.0f;
        
        // Set different decay rates for different needs
        switch (Need)
        {
            case ENPC_SurvivalNeed::Water:
                NeedData.DecayRate = 2.0f; // Water is most urgent
                break;
            case ENPC_SurvivalNeed::Food:
                NeedData.DecayRate = 1.5f;
                break;
            case ENPC_SurvivalNeed::Safety:
                NeedData.DecayRate = 3.0f; // Safety degrades quickly in dangerous areas
                break;
            case ENPC_SurvivalNeed::Rest:
                NeedData.DecayRate = 1.0f;
                break;
            case ENPC_SurvivalNeed::Warmth:
                NeedData.DecayRate = 1.2f;
                break;
            case ENPC_SurvivalNeed::Social:
                NeedData.DecayRate = 0.5f; // Social needs degrade slowly
                break;
            case ENPC_SurvivalNeed::Shelter:
                NeedData.DecayRate = 0.3f; // Shelter need is long-term
                break;
            default:
                NeedData.DecayRate = 1.0f;
                break;
        }
        
        SurvivalNeeds.Add(Need, NeedData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d survival needs"), SurvivalNeeds.Num());
}

void UNPC_SurvivalBehaviorSystem::UpdateSurvivalNeeds(float DeltaTime)
{
    for (auto& NeedPair : SurvivalNeeds)
    {
        FNPC_SurvivalNeedData& NeedData = NeedPair.Value;
        
        // Decay need over time if not being satisfied
        if (!NeedData.bIsBeingSatisfied)
        {
            float DecayAmount = NeedData.DecayRate * DeltaTime;
            NeedData.CurrentLevel = FMath::Max(0.0f, NeedData.CurrentLevel - DecayAmount);
            NeedData.TimeSinceLastSatisfied += DeltaTime;
        }
        
        // Calculate urgency based on current level
        CalculateNeedUrgency(NeedData);
    }
    
    // Update resource distances
    UpdateResourceDistances();
}

ENPC_SurvivalNeed UNPC_SurvivalBehaviorSystem::GetMostUrgentNeed() const
{
    ENPC_SurvivalNeed MostUrgentNeed = ENPC_SurvivalNeed::None;
    ENPC_SurvivalUrgency HighestUrgency = ENPC_SurvivalUrgency::None;
    float LowestLevel = 100.0f;
    
    for (const auto& NeedPair : SurvivalNeeds)
    {
        const FNPC_SurvivalNeedData& NeedData = NeedPair.Value;
        
        // Priority: Emergency > Critical > High > Medium > Low
        // Within same urgency, choose lowest level
        if (NeedData.UrgencyLevel > HighestUrgency || 
            (NeedData.UrgencyLevel == HighestUrgency && NeedData.CurrentLevel < LowestLevel))
        {
            HighestUrgency = NeedData.UrgencyLevel;
            LowestLevel = NeedData.CurrentLevel;
            MostUrgentNeed = NeedData.NeedType;
        }
    }
    
    return MostUrgentNeed;
}

void UNPC_SurvivalBehaviorSystem::SatisfyNeed(ENPC_SurvivalNeed NeedType, float Amount)
{
    if (FNPC_SurvivalNeedData* NeedData = SurvivalNeeds.Find(NeedType))
    {
        float OldLevel = NeedData->CurrentLevel;
        NeedData->CurrentLevel = FMath::Min(NeedData->MaxLevel, NeedData->CurrentLevel + Amount);
        NeedData->TimeSinceLastSatisfied = 0.0f;
        NeedData->bIsBeingSatisfied = true;
        
        UE_LOG(LogTemp, Log, TEXT("Satisfied need %d: %.1f -> %.1f (+%.1f)"), 
               (int32)NeedType, OldLevel, NeedData->CurrentLevel, Amount);
        
        // Reset satisfaction flag after a short delay
        GetWorld()->GetTimerManager().SetTimerForNextTick([this, NeedType]()
        {
            if (FNPC_SurvivalNeedData* Data = SurvivalNeeds.Find(NeedType))
            {
                Data->bIsBeingSatisfied = false;
            }
        });
    }
}

float UNPC_SurvivalBehaviorSystem::GetNeedLevel(ENPC_SurvivalNeed NeedType) const
{
    if (const FNPC_SurvivalNeedData* NeedData = SurvivalNeeds.Find(NeedType))
    {
        return NeedData->CurrentLevel;
    }
    return 0.0f;
}

ENPC_SurvivalUrgency UNPC_SurvivalBehaviorSystem::GetNeedUrgency(ENPC_SurvivalNeed NeedType) const
{
    if (const FNPC_SurvivalNeedData* NeedData = SurvivalNeeds.Find(NeedType))
    {
        return NeedData->UrgencyLevel;
    }
    return ENPC_SurvivalUrgency::None;
}

void UNPC_SurvivalBehaviorSystem::DiscoverResource(FVector Location, ENPC_SurvivalNeed ResourceType, float Quality)
{
    // Check if we already know about this resource (within a small radius)
    for (const auto& ExistingResource : KnownResources)
    {
        if (ExistingResource.ResourceType == ResourceType && 
            FVector::Dist(ExistingResource.Location, Location) < 100.0f)
        {
            return; // Already known
        }
    }
    
    // Add new resource
    FNPC_SurvivalResourceLocation NewResource;
    NewResource.Location = Location;
    NewResource.ResourceType = ResourceType;
    NewResource.Quality = Quality;
    NewResource.Distance = GetOwner() ? FVector::Dist(GetOwner()->GetActorLocation(), Location) : 0.0f;
    NewResource.bIsOccupied = false;
    NewResource.LastVisitedTime = GetWorld()->GetTimeSeconds();
    
    KnownResources.Add(NewResource);
    
    UE_LOG(LogTemp, Log, TEXT("Discovered new resource: Type %d at %s, Quality %.1f"), 
           (int32)ResourceType, *Location.ToString(), Quality);
}

FNPC_SurvivalResourceLocation UNPC_SurvivalBehaviorSystem::FindBestResource(ENPC_SurvivalNeed NeedType) const
{
    FNPC_SurvivalResourceLocation BestResource;
    float BestScore = -1.0f;
    
    for (const auto& Resource : KnownResources)
    {
        if (Resource.ResourceType != NeedType || Resource.bIsOccupied)
        {
            continue;
        }
        
        // Calculate score based on quality and distance
        float DistanceFactor = FMath::Max(0.1f, 1.0f - (Resource.Distance / ResourceSearchRadius));
        float QualityFactor = Resource.Quality / 100.0f;
        float Score = (QualityFactor * 0.6f) + (DistanceFactor * 0.4f);
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestResource = Resource;
        }
    }
    
    return BestResource;
}

void UNPC_SurvivalBehaviorSystem::UpdateResourceMemory(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old resources from memory
    KnownResources.RemoveAll([this, CurrentTime](const FNPC_SurvivalResourceLocation& Resource)
    {
        return (CurrentTime - Resource.LastVisitedTime) > ResourceMemoryDuration;
    });
    
    // Update distances to remaining resources
    UpdateResourceDistances();
}

bool UNPC_SurvivalBehaviorSystem::IsResourceStillValid(const FNPC_SurvivalResourceLocation& Resource) const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - Resource.LastVisitedTime) <= ResourceMemoryDuration;
}

void UNPC_SurvivalBehaviorSystem::EvaluateSurvivalPriorities()
{
    ENPC_SurvivalNeed NewPriorityNeed = GetMostUrgentNeed();
    
    if (NewPriorityNeed != CurrentPriorityNeed)
    {
        CurrentPriorityNeed = NewPriorityNeed;
        bIsSeekingResource = false; // Reset seeking state when priority changes
        
        UE_LOG(LogTemp, Log, TEXT("Priority need changed to: %d"), (int32)CurrentPriorityNeed);
    }
}

void UNPC_SurvivalBehaviorSystem::SetSurvivalTarget(ENPC_SurvivalNeed NeedType, FVector TargetLocation)
{
    CurrentPriorityNeed = NeedType;
    CurrentTargetLocation = TargetLocation;
    bIsSeekingResource = true;
    
    UE_LOG(LogTemp, Log, TEXT("Set survival target: Need %d at %s"), 
           (int32)NeedType, *TargetLocation.ToString());
}

bool UNPC_SurvivalBehaviorSystem::ShouldSeekResource(ENPC_SurvivalNeed NeedType) const
{
    if (const FNPC_SurvivalNeedData* NeedData = SurvivalNeeds.Find(NeedType))
    {
        return NeedData->UrgencyLevel >= ENPC_SurvivalUrgency::Medium;
    }
    return false;
}

void UNPC_SurvivalBehaviorSystem::ExecuteSurvivalBehavior()
{
    if (CurrentPriorityNeed == ENPC_SurvivalNeed::None)
    {
        return;
    }
    
    // If not currently seeking a resource, find one
    if (!bIsSeekingResource)
    {
        FNPC_SurvivalResourceLocation BestResource = FindBestResource(CurrentPriorityNeed);
        if (BestResource.ResourceType != ENPC_SurvivalNeed::None)
        {
            SetSurvivalTarget(CurrentPriorityNeed, BestResource.Location);
        }
    }
    
    // Check if we've reached our target
    if (bIsSeekingResource && GetOwner())
    {
        float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTargetLocation);
        if (DistanceToTarget < 200.0f) // Within interaction range
        {
            // Satisfy the need
            SatisfyNeed(CurrentPriorityNeed, 25.0f);
            bIsSeekingResource = false;
            CurrentTargetLocation = FVector::ZeroVector;
        }
    }
}

void UNPC_SurvivalBehaviorSystem::CalculateNeedUrgency(FNPC_SurvivalNeedData& NeedData)
{
    float Level = NeedData.CurrentLevel;
    
    if (Level <= 10.0f)
    {
        NeedData.UrgencyLevel = ENPC_SurvivalUrgency::Emergency;
    }
    else if (Level <= 25.0f)
    {
        NeedData.UrgencyLevel = ENPC_SurvivalUrgency::Critical;
    }
    else if (Level <= 40.0f)
    {
        NeedData.UrgencyLevel = ENPC_SurvivalUrgency::High;
    }
    else if (Level <= 60.0f)
    {
        NeedData.UrgencyLevel = ENPC_SurvivalUrgency::Medium;
    }
    else
    {
        NeedData.UrgencyLevel = ENPC_SurvivalUrgency::Low;
    }
}

void UNPC_SurvivalBehaviorSystem::UpdateResourceDistances()
{
    if (!GetOwner())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (auto& Resource : KnownResources)
    {
        Resource.Distance = FVector::Dist(OwnerLocation, Resource.Location);
    }
}

void UNPC_SurvivalBehaviorSystem::CleanupOldResources()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    KnownResources.RemoveAll([this, CurrentTime](const FNPC_SurvivalResourceLocation& Resource)
    {
        return !IsResourceStillValid(Resource);
    });
}