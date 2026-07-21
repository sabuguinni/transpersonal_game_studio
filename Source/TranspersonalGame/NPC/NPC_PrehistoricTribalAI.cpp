#include "NPC_PrehistoricTribalAI.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UNPC_PrehistoricTribalAI::UNPC_PrehistoricTribalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    TribalRole = ENPC_TribalRole::Gatherer;
    TribeName = TEXT("Stone River Tribe");
    TribalHierarchy = 1;
    Experience = 0.0f;
    
    CurrentActivity = ENPC_TribalActivity::Idle;
    
    // Behavior parameters
    AlertnessRadius = 2000.0f;
    FleeDistance = 5000.0f;
    SocialDistance = 500.0f;
    WorkRadius = 1500.0f;
    
    // Daily routine
    DayStartTime = 6.0f;
    DayEndTime = 20.0f;
    
    // Communication
    CommunicationRange = 3000.0f;
    
    // Initialize vocal alerts
    VocalAlerts.Add(TEXT("Danger approaching from the north!"));
    VocalAlerts.Add(TEXT("Large predator spotted!"));
    VocalAlerts.Add(TEXT("Retreat to safe zone!"));
    VocalAlerts.Add(TEXT("Food source discovered!"));
    VocalAlerts.Add(TEXT("Water found nearby!"));
    
    // Initialize daily schedule based on role
    DailySchedule.Add(ENPC_TribalActivity::Idle);
    DailySchedule.Add(ENPC_TribalActivity::Gathering);
    DailySchedule.Add(ENPC_TribalActivity::Socializing);
    DailySchedule.Add(ENPC_TribalActivity::Sleeping);
    
    // Initialize internal timers
    LastActivityChange = 0.0f;
    LastNeedsUpdate = 0.0f;
    LastKnowledgeShare = 0.0f;
}

void UNPC_PrehistoricTribalAI::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalBehavior();
}

void UNPC_PrehistoricTribalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update needs every 5 seconds
    if (GetWorld()->GetTimeSeconds() - LastNeedsUpdate > 5.0f)
    {
        UpdateNeeds(DeltaTime);
        LastNeedsUpdate = GetWorld()->GetTimeSeconds();
    }
    
    // Process daily routine every 10 seconds
    if (GetWorld()->GetTimeSeconds() - LastActivityChange > 10.0f)
    {
        ProcessDailyRoutine();
        LastActivityChange = GetWorld()->GetTimeSeconds();
    }
    
    // Share knowledge with nearby tribal members every 30 seconds
    if (GetWorld()->GetTimeSeconds() - LastKnowledgeShare > 30.0f)
    {
        ProcessSocialInteractions();
        LastKnowledgeShare = GetWorld()->GetTimeSeconds();
    }
    
    // Update knowledge based on surroundings
    UpdateKnowledge();
    
    // Execute role-specific behavior
    ExecuteRoleSpecificBehavior();
}

