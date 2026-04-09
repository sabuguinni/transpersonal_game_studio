#include "NPCBehaviorAgent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnrealMathUtility.h"

ANPCBehaviorAgent::ANPCBehaviorAgent()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS para performance

    // Initialize components
    BehaviorComponent = CreateDefaultSubobject<UNPCBehaviorComponent>(TEXT("BehaviorComponent"));
    MemoryComponent = CreateDefaultSubobject<UNPCMemoryComponent>(TEXT("MemoryComponent"));
    PersonalityComponent = CreateDefaultSubobject<UDinosaurPersonalityComponent>(TEXT("PersonalityComponent"));
    
    // Setup AI Controller class
    AIControllerClass = ADinosaurAIController::StaticClass();
    
    // Initialize basic properties
    Species = EDinosaurSpecies::Compsognathus;
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    DomesticationLevel = EDomesticationLevel::Wild;
    
    // Initialize needs
    CurrentNeeds.Hunger = 0.3f;
    CurrentNeeds.Thirst = 0.2f;
    CurrentNeeds.Energy = 0.8f;
    CurrentNeeds.Social = 0.5f;
    CurrentNeeds.Safety = 0.6f;
    
    // Setup collision
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    
    // Initialize movement
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void ANPCBehaviorAgent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize unique personality
    InitializePersonality();
    
    // Setup territory around spawn location
    if (TerritoryCenter.IsZero())
    {
        TerritoryCenter = GetActorLocation();
    }
    
    // Initialize daily routines
    SetupDailyRoutines();
    
    // Setup AI Controller
    if (ADinosaurAIController* DinoController = Cast<ADinosaurAIController>(GetController()))
    {
        DinoController->InitializeBehaviorTree();
    }
    
    // Register with NPC Manager
    RegisterWithNPCManager();
    
    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior Agent initialized: %s (%s)"), 
           *GetName(), *UEnum::GetValueAsString(Species));
}

void ANPCBehaviorAgent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update needs
    UpdateNeeds(DeltaTime);
    
    // Process daily routine
    ProcessDailyRoutine();
    
    // Update behavior based on current needs
    UpdateBehaviorBasedOnNeeds();
    
    // Process social interactions
    ProcessSocialInteractions(DeltaTime);
    
    // Update memory system
    if (MemoryComponent)
    {
        MemoryComponent->UpdateMemory(DeltaTime);
    }
    
    // Update domestication progress
    UpdateDomestication(DeltaTime);
}

