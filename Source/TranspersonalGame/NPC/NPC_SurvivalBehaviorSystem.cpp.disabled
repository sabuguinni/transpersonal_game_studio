#include "NPC_SurvivalBehaviorSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_SurvivalBehaviorSystem::UNPC_SurvivalBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize survival stats
    SurvivalStats.Hunger = 100.0f;
    SurvivalStats.Thirst = 100.0f;
    SurvivalStats.Energy = 100.0f;
    SurvivalStats.Safety = 100.0f;
    SurvivalStats.Temperature = 20.0f;
    
    // Initialize behavior state
    CurrentBehavior = ENPC_SurvivalBehavior::Idle;
    
    // Initialize configuration
    HungerDecayRate = 5.0f;
    ThirstDecayRate = 8.0f;
    EnergyDecayRate = 3.0f;
    ResourceSearchRadius = 5000.0f;
    CriticalNeedThreshold = 25.0f;
    SafetyRadius = 1000.0f;
    BehaviorUpdateInterval = 2.0f;
    LastBehaviorUpdate = 0.0f;
}

void UNPC_SurvivalBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize known resources with some basic locations
    FNPC_ResourceLocation WaterSource;
    WaterSource.Location = FVector(0, 3000, 200);
    WaterSource.ResourceType = ENPC_SurvivalNeed::Water;
    WaterSource.Quality = 1.0f;
    WaterSource.bIsAvailable = true;
    AddKnownResource(WaterSource);
    
    FNPC_ResourceLocation FoodSource;
    FoodSource.Location = FVector(1500, -1000, 250);
    FoodSource.ResourceType = ENPC_SurvivalNeed::Food;
    FoodSource.Quality = 0.8f;
    FoodSource.bIsAvailable = true;
    AddKnownResource(FoodSource);
    
    FNPC_ResourceLocation ShelterSource;
    ShelterSource.Location = FVector(-2000, -500, 400);
    ShelterSource.ResourceType = ENPC_SurvivalNeed::Shelter;
    ShelterSource.Quality = 1.0f;
    ShelterSource.bIsAvailable = true;
    AddKnownResource(ShelterSource);
    
    UE_LOG(LogTemp, Log, TEXT("NPC Survival Behavior System initialized with %d known resources"), KnownResources.Num());
}

void UNPC_SurvivalBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update survival stats
    UpdateSurvivalStats(DeltaTime);
    
    // Update environmental factors
    UpdateEnvironmentalFactors();
    
    // Process survival behavior
    ProcessSurvivalBehavior(DeltaTime);
}

void UNPC_SurvivalBehaviorSystem::UpdateSurvivalStats(float DeltaTime)
{
    // Decay survival stats over time
    DecaySurvivalStats(DeltaTime);
    
    // Clamp values
    SurvivalStats.Hunger = FMath::Clamp(SurvivalStats.Hunger, 0.0f, 100.0f);
    SurvivalStats.Thirst = FMath::Clamp(SurvivalStats.Thirst, 0.0f, 100.0f);
    SurvivalStats.Energy = FMath::Clamp(SurvivalStats.Energy, 0.0f, 100.0f);
    SurvivalStats.Safety = FMath::Clamp(SurvivalStats.Safety, 0.0f, 100.0f);
}

ENPC_SurvivalNeed UNPC_SurvivalBehaviorSystem::GetMostUrgentNeed() const
{
    // Check for critical needs first
    if (SurvivalStats.Thirst <= CriticalNeedThreshold)
    {
        return ENPC_SurvivalNeed::Water;
    }
    
    if (SurvivalStats.Safety <= CriticalNeedThreshold)
    {
        return ENPC_SurvivalNeed::Safety;
    }
    
    if (SurvivalStats.Hunger <= CriticalNeedThreshold)
    {
        return ENPC_SurvivalNeed::Food;
    }
    
    if (SurvivalStats.Energy <= CriticalNeedThreshold)
    {
        return ENPC_SurvivalNeed::Rest;
    }
    
    // Find the lowest non-critical need
    float LowestValue = 100.0f;
    ENPC_SurvivalNeed MostUrgent = ENPC_SurvivalNeed::None;
    
    if (SurvivalStats.Thirst < LowestValue)
    {
        LowestValue = SurvivalStats.Thirst;
        MostUrgent = ENPC_SurvivalNeed::Water;
    }
    
    if (SurvivalStats.Hunger < LowestValue)
    {
        LowestValue = SurvivalStats.Hunger;
        MostUrgent = ENPC_SurvivalNeed::Food;
    }
    
    if (SurvivalStats.Energy < LowestValue)
    {
        LowestValue = SurvivalStats.Energy;
        MostUrgent = ENPC_SurvivalNeed::Rest;
    }
    
    return MostUrgent;
}

