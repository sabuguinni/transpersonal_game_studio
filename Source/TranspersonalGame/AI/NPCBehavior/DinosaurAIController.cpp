#include "DinosaurAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NPCBehaviorSystemCore.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize Behavior Tree Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    
    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1800.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Initialize default values
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    bIsInCombat = false;
    bIsPlayerVisible = false;
    PlayerThreatLevel = 0.0f;
    LastPlayerSightingTime = 0.0f;
    
    // Set default AI focus
    SetPerceptionComponent(*AIPerceptionComponent);
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ADinosaurAIController::OnTargetForgotten);
    }
    
    // Initialize behavior tree if available
    if (BehaviorTreeAsset && BlackboardAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
        InitializeBlackboardValues();
    }
    
    // Start daily routine timer
    GetWorld()->GetTimerManager().SetTimer(
        RoutineUpdateTimer,
        this,
        &ADinosaurAIController::UpdateDailyRoutine,
        RoutineUpdateInterval,
        true
    );
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateBehaviorState(DeltaTime);
    UpdateBlackboardValues();
    ProcessMemoryDecay(DeltaTime);
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    // Get NPC behavior component from possessed pawn
    if (InPawn)
    {
        NPCBehaviorComponent = InPawn->FindComponentByClass<UNPCBehaviorComponent>();
        if (NPCBehaviorComponent)
        {
            // Sync species and personality data
            Species = NPCBehaviorComponent->Species;
            Personality = NPCBehaviorComponent->Personality;
            
            // Set appropriate behavior tree based on species
            SetBehaviorTreeForSpecies(Species);
        }
    }
}

void ADinosaurAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;
    
    // Check if it's the player
    if (Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled())
    {
        HandlePlayerPerception(Actor, Stimulus);
    }
    else
    {
        HandleOtherActorPerception(Actor, Stimulus);
    }
    
    // Update memory
    if (NPCBehaviorComponent)
    {
        float ThreatLevel = CalculateThreatLevel(Actor);
        NPCBehaviorComponent->RememberActor(Actor, ThreatLevel);
    }
}

void ADinosaurAIController::OnTargetForgotten(AActor* Actor)
{
    if (!Actor) return;
    
    if (Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled())
    {
        bIsPlayerVisible = false;
        LastPlayerSightingTime = GetWorld()->GetTimeSeconds();
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsBool(TEXT("IsPlayerVisible"), false);
        }
    }
}

void ADinosaurAIController::SetBehaviorState(EDinosaurBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        EDinosaurBehaviorState PreviousState = CurrentBehaviorState;
        CurrentBehaviorState = NewState;
        
        // Update NPC behavior component
        if (NPCBehaviorComponent)
        {
            NPCBehaviorComponent->SetBehaviorState(NewState);
        }
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
        
        // Handle state-specific logic
        OnBehaviorStateChanged(PreviousState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s changed behavior from %s to %s"), 
               *GetPawn()->GetName(),
               *UEnum::GetValueAsString(PreviousState),
               *UEnum::GetValueAsString(NewState));
    }
}

void ADinosaurAIController::HandlePlayerPerception(AActor* Player, const FAIStimulus& Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        bIsPlayerVisible = true;
        LastPlayerSightingTime = GetWorld()->GetTimeSeconds();
        
        // Calculate threat based on distance and species personality
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
        float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
        
        PlayerThreatLevel = DistanceFactor * Personality.Fearfulness;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsBool(TEXT("IsPlayerVisible"), true);
            BlackboardComponent->SetValueAsObject(TEXT("PlayerActor"), Player);
            BlackboardComponent->SetValueAsVector(TEXT("PlayerLocation"), Player->GetActorLocation());
            BlackboardComponent->SetValueAsFloat(TEXT("PlayerThreatLevel"), PlayerThreatLevel);
        }
        
        // Determine reaction based on species and personality
        DeterminePlayerReaction();
    }
}

