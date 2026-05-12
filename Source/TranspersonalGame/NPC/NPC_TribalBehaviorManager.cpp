#include "NPC_TribalBehaviorManager.h"
#include "NPC_TribalSocialSystem.h"
#include "NPC_MemorySystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ANPC_TribalBehaviorManager::ANPC_TribalBehaviorManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize components
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Initialize tribal properties
    TribeSize = 12;
    TribeLeadershipStyle = ENPC_LeadershipStyle::Elder;
    TribeAggressionLevel = 0.3f;
    TribeCohesion = 0.8f;
    TribeResourceLevel = 0.6f;
    TribeMorale = 0.7f;
    
    // Initialize settlement properties
    SettlementRadius = 2000.0f;
    DefensiveRadius = 3500.0f;
    HuntingRadius = 8000.0f;
    
    // Initialize behavior timers
    DailyRoutineTimer = 0.0f;
    DailyRoutineDuration = 86400.0f; // 24 hours in seconds
    
    // Initialize threat response
    ThreatLevel = ENPC_ThreatLevel::None;
    LastThreatTime = 0.0f;
    ThreatCooldownDuration = 300.0f; // 5 minutes
    
    // Initialize resource management
    FoodSupply = 100.0f;
    WaterSupply = 100.0f;
    ToolQuality = 0.5f;
    ShelterQuality = 0.6f;
}

void ANPC_TribalBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTribalStructure();
    SetupDailyRoutines();
    RegisterWithSocialSystem();
}

void ANPC_TribalBehaviorManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateDailyRoutines(DeltaTime);
    UpdateThreatAssessment(DeltaTime);
    UpdateResourceManagement(DeltaTime);
    UpdateTribalMorale(DeltaTime);
}

void ANPC_TribalBehaviorManager::InitializeTribalStructure()
{
    // Create tribal hierarchy
    TribeMembers.Empty();
    
    // Add leader
    FNPC_TribalMember Leader;
    Leader.MemberID = FGuid::NewGuid();
    Leader.Role = ENPC_TribalRole::Leader;
    Leader.Age = 45;
    Leader.Experience = 0.9f;
    Leader.Loyalty = 1.0f;
    Leader.Health = 100.0f;
    Leader.Name = TEXT("Chief Ironspear");
    TribeMembers.Add(Leader);
    
    // Add hunters
    for (int32 i = 0; i < 4; i++)
    {
        FNPC_TribalMember Hunter;
        Hunter.MemberID = FGuid::NewGuid();
        Hunter.Role = ENPC_TribalRole::Hunter;
        Hunter.Age = FMath::RandRange(25, 40);
        Hunter.Experience = FMath::RandRange(0.6f, 0.8f);
        Hunter.Loyalty = FMath::RandRange(0.7f, 0.9f);
        Hunter.Health = 100.0f;
        Hunter.Name = FString::Printf(TEXT("Hunter_%d"), i + 1);
        TribeMembers.Add(Hunter);
    }
    
    // Add gatherers
    for (int32 i = 0; i < 3; i++)
    {
        FNPC_TribalMember Gatherer;
        Gatherer.MemberID = FGuid::NewGuid();
        Gatherer.Role = ENPC_TribalRole::Gatherer;
        Gatherer.Age = FMath::RandRange(20, 35);
        Gatherer.Experience = FMath::RandRange(0.5f, 0.7f);
        Gatherer.Loyalty = FMath::RandRange(0.6f, 0.8f);
        Gatherer.Health = 100.0f;
        Gatherer.Name = FString::Printf(TEXT("Gatherer_%d"), i + 1);
        TribeMembers.Add(Gatherer);
    }
    
    // Add scouts
    for (int32 i = 0; i < 2; i++)
    {
        FNPC_TribalMember Scout;
        Scout.MemberID = FGuid::NewGuid();
        Scout.Role = ENPC_TribalRole::Scout;
        Scout.Age = FMath::RandRange(18, 30);
        Scout.Experience = FMath::RandRange(0.4f, 0.6f);
        Scout.Loyalty = FMath::RandRange(0.7f, 0.9f);
        Scout.Health = 100.0f;
        Scout.Name = FString::Printf(TEXT("Scout_%d"), i + 1);
        TribeMembers.Add(Scout);
    }
    
    // Add elders
    for (int32 i = 0; i < 2; i++)
    {
        FNPC_TribalMember Elder;
        Elder.MemberID = FGuid::NewGuid();
        Elder.Role = ENPC_TribalRole::Elder;
        Elder.Age = FMath::RandRange(50, 70);
        Elder.Experience = 1.0f;
        Elder.Loyalty = 1.0f;
        Elder.Health = 80.0f;
        Elder.Name = FString::Printf(TEXT("Elder_%d"), i + 1);
        TribeMembers.Add(Elder);
    }
    
    // Add children
    FNPC_TribalMember Child;
    Child.MemberID = FGuid::NewGuid();
    Child.Role = ENPC_TribalRole::Child;
    Child.Age = FMath::RandRange(8, 16);
    Child.Experience = 0.1f;
    Child.Loyalty = 0.9f;
    Child.Health = 90.0f;
    Child.Name = TEXT("Young_One");
    TribeMembers.Add(Child);
    
    UE_LOG(LogTemp, Warning, TEXT("Tribal structure initialized with %d members"), TribeMembers.Num());
}