void UNPC_SurvivalBehaviorSystem::SeekResource(ENPC_SurvivalNeed NeedType)
{
    FNPC_ResourceLocation TargetResource;
    if (FindNearestResource(NeedType, TargetResource))
    {
        CurrentTarget = TargetResource;
        SetSurvivalBehavior(ENPC_SurvivalBehavior::Seeking);
        
        UE_LOG(LogTemp, Log, TEXT("NPC seeking %s resource at location (%f, %f, %f)"), 
            *UEnum::GetValueAsString(NeedType),
            TargetResource.Location.X, TargetResource.Location.Y, TargetResource.Location.Z);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No %s resource found within search radius"), *UEnum::GetValueAsString(NeedType));
        SetSurvivalBehavior(ENPC_SurvivalBehavior::Idle);
    }
}

void UNPC_SurvivalBehaviorSystem::ConsumeResource(const FNPC_ResourceLocation& Resource)
{
    switch (Resource.ResourceType)
    {
        case ENPC_SurvivalNeed::Water:
            SurvivalStats.Thirst = FMath::Min(100.0f, SurvivalStats.Thirst + (50.0f * Resource.Quality));
            break;
            
        case ENPC_SurvivalNeed::Food:
            SurvivalStats.Hunger = FMath::Min(100.0f, SurvivalStats.Hunger + (40.0f * Resource.Quality));
            break;
            
        case ENPC_SurvivalNeed::Shelter:
            SurvivalStats.Safety = FMath::Min(100.0f, SurvivalStats.Safety + (30.0f * Resource.Quality));
            SurvivalStats.Temperature = 20.0f; // Normalize temperature in shelter
            break;
            
        case ENPC_SurvivalNeed::Rest:
            SurvivalStats.Energy = FMath::Min(100.0f, SurvivalStats.Energy + (60.0f * Resource.Quality));
            break;
            
        default:
            break;
    }
    
    SetSurvivalBehavior(ENPC_SurvivalBehavior::Consuming);
    UE_LOG(LogTemp, Log, TEXT("NPC consuming %s resource"), *UEnum::GetValueAsString(Resource.ResourceType));
}

bool UNPC_SurvivalBehaviorSystem::FindNearestResource(ENPC_SurvivalNeed NeedType, FNPC_ResourceLocation& OutResource) const
{
    float NearestDistance = ResourceSearchRadius;
    bool bFoundResource = false;
    FVector NPCLocation = GetOwner()->GetActorLocation();
    
    for (const FNPC_ResourceLocation& Resource : KnownResources)
    {
        if (Resource.ResourceType == NeedType && Resource.bIsAvailable)
        {
            float Distance = FVector::Dist(NPCLocation, Resource.Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                OutResource = Resource;
                bFoundResource = true;
            }
        }
    }
    
    return bFoundResource;
}

void UNPC_SurvivalBehaviorSystem::AddKnownResource(const FNPC_ResourceLocation& Resource)
{
    // Check if resource already exists at this location
    for (FNPC_ResourceLocation& ExistingResource : KnownResources)
    {
        if (FVector::Dist(ExistingResource.Location, Resource.Location) < 100.0f)
        {
            // Update existing resource
            ExistingResource = Resource;
            return;
        }
    }
    
    // Add new resource
    KnownResources.Add(Resource);
}

void UNPC_SurvivalBehaviorSystem::SetSurvivalBehavior(ENPC_SurvivalBehavior NewBehavior)
{
    if (CurrentBehavior != NewBehavior)
    {
        CurrentBehavior = NewBehavior;
        UE_LOG(LogTemp, Log, TEXT("NPC behavior changed to: %s"), *UEnum::GetValueAsString(NewBehavior));
    }
}

ENPC_SurvivalState UNPC_SurvivalBehaviorSystem::GetSurvivalState(ENPC_SurvivalNeed NeedType) const
{
    float Value = 100.0f;
    
    switch (NeedType)
    {
        case ENPC_SurvivalNeed::Water:
            Value = SurvivalStats.Thirst;
            break;
        case ENPC_SurvivalNeed::Food:
            Value = SurvivalStats.Hunger;
            break;
        case ENPC_SurvivalNeed::Rest:
            Value = SurvivalStats.Energy;
            break;
        case ENPC_SurvivalNeed::Safety:
            Value = SurvivalStats.Safety;
            break;
        default:
            return ENPC_SurvivalState::Satisfied;
    }
    
    if (Value >= 80.0f) return ENPC_SurvivalState::Satisfied;
    if (Value >= 60.0f) return ENPC_SurvivalState::Mild;
    if (Value >= 40.0f) return ENPC_SurvivalState::Moderate;
    if (Value >= 20.0f) return ENPC_SurvivalState::Severe;
    return ENPC_SurvivalState::Critical;
}

void UNPC_SurvivalBehaviorSystem::UpdateEnvironmentalFactors()
{
    // Update safety based on nearby threats
    if (IsInDanger())
    {
        SurvivalStats.Safety = FMath::Max(0.0f, SurvivalStats.Safety - 10.0f);
    }
    else if (IsInSafeArea())
    {
        SurvivalStats.Safety = FMath::Min(100.0f, SurvivalStats.Safety + 5.0f);
    }
}

