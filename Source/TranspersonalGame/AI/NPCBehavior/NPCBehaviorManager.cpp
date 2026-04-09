#include "NPCBehaviorManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NPCBehaviorAgent.h"
#include "Components/NPCMemorySystem.h"

ANPCBehaviorManager::ANPCBehaviorManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second for performance
    
    // Initialize default values
    GlobalTimeScale = 1.0f;
    MaxActiveNPCs = 50;
    NPCUpdateRadius = 2000.0f;
    
    // Initialize ecosystem parameters
    EcosystemBalance.HerbivorePopulation = 0;
    EcosystemBalance.CarnivorePopulation = 0;
    EcosystemBalance.FoodAvailability = 1.0f;
    EcosystemBalance.WaterAvailability = 1.0f;
    EcosystemBalance.TerritorialPressure = 0.0f;
}

void ANPCBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all NPCs in the world
    RefreshNPCList();
    
    // Initialize ecosystem monitoring
    InitializeEcosystemMonitoring();
    
    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior Manager initialized with %d NPCs"), RegisteredNPCs.Num());
}

void ANPCBehaviorManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update ecosystem balance
    UpdateEcosystemBalance(DeltaTime);
    
    // Manage NPC activation based on player proximity
    ManageNPCActivation();
    
    // Process global events
    ProcessGlobalEvents(DeltaTime);
    
    // Update social hierarchies
    UpdateSocialHierarchies(DeltaTime);
    
    // Manage resource distribution
    ManageResourceDistribution();
}

void ANPCBehaviorManager::RefreshNPCList()
{
    RegisteredNPCs.Empty();
    
    TArray<AActor*> FoundNPCs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPCBehaviorAgent::StaticClass(), FoundNPCs);
    
    for (AActor* Actor : FoundNPCs)
    {
        if (ANPCBehaviorAgent* NPC = Cast<ANPCBehaviorAgent>(Actor))
        {
            RegisterNPC(NPC);
        }
    }
}

void ANPCBehaviorManager::RegisterNPC(ANPCBehaviorAgent* NPC)
{
    if (!NPC || RegisteredNPCs.Contains(NPC))
        return;
        
    RegisteredNPCs.Add(NPC);
    
    // Update ecosystem counts
    if (NPC->IsHerbivore())
    {
        EcosystemBalance.HerbivorePopulation++;
    }
    else
    {
        EcosystemBalance.CarnivorePopulation++;
    }
    
    // Assign to appropriate groups
    AssignNPCToGroups(NPC);
    
    UE_LOG(LogTemp, Log, TEXT("Registered NPC: %s (%s)"), 
           *NPC->GetName(), 
           *UEnum::GetValueAsString(NPC->GetSpecies()));
}

void ANPCBehaviorManager::UnregisterNPC(ANPCBehaviorAgent* NPC)
{
    if (!NPC || !RegisteredNPCs.Contains(NPC))
        return;
        
    RegisteredNPCs.Remove(NPC);
    
    // Update ecosystem counts
    if (NPC->IsHerbivore())
    {
        EcosystemBalance.HerbivorePopulation--;
    }
    else
    {
        EcosystemBalance.CarnivorePopulation--;
    }
    
    // Remove from groups
    RemoveNPCFromGroups(NPC);
}

void ANPCBehaviorManager::AssignNPCToGroups(ANPCBehaviorAgent* NPC)
{
    if (!NPC)
        return;
        
    EDinosaurSpecies Species = NPC->GetSpecies();
    
    // Find or create species group
    FNPCGroup* SpeciesGroup = nullptr;
    for (FNPCGroup& Group : NPCGroups)
    {
        if (Group.GroupType == ENPCGroupType::Species && Group.Species == Species)
        {
            SpeciesGroup = &Group;
            break;
        }
    }
    
    if (!SpeciesGroup)
    {
        // Create new species group
        FNPCGroup NewGroup;
        NewGroup.GroupType = ENPCGroupType::Species;
        NewGroup.Species = Species;
        NewGroup.GroupName = FString::Printf(TEXT("%s Pack"), *UEnum::GetValueAsString(Species));
        NewGroup.MaxMembers = GetMaxGroupSizeForSpecies(Species);
        NPCGroups.Add(NewGroup);
        SpeciesGroup = &NPCGroups.Last();
    }
    
    // Add to group if space available
    if (SpeciesGroup->Members.Num() < SpeciesGroup->MaxMembers)
    {
        SpeciesGroup->Members.Add(NPC);
        NPC->SetGroupID(SpeciesGroup->GroupID);
    }
    
    // Also assign to territorial groups based on location
    AssignToTerritorialGroup(NPC);
}

