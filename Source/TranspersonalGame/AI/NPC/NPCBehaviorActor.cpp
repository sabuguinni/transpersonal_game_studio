#include "NPCBehaviorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ANPCBehaviorActor::ANPCBehaviorActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Setup character mesh (basic capsule for now)
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Create pawn sensing component
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SightRadius = 1500.0f;
    PawnSensing->HearingThreshold = 1200.0f;
    PawnSensing->LOSHearingThreshold = 800.0f;
    PawnSensing->SetPeripheralVisionAngle(90.0f);
    
    // Bind sensing events
    PawnSensing->OnSeePawn.AddDynamic(this, &ANPCBehaviorActor::OnSeePawn);
    PawnSensing->OnHearNoise.AddDynamic(this, &ANPCBehaviorActor::OnHearNoise);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(InteractionRange);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Setup movement
    GetCharacterMovement()->MaxWalkSpeed = 300.0f;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    // Initialize default values
    NPCName = TEXT("Unnamed NPC");
    Profession = ENPC_Profession::Gatherer;
    Age = 25;
    FearLevel = 0.0f;
    SocialNeed = 50.0f;
    CurrentState = ENPC_BehaviorState::Idle;
    PatrolRadius = 1000.0f;
    InteractionRange = 300.0f;
    MaxMemoryEntries = 10;
    MemoryDecayRate = 0.1f;
    
    // AI Controller class
    AIControllerClass = AAIController::StaticClass();
}

void ANPCBehaviorActor::BeginPlay()
{
    Super::BeginPlay();
    
    HomeLocation = GetActorLocation();
    CurrentTarget = HomeLocation;
    
    InitializeSchedule();
    
    // Start behavior tree if available
    if (BehaviorTreeAsset && GetController())
    {
        if (AAIController* AIController = Cast<AAIController>(GetController()))
        {
            AIController->RunBehaviorTree(BehaviorTreeAsset);
        }
    }
}

void ANPCBehaviorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMemories(DeltaTime);
    ProcessBehaviorLogic(DeltaTime);
    HandleMovement(DeltaTime);
    UpdateFearLevel(DeltaTime);
    
    StateChangeTimer += DeltaTime;
    
    // Update schedule every 60 seconds
    if (GetWorld()->GetTimeSeconds() - LastScheduleUpdate > 60.0f)
    {
        UpdateDailySchedule();
        LastScheduleUpdate = GetWorld()->GetTimeSeconds();
    }
}

void ANPCBehaviorActor::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        // Update blackboard if AI controller exists
        if (AAIController* AIController = Cast<AAIController>(GetController()))
        {
            if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
            {
                BlackboardComp->SetValueAsEnum(TEXT("BehaviorState"), (uint8)CurrentState);
            }
        }
    }
}

void ANPCBehaviorActor::UpdateDailySchedule()
{
    if (DailySchedule.Num() == 0) return;
    
    // Simple time simulation - use game time
    float CurrentHour = FMath::Fmod(GetWorld()->GetTimeSeconds() / 3600.0f, 24.0f);
    
    // Find appropriate schedule entry
    for (int32 i = 0; i < DailySchedule.Num(); i++)
    {
        const FNPC_DailySchedule& Schedule = DailySchedule[i];
        if (CurrentHour >= Schedule.StartHour && CurrentHour < Schedule.EndHour)
        {
            CurrentScheduleIndex = i;
            SetBehaviorState(Schedule.ScheduledActivity);
            
            if (Schedule.WorkLocation != FVector::ZeroVector)
            {
                CurrentTarget = HomeLocation + Schedule.WorkLocation;
            }
            break;
        }
    }
}

void ANPCBehaviorActor::AddMemory(AActor* Actor, FVector Location, float ThreatLevel)
{
    if (!Actor) return;
    
    // Check if we already have memory of this actor
    for (FNPC_Memory& Memory : ShortTermMemory)
    {
        if (Memory.RememberedActor == Actor)
        {
            // Update existing memory
            Memory.LastKnownLocation = Location;
            Memory.Timestamp = GetWorld()->GetTimeSeconds();
            Memory.ThreatLevel = FMath::Max(Memory.ThreatLevel, ThreatLevel);
            return;
        }
    }
    
    // Add new memory
    FNPC_Memory NewMemory;
    NewMemory.RememberedActor = Actor;
    NewMemory.LastKnownLocation = Location;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.ThreatLevel = ThreatLevel;
    
    ShortTermMemory.Add(NewMemory);
    
    // Remove oldest memories if we exceed limit
    while (ShortTermMemory.Num() > MaxMemoryEntries)
    {
        ShortTermMemory.RemoveAt(0);
    }
}