void ANPCBehaviorAgent::InitializePersonality()
{
    if (!PersonalityComponent)
        return;
        
    // Generate species-based personality with individual variation
    FDinosaurPersonality NewPersonality;
    
    switch (Species)
    {
        case EDinosaurSpecies::TyrannosaurusRex:
            NewPersonality.Aggressiveness = FMath::RandRange(0.8f, 1.0f);
            NewPersonality.Fearfulness = FMath::RandRange(0.0f, 0.2f);
            NewPersonality.Territoriality = FMath::RandRange(0.7f, 1.0f);
            NewPersonality.Intelligence = FMath::RandRange(0.6f, 0.8f);
            NewPersonality.Sociability = FMath::RandRange(0.0f, 0.3f);
            NewPersonality.Curiosity = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case EDinosaurSpecies::Velociraptor:
            NewPersonality.Aggressiveness = FMath::RandRange(0.6f, 0.9f);
            NewPersonality.Intelligence = FMath::RandRange(0.8f, 1.0f);
            NewPersonality.Sociability = FMath::RandRange(0.7f, 1.0f);
            NewPersonality.Curiosity = FMath::RandRange(0.7f, 1.0f);
            NewPersonality.Fearfulness = FMath::RandRange(0.2f, 0.5f);
            NewPersonality.Territoriality = FMath::RandRange(0.5f, 0.8f);
            break;
            
        case EDinosaurSpecies::Triceratops:
            NewPersonality.Aggressiveness = FMath::RandRange(0.3f, 0.7f);
            NewPersonality.Fearfulness = FMath::RandRange(0.2f, 0.5f);
            NewPersonality.Territoriality = FMath::RandRange(0.4f, 0.8f);
            NewPersonality.Sociability = FMath::RandRange(0.5f, 0.8f);
            NewPersonality.Intelligence = FMath::RandRange(0.4f, 0.7f);
            NewPersonality.Curiosity = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case EDinosaurSpecies::Compsognathus:
            NewPersonality.Fearfulness = FMath::RandRange(0.7f, 1.0f);
            NewPersonality.Curiosity = FMath::RandRange(0.6f, 0.9f);
            NewPersonality.Sociability = FMath::RandRange(0.8f, 1.0f);
            NewPersonality.Aggressiveness = FMath::RandRange(0.1f, 0.4f);
            NewPersonality.Intelligence = FMath::RandRange(0.5f, 0.7f);
            NewPersonality.Territoriality = FMath::RandRange(0.2f, 0.5f);
            break;
            
        case EDinosaurSpecies::Parasaurolophus:
            NewPersonality.Sociability = FMath::RandRange(0.8f, 1.0f);
            NewPersonality.Fearfulness = FMath::RandRange(0.6f, 0.9f);
            NewPersonality.Curiosity = FMath::RandRange(0.4f, 0.7f);
            NewPersonality.Aggressiveness = FMath::RandRange(0.0f, 0.3f);
            NewPersonality.Intelligence = FMath::RandRange(0.5f, 0.8f);
            NewPersonality.Territoriality = FMath::RandRange(0.3f, 0.6f);
            break;
            
        default:
            // Default balanced personality
            NewPersonality.Aggressiveness = FMath::RandRange(0.3f, 0.7f);
            NewPersonality.Fearfulness = FMath::RandRange(0.3f, 0.7f);
            NewPersonality.Curiosity = FMath::RandRange(0.3f, 0.7f);
            NewPersonality.Sociability = FMath::RandRange(0.3f, 0.7f);
            NewPersonality.Territoriality = FMath::RandRange(0.3f, 0.7f);
            NewPersonality.Intelligence = FMath::RandRange(0.3f, 0.7f);
            break;
    }
    
    PersonalityComponent->SetPersonality(NewPersonality);
    
    // Generate unique name
    GenerateUniqueName();
}

void ANPCBehaviorAgent::GenerateUniqueName()
{
    // Simple name generation based on species and personality
    TArray<FString> NamePrefixes = {
        TEXT("Alpha"), TEXT("Beta"), TEXT("Gamma"), TEXT("Delta"), TEXT("Echo"),
        TEXT("Zeta"), TEXT("Theta"), TEXT("Kappa"), TEXT("Lambda"), TEXT("Sigma")
    };
    
    TArray<FString> NameSuffixes = {
        TEXT("Claw"), TEXT("Fang"), TEXT("Horn"), TEXT("Scale"), TEXT("Roar"),
        TEXT("Strike"), TEXT("Swift"), TEXT("Mighty"), TEXT("Wise"), TEXT("Bold")
    };
    
    FString Prefix = NamePrefixes[FMath::RandRange(0, NamePrefixes.Num() - 1)];
    FString Suffix = NameSuffixes[FMath::RandRange(0, NameSuffixes.Num() - 1)];
    
    IndividualName = FString::Printf(TEXT("%s%s"), *Prefix, *Suffix);
}