void ANPCBehaviorManager::AssignToTerritorialGroup(ANPCBehaviorAgent* NPC)
{
    if (!NPC)
        return;
        
    FVector NPCLocation = NPC->GetActorLocation();
    
    // Find nearby territorial group
    FNPCGroup* TerritorialGroup = nullptr;
    for (FNPCGroup& Group : NPCGroups)
    {
        if (Group.GroupType == ENPCGroupType::Territorial)
        {
            float Distance = FVector::Dist(NPCLocation, Group.CenterLocation);
            if (Distance < Group.TerritoryRadius)
            {
                TerritorialGroup = &Group;
                break;
            }
        }
    }
    
    if (!TerritorialGroup)
    {
        // Create new territorial group
        FNPCGroup NewGroup;
        NewGroup.GroupType = ENPCGroupType::Territorial;
        NewGroup.GroupName = FString::Printf(TEXT("Territory_%d"), NPCGroups.Num());
        NewGroup.CenterLocation = NPCLocation;
        NewGroup.TerritoryRadius = 1500.0f; // 15 meter radius
        NewGroup.MaxMembers = 10;
        NPCGroups.Add(NewGroup);
        TerritorialGroup = &NPCGroups.Last();
    }
    
    if (TerritorialGroup->Members.Num() < TerritorialGroup->MaxMembers)
    {
        TerritorialGroup->Members.Add(NPC);
    }
}

void ANPCBehaviorManager::RemoveNPCFromGroups(ANPCBehaviorAgent* NPC)
{
    for (FNPCGroup& Group : NPCGroups)
    {
        Group.Members.Remove(NPC);
    }
    
    // Clean up empty groups
    NPCGroups.RemoveAll([](const FNPCGroup& Group) {
        return Group.Members.Num() == 0;
    });
}

int32 ANPCBehaviorManager::GetMaxGroupSizeForSpecies(EDinosaurSpecies Species) const
{
    switch (Species)
    {
        case EDinosaurSpecies::Velociraptor:
            return 6; // Pack hunters
        case EDinosaurSpecies::Compsognathus:
            return 12; // Small flocking species
        case EDinosaurSpecies::Parasaurolophus:
            return 8; // Herd animals
        case EDinosaurSpecies::Triceratops:
            return 4; // Small family groups
        case EDinosaurSpecies::TyrannosaurusRex:
            return 2; // Mostly solitary, occasional pairs
        default:
            return 3; // Default small group
    }
}

void ANPCBehaviorManager::UpdateEcosystemBalance(float DeltaTime)
{
    // Calculate food availability based on herbivore population
    float IdealHerbivoreRatio = 0.7f; // 70% herbivores
    float CurrentHerbivoreRatio = RegisteredNPCs.Num() > 0 ? 
        (float)EcosystemBalance.HerbivorePopulation / RegisteredNPCs.Num() : 0.0f;
    
    // Adjust food availability
    if (CurrentHerbivoreRatio > IdealHerbivoreRatio)
    {
        EcosystemBalance.FoodAvailability = FMath::Clamp(EcosystemBalance.FoodAvailability - (0.1f * DeltaTime), 0.1f, 1.0f);
    }
    else
    {
        EcosystemBalance.FoodAvailability = FMath::Clamp(EcosystemBalance.FoodAvailability + (0.05f * DeltaTime), 0.1f, 1.0f);
    }
    
    // Calculate territorial pressure
    float NPCDensity = RegisteredNPCs.Num() / 10000.0f; // NPCs per 100x100 meter area
    EcosystemBalance.TerritorialPressure = FMath::Clamp(NPCDensity, 0.0f, 1.0f);
    
    // Water availability (could be affected by season, weather, etc.)
    EcosystemBalance.WaterAvailability = FMath::Clamp(EcosystemBalance.WaterAvailability, 0.5f, 1.0f);
}

void ANPCBehaviorManager::ManageNPCActivation()
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
        return;
        
    FVector PlayerLocation = Player->GetActorLocation();
    
    // Sort NPCs by distance to player
    RegisteredNPCs.Sort([PlayerLocation](const ANPCBehaviorAgent& A, const ANPCBehaviorAgent& B) {
        float DistA = FVector::DistSquared(PlayerLocation, A.GetActorLocation());
        float DistB = FVector::DistSquared(PlayerLocation, B.GetActorLocation());
        return DistA < DistB;
    });
    
    // Activate closest NPCs, deactivate distant ones
    int32 ActiveCount = 0;
    for (ANPCBehaviorAgent* NPC : RegisteredNPCs)
    {
        if (!NPC)
            continue;
            
        float Distance = FVector::Dist(PlayerLocation, NPC->GetActorLocation());
        
        if (Distance < NPCUpdateRadius && ActiveCount < MaxActiveNPCs)
        {
            NPC->SetActorTickEnabled(true);
            ActiveCount++;
        }
        else
        {
            NPC->SetActorTickEnabled(false);
        }
    }
}