void ANPC_TribalBehaviorManager::SetupDailyRoutines()
{
    DailyActivities.Empty();
    
    // Dawn activities (5-7 AM)
    FNPC_DailyActivity DawnActivity;
    DawnActivity.ActivityType = ENPC_ActivityType::WakeUp;
    DawnActivity.StartHour = 5.0f;
    DawnActivity.Duration = 2.0f;
    DawnActivity.Priority = 0.9f;
    DawnActivity.RequiredRoles.Add(ENPC_TribalRole::Leader);
    DailyActivities.Add(DawnActivity);
    
    // Morning hunt preparation (7-9 AM)
    FNPC_DailyActivity HuntPrepActivity;
    HuntPrepActivity.ActivityType = ENPC_ActivityType::HuntingPreparation;
    HuntPrepActivity.StartHour = 7.0f;
    HuntPrepActivity.Duration = 2.0f;
    HuntPrepActivity.Priority = 0.8f;
    HuntPrepActivity.RequiredRoles.Add(ENPC_TribalRole::Hunter);
    DailyActivities.Add(HuntPrepActivity);
    
    // Hunting expedition (9 AM - 3 PM)
    FNPC_DailyActivity HuntActivity;
    HuntActivity.ActivityType = ENPC_ActivityType::Hunting;
    HuntActivity.StartHour = 9.0f;
    HuntActivity.Duration = 6.0f;
    HuntActivity.Priority = 1.0f;
    HuntActivity.RequiredRoles.Add(ENPC_TribalRole::Hunter);
    HuntActivity.RequiredRoles.Add(ENPC_TribalRole::Scout);
    DailyActivities.Add(HuntActivity);
    
    // Gathering (8 AM - 2 PM)
    FNPC_DailyActivity GatherActivity;
    GatherActivity.ActivityType = ENPC_ActivityType::Gathering;
    GatherActivity.StartHour = 8.0f;
    GatherActivity.Duration = 6.0f;
    GatherActivity.Priority = 0.7f;
    GatherActivity.RequiredRoles.Add(ENPC_TribalRole::Gatherer);
    DailyActivities.Add(GatherActivity);
    
    // Evening meal (6-8 PM)
    FNPC_DailyActivity MealActivity;
    MealActivity.ActivityType = ENPC_ActivityType::CommunalMeal;
    MealActivity.StartHour = 18.0f;
    MealActivity.Duration = 2.0f;
    MealActivity.Priority = 0.9f;
    DailyActivities.Add(MealActivity);
    
    // Night watch (10 PM - 5 AM)
    FNPC_DailyActivity WatchActivity;
    WatchActivity.ActivityType = ENPC_ActivityType::NightWatch;
    WatchActivity.StartHour = 22.0f;
    WatchActivity.Duration = 7.0f;
    WatchActivity.Priority = 0.8f;
    WatchActivity.RequiredRoles.Add(ENPC_TribalRole::Scout);
    DailyActivities.Add(WatchActivity);
}

void ANPC_TribalBehaviorManager::UpdateDailyRoutines(float DeltaTime)
{
    DailyRoutineTimer += DeltaTime;
    
    // Get current hour (0-24)
    float CurrentHour = FMath::Fmod(DailyRoutineTimer / 3600.0f, 24.0f);
    
    // Check which activities should be active
    for (const FNPC_DailyActivity& Activity : DailyActivities)
    {
        bool bShouldBeActive = false;
        
        if (Activity.StartHour + Activity.Duration <= 24.0f)
        {
            // Activity doesn't cross midnight
            bShouldBeActive = (CurrentHour >= Activity.StartHour && 
                             CurrentHour < Activity.StartHour + Activity.Duration);
        }
        else
        {
            // Activity crosses midnight
            bShouldBeActive = (CurrentHour >= Activity.StartHour || 
                             CurrentHour < FMath::Fmod(Activity.StartHour + Activity.Duration, 24.0f));
        }
        
        if (bShouldBeActive)
        {
            ExecuteTribalActivity(Activity);
        }
    }
}