void ADinosaurAIController::HandleOtherActorPerception(AActor* Actor, const FAIStimulus& Stimulus)
{
    if (!Stimulus.WasSuccessfullySensed()) return;
    
    // Handle other dinosaurs
    if (Actor->GetClass()->IsChildOf(ADinosaurNPC::StaticClass()))
    {
        ADinosaurNPC* OtherDinosaur = Cast<ADinosaurNPC>(Actor);
        if (OtherDinosaur && OtherDinosaur->NPCBehaviorComponent)
        {
            HandleDinosaurInteraction(OtherDinosaur);
        }
    }
    
    // Handle environmental stimuli (food, water, etc.)
    HandleEnvironmentalStimuli(Actor);
}

void ADinosaurAIController::DeterminePlayerReaction()
{
    if (!NPCBehaviorComponent) return;
    
    // Base reaction on species type and domestication level
    switch (Species)
    {
        case EDinosaurSpecies::TyrannosaurusRex:
        case EDinosaurSpecies::Allosaurus:
        case EDinosaurSpecies::Spinosaurus:
            // Apex predators - aggressive or territorial
            if (PlayerThreatLevel < 0.3f && Personality.Aggressiveness > 0.6f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Hunting);
            }
            else
            {
                SetBehaviorState(EDinosaurBehaviorState::Territorial);
            }
            break;
            
        case EDinosaurSpecies::Velociraptor:
        case EDinosaurSpecies::Deinonychus:
            // Pack hunters - investigate or hunt
            if (Personality.Curiosity > 0.5f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Investigating);
            }
            else if (Personality.Aggressiveness > 0.7f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Hunting);
            }
            break;
            
        case EDinosaurSpecies::Compsognathus:
        case EDinosaurSpecies::Parasaurolophus:
        case EDinosaurSpecies::Triceratops:
            // Small herbivores - flee or investigate based on domestication
            if (NPCBehaviorComponent->DomesticationLevel >= EDomesticationLevel::Curious)
            {
                SetBehaviorState(EDinosaurBehaviorState::Investigating);
            }
            else if (PlayerThreatLevel > 0.5f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Fleeing);
            }
            break;
            
        default:
            // Large herbivores - mostly ignore unless threatened
            if (PlayerThreatLevel > 0.7f)
            {
                SetBehaviorState(EDinosaurBehaviorState::Territorial);
            }
            break;
    }
}

void ADinosaurAIController::HandleDinosaurInteraction(ADinosaurNPC* OtherDinosaur)
{
    if (!OtherDinosaur || !NPCBehaviorComponent) return;
    
    EDinosaurSpecies OtherSpecies = OtherDinosaur->NPCBehaviorComponent->Species;
    
    // Predator-prey relationships
    if (IsSpeciesPredator(Species) && IsSpeciesPrey(OtherSpecies))
    {
        if (NPCBehaviorComponent->CurrentNeeds.Hunger > 0.6f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Hunting);
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("HuntTarget"), OtherDinosaur);
            }
        }
    }
    else if (IsSpeciesPrey(Species) && IsSpeciesPredator(OtherSpecies))
    {
        SetBehaviorState(EDinosaurBehaviorState::Fleeing);
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("FleeTarget"), OtherDinosaur);
        }
    }
    else if (Species == OtherSpecies)
    {
        // Same species interaction
        if (Personality.Sociability > 0.6f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Socializing);
        }
    }
}

void ADinosaurAIController::HandleEnvironmentalStimuli(AActor* StimulusActor)
{
    if (!StimulusActor || !NPCBehaviorComponent) return;
    
    // Check for water sources
    if (StimulusActor->GetName().Contains(TEXT("Water")) && 
        NPCBehaviorComponent->CurrentNeeds.Thirst > 0.4f)
    {
        NPCBehaviorComponent->RememberLocation(StimulusActor->GetActorLocation(), TEXT("Water"));
        SetBehaviorState(EDinosaurBehaviorState::Drinking);
    }
    
    // Check for food sources
    if (StimulusActor->GetName().Contains(TEXT("Food")) && 
        NPCBehaviorComponent->CurrentNeeds.Hunger > 0.4f)
    {
        NPCBehaviorComponent->RememberLocation(StimulusActor->GetActorLocation(), TEXT("Food"));
        SetBehaviorState(EDinosaurBehaviorState::Foraging);
    }
}