void UNPC_PrehistoricTribalAI::SetTribalRole(ENPC_TribalRole NewRole)
{
    TribalRole = NewRole;
    
    // Update daily schedule based on new role
    DailySchedule.Empty();
    
    switch (NewRole)
    {
        case ENPC_TribalRole::Hunter:
            DailySchedule.Add(ENPC_TribalActivity::Hunting);
            DailySchedule.Add(ENPC_TribalActivity::Patrolling);
            DailySchedule.Add(ENPC_TribalActivity::Socializing);
            DailySchedule.Add(ENPC_TribalActivity::Sleeping);
            break;
            
        case ENPC_TribalRole::Gatherer:
            DailySchedule.Add(ENPC_TribalActivity::Gathering);
            DailySchedule.Add(ENPC_TribalActivity::Crafting);
            DailySchedule.Add(ENPC_TribalActivity::Socializing);
            DailySchedule.Add(ENPC_TribalActivity::Sleeping);
            break;
            
        case ENPC_TribalRole::Scout:
            DailySchedule.Add(ENPC_TribalActivity::Patrolling);
            DailySchedule.Add(ENPC_TribalActivity::AlertWatch);
            DailySchedule.Add(ENPC_TribalActivity::Socializing);
            DailySchedule.Add(ENPC_TribalActivity::Sleeping);
            break;
            
        case ENPC_TribalRole::Elder:
            DailySchedule.Add(ENPC_TribalActivity::Socializing);
            DailySchedule.Add(ENPC_TribalActivity::Crafting);
            DailySchedule.Add(ENPC_TribalActivity::Idle);
            DailySchedule.Add(ENPC_TribalActivity::Sleeping);
            break;
            
        case ENPC_TribalRole::Crafter:
            DailySchedule.Add(ENPC_TribalActivity::Crafting);
            DailySchedule.Add(ENPC_TribalActivity::Gathering);
            DailySchedule.Add(ENPC_TribalActivity::Socializing);
            DailySchedule.Add(ENPC_TribalActivity::Sleeping);
            break;
            
        case ENPC_TribalRole::Guardian:
            DailySchedule.Add(ENPC_TribalActivity::Patrolling);
            DailySchedule.Add(ENPC_TribalActivity::AlertWatch);
            DailySchedule.Add(ENPC_TribalActivity::Hunting);
            DailySchedule.Add(ENPC_TribalActivity::Sleeping);
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Tribal role changed to: %d"), (int32)NewRole);
}

void UNPC_PrehistoricTribalAI::UpdateNeeds(float DeltaTime)
{
    // Decrease needs over time
    Needs.Hunger = FMath::Max(0.0f, Needs.Hunger - (DeltaTime * 0.5f));
    Needs.Thirst = FMath::Max(0.0f, Needs.Thirst - (DeltaTime * 0.8f));
    Needs.Rest = FMath::Max(0.0f, Needs.Rest - (DeltaTime * 0.3f));
    
    // Safety decreases if in danger
    if (IsInDanger())
    {
        Needs.Safety = FMath::Max(0.0f, Needs.Safety - (DeltaTime * 2.0f));
    }
    else
    {
        Needs.Safety = FMath::Min(100.0f, Needs.Safety + (DeltaTime * 1.0f));
    }
    
    // Social need decreases when alone
    Needs.Social = FMath::Max(0.0f, Needs.Social - (DeltaTime * 0.2f));
    
    // Update activity based on critical needs
    UpdateActivityBasedOnNeeds();
}

void UNPC_PrehistoricTribalAI::ProcessDailyRoutine()
{
    if (IsInDanger())
    {
        CurrentActivity = ENPC_TribalActivity::Fleeing;
        return;
    }
    
    // Determine optimal activity based on needs and time of day
    ENPC_TribalActivity OptimalActivity = DetermineOptimalActivity();
    
    if (OptimalActivity != CurrentActivity)
    {
        CurrentActivity = OptimalActivity;
        ExecuteTribalActivity(CurrentActivity);
    }
}

void UNPC_PrehistoricTribalAI::RespondToDanger(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
        return;
    
    FVector ThreatLocation = ThreatActor->GetActorLocation();
    
    // Add to known danger zones
    AddDangerZone(ThreatLocation, ThreatLevel * 1000.0f);
    
    // Update safety need
    Needs.Safety = FMath::Max(0.0f, Needs.Safety - (ThreatLevel * 20.0f));
    
    // Broadcast alert to other tribal members
    FString AlertMessage = FString::Printf(TEXT("Danger level %.1f detected at location!"), ThreatLevel);
    BroadcastAlert(AlertMessage);
    
    // Change activity to fleeing if threat is severe
    if (ThreatLevel > 0.7f)
    {
        CurrentActivity = ENPC_TribalActivity::Fleeing;
    }
    else if (ThreatLevel > 0.4f)
    {
        CurrentActivity = ENPC_TribalActivity::AlertWatch;
    }
    
    Knowledge.LastDangerEncounter = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("Tribal member responding to danger level: %.2f"), ThreatLevel);
}

