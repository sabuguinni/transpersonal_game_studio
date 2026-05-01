#include "TribalNPCBehavior.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AI/NavigationSystemBase.h"
#include "NavigationSystem.h"

UNPC_TribalNPCBehavior::UNPC_TribalNPCBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UNPC_TribalNPCBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner character and movement component
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        HomeLocation = OwnerCharacter->GetActorLocation();
        
        // Initialize survival stats based on personality
        switch (Personality)
        {
            case ENPC_TribalPersonality::Cautious:
                SurvivalStats.Fear = 30.0f;
                SurvivalStats.Energy = 80.0f;
                break;
            case ENPC_TribalPersonality::Aggressive:
                SurvivalStats.Fear = 10.0f;
                SurvivalStats.Energy = 90.0f;
                break;
            case ENPC_TribalPersonality::Curious:
                SurvivalStats.Fear = 20.0f;
                SurvivalStats.Social = 70.0f;
                break;
            case ENPC_TribalPersonality::Fearful:
                SurvivalStats.Fear = 60.0f;
                SurvivalStats.Comfort = 30.0f;
                break;
            case ENPC_TribalPersonality::Leader:
                SurvivalStats.Social = 80.0f;
                SurvivalStats.Energy = 85.0f;
                break;
            case ENPC_TribalPersonality::Follower:
                SurvivalStats.Social = 60.0f;
                SurvivalStats.Fear = 25.0f;
                break;
        }
        
        // Add home location to memory
        AddLocationMemory(HomeLocation, "Home", 10.0f);
    }
}

void UNPC_TribalNPCBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter)
        return;
    
    // Update survival needs
    UpdateSurvivalNeeds(DeltaTime);
    
    // Update behavior state
    UpdateBehaviorState(DeltaTime);
    
    // Execute current state behavior
    ExecuteCurrentState(DeltaTime);
    
    // Check for threats and opportunities
    CheckForThreats();
    CheckForOpportunities();
}

void UNPC_TribalNPCBehavior::SetTribalState(ENPC_TribalState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        bHasTarget = false;
        
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed state to %d"), 
               *OwnerCharacter->GetName(), (int32)NewState);
    }
}

void UNPC_TribalNPCBehavior::AddLocationMemory(FVector Location, FString LocationType, float Importance)
{
    FNPC_LocationMemory NewMemory;
    NewMemory.Location = Location;
    NewMemory.LocationType = LocationType;
    NewMemory.LastVisited = GetWorld()->GetTimeSeconds();
    NewMemory.Importance = Importance;
    NewMemory.bIsSafe = IsLocationSafe(Location);
    
    KnownLocations.Add(NewMemory);
    
    // Limit memory to prevent infinite growth
    if (KnownLocations.Num() > 50)
    {
        KnownLocations.RemoveAt(0);
    }
}

FVector UNPC_TribalNPCBehavior::FindNearestLocationOfType(FString LocationType)
{
    FVector NearestLocation = FVector::ZeroVector;
    float NearestDistance = FLT_MAX;
    FVector CurrentLocation = OwnerCharacter->GetActorLocation();
    
    for (const FNPC_LocationMemory& Memory : KnownLocations)
    {
        if (Memory.LocationType == LocationType && Memory.bIsSafe)
        {
            float Distance = FVector::Dist(CurrentLocation, Memory.Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestLocation = Memory.Location;
            }
        }
    }
    
    return NearestLocation;
}

void UNPC_TribalNPCBehavior::UpdateSurvivalNeeds(float DeltaTime)
{
    SurvivalTimer += DeltaTime;
    
    if (SurvivalTimer >= 1.0f) // Update every second
    {
        // Decay survival stats
        SurvivalStats.Hunger = FMath::Max(0.0f, SurvivalStats.Hunger - SurvivalDecayRate);
        SurvivalStats.Thirst = FMath::Max(0.0f, SurvivalStats.Thirst - SurvivalDecayRate * 1.2f);
        SurvivalStats.Energy = FMath::Max(0.0f, SurvivalStats.Energy - SurvivalDecayRate * 0.5f);
        
        // Fear naturally decreases over time if safe
        if (CurrentState != ENPC_TribalState::Fleeing)
        {
            SurvivalStats.Fear = FMath::Max(0.0f, SurvivalStats.Fear - SurvivalDecayRate * 2.0f);
        }
        
        SurvivalTimer = 0.0f;
    }
}