void ANPCBehaviorAgent::SetupDailyRoutines()
{
    DailyRoutines.Empty();
    
    if (IsHerbivore())
    {
        // Herbivore routines: foraging, resting, socializing
        FDailyRoutine MorningForaging;
        MorningForaging.StartTime = 6.0f;
        MorningForaging.EndTime = 10.0f;
        MorningForaging.Activity = EDinosaurBehaviorState::Foraging;
        MorningForaging.Priority = 1.0f;
        MorningForaging.TargetLocation = FindNearestFoodSource();
        DailyRoutines.Add(MorningForaging);
        
        FDailyRoutine MidDayRest;
        MidDayRest.StartTime = 12.0f;
        MidDayRest.EndTime = 14.0f;
        MidDayRest.Activity = EDinosaurBehaviorState::Resting;
        MidDayRest.Priority = 0.8f;
        MidDayRest.TargetLocation = FindSafeRestingSpot();
        DailyRoutines.Add(MidDayRest);
        
        FDailyRoutine EveningForaging;
        EveningForaging.StartTime = 16.0f;
        EveningForaging.EndTime = 19.0f;
        EveningForaging.Activity = EDinosaurBehaviorState::Foraging;
        EveningForaging.Priority = 1.0f;
        EveningForaging.TargetLocation = FindNearestFoodSource();
        DailyRoutines.Add(EveningForaging);
        
        FDailyRoutine EveningSocial;
        EveningSocial.StartTime = 19.0f;
        EveningSocial.EndTime = 21.0f;
        EveningSocial.Activity = EDinosaurBehaviorState::Socializing;
        EveningSocial.Priority = 0.6f;
        DailyRoutines.Add(EveningSocial);
    }
    else
    {
        // Carnivore routines: hunting, patrolling, resting
        FDailyRoutine DawnHunt;
        DawnHunt.StartTime = 5.0f;
        DawnHunt.EndTime = 8.0f;
        DawnHunt.Activity = EDinosaurBehaviorState::Hunting;
        DawnHunt.Priority = 1.0f;
        DailyRoutines.Add(DawnHunt);
        
        FDailyRoutine MorningPatrol;
        MorningPatrol.StartTime = 8.0f;
        MorningPatrol.EndTime = 12.0f;
        MorningPatrol.Activity = EDinosaurBehaviorState::Patrolling;
        MorningPatrol.Priority = 0.7f;
        DailyRoutines.Add(MorningPatrol);
        
        FDailyRoutine AfternoonRest;
        AfternoonRest.StartTime = 12.0f;
        AfternoonRest.EndTime = 16.0f;
        AfternoonRest.Activity = EDinosaurBehaviorState::Resting;
        AfternoonRest.Priority = 0.8f;
        DailyRoutines.Add(AfternoonRest);
        
        FDailyRoutine EveningHunt;
        EveningHunt.StartTime = 18.0f;
        EveningHunt.EndTime = 21.0f;
        EveningHunt.Activity = EDinosaurBehaviorState::Hunting;
        EveningHunt.Priority = 1.0f;
        DailyRoutines.Add(EveningHunt);
    }
    
    // All species need water
    FDailyRoutine MorningDrink;
    MorningDrink.StartTime = 7.0f;
    MorningDrink.EndTime = 8.0f;
    MorningDrink.Activity = EDinosaurBehaviorState::Drinking;
    MorningDrink.Priority = 0.9f;
    MorningDrink.TargetLocation = FindNearestWaterSource();
    DailyRoutines.Add(MorningDrink);
    
    FDailyRoutine EveningDrink;
    EveningDrink.StartTime = 17.0f;
    EveningDrink.EndTime = 18.0f;
    EveningDrink.Activity = EDinosaurBehaviorState::Drinking;
    EveningDrink.Priority = 0.9f;
    MorningDrink.TargetLocation = FindNearestWaterSource();
    DailyRoutines.Add(EveningDrink);
}

bool ANPCBehaviorAgent::IsHerbivore() const
{
    switch (Species)
    {
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Triceratops:
        case EDinosaurSpecies::Brachiosaurus:
        case EDinosaurSpecies::Diplodocus:
        case EDinosaurSpecies::Stegosaurus:
            return true;
        default:
            return false;
    }
}