bool UNPC_SurvivalBehaviorSystem::IsInSafeArea() const
{
    // Check if NPC is near shelter or in a protected area
    FNPC_ResourceLocation ShelterResource;
    if (FindNearestResource(ENPC_SurvivalNeed::Shelter, ShelterResource))
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ShelterResource.Location);
        return Distance < SafetyRadius;
    }
    return false;
}

FVector UNPC_SurvivalBehaviorSystem::GetSafeLocation() const
{
    FNPC_ResourceLocation ShelterResource;
    if (FindNearestResource(ENPC_SurvivalNeed::Shelter, ShelterResource))
    {
        return ShelterResource.Location;
    }
    return GetOwner()->GetActorLocation();
}

void UNPC_SurvivalBehaviorSystem::ProcessSurvivalBehavior(float DeltaTime)
{
    LastBehaviorUpdate += DeltaTime;
    
    if (LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        LastBehaviorUpdate = 0.0f;
        
        // Determine behavior based on current needs
        ENPC_SurvivalNeed UrgentNeed = GetMostUrgentNeed();
        
        switch (CurrentBehavior)
        {
            case ENPC_SurvivalBehavior::Idle:
                if (UrgentNeed != ENPC_SurvivalNeed::None)
                {
                    SeekResource(UrgentNeed);
                }
                break;
                
            case ENPC_SurvivalBehavior::Seeking:
                // Check if we've reached the target
                if (FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget.Location) < 200.0f)
                {
                    ConsumeResource(CurrentTarget);
                }
                else
                {
                    MoveTowardsTarget();
                }
                break;
                
            case ENPC_SurvivalBehavior::Consuming:
                // Consuming behavior lasts for a short time
                SetSurvivalBehavior(ENPC_SurvivalBehavior::Idle);
                break;
                
            case ENPC_SurvivalBehavior::Fleeing:
                if (!IsInDanger())
                {
                    SetSurvivalBehavior(ENPC_SurvivalBehavior::Idle);
                }
                break;
                
            default:
                break;
        }
        
        // Override with danger response
        if (IsInDanger() && CurrentBehavior != ENPC_SurvivalBehavior::Fleeing)
        {
            SetSurvivalBehavior(ENPC_SurvivalBehavior::Fleeing);
        }
    }
}

bool UNPC_SurvivalBehaviorSystem::ShouldSeekShelter() const
{
    return GetSurvivalState(ENPC_SurvivalNeed::Safety) >= ENPC_SurvivalState::Moderate || 
           SurvivalStats.Temperature < 5.0f || SurvivalStats.Temperature > 35.0f;
}

bool UNPC_SurvivalBehaviorSystem::ShouldRest() const
{
    return GetSurvivalState(ENPC_SurvivalNeed::Rest) >= ENPC_SurvivalState::Moderate;
}

bool UNPC_SurvivalBehaviorSystem::IsInDanger() const
{
    // Simple danger detection - can be expanded with threat detection
    return SurvivalStats.Safety < 30.0f;
}

void UNPC_SurvivalBehaviorSystem::DecaySurvivalStats(float DeltaTime)
{
    // Decay rates per second
    SurvivalStats.Hunger -= HungerDecayRate * DeltaTime;
    SurvivalStats.Thirst -= ThirstDecayRate * DeltaTime;
    SurvivalStats.Energy -= EnergyDecayRate * DeltaTime;
    
    // Temperature affects other stats
    if (SurvivalStats.Temperature < 10.0f || SurvivalStats.Temperature > 30.0f)
    {
        SurvivalStats.Energy -= 2.0f * DeltaTime;
        SurvivalStats.Thirst -= 2.0f * DeltaTime;
    }
}

void UNPC_SurvivalBehaviorSystem::UpdateResourceAvailability()
{
    // Update availability of known resources
    for (FNPC_ResourceLocation& Resource : KnownResources)
    {
        // Resources can become unavailable due to various factors
        // This is a placeholder for more complex resource management
        if (!Resource.bIsAvailable && FMath::RandRange(0.0f, 1.0f) < 0.1f)
        {
            Resource.bIsAvailable = true; // 10% chance to become available again
        }
    }
}

void UNPC_SurvivalBehaviorSystem::EvaluateThreatLevel()
{
    // Placeholder for threat evaluation
    // Could check for nearby predators, environmental hazards, etc.
}

bool UNPC_SurvivalBehaviorSystem::IsResourceAccessible(const FNPC_ResourceLocation& Resource) const
{
    // Check if the resource is accessible (not blocked by obstacles)
    // This is a simplified check - could be expanded with pathfinding
    return Resource.bIsAvailable;
}

void UNPC_SurvivalBehaviorSystem::MoveTowardsTarget()
{
    // Simple movement towards target
    // In a full implementation, this would use proper AI movement
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector Direction = (CurrentTarget.Location - CurrentLocation).GetSafeNormal();
    
    // This is a placeholder - actual movement would be handled by movement component
    UE_LOG(LogTemp, VeryVerbose, TEXT("NPC moving towards target: %s"), 
        *CurrentTarget.Location.ToString());
}