ENPC_SurvivalNeed UNPC_TribalNPCBehavior::GetHighestPriorityNeed()
{
    // Fear overrides everything
    if (SurvivalStats.Fear > 70.0f)
        return ENPC_SurvivalNeed::Safety;
    
    // Critical needs
    if (SurvivalStats.Thirst < 20.0f)
        return ENPC_SurvivalNeed::Water;
    
    if (SurvivalStats.Hunger < 20.0f)
        return ENPC_SurvivalNeed::Food;
    
    if (SurvivalStats.Energy < 20.0f)
        return ENPC_SurvivalNeed::Rest;
    
    // Secondary needs
    if (SurvivalStats.Comfort < 30.0f)
        return ENPC_SurvivalNeed::Shelter;
    
    if (SurvivalStats.Social < 30.0f)
        return ENPC_SurvivalNeed::Social;
    
    return ENPC_SurvivalNeed::Food; // Default
}

void UNPC_TribalNPCBehavior::ReactToPlayer(ACharacter* PlayerCharacter, float Distance)
{
    if (!PlayerCharacter)
        return;
    
    // Personality-based reactions
    switch (Personality)
    {
        case ENPC_TribalPersonality::Cautious:
            if (Distance < 800.0f)
            {
                SurvivalStats.Fear += 20.0f;
                if (SurvivalStats.Fear > 50.0f)
                    SetTribalState(ENPC_TribalState::Fleeing);
            }
            break;
            
        case ENPC_TribalPersonality::Curious:
            if (Distance < 1000.0f && Distance > 400.0f)
            {
                // Move closer but maintain distance
                CurrentTarget = PlayerCharacter->GetActorLocation();
                bHasTarget = true;
            }
            break;
            
        case ENPC_TribalPersonality::Fearful:
            if (Distance < 1200.0f)
            {
                SurvivalStats.Fear += 30.0f;
                SetTribalState(ENPC_TribalState::Fleeing);
            }
            break;
            
        case ENPC_TribalPersonality::Aggressive:
            if (Distance < 600.0f)
            {
                SurvivalStats.Fear += 10.0f;
                // Could implement aggressive behavior here
            }
            break;
    }
}

void UNPC_TribalNPCBehavior::ReactToDinosaur(AActor* DinosaurActor, float Distance)
{
    if (!DinosaurActor)
        return;
    
    // All NPCs fear dinosaurs
    if (Distance < 1500.0f)
    {
        SurvivalStats.Fear += 40.0f;
        SetTribalState(ENPC_TribalState::Fleeing);
        
        // Remember this location as dangerous
        AddLocationMemory(DinosaurActor->GetActorLocation(), "Danger", 5.0f);
    }
}

void UNPC_TribalNPCBehavior::UpdateBehaviorState(float DeltaTime)
{
    StateTimer += DeltaTime;
    
    // Check if we need to change state based on survival needs
    ENPC_SurvivalNeed PriorityNeed = GetHighestPriorityNeed();
    
    switch (PriorityNeed)
    {
        case ENPC_SurvivalNeed::Safety:
            if (CurrentState != ENPC_TribalState::Fleeing)
                SetTribalState(ENPC_TribalState::Fleeing);
            break;
            
        case ENPC_SurvivalNeed::Water:
        case ENPC_SurvivalNeed::Food:
            if (CurrentState != ENPC_TribalState::Gathering)
                SetTribalState(ENPC_TribalState::Gathering);
            break;
            
        case ENPC_SurvivalNeed::Rest:
            if (CurrentState != ENPC_TribalState::Resting)
                SetTribalState(ENPC_TribalState::Resting);
            break;
            
        case ENPC_SurvivalNeed::Shelter:
            if (CurrentState != ENPC_TribalState::Crafting)
                SetTribalState(ENPC_TribalState::Crafting);
            break;
            
        case ENPC_SurvivalNeed::Social:
            if (CurrentState != ENPC_TribalState::Socializing)
                SetTribalState(ENPC_TribalState::Socializing);
            break;
    }
    
    // Random state changes for variety
    if (StateTimer > StateChangeInterval && SurvivalStats.Fear < 30.0f)
    {
        int32 RandomState = FMath::RandRange(0, 3);
        switch (RandomState)
        {
            case 0: SetTribalState(ENPC_TribalState::Idle); break;
            case 1: SetTribalState(ENPC_TribalState::Patrolling); break;
            case 2: SetTribalState(ENPC_TribalState::Gathering); break;
            case 3: SetTribalState(ENPC_TribalState::Crafting); break;
        }
    }
}