void UNPC_PrehistoricTribalAI::ShareKnowledge(UNPC_PrehistoricTribalAI* OtherTribalMember)
{
    if (!OtherTribalMember)
        return;
    
    // Share danger zones
    for (const FVector& DangerZone : Knowledge.KnownDangerZones)
    {
        OtherTribalMember->Knowledge.KnownDangerZones.AddUnique(DangerZone);
    }
    
    // Share resource locations
    for (const FVector& ResourceLocation : Knowledge.KnownResourceLocations)
    {
        OtherTribalMember->Knowledge.KnownResourceLocations.AddUnique(ResourceLocation);
    }
    
    // Share safe zones
    for (const FVector& SafeZone : Knowledge.SafeZones)
    {
        OtherTribalMember->Knowledge.SafeZones.AddUnique(SafeZone);
    }
    
    // Share species knowledge
    for (const auto& SpeciesInfo : Knowledge.SpeciesKnowledge)
    {
        float* ExistingKnowledge = OtherTribalMember->Knowledge.SpeciesKnowledge.Find(SpeciesInfo.Key);
        if (!ExistingKnowledge || *ExistingKnowledge < SpeciesInfo.Value)
        {
            OtherTribalMember->Knowledge.SpeciesKnowledge.Add(SpeciesInfo.Key, SpeciesInfo.Value);
        }
    }
    
    // Increase social need satisfaction
    Needs.Social = FMath::Min(100.0f, Needs.Social + 10.0f);
    OtherTribalMember->Needs.Social = FMath::Min(100.0f, OtherTribalMember->Needs.Social + 10.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Knowledge shared between tribal members"));
}

void UNPC_PrehistoricTribalAI::ExecuteTribalActivity(ENPC_TribalActivity Activity)
{
    switch (Activity)
    {
        case ENPC_TribalActivity::Hunting:
            // Look for prey animals
            Needs.Hunger = FMath::Min(100.0f, Needs.Hunger + 15.0f);
            Experience += 1.0f;
            break;
            
        case ENPC_TribalActivity::Gathering:
            // Search for plants and resources
            Needs.Hunger = FMath::Min(100.0f, Needs.Hunger + 10.0f);
            Needs.Thirst = FMath::Min(100.0f, Needs.Thirst + 5.0f);
            break;
            
        case ENPC_TribalActivity::Crafting:
            // Create tools and items
            Experience += 0.5f;
            break;
            
        case ENPC_TribalActivity::Socializing:
            // Interact with other tribal members
            Needs.Social = FMath::Min(100.0f, Needs.Social + 20.0f);
            break;
            
        case ENPC_TribalActivity::Sleeping:
            // Rest and recover
            Needs.Rest = FMath::Min(100.0f, Needs.Rest + 30.0f);
            break;
            
        case ENPC_TribalActivity::Fleeing:
            // Move to safe zone
            Needs.Safety = FMath::Min(100.0f, Needs.Safety + 5.0f);
            break;
            
        case ENPC_TribalActivity::AlertWatch:
            // Scan for threats
            Needs.Safety = FMath::Min(100.0f, Needs.Safety + 10.0f);
            break;
            
        case ENPC_TribalActivity::Patrolling:
            // Scout the area
            Experience += 0.3f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Executing tribal activity: %d"), (int32)Activity);
}

bool UNPC_PrehistoricTribalAI::IsInDanger() const
{
    // Check if recently encountered danger
    if (GetWorld()->GetTimeSeconds() - Knowledge.LastDangerEncounter < 60.0f)
    {
        return true;
    }
    
    // Check if in known danger zone
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    for (const FVector& DangerZone : Knowledge.KnownDangerZones)
    {
        if (FVector::Dist(CurrentLocation, DangerZone) < 1000.0f)
        {
            return true;
        }
    }
    
    return Needs.Safety < 30.0f;
}

FVector UNPC_PrehistoricTribalAI::FindNearestSafeZone() const
{
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector NearestSafeZone = CurrentLocation;
    float MinDistance = FLT_MAX;
    
    for (const FVector& SafeZone : Knowledge.SafeZones)
    {
        float Distance = FVector::Dist(CurrentLocation, SafeZone);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestSafeZone = SafeZone;
        }
    }
    
    return NearestSafeZone;
}

void UNPC_PrehistoricTribalAI::AddDangerZone(FVector Location, float Radius)
{
    Knowledge.KnownDangerZones.AddUnique(Location);
    
    // Limit the number of remembered danger zones
    if (Knowledge.KnownDangerZones.Num() > 20)
    {
        Knowledge.KnownDangerZones.RemoveAt(0);
    }
}

void UNPC_PrehistoricTribalAI::BroadcastAlert(const FString& AlertMessage)
{
    if (!GetWorld())
        return;
    
    // Find other tribal members within communication range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
            continue;
        
        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance <= CommunicationRange)
        {
            UNPC_PrehistoricTribalAI* OtherTribal = Actor->FindComponentByClass<UNPC_PrehistoricTribalAI>();
            if (OtherTribal)
            {
                // Share the alert information
                OtherTribal->Needs.Safety = FMath::Max(0.0f, OtherTribal->Needs.Safety - 10.0f);
                UE_LOG(LogTemp, Warning, TEXT("Alert broadcast to tribal member: %s"), *AlertMessage);
            }
        }
    }
}