void ANPCBehaviorManager::ProcessGlobalEvents(float DeltaTime)
{
    // Process active global events
    for (int32 i = ActiveGlobalEvents.Num() - 1; i >= 0; i--)
    {
        FGlobalNPCEvent& Event = ActiveGlobalEvents[i];
        Event.Duration -= DeltaTime;
        
        // Apply event effects
        ApplyGlobalEventEffects(Event, DeltaTime);
        
        // Remove expired events
        if (Event.Duration <= 0.0f)
        {
            OnGlobalEventEnded(Event);
            ActiveGlobalEvents.RemoveAt(i);
        }
    }
    
    // Randomly trigger new events
    if (FMath::RandRange(0.0f, 1.0f) < 0.001f) // 0.1% chance per second
    {
        TriggerRandomGlobalEvent();
    }
}

void ANPCBehaviorManager::TriggerRandomGlobalEvent()
{
    TArray<EGlobalEventType> PossibleEvents = {
        EGlobalEventType::FoodScarcity,
        EGlobalEventType::WaterScarcity,
        EGlobalEventType::TerritorialDispute,
        EGlobalEventType::MigrationUrge,
        EGlobalEventType::PredatorAlert
    };
    
    EGlobalEventType EventType = PossibleEvents[FMath::RandRange(0, PossibleEvents.Num() - 1)];
    
    FGlobalNPCEvent NewEvent;
    NewEvent.EventType = EventType;
    NewEvent.Duration = FMath::RandRange(300.0f, 1800.0f); // 5-30 minutes
    NewEvent.Intensity = FMath::RandRange(0.3f, 1.0f);
    
    switch (EventType)
    {
        case EGlobalEventType::FoodScarcity:
            NewEvent.EventName = TEXT("Food Scarcity");
            NewEvent.Description = TEXT("Food sources are becoming scarce, increasing competition");
            break;
        case EGlobalEventType::WaterScarcity:
            NewEvent.EventName = TEXT("Drought");
            NewEvent.Description = TEXT("Water sources are drying up");
            break;
        case EGlobalEventType::TerritorialDispute:
            NewEvent.EventName = TEXT("Territorial Conflict");
            NewEvent.Description = TEXT("Increased territorial aggression among species");
            break;
        case EGlobalEventType::MigrationUrge:
            NewEvent.EventName = TEXT("Migration Season");
            NewEvent.Description = TEXT("Dinosaurs feel the urge to migrate to new territories");
            break;
        case EGlobalEventType::PredatorAlert:
            NewEvent.EventName = TEXT("Apex Predator Alert");
            NewEvent.Description = TEXT("A major predator is active in the area");
            break;
    }
    
    ActiveGlobalEvents.Add(NewEvent);
    OnGlobalEventStarted(NewEvent);
    
    UE_LOG(LogTemp, Warning, TEXT("Global Event Started: %s"), *NewEvent.EventName);
}

void ANPCBehaviorManager::ApplyGlobalEventEffects(const FGlobalNPCEvent& Event, float DeltaTime)
{
    for (ANPCBehaviorAgent* NPC : RegisteredNPCs)
    {
        if (!NPC)
            continue;
            
        switch (Event.EventType)
        {
            case EGlobalEventType::FoodScarcity:
                // Increase hunger rate
                NPC->ModifyNeedDecayRate(TEXT("Hunger"), 1.0f + Event.Intensity);
                break;
                
            case EGlobalEventType::WaterScarcity:
                // Increase thirst rate
                NPC->ModifyNeedDecayRate(TEXT("Thirst"), 1.0f + Event.Intensity);
                break;
                
            case EGlobalEventType::TerritorialDispute:
                // Increase aggression
                if (NPC->GetPersonalityComponent())
                {
                    FDinosaurPersonality Personality = NPC->GetPersonalityComponent()->GetPersonality();
                    Personality.Aggressiveness = FMath::Clamp(Personality.Aggressiveness + (Event.Intensity * 0.2f), 0.0f, 1.0f);
                    NPC->GetPersonalityComponent()->SetPersonality(Personality);
                }
                break;
                
            case EGlobalEventType::MigrationUrge:
                // Increase movement and exploration
                NPC->SetBehaviorState(EDinosaurBehaviorState::Patrolling);
                break;
                
            case EGlobalEventType::PredatorAlert:
                // Increase fear and alertness
                if (NPC->IsHerbivore())
                {
                    NPC->SetBehaviorState(EDinosaurBehaviorState::Investigating);
                }
                break;
        }
    }
}