void UNPC_TribalNPCBehavior::ExecuteCurrentState(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_TribalState::Idle:
            IdleBehavior();
            break;
        case ENPC_TribalState::Gathering:
            GatheringBehavior();
            break;
        case ENPC_TribalState::Patrolling:
            PatrollingBehavior();
            break;
        case ENPC_TribalState::Fleeing:
            FleeingBehavior();
            break;
        case ENPC_TribalState::Socializing:
            SocializingBehavior();
            break;
        case ENPC_TribalState::Resting:
            RestingBehavior();
            break;
        case ENPC_TribalState::Hunting:
            HuntingBehavior();
            break;
        case ENPC_TribalState::Crafting:
            CraftingBehavior();
            break;
    }
}

void UNPC_TribalNPCBehavior::MoveToTarget(FVector TargetLocation)
{
    if (!OwnerCharacter || !MovementComponent)
        return;
    
    FVector Direction = (TargetLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();
    MovementComponent->AddInputVector(Direction);
    
    // Face movement direction
    if (!Direction.IsNearlyZero())
    {
        FRotator NewRotation = Direction.Rotation();
        OwnerCharacter->SetActorRotation(NewRotation);
    }
}

void UNPC_TribalNPCBehavior::IdleBehavior()
{
    // Look around, minimal movement
    if (StateTimer > 2.0f)
    {
        FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
        FRotator NewRotation = CurrentRotation + FRotator(0, FMath::RandRange(-45.0f, 45.0f), 0);
        OwnerCharacter->SetActorRotation(NewRotation);
        StateTimer = 0.0f;
    }
}

void UNPC_TribalNPCBehavior::GatheringBehavior()
{
    if (!bHasTarget)
    {
        // Find a gathering spot
        FVector GatherSpot = FindNearestLocationOfType("GatheringSpot");
        if (GatherSpot.IsZero())
        {
            // Create random gathering target within patrol radius
            FVector RandomDirection = FMath::VRand();
            RandomDirection.Z = 0;
            CurrentTarget = HomeLocation + RandomDirection * FMath::RandRange(200.0f, PatrolRadius);
        }
        else
        {
            CurrentTarget = GatherSpot;
        }
        bHasTarget = true;
    }
    
    MoveToTarget(CurrentTarget);
    
    // Simulate gathering
    if (FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget) < 100.0f)
    {
        SurvivalStats.Hunger = FMath::Min(100.0f, SurvivalStats.Hunger + 10.0f);
        SurvivalStats.Thirst = FMath::Min(100.0f, SurvivalStats.Thirst + 5.0f);
        bHasTarget = false;
        StateTimer = 0.0f;
    }
}

void UNPC_TribalNPCBehavior::PatrollingBehavior()
{
    if (!bHasTarget)
    {
        // Choose random patrol point
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0;
        CurrentTarget = HomeLocation + RandomDirection * FMath::RandRange(PatrolRadius * 0.5f, PatrolRadius);
        bHasTarget = true;
    }
    
    MoveToTarget(CurrentTarget);
    
    if (FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget) < 150.0f)
    {
        bHasTarget = false;
        StateTimer = 0.0f;
    }
}