void UNPC_PrehistoricTribalAI::InitializeTribalBehavior()
{
    // Add some initial safe zones
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    Knowledge.SafeZones.Add(OwnerLocation + FVector(0, 0, 500));
    Knowledge.SafeZones.Add(OwnerLocation + FVector(2000, 0, 0));
    Knowledge.SafeZones.Add(OwnerLocation + FVector(-2000, 0, 0));
    
    // Initialize species knowledge
    Knowledge.SpeciesKnowledge.Add(TEXT("T-Rex"), 0.9f); // Very dangerous
    Knowledge.SpeciesKnowledge.Add(TEXT("Raptor"), 0.8f); // Dangerous
    Knowledge.SpeciesKnowledge.Add(TEXT("Herbivore"), 0.1f); // Safe
    
    UE_LOG(LogTemp, Log, TEXT("Tribal behavior initialized for role: %d"), (int32)TribalRole);
}

void UNPC_PrehistoricTribalAI::UpdateActivityBasedOnNeeds()
{
    // Critical needs override daily routine
    if (Needs.Safety < 20.0f)
    {
        CurrentActivity = ENPC_TribalActivity::Fleeing;
    }
    else if (Needs.Hunger < 20.0f)
    {
        CurrentActivity = (TribalRole == ENPC_TribalRole::Hunter) ? 
            ENPC_TribalActivity::Hunting : ENPC_TribalActivity::Gathering;
    }
    else if (Needs.Thirst < 20.0f)
    {
        CurrentActivity = ENPC_TribalActivity::Gathering;
    }
    else if (Needs.Rest < 20.0f)
    {
        CurrentActivity = ENPC_TribalActivity::Sleeping;
    }
}

void UNPC_PrehistoricTribalAI::ProcessSocialInteractions()
{
    if (!GetWorld())
        return;
    
    // Find nearby tribal members for knowledge sharing
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
            continue;
        
        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance <= SocialDistance)
        {
            UNPC_PrehistoricTribalAI* OtherTribal = Actor->FindComponentByClass<UNPC_PrehistoricTribalAI>();
            if (OtherTribal)
            {
                ShareKnowledge(OtherTribal);
                break; // Share with one member at a time
            }
        }
    }
}

void UNPC_PrehistoricTribalAI::UpdateKnowledge()
{
    // This would normally involve perception system
    // For now, just update based on current location and time
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Randomly discover resource locations
    if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per tick
    {
        FVector ResourceLocation = CurrentLocation + FVector(
            FMath::RandRange(-1000.0f, 1000.0f),
            FMath::RandRange(-1000.0f, 1000.0f),
            0.0f
        );
        Knowledge.KnownResourceLocations.AddUnique(ResourceLocation);
    }
}

ENPC_TribalActivity UNPC_PrehistoricTribalAI::DetermineOptimalActivity()
{
    // Check critical needs first
    if (Needs.Safety < 30.0f)
        return ENPC_TribalActivity::Fleeing;
    
    if (Needs.Rest < 25.0f)
        return ENPC_TribalActivity::Sleeping;
    
    if (Needs.Hunger < 30.0f)
        return (TribalRole == ENPC_TribalRole::Hunter) ? 
            ENPC_TribalActivity::Hunting : ENPC_TribalActivity::Gathering;
    
    // Follow daily schedule based on role
    if (DailySchedule.Num() > 0)
    {
        int32 ScheduleIndex = FMath::RandRange(0, DailySchedule.Num() - 1);
        return DailySchedule[ScheduleIndex];
    }
    
    return ENPC_TribalActivity::Idle;
}

void UNPC_PrehistoricTribalAI::ExecuteRoleSpecificBehavior()
{
    switch (TribalRole)
    {
        case ENPC_TribalRole::Scout:
            // Scouts are more alert and detect danger earlier
            if (Needs.Safety < 50.0f && CurrentActivity != ENPC_TribalActivity::AlertWatch)
            {
                CurrentActivity = ENPC_TribalActivity::AlertWatch;
            }
            break;
            
        case ENPC_TribalRole::Guardian:
            // Guardians prioritize safety of the group
            if (Needs.Safety < 60.0f)
            {
                CurrentActivity = ENPC_TribalActivity::Patrolling;
            }
            break;
            
        case ENPC_TribalRole::Elder:
            // Elders share knowledge more frequently
            if (Needs.Social > 80.0f && FMath::RandRange(0.0f, 1.0f) < 0.1f)
            {
                ProcessSocialInteractions();
            }
            break;
    }
}