void ANPCBehaviorManager::UpdateSocialHierarchies(float DeltaTime)
{
    for (FNPCGroup& Group : NPCGroups)
    {
        if (Group.Members.Num() < 2)
            continue;
            
        // Update group hierarchy based on personality traits
        Group.Members.Sort([](const ANPCBehaviorAgent& A, const ANPCBehaviorAgent& B) {
            if (!A.GetPersonalityComponent() || !B.GetPersonalityComponent())
                return false;
                
            FDinosaurPersonality PersonalityA = A.GetPersonalityComponent()->GetPersonality();
            FDinosaurPersonality PersonalityB = B.GetPersonalityComponent()->GetPersonality();
            
            // Dominance based on aggression + intelligence - fearfulness
            float DominanceA = PersonalityA.Aggressiveness + PersonalityA.Intelligence - PersonalityA.Fearfulness;
            float DominanceB = PersonalityB.Aggressiveness + PersonalityB.Intelligence - PersonalityB.Fearfulness;
            
            return DominanceA > DominanceB;
        });
        
        // Assign hierarchy positions
        for (int32 i = 0; i < Group.Members.Num(); i++)
        {
            if (Group.Members[i])
            {
                Group.Members[i]->SetHierarchyPosition(i);
            }
        }
    }
}

void ANPCBehaviorManager::ManageResourceDistribution()
{
    // Distribute food and water availability information to NPCs
    for (ANPCBehaviorAgent* NPC : RegisteredNPCs)
    {
        if (!NPC)
            continue;
            
        // Update NPC's knowledge of resource availability
        if (UNPCMemoryComponent* Memory = NPC->GetMemoryComponent())
        {
            Memory->UpdateResourceAvailability(EcosystemBalance.FoodAvailability, EcosystemBalance.WaterAvailability);
        }
    }
}

void ANPCBehaviorManager::OnGlobalEventStarted(const FGlobalNPCEvent& Event)
{
    // Broadcast to all interested systems
    OnGlobalEventStartedDelegate.Broadcast(Event);
}

void ANPCBehaviorManager::OnGlobalEventEnded(const FGlobalNPCEvent& Event)
{
    // Reset any temporary modifications
    for (ANPCBehaviorAgent* NPC : RegisteredNPCs)
    {
        if (!NPC)
            continue;
            
        // Reset need decay rates
        NPC->ResetNeedDecayRates();
        
        // Reset personality modifications
        if (Event.EventType == EGlobalEventType::TerritorialDispute)
        {
            NPC->ResetPersonalityToBase();
        }
    }
    
    OnGlobalEventEndedDelegate.Broadcast(Event);
    
    UE_LOG(LogTemp, Log, TEXT("Global Event Ended: %s"), *Event.EventName);
}

TArray<ANPCBehaviorAgent*> ANPCBehaviorManager::GetNPCsInRadius(FVector Center, float Radius) const
{
    TArray<ANPCBehaviorAgent*> Result;
    
    for (ANPCBehaviorAgent* NPC : RegisteredNPCs)
    {
        if (!NPC)
            continue;
            
        float Distance = FVector::Dist(Center, NPC->GetActorLocation());
        if (Distance <= Radius)
        {
            Result.Add(NPC);
        }
    }
    
    return Result;
}

TArray<ANPCBehaviorAgent*> ANPCBehaviorManager::GetNPCsBySpecies(EDinosaurSpecies Species) const
{
    TArray<ANPCBehaviorAgent*> Result;
    
    for (ANPCBehaviorAgent* NPC : RegisteredNPCs)
    {
        if (NPC && NPC->GetSpecies() == Species)
        {
            Result.Add(NPC);
        }
    }
    
    return Result;
}

FNPCGroup* ANPCBehaviorManager::GetGroupByID(int32 GroupID)
{
    for (FNPCGroup& Group : NPCGroups)
    {
        if (Group.GroupID == GroupID)
        {
            return &Group;
        }
    }
    return nullptr;
}

void ANPCBehaviorManager::InitializeEcosystemMonitoring()
{
    // Set up initial ecosystem parameters
    EcosystemBalance.FoodAvailability = 1.0f;
    EcosystemBalance.WaterAvailability = 1.0f;
    EcosystemBalance.TerritorialPressure = 0.0f;
    
    // Count initial populations
    EcosystemBalance.HerbivorePopulation = 0;
    EcosystemBalance.CarnivorePopulation = 0;
    
    for (ANPCBehaviorAgent* NPC : RegisteredNPCs)
    {
        if (!NPC)
            continue;
            
        if (NPC->IsHerbivore())
        {
            EcosystemBalance.HerbivorePopulation++;
        }
        else
        {
            EcosystemBalance.CarnivorePopulation++;
        }
    }
}