void UNPC_TribalNPCBehavior::FleeingBehavior()
{
    // Run away from danger towards home
    FVector FleeDirection = (HomeLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();
    CurrentTarget = OwnerCharacter->GetActorLocation() + FleeDirection * FleeDistance;
    
    MoveToTarget(CurrentTarget);
    
    // Increase movement speed when fleeing
    if (MovementComponent)
    {
        MovementComponent->MaxWalkSpeed = MovementSpeed * 1.5f;
    }
    
    // Stop fleeing when fear decreases
    if (SurvivalStats.Fear < 20.0f)
    {
        SetTribalState(ENPC_TribalState::Idle);
        if (MovementComponent)
        {
            MovementComponent->MaxWalkSpeed = MovementSpeed;
        }
    }
}

void UNPC_TribalNPCBehavior::SocializingBehavior()
{
    // Look for other NPCs nearby
    // For now, just increase social stat
    SurvivalStats.Social = FMath::Min(100.0f, SurvivalStats.Social + 5.0f);
}

void UNPC_TribalNPCBehavior::RestingBehavior()
{
    // Find shelter or safe spot
    FVector ShelterSpot = FindNearestLocationOfType("ShelterArea");
    if (!ShelterSpot.IsZero())
    {
        if (!bHasTarget)
        {
            CurrentTarget = ShelterSpot;
            bHasTarget = true;
        }
        
        if (FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget) > 100.0f)
        {
            MoveToTarget(CurrentTarget);
        }
        else
        {
            // Rest and recover energy
            SurvivalStats.Energy = FMath::Min(100.0f, SurvivalStats.Energy + 15.0f);
            SurvivalStats.Comfort = FMath::Min(100.0f, SurvivalStats.Comfort + 10.0f);
        }
    }
    else
    {
        // Rest in place
        SurvivalStats.Energy = FMath::Min(100.0f, SurvivalStats.Energy + 10.0f);
    }
}

void UNPC_TribalNPCBehavior::HuntingBehavior()
{
    // Basic hunting behavior - look for small prey
    PatrollingBehavior(); // Use patrol behavior for now
}

void UNPC_TribalNPCBehavior::CraftingBehavior()
{
    // Simulate crafting activities
    SurvivalStats.Comfort = FMath::Min(100.0f, SurvivalStats.Comfort + 8.0f);
    SurvivalStats.Energy = FMath::Max(0.0f, SurvivalStats.Energy - 5.0f);
}

void UNPC_TribalNPCBehavior::CheckForThreats()
{
    // Check for player and dinosaurs nearby
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    FVector MyLocation = OwnerCharacter->GetActorLocation();
    
    // Check for player
    ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (Player)
    {
        float Distance = FVector::Dist(MyLocation, Player->GetActorLocation());
        if (Distance < 1500.0f)
        {
            ReactToPlayer(Player, Distance);
        }
    }
    
    // Check for dinosaurs (simplified - would need proper dinosaur detection)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != OwnerCharacter && Actor != Player)
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            if (Distance < 2000.0f)
            {
                // Assume it's a dinosaur for now
                ReactToDinosaur(Actor, Distance);
            }
        }
    }
}

void UNPC_TribalNPCBehavior::CheckForOpportunities()
{
    // Look for resources, other NPCs, etc.
    // This would be expanded based on specific game needs
}

void UNPC_TribalNPCBehavior::UpdatePersonalityInfluence()
{
    // Personality affects behavior decisions
    // This could modify state change probabilities, reaction thresholds, etc.
}

bool UNPC_TribalNPCBehavior::IsLocationSafe(FVector Location)
{
    // Basic safety check - could be expanded with threat detection
    return CalculateLocationSafety(Location) > 0.5f;
}

float UNPC_TribalNPCBehavior::CalculateLocationSafety(FVector Location)
{
    float Safety = 1.0f;
    
    // Distance from home affects safety
    float DistanceFromHome = FVector::Dist(Location, HomeLocation);
    if (DistanceFromHome > PatrolRadius)
    {
        Safety -= (DistanceFromHome - PatrolRadius) / 1000.0f;
    }
    
    // Check for known dangerous locations
    for (const FNPC_LocationMemory& Memory : KnownLocations)
    {
        if (Memory.LocationType == "Danger")
        {
            float DistanceFromDanger = FVector::Dist(Location, Memory.Location);
            if (DistanceFromDanger < 500.0f)
            {
                Safety -= 0.5f;
            }
        }
    }
    
    return FMath::Clamp(Safety, 0.0f, 1.0f);
}