void ANPC_TribalBehaviorManager::UpdateThreatAssessment(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay threat level over time
    if (CurrentTime - LastThreatTime > ThreatCooldownDuration)
    {
        if (ThreatLevel != ENPC_ThreatLevel::None)
        {
            // Gradually reduce threat level
            int32 CurrentThreatInt = static_cast<int32>(ThreatLevel);
            if (CurrentThreatInt > 0)
            {
                ThreatLevel = static_cast<ENPC_ThreatLevel>(CurrentThreatInt - 1);
                LastThreatTime = CurrentTime;
            }
        }
    }
    
    // Scan for nearby threats
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::class, NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            
            // Check if it's a dinosaur threat
            if (Actor->GetName().Contains(TEXT("Raptor")) && Distance < DefensiveRadius)
            {
                RaiseThreatLevel(ENPC_ThreatLevel::High);
                break;
            }
            else if (Actor->GetName().Contains(TEXT("TRex")) && Distance < DefensiveRadius * 1.5f)
            {
                RaiseThreatLevel(ENPC_ThreatLevel::Critical);
                break;
            }
        }
    }
}

void ANPC_TribalBehaviorManager::RaiseThreatLevel(ENPC_ThreatLevel NewThreatLevel)
{
    if (NewThreatLevel > ThreatLevel)
    {
        ThreatLevel = NewThreatLevel;
        LastThreatTime = GetWorld()->GetTimeSeconds();
        
        // Trigger appropriate response
        switch (ThreatLevel)
        {
        case ENPC_ThreatLevel::Low:
            // Increase alertness
            break;
        case ENPC_ThreatLevel::Medium:
            // Send scouts to investigate
            break;
        case ENPC_ThreatLevel::High:
            // Defensive positions
            OrganizeDefensiveResponse();
            break;
        case ENPC_ThreatLevel::Critical:
            // Emergency evacuation protocols
            InitiateEmergencyProtocols();
            break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Tribe threat level raised to: %d"), static_cast<int32>(ThreatLevel));
    }
}

void ANPC_TribalBehaviorManager::OrganizeDefensiveResponse()
{
    // Gather all hunters and scouts for defense
    for (FNPC_TribalMember& Member : TribeMembers)
    {
        if (Member.Role == ENPC_TribalRole::Hunter || Member.Role == ENPC_TribalRole::Scout)
        {
            Member.CurrentActivity = ENPC_ActivityType::Defense;
        }
        else if (Member.Role == ENPC_TribalRole::Child || Member.Role == ENPC_TribalRole::Elder)
        {
            Member.CurrentActivity = ENPC_ActivityType::Shelter;
        }
    }
    
    // Reduce morale due to threat
    TribeMorale = FMath::Max(0.0f, TribeMorale - 0.2f);
}

void ANPC_TribalBehaviorManager::InitiateEmergencyProtocols()
{
    // All non-essential members to shelter
    for (FNPC_TribalMember& Member : TribeMembers)
    {
        if (Member.Role != ENPC_TribalRole::Hunter && Member.Role != ENPC_TribalRole::Leader)
        {
            Member.CurrentActivity = ENPC_ActivityType::Shelter;
        }
        else
        {
            Member.CurrentActivity = ENPC_ActivityType::Defense;
        }
    }
    
    // Significant morale impact
    TribeMorale = FMath::Max(0.0f, TribeMorale - 0.4f);
}

void ANPC_TribalBehaviorManager::UpdateResourceManagement(float DeltaTime)
{
    // Consume resources over time
    float ConsumptionRate = TribeSize * 0.1f * DeltaTime;
    
    FoodSupply = FMath::Max(0.0f, FoodSupply - ConsumptionRate);
    WaterSupply = FMath::Max(0.0f, WaterSupply - ConsumptionRate * 0.5f);
    
    // Resource gathering based on active gatherers
    int32 ActiveGatherers = 0;
    int32 ActiveHunters = 0;
    
    for (const FNPC_TribalMember& Member : TribeMembers)
    {
        if (Member.CurrentActivity == ENPC_ActivityType::Gathering)
        {
            ActiveGatherers++;
        }
        else if (Member.CurrentActivity == ENPC_ActivityType::Hunting)
        {
            ActiveHunters++;
        }
    }
    
    // Gathering increases food and water
    if (ActiveGatherers > 0)
    {
        float GatheringRate = ActiveGatherers * 0.5f * DeltaTime;
        FoodSupply = FMath::Min(100.0f, FoodSupply + GatheringRate);
        WaterSupply = FMath::Min(100.0f, WaterSupply + GatheringRate * 0.3f);
    }
    
    // Hunting increases food significantly
    if (ActiveHunters > 0)
    {
        float HuntingRate = ActiveHunters * 1.0f * DeltaTime;
        FoodSupply = FMath::Min(100.0f, FoodSupply + HuntingRate);
    }
    
    // Update resource level for decision making
    TribeResourceLevel = (FoodSupply + WaterSupply) / 200.0f;
}

void ANPC_TribalBehaviorManager::UpdateTribalMorale(float DeltaTime)
{
    float MoraleChange = 0.0f;
    
    // Resource levels affect morale
    if (TribeResourceLevel > 0.8f)
    {
        MoraleChange += 0.1f * DeltaTime;
    }
    else if (TribeResourceLevel < 0.3f)
    {
        MoraleChange -= 0.2f * DeltaTime;
    }
    
    // Threat level affects morale
    switch (ThreatLevel)
    {
    case ENPC_ThreatLevel::None:
        MoraleChange += 0.05f * DeltaTime;
        break;
    case ENPC_ThreatLevel::Low:
        // No change
        break;
    case ENPC_ThreatLevel::Medium:
        MoraleChange -= 0.1f * DeltaTime;
        break;
    case ENPC_ThreatLevel::High:
        MoraleChange -= 0.2f * DeltaTime;
        break;
    case ENPC_ThreatLevel::Critical:
        MoraleChange -= 0.3f * DeltaTime;
        break;
    }
    
    // Apply morale change
    TribeMorale = FMath::Clamp(TribeMorale + MoraleChange, 0.0f, 1.0f);
    
    // Morale affects cohesion
    TribeCohesion = FMath::Lerp(TribeCohesion, TribeMorale, 0.1f * DeltaTime);
}

void ANPC_TribalBehaviorManager::ExecuteTribalActivity(const FNPC_DailyActivity& Activity)
{
    // Assign appropriate tribe members to this activity
    for (FNPC_TribalMember& Member : TribeMembers)
    {
        // Skip if member is in emergency mode
        if (ThreatLevel >= ENPC_ThreatLevel::High && 
            (Member.Role == ENPC_TribalRole::Child || Member.Role == ENPC_TribalRole::Elder))
        {
            continue;
        }
        
        // Check if member's role is required for this activity
        if (Activity.RequiredRoles.Num() == 0 || Activity.RequiredRoles.Contains(Member.Role))
        {
            Member.CurrentActivity = Activity.ActivityType;
        }
    }
}

void ANPC_TribalBehaviorManager::RegisterWithSocialSystem()
{
    // Find and register with the social system
    TArray<AActor*> SocialSystems;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPC_TribalSocialSystem::StaticClass(), SocialSystems);
    
    if (SocialSystems.Num() > 0)
    {
        ANPC_TribalSocialSystem* SocialSystem = Cast<ANPC_TribalSocialSystem>(SocialSystems[0]);
        if (SocialSystem)
        {
            // Register this tribe with the social system
            UE_LOG(LogTemp, Warning, TEXT("Tribal behavior manager registered with social system"));
        }
    }
}

FNPC_TribalMember* ANPC_TribalBehaviorManager::GetTribalLeader()
{
    for (FNPC_TribalMember& Member : TribeMembers)
    {
        if (Member.Role == ENPC_TribalRole::Leader)
        {
            return &Member;
        }
    }
    return nullptr;
}

TArray<FNPC_TribalMember> ANPC_TribalBehaviorManager::GetMembersByRole(ENPC_TribalRole Role)
{
    TArray<FNPC_TribalMember> RoleMembers;
    
    for (const FNPC_TribalMember& Member : TribeMembers)
    {
        if (Member.Role == Role)
        {
            RoleMembers.Add(Member);
        }
    }
    
    return RoleMembers;
}

float ANPC_TribalBehaviorManager::GetTribalEffectiveness()
{
    return (TribeCohesion + TribeMorale + TribeResourceLevel) / 3.0f;
}