FNPC_Memory ANPCBehaviorActor::GetMemoryOfActor(AActor* Actor)
{
    for (const FNPC_Memory& Memory : ShortTermMemory)
    {
        if (Memory.RememberedActor == Actor)
        {
            return Memory;
        }
    }
    
    return FNPC_Memory(); // Return empty memory if not found
}

void ANPCBehaviorActor::UpdateMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay and remove old memories
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        FNPC_Memory& Memory = ShortTermMemory[i];
        
        // Decay threat level over time
        Memory.ThreatLevel = FMath::Max(0.0f, Memory.ThreatLevel - (MemoryDecayRate * DeltaTime));
        
        // Remove memories older than 5 minutes or with no threat
        float MemoryAge = CurrentTime - Memory.Timestamp;
        if (MemoryAge > 300.0f || Memory.ThreatLevel <= 0.1f)
        {
            ShortTermMemory.RemoveAt(i);
        }
    }
}

void ANPCBehaviorActor::OnPlayerApproach(AActor* Player)
{
    if (!Player) return;
    
    AddMemory(Player, Player->GetActorLocation(), 0.2f); // Low threat for player
    
    // React based on profession and current state
    switch (Profession)
    {
        case ENPC_Profession::Elder:
            if (CurrentState == ENPC_BehaviorState::Idle)
            {
                SetBehaviorState(ENPC_BehaviorState::Socializing);
            }
            break;
            
        case ENPC_Profession::Hunter:
            SetBehaviorState(ENPC_BehaviorState::Investigating);
            break;
            
        case ENPC_Profession::Child:
            if (FearLevel > 30.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Fleeing);
            }
            break;
            
        default:
            break;
    }
}

void ANPCBehaviorActor::OnDinosaurSighted(AActor* Dinosaur)
{
    if (!Dinosaur) return;
    
    float ThreatLevel = 0.8f; // High threat for dinosaurs
    AddMemory(Dinosaur, Dinosaur->GetActorLocation(), ThreatLevel);
    
    // Increase fear level
    FearLevel = FMath::Min(100.0f, FearLevel + 25.0f);
    
    // Most NPCs should flee from dinosaurs
    if (Profession != ENPC_Profession::Hunter || FearLevel > 60.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
        CurrentTarget = HomeLocation; // Run home
    }
}

FVector ANPCBehaviorActor::GetRandomPatrolPoint()
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    return HomeLocation + (RandomDirection * RandomDistance);
}

bool ANPCBehaviorActor::ShouldFlee()
{
    // Check for high-threat memories
    for (const FNPC_Memory& Memory : ShortTermMemory)
    {
        if (Memory.ThreatLevel > 0.5f)
        {
            float MemoryAge = GetWorld()->GetTimeSeconds() - Memory.Timestamp;
            if (MemoryAge < 60.0f) // Recent threat
            {
                return true;
            }
        }
    }
    
    return FearLevel > 70.0f;
}

void ANPCBehaviorActor::OnSeePawn(APawn* Pawn)
{
    if (!Pawn) return;
    
    // Determine if this is a player or dinosaur
    if (Pawn->IsA<ACharacter>() && Pawn->IsPlayerControlled())
    {
        OnPlayerApproach(Pawn);
    }
    else
    {
        // Assume it's a dinosaur or other threat
        OnDinosaurSighted(Pawn);
    }
}

void ANPCBehaviorActor::OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
    if (Volume > 0.5f) // Only react to significant noise
    {
        AddMemory(NoiseInstigator, Location, Volume * 0.3f);
        
        if (CurrentState == ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
            CurrentTarget = Location;
        }
    }
}

