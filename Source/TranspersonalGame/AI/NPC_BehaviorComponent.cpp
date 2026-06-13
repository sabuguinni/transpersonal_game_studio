#include "NPC_BehaviorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    BehaviorTreeAsset = nullptr;
    BehaviorRadius = 2000.0f;
    PlayerDetectionRadius = 1500.0f;
    MemoryDuration = 300.0f; // 5 minutes
    bFollowDailyRoutine = true;
    SocialInteractionRadius = 800.0f;
    bCanSocialize = true;
    CurrentGameTime = 0.0f;
    LastKnownPlayer = nullptr;
    LastPlayerDetectionTime = 0.0f;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default daily routine if none set
    if (DailyRoutines.Num() == 0)
    {
        // Morning routine: gathering
        FNPC_DailyRoutine MorningRoutine;
        MorningRoutine.StartTime = 6.0f;
        MorningRoutine.EndTime = 12.0f;
        MorningRoutine.Activity = ENPC_BehaviorState::Gathering;
        MorningRoutine.TargetLocation = GetOwner()->GetActorLocation() + FVector(500, 0, 0);
        DailyRoutines.Add(MorningRoutine);
        
        // Afternoon routine: patrolling
        FNPC_DailyRoutine AfternoonRoutine;
        AfternoonRoutine.StartTime = 12.0f;
        AfternoonRoutine.EndTime = 18.0f;
        AfternoonRoutine.Activity = ENPC_BehaviorState::Patrolling;
        AfternoonRoutine.TargetLocation = GetOwner()->GetActorLocation() + FVector(0, 500, 0);
        DailyRoutines.Add(AfternoonRoutine);
        
        // Evening routine: socializing
        FNPC_DailyRoutine EveningRoutine;
        EveningRoutine.StartTime = 18.0f;
        EveningRoutine.EndTime = 22.0f;
        EveningRoutine.Activity = ENPC_BehaviorState::Socializing;
        EveningRoutine.TargetLocation = GetOwner()->GetActorLocation();
        DailyRoutines.Add(EveningRoutine);
        
        // Night routine: idle
        FNPC_DailyRoutine NightRoutine;
        NightRoutine.StartTime = 22.0f;
        NightRoutine.EndTime = 6.0f;
        NightRoutine.Activity = ENPC_BehaviorState::Idle;
        NightRoutine.TargetLocation = GetOwner()->GetActorLocation();
        DailyRoutines.Add(NightRoutine);
    }
    
    StartBehaviorTree();
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CurrentGameTime += DeltaTime;
    
    UpdateMemory(DeltaTime);
    ProcessDailyRoutine();
    HandlePlayerDetection();
}

void UNPC_BehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        
        // Update blackboard if available
        AAIController* AIController = GetAIController();
        if (AIController && AIController->GetBlackboardComponent())
        {
            AIController->GetBlackboardComponent()->SetValueAsEnum(TEXT("BehaviorState"), static_cast<uint8>(NewState));
        }
    }
}

void UNPC_BehaviorComponent::StartBehaviorTree()
{
    if (BehaviorTreeAsset)
    {
        AAIController* AIController = GetAIController();
        if (AIController)
        {
            AIController->RunBehaviorTree(BehaviorTreeAsset);
        }
    }
}

void UNPC_BehaviorComponent::StopBehaviorTree()
{
    AAIController* AIController = GetAIController();
    if (AIController)
    {
        UBehaviorTreeComponent* BTComp = AIController->FindComponentByClass<UBehaviorTreeComponent>();
        if (BTComp)
        {
            BTComp->StopTree();
        }
    }
}

void UNPC_BehaviorComponent::RememberLocation(FVector Location)
{
    NPCMemory.KnownLocations.AddUnique(Location);
    
    // Limit memory size
    if (NPCMemory.KnownLocations.Num() > 20)
    {
        NPCMemory.KnownLocations.RemoveAt(0);
    }
}

void UNPC_BehaviorComponent::RememberActor(AActor* Actor)
{
    if (Actor && !NPCMemory.KnownActors.Contains(Actor))
    {
        NPCMemory.KnownActors.AddUnique(Actor);
        
        // Limit memory size
        if (NPCMemory.KnownActors.Num() > 10)
        {
            NPCMemory.KnownActors.RemoveAt(0);
        }
    }
}

bool UNPC_BehaviorComponent::IsLocationKnown(FVector Location, float Tolerance)
{
    for (const FVector& KnownLocation : NPCMemory.KnownLocations)
    {
        if (FVector::Dist(KnownLocation, Location) <= Tolerance)
        {
            return true;
        }
    }
    return false;
}

FNPC_DailyRoutine UNPC_BehaviorComponent::GetCurrentRoutine()
{
    if (!bFollowDailyRoutine || DailyRoutines.Num() == 0)
    {
        return FNPC_DailyRoutine();
    }
    
    float TimeOfDay = FMath::Fmod(CurrentGameTime / 3600.0f, 24.0f); // Convert to hours of day
    
    for (const FNPC_DailyRoutine& Routine : DailyRoutines)
    {
        if (Routine.StartTime <= Routine.EndTime)
        {
            // Normal time range
            if (TimeOfDay >= Routine.StartTime && TimeOfDay < Routine.EndTime)
            {
                return Routine;
            }
        }
        else
        {
            // Overnight routine (e.g., 22:00 to 6:00)
            if (TimeOfDay >= Routine.StartTime || TimeOfDay < Routine.EndTime)
            {
                return Routine;
            }
        }
    }
    
    return DailyRoutines[0]; // Default to first routine
}

void UNPC_BehaviorComponent::UpdateDailyRoutine(float CurrentTime)
{
    CurrentGameTime = CurrentTime;
    ProcessDailyRoutine();
}

void UNPC_BehaviorComponent::RegisterNearbyNPC(AActor* NPC)
{
    if (NPC && !KnownNPCs.Contains(NPC))
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), NPC->GetActorLocation());
        if (Distance <= SocialInteractionRadius)
        {
            KnownNPCs.AddUnique(NPC);
        }
    }
}

TArray<AActor*> UNPC_BehaviorComponent::GetNearbyNPCs()
{
    TArray<AActor*> NearbyNPCs;
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* NPC : KnownNPCs)
    {
        if (NPC && IsValid(NPC))
        {
            float Distance = FVector::Dist(MyLocation, NPC->GetActorLocation());
            if (Distance <= SocialInteractionRadius)
            {
                NearbyNPCs.Add(NPC);
            }
        }
    }
    
    return NearbyNPCs;
}

AAIController* UNPC_BehaviorComponent::GetAIController()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn)
    {
        return Cast<AAIController>(OwnerPawn->GetController());
    }
    return nullptr;
}

void UNPC_BehaviorComponent::UpdateMemory(float DeltaTime)
{
    // Clean up old memories
    for (int32 i = NPCMemory.KnownActors.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(NPCMemory.KnownActors[i]))
        {
            NPCMemory.KnownActors.RemoveAt(i);
        }
    }
    
    // Update player threat assessment
    if (CurrentGameTime - NPCMemory.LastPlayerInteraction > MemoryDuration)
    {
        NPCMemory.bPlayerIsThreat = false;
    }
}

void UNPC_BehaviorComponent::ProcessDailyRoutine()
{
    if (!bFollowDailyRoutine)
    {
        return;
    }
    
    FNPC_DailyRoutine CurrentRoutine = GetCurrentRoutine();
    
    // Update behavior state based on routine
    if (CurrentBehaviorState != CurrentRoutine.Activity)
    {
        SetBehaviorState(CurrentRoutine.Activity);
        
        // Update blackboard target location
        AAIController* AIController = GetAIController();
        if (AIController && AIController->GetBlackboardComponent())
        {
            AIController->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), CurrentRoutine.TargetLocation);
        }
    }
}

void UNPC_BehaviorComponent::HandlePlayerDetection()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find player
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }
    
    AActor* Player = PlayerController->GetPawn();
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    
    if (DistanceToPlayer <= PlayerDetectionRadius)
    {
        // Player detected
        LastKnownPlayer = Player;
        LastPlayerDetectionTime = CurrentGameTime;
        NPCMemory.LastPlayerInteraction = CurrentGameTime;
        
        // Remember player location
        RememberLocation(Player->GetActorLocation());
        RememberActor(Player);
        
        // Update blackboard
        AAIController* AIController = GetAIController();
        if (AIController && AIController->GetBlackboardComponent())
        {
            AIController->GetBlackboardComponent()->SetValueAsObject(TEXT("PlayerActor"), Player);
            AIController->GetBlackboardComponent()->SetValueAsVector(TEXT("PlayerLocation"), Player->GetActorLocation());
            AIController->GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDetected"), true);
        }
        
        // Determine reaction based on current state
        if (CurrentBehaviorState == ENPC_BehaviorState::Idle || CurrentBehaviorState == ENPC_BehaviorState::Gathering)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
        }
    }
    else if (CurrentGameTime - LastPlayerDetectionTime > 10.0f) // Lost player for 10 seconds
    {
        // Player lost
        AAIController* AIController = GetAIController();
        if (AIController && AIController->GetBlackboardComponent())
        {
            AIController->GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDetected"), false);
        }
        
        // Return to routine if investigating
        if (CurrentBehaviorState == ENPC_BehaviorState::Investigating)
        {
            FNPC_DailyRoutine CurrentRoutine = GetCurrentRoutine();
            SetBehaviorState(CurrentRoutine.Activity);
        }
    }
}