void ANPCBehaviorAgent::UpdateNeeds(float DeltaTime)
{
    // Natural degradation of needs
    float BaseDecayRate = 0.001f; // 0.1% per second
    
    // Hunger increases faster for carnivores
    float HungerDecay = IsHerbivore() ? BaseDecayRate : BaseDecayRate * 1.5f;
    CurrentNeeds.Hunger = FMath::Clamp(CurrentNeeds.Hunger + (HungerDecay * DeltaTime), 0.0f, 1.0f);
    
    // Thirst increases for all
    CurrentNeeds.Thirst = FMath::Clamp(CurrentNeeds.Thirst + (BaseDecayRate * 1.2f * DeltaTime), 0.0f, 1.0f);
    
    // Energy decreases with activity
    float EnergyDecay = BaseDecayRate;
    if (CurrentBehaviorState == EDinosaurBehaviorState::Hunting || 
        CurrentBehaviorState == EDinosaurBehaviorState::Fleeing)
    {
        EnergyDecay *= 3.0f;
    }
    else if (CurrentBehaviorState == EDinosaurBehaviorState::Resting)
    {
        EnergyDecay *= -2.0f; // Resting restores energy
    }
    
    CurrentNeeds.Energy = FMath::Clamp(CurrentNeeds.Energy - (EnergyDecay * DeltaTime), 0.0f, 1.0f);
    
    // Social need increases when alone
    if (GetNearbyDinosaurCount() == 0)
    {
        CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social + (BaseDecayRate * 0.5f * DeltaTime), 0.0f, 1.0f);
    }
    else
    {
        CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social - (BaseDecayRate * 2.0f * DeltaTime), 0.0f, 1.0f);
    }
}

void ANPCBehaviorAgent::ProcessDailyRoutine()
{
    // Get current time from world
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeOfDay = FMath::Fmod(CurrentTime / 3600.0f, 24.0f); // Convert to hours of day
    
    // Find best routine for current time
    FDailyRoutine* BestRoutine = nullptr;
    float BestPriority = 0.0f;
    
    for (FDailyRoutine& Routine : DailyRoutines)
    {
        bool IsInTimeRange = false;
        
        if (Routine.StartTime <= Routine.EndTime)
        {
            IsInTimeRange = (TimeOfDay >= Routine.StartTime && TimeOfDay <= Routine.EndTime);
        }
        else
        {
            // Crosses midnight
            IsInTimeRange = (TimeOfDay >= Routine.StartTime || TimeOfDay <= Routine.EndTime);
        }
        
        if (IsInTimeRange && Routine.Priority > BestPriority)
        {
            BestRoutine = &Routine;
            BestPriority = Routine.Priority;
        }
    }
    
    // Apply routine if found and different from current state
    if (BestRoutine && CurrentBehaviorState != BestRoutine->Activity)
    {
        if (CanPerformActivity(BestRoutine->Activity))
        {
            SetBehaviorState(BestRoutine->Activity);
        }
    }
}

bool ANPCBehaviorAgent::CanPerformActivity(EDinosaurBehaviorState Activity) const
{
    switch (Activity)
    {
        case EDinosaurBehaviorState::Foraging:
        case EDinosaurBehaviorState::Hunting:
            return CurrentNeeds.Energy > 0.2f;
            
        case EDinosaurBehaviorState::Drinking:
            return CurrentNeeds.Energy > 0.1f;
            
        case EDinosaurBehaviorState::Resting:
            return true; // Can always rest
            
        case EDinosaurBehaviorState::Socializing:
            return CurrentNeeds.Energy > 0.3f;
            
        default:
            return true;
    }
}

void ANPCBehaviorAgent::UpdateBehaviorBasedOnNeeds()
{
    // Override routine if critical needs
    if (CurrentNeeds.Thirst > 0.8f && CurrentBehaviorState != EDinosaurBehaviorState::Drinking)
    {
        SetBehaviorState(EDinosaurBehaviorState::Drinking);
    }
    else if (CurrentNeeds.Hunger > 0.8f)
    {
        if (IsHerbivore() && CurrentBehaviorState != EDinosaurBehaviorState::Foraging)
        {
            SetBehaviorState(EDinosaurBehaviorState::Foraging);
        }
        else if (!IsHerbivore() && CurrentBehaviorState != EDinosaurBehaviorState::Hunting)
        {
            SetBehaviorState(EDinosaurBehaviorState::Hunting);
        }
    }
    else if (CurrentNeeds.Energy < 0.2f && CurrentBehaviorState != EDinosaurBehaviorState::Resting)
    {
        SetBehaviorState(EDinosaurBehaviorState::Resting);
    }
}

void ANPCBehaviorAgent::ProcessSocialInteractions(float DeltaTime)
{
    // Find nearby dinosaurs
    TArray<AActor*> NearbyDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPCBehaviorAgent::StaticClass(), NearbyDinosaurs);
    
    for (AActor* Actor : NearbyDinosaurs)
    {
        if (Actor == this) continue;
        
        ANPCBehaviorAgent* OtherDino = Cast<ANPCBehaviorAgent>(Actor);
        if (!OtherDino) continue;
        
        float Distance = FVector::Dist(GetActorLocation(), OtherDino->GetActorLocation());
        
        // Social interaction range
        if (Distance < 500.0f)
        {
            ProcessSocialInteractionWith(OtherDino, DeltaTime);
        }
    }
}

void ANPCBehaviorAgent::ProcessSocialInteractionWith(ANPCBehaviorAgent* OtherDino, float DeltaTime)
{
    if (!OtherDino || !PersonalityComponent || !OtherDino->PersonalityComponent)
        return;
        
    FDinosaurPersonality MyPersonality = PersonalityComponent->GetPersonality();
    FDinosaurPersonality OtherPersonality = OtherDino->PersonalityComponent->GetPersonality();
    
    // Same species interactions
    if (Species == OtherDino->Species)
    {
        // Pack behavior for social species
        if (MyPersonality.Sociability > 0.6f && OtherPersonality.Sociability > 0.6f)
        {
            // Reduce social need when with compatible dinosaurs
            CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social - (0.001f * DeltaTime), 0.0f, 1.0f);
            
            // Synchronize some behaviors
            if (CurrentBehaviorState == EDinosaurBehaviorState::Socializing)
            {
                OtherDino->SetBehaviorState(EDinosaurBehaviorState::Socializing);
            }
        }
    }
    else
    {
        // Different species interactions
        bool IAmPredator = !IsHerbivore();
        bool OtherIsPrey = OtherDino->IsHerbivore();
        
        if (IAmPredator && OtherIsPrey)
        {
            // Predator-prey interaction
            if (CurrentNeeds.Hunger > 0.6f && MyPersonality.Aggressiveness > 0.5f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Hunting);
                OtherDino->SetBehaviorState(EDinosaurBehaviorState::Fleeing);
            }
        }
    }
}

void ANPCBehaviorAgent::UpdateDomestication(float DeltaTime)
{
    if (!bCanBeDomesticated)
        return;
        
    // Find player
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player)
        return;
        
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    
    // Player proximity affects domestication
    if (DistanceToPlayer < 300.0f) // Close to player
    {
        if (PersonalityComponent)
        {
            FDinosaurPersonality Personality = PersonalityComponent->GetPersonality();
            
            // Curious and less fearful dinosaurs domesticate faster
            float DomesticationRate = (Personality.Curiosity - Personality.Fearfulness) * 0.0001f * DeltaTime;
            
            if (DomesticationRate > 0.0f)
            {
                DomesticationProgress = FMath::Clamp(DomesticationProgress + DomesticationRate, 0.0f, 1.0f);
                
                // Update domestication level based on progress
                if (DomesticationProgress > 0.9f)
                    DomesticationLevel = EDomesticationLevel::Domesticated;
                else if (DomesticationProgress > 0.7f)
                    DomesticationLevel = EDomesticationLevel::Bonded;
                else if (DomesticationProgress > 0.5f)
                    DomesticationLevel = EDomesticationLevel::Friendly;
                else if (DomesticationProgress > 0.3f)
                    DomesticationLevel = EDomesticationLevel::Tolerant;
                else if (DomesticationProgress > 0.1f)
                    DomesticationLevel = EDomesticationLevel::Curious;
                else
                    DomesticationLevel = EDomesticationLevel::Wary;
            }
        }
    }
}

void ANPCBehaviorAgent::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
        return;
        
    EDinosaurBehaviorState OldState = CurrentBehaviorState;
    CurrentBehaviorState = NewState;
    
    // Update AI Controller blackboard
    if (ADinosaurAIController* DinoController = Cast<ADinosaurAIController>(GetController()))
    {
        DinoController->UpdateBehaviorState(NewState);
    }
    
    // Broadcast state change
    OnBehaviorStateChanged.Broadcast(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("%s changed behavior from %s to %s"), 
           *GetName(), 
           *UEnum::GetValueAsString(OldState),
           *UEnum::GetValueAsString(NewState));
}

int32 ANPCBehaviorAgent::GetNearbyDinosaurCount() const
{
    TArray<AActor*> NearbyDinosaurs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANPCBehaviorAgent::StaticClass(), NearbyDinosaurs);
    
    int32 Count = 0;
    for (AActor* Actor : NearbyDinosaurs)
    {
        if (Actor == this) continue;
        
        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance < 1000.0f) // 10 meter radius
        {
            Count++;
        }
    }
    
    return Count;
}

FVector ANPCBehaviorAgent::FindNearestWaterSource() const
{
    // For now, return a location near territory center
    // In full implementation, this would query the world for actual water sources
    return TerritoryCenter + FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
}

FVector ANPCBehaviorAgent::FindNearestFoodSource() const
{
    // For now, return a location within territory
    // In full implementation, this would query for vegetation or prey
    return TerritoryCenter + FVector(FMath::RandRange(-800.0f, 800.0f), FMath::RandRange(-800.0f, 800.0f), 0.0f);
}

FVector ANPCBehaviorAgent::FindSafeRestingSpot() const
{
    // Find elevated or secluded spot within territory
    return TerritoryCenter + FVector(FMath::RandRange(-300.0f, 300.0f), FMath::RandRange(-300.0f, 300.0f), 50.0f);
}

void ANPCBehaviorAgent::RegisterWithNPCManager()
{
    // Find and register with NPC Manager for global coordination
    // This would be implemented when the NPC Manager system is complete
    UE_LOG(LogTemp, Log, TEXT("NPC %s registered with NPC Manager"), *GetName());
}

void ANPCBehaviorAgent::OnPlayerInteraction(bool bPositive)
{
    if (MemoryComponent)
    {
        MemoryComponent->RecordPlayerInteraction(bPositive);
    }
    
    // Adjust domestication based on interaction
    if (bCanBeDomesticated)
    {
        float InteractionImpact = bPositive ? 0.05f : -0.1f;
        DomesticationProgress = FMath::Clamp(DomesticationProgress + InteractionImpact, 0.0f, 1.0f);
    }
}

float ANPCBehaviorAgent::GetMostUrgentNeed() const
{
    float MaxNeed = FMath::Max({CurrentNeeds.Hunger, CurrentNeeds.Thirst, 1.0f - CurrentNeeds.Energy, CurrentNeeds.Social});
    return MaxNeed;
}

void ANPCBehaviorAgent::SatisfyNeed(const FString& NeedType, float Amount)
{
    if (NeedType == TEXT("Hunger"))
    {
        CurrentNeeds.Hunger = FMath::Clamp(CurrentNeeds.Hunger - Amount, 0.0f, 1.0f);
    }
    else if (NeedType == TEXT("Thirst"))
    {
        CurrentNeeds.Thirst = FMath::Clamp(CurrentNeeds.Thirst - Amount, 0.0f, 1.0f);
    }
    else if (NeedType == TEXT("Energy"))
    {
        CurrentNeeds.Energy = FMath::Clamp(CurrentNeeds.Energy + Amount, 0.0f, 1.0f);
    }
    else if (NeedType == TEXT("Social"))
    {
        CurrentNeeds.Social = FMath::Clamp(CurrentNeeds.Social - Amount, 0.0f, 1.0f);
    }
}