void ANPCBehaviorActor::InitializeSchedule()
{
    // Create default schedule based on profession
    DailySchedule.Empty();
    
    switch (Profession)
    {
        case ENPC_Profession::Elder:
            {
                FNPC_DailySchedule MorningSchedule;
                MorningSchedule.StartHour = 6.0f;
                MorningSchedule.EndHour = 12.0f;
                MorningSchedule.ScheduledActivity = ENPC_BehaviorState::Socializing;
                DailySchedule.Add(MorningSchedule);
                
                FNPC_DailySchedule AfternoonSchedule;
                AfternoonSchedule.StartHour = 12.0f;
                AfternoonSchedule.EndHour = 18.0f;
                AfternoonSchedule.ScheduledActivity = ENPC_BehaviorState::Working;
                DailySchedule.Add(AfternoonSchedule);
            }
            break;
            
        case ENPC_Profession::Hunter:
            {
                FNPC_DailySchedule HuntingSchedule;
                HuntingSchedule.StartHour = 5.0f;
                HuntingSchedule.EndHour = 11.0f;
                HuntingSchedule.ScheduledActivity = ENPC_BehaviorState::Patrolling;
                HuntingSchedule.WorkLocation = FVector(2000, 0, 0);
                DailySchedule.Add(HuntingSchedule);
            }
            break;
            
        case ENPC_Profession::Gatherer:
            {
                FNPC_DailySchedule GatheringSchedule;
                GatheringSchedule.StartHour = 7.0f;
                GatheringSchedule.EndHour = 16.0f;
                GatheringSchedule.ScheduledActivity = ENPC_BehaviorState::Working;
                GatheringSchedule.WorkLocation = FVector(0, 1500, 0);
                DailySchedule.Add(GatheringSchedule);
            }
            break;
            
        default:
            {
                FNPC_DailySchedule DefaultSchedule;
                DefaultSchedule.StartHour = 8.0f;
                DefaultSchedule.EndHour = 17.0f;
                DefaultSchedule.ScheduledActivity = ENPC_BehaviorState::Working;
                DailySchedule.Add(DefaultSchedule);
            }
            break;
    }
}

void ANPCBehaviorActor::ProcessBehaviorLogic(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            if (StateChangeTimer > 10.0f) // Change state after 10 seconds
            {
                if (ShouldFlee())
                {
                    SetBehaviorState(ENPC_BehaviorState::Fleeing);
                }
                else if (FMath::RandRange(0.0f, 1.0f) > 0.7f)
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                    CurrentTarget = GetRandomPatrolPoint();
                }
            }
            break;
            
        case ENPC_BehaviorState::Patrolling:
            {
                float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget);
                if (DistanceToTarget < 100.0f || StateChangeTimer > 30.0f)
                {
                    if (ShouldFlee())
                    {
                        SetBehaviorState(ENPC_BehaviorState::Fleeing);
                    }
                    else
                    {
                        CurrentTarget = GetRandomPatrolPoint();
                        StateChangeTimer = 0.0f;
                    }
                }
            }
            break;
            
        case ENPC_BehaviorState::Fleeing:
            {
                float DistanceToHome = FVector::Dist(GetActorLocation(), HomeLocation);
                if (DistanceToHome < 200.0f && !ShouldFlee())
                {
                    SetBehaviorState(ENPC_BehaviorState::Idle);
                    FearLevel = FMath::Max(0.0f, FearLevel - 20.0f);
                }
            }
            break;
            
        case ENPC_BehaviorState::Investigating:
            if (StateChangeTimer > 15.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            break;
            
        default:
            break;
    }
}

void ANPCBehaviorActor::HandleMovement(float DeltaTime)
{
    if (CurrentState == ENPC_BehaviorState::Idle || CurrentState == ENPC_BehaviorState::Socializing)
    {
        return; // No movement needed
    }
    
    FVector Direction = (CurrentTarget - GetActorLocation()).GetSafeNormal();
    float Distance = FVector::Dist(GetActorLocation(), CurrentTarget);
    
    if (Distance > 50.0f)
    {
        // Move towards target
        AddMovementInput(Direction, 1.0f);
        
        // Adjust speed based on behavior state
        float SpeedMultiplier = 1.0f;
        switch (CurrentState)
        {
            case ENPC_BehaviorState::Fleeing:
                SpeedMultiplier = 1.5f;
                break;
            case ENPC_BehaviorState::Patrolling:
                SpeedMultiplier = 0.7f;
                break;
            case ENPC_BehaviorState::Working:
                SpeedMultiplier = 0.8f;
                break;
            default:
                break;
        }
        
        GetCharacterMovement()->MaxWalkSpeed = 300.0f * SpeedMultiplier;
    }
}

void ANPCBehaviorActor::UpdateFearLevel(float DeltaTime)
{
    // Gradually reduce fear over time when safe
    if (!ShouldFlee())
    {
        FearLevel = FMath::Max(0.0f, FearLevel - (10.0f * DeltaTime));
    }
    
    // Increase social need over time
    SocialNeed = FMath::Min(100.0f, SocialNeed + (5.0f * DeltaTime));
}