float ADinosaurAIController::CalculateThreatLevel(AActor* Actor) const
{
    if (!Actor) return 0.0f;
    
    float ThreatLevel = 0.0f;
    
    // Player threat calculation
    if (Actor->IsA<APawn>() && Cast<APawn>(Actor)->IsPlayerControlled())
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
        float DistanceFactor = FMath::Clamp(1.0f - (Distance / 1500.0f), 0.0f, 1.0f);
        
        // Base threat on species and personality
        if (IsSpeciesPredator(Species))
        {
            ThreatLevel = -0.3f + (Personality.Aggressiveness * 0.8f); // Predators see player as prey/challenge
        }
        else
        {
            ThreatLevel = 0.2f + (Personality.Fearfulness * 0.8f); // Prey sees player as threat
        }
        
        ThreatLevel *= DistanceFactor;
    }
    else if (Actor->GetClass()->IsChildOf(ADinosaurNPC::StaticClass()))
    {
        // Other dinosaur threat calculation
        ADinosaurNPC* OtherDinosaur = Cast<ADinosaurNPC>(Actor);
        if (OtherDinosaur && OtherDinosaur->NPCBehaviorComponent)
        {
            EDinosaurSpecies OtherSpecies = OtherDinosaur->NPCBehaviorComponent->Species;
            
            if (IsSpeciesPredator(OtherSpecies) && IsSpeciesPrey(Species))
            {
                ThreatLevel = 0.8f; // High threat
            }
            else if (Species == OtherSpecies && Personality.TerritorialInstinct > 0.6f)
            {
                ThreatLevel = 0.4f; // Moderate threat (territorial)
            }
            else
            {
                ThreatLevel = 0.1f; // Low threat
            }
        }
    }
    
    return FMath::Clamp(ThreatLevel, -1.0f, 1.0f);
}

bool ADinosaurAIController::IsSpeciesPredator(EDinosaurSpecies InSpecies) const
{
    switch (InSpecies)
    {
        case EDinosaurSpecies::TyrannosaurusRex:
        case EDinosaurSpecies::Allosaurus:
        case EDinosaurSpecies::Spinosaurus:
        case EDinosaurSpecies::Velociraptor:
        case EDinosaurSpecies::Deinonychus:
            return true;
        default:
            return false;
    }
}

bool ADinosaurAIController::IsSpeciesPrey(EDinosaurSpecies InSpecies) const
{
    switch (InSpecies)
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

void ADinosaurAIController::SetBehaviorTreeForSpecies(EDinosaurSpecies InSpecies)
{
    // This would typically load different behavior trees for different species
    // For now, we'll use a generic approach
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
}

void ADinosaurAIController::UpdateBehaviorState(float DeltaTime)
{
    if (!NPCBehaviorComponent) return;
    
    // Update needs and check for state changes
    NPCBehaviorComponent->UpdateNeeds(DeltaTime);
    
    // Check if current state should change based on needs
    float MostUrgentNeed = NPCBehaviorComponent->GetMostUrgentNeed();
    
    if (MostUrgentNeed > 0.8f && CurrentBehaviorState != EDinosaurBehaviorState::Fleeing)
    {
        // Critical need - override current behavior
        if (NPCBehaviorComponent->CurrentNeeds.Thirst > 0.8f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Drinking);
        }
        else if (NPCBehaviorComponent->CurrentNeeds.Hunger > 0.8f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Foraging);
        }
        else if (NPCBehaviorComponent->CurrentNeeds.Fatigue > 0.8f)
        {
            SetBehaviorState(EDinosaurBehaviorState::Resting);
        }
    }
}

void ADinosaurAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent || !NPCBehaviorComponent) return;
    
    // Update basic values
    BlackboardComponent->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(CurrentBehaviorState));
    BlackboardComponent->SetValueAsFloat(TEXT("Hunger"), NPCBehaviorComponent->CurrentNeeds.Hunger);
    BlackboardComponent->SetValueAsFloat(TEXT("Thirst"), NPCBehaviorComponent->CurrentNeeds.Thirst);
    BlackboardComponent->SetValueAsFloat(TEXT("Fatigue"), NPCBehaviorComponent->CurrentNeeds.Fatigue);
    BlackboardComponent->SetValueAsBool(TEXT("IsPlayerVisible"), bIsPlayerVisible);
    BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), bIsInCombat);
    
    // Update personality traits
    BlackboardComponent->SetValueAsFloat(TEXT("Aggressiveness"), Personality.Aggressiveness);
    BlackboardComponent->SetValueAsFloat(TEXT("Fearfulness"), Personality.Fearfulness);
    BlackboardComponent->SetValueAsFloat(TEXT("Curiosity"), Personality.Curiosity);
}

void ADinosaurAIController::InitializeBlackboardValues()
{
    if (!BlackboardComponent) return;
    
    // Initialize with default values
    UpdateBlackboardValues();
    
    // Set species-specific values
    BlackboardComponent->SetValueAsEnum(TEXT("Species"), static_cast<uint8>(Species));
    BlackboardComponent->SetValueAsEnum(TEXT("DomesticationLevel"), 
                                       static_cast<uint8>(NPCBehaviorComponent ? NPCBehaviorComponent->DomesticationLevel : EDomesticationLevel::Wild));
}

void ADinosaurAIController::UpdateDailyRoutine()
{
    if (!NPCBehaviorComponent) return;
    
    // Simple routine: patrol between known locations during active hours
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeOfDay = FMath::Fmod(CurrentTime / 3600.0f, 24.0f); // Convert to hours
    
    // Check if it's active time based on personality
    bool bIsActiveTime = false;
    if (Personality.PreferredActivityTime < 0.3f) // Nocturnal
    {
        bIsActiveTime = (TimeOfDay > 20.0f || TimeOfDay < 6.0f);
    }
    else if (Personality.PreferredActivityTime > 0.7f) // Diurnal
    {
        bIsActiveTime = (TimeOfDay > 6.0f && TimeOfDay < 20.0f);
    }
    else // Crepuscular
    {
        bIsActiveTime = ((TimeOfDay > 5.0f && TimeOfDay < 8.0f) || (TimeOfDay > 18.0f && TimeOfDay < 21.0f));
    }
    
    if (bIsActiveTime && CurrentBehaviorState == EDinosaurBehaviorState::Resting)
    {
        SetBehaviorState(EDinosaurBehaviorState::Patrolling);
    }
    else if (!bIsActiveTime && CurrentBehaviorState == EDinosaurBehaviorState::Patrolling)
    {
        SetBehaviorState(EDinosaurBehaviorState::Resting);
    }
}

void ADinosaurAIController::ProcessMemoryDecay(float DeltaTime)
{
    if (!NPCBehaviorComponent) return;
    
    // Decay old memories
    FDateTime CurrentTime = FDateTime::Now();
    TArray<AActor*> ActorsToRemove;
    
    for (auto& MemoryPair : NPCBehaviorComponent->Memory.LastSeenActors)
    {
        FTimespan TimeSinceLastSeen = CurrentTime - MemoryPair.Value;
        if (TimeSinceLastSeen.GetTotalMinutes() > 30.0f) // Forget after 30 minutes
        {
            ActorsToRemove.Add(MemoryPair.Key);
        }
    }
    
    // Remove old memories
    for (AActor* ActorToRemove : ActorsToRemove)
    {
        NPCBehaviorComponent->Memory.LastSeenActors.Remove(ActorToRemove);
        NPCBehaviorComponent->Memory.KnownActorsThreatLevel.Remove(ActorToRemove);
    }
}

void ADinosaurAIController::OnBehaviorStateChanged(EDinosaurBehaviorState OldState, EDinosaurBehaviorState NewState)
{
    // Handle state-specific initialization
    switch (NewState)
    {
        case EDinosaurBehaviorState::Hunting:
            bIsInCombat = true;
            break;
        case EDinosaurBehaviorState::Fleeing:
            bIsInCombat = false;
            break;
        case EDinosaurBehaviorState::Resting:
            bIsInCombat = false;
            break;
        default:
            break;
    }
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), bIsInCombat);
    }
}