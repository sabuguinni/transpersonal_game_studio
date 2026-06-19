#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.25f; // 4Hz update — sufficient for NPC logic
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Populate default dialogue lines for Elder NPC
    // Audio URLs from TTS generation in PROD_CYCLE_AUTO_20260619_007
    FNPC_DialogueLine ElderLine1;
    ElderLine1.Text = TEXT("Stay low. The T-Rex hunts by movement. If you freeze, it loses you in the brush. Wait for it to pass, then run.");
    ElderLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781857929567_SurvivorNPC_Elder.mp3");
    ElderLine1.TriggerRadius = 500.0f;

    FNPC_DialogueLine HunterLine1;
    HunterLine1.Text = TEXT("The raptors move in packs. Kill one and the others scatter — but they remember. They will come back for you at night.");
    HunterLine1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781857939292_SurvivorNPC_Hunter.mp3");
    HunterLine1.TriggerRadius = 400.0f;

    DialogueLines.Add(ElderLine1);
    DialogueLines.Add(HunterLine1);

    CurrentState = ENPC_BehaviorState::Idle;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateBehavior(DeltaTime);
    DecayMemories(DeltaTime);
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
    }
}

void UNPCBehaviorComponent::RecordMemory(const FString& EventTag, const FVector& Location, float ThreatLevel)
{
    // Check if memory already exists for this event tag — update if so
    for (FNPC_Memory& Mem : MemoryBank)
    {
        if (Mem.EventTag == EventTag)
        {
            Mem.EventLocation = Location;
            Mem.ThreatLevel = FMath::Max(Mem.ThreatLevel, ThreatLevel);
            if (GetOwner() && GetOwner()->GetWorld())
            {
                Mem.TimeStamp = GetOwner()->GetWorld()->GetTimeSeconds();
            }
            return;
        }
    }

    // New memory entry
    FNPC_Memory NewMemory;
    NewMemory.EventTag = EventTag;
    NewMemory.EventLocation = Location;
    NewMemory.ThreatLevel = ThreatLevel;
    if (GetOwner() && GetOwner()->GetWorld())
    {
        NewMemory.TimeStamp = GetOwner()->GetWorld()->GetTimeSeconds();
    }
    MemoryBank.Add(NewMemory);

    // Transition to alert if threat is significant
    if (ThreatLevel > 0.5f && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
    if (ThreatLevel > 0.8f)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
}

bool UNPCBehaviorComponent::HasMemoryOfThreat(float MinThreatLevel) const
{
    for (const FNPC_Memory& Mem : MemoryBank)
    {
        if (Mem.ThreatLevel >= MinThreatLevel)
        {
            return true;
        }
    }
    return false;
}

FString UNPCBehaviorComponent::GetContextualDialogue(const FVector& PlayerLocation) const
{
    if (!GetOwner()) return FString();

    const FVector OwnerLocation = GetOwner()->GetActorLocation();
    const float DistToPlayer = FVector::Dist(OwnerLocation, PlayerLocation);

    // Return nearest in-range dialogue
    for (const FNPC_DialogueLine& Line : DialogueLines)
    {
        if (DistToPlayer <= Line.TriggerRadius)
        {
            return Line.Text;
        }
    }

    // Contextual fallback based on state
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Alert:
            return TEXT("Something is out there. Stay close.");
        case ENPC_BehaviorState::Flee:
            return TEXT("Run! Do not look back!");
        case ENPC_BehaviorState::Shelter:
            return TEXT("We are safe here for now. Rest.");
        default:
            return FString();
    }
}

void UNPCBehaviorComponent::UpdateBehavior(float DeltaTime)
{
    StateTimer += DeltaTime;

    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            // After 10s idle, begin patrol
            if (StateTimer > 10.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;

        case ENPC_BehaviorState::Patrol:
            // Patrol continues until threat detected (via RecordMemory)
            // Return to idle after 30s patrol cycle
            if (StateTimer > 30.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            break;

        case ENPC_BehaviorState::Alert:
            // Stay alert for 15s then return to patrol
            if (StateTimer > 15.0f && !HasMemoryOfThreat(0.5f))
            {
                SetBehaviorState(ENPC_BehaviorState::Patrol);
            }
            break;

        case ENPC_BehaviorState::Flee:
            // Flee for 8s then seek shelter
            if (StateTimer > 8.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Shelter);
            }
            break;

        case ENPC_BehaviorState::Shelter:
            // Stay sheltered for 20s then cautiously patrol
            if (StateTimer > 20.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Alert);
            }
            break;

        default:
            break;
    }
}

void UNPCBehaviorComponent::DecayMemories(float DeltaTime)
{
    // Decay threat levels over time — NPCs forget danger gradually
    for (FNPC_Memory& Mem : MemoryBank)
    {
        Mem.ThreatLevel = FMath::Max(0.0f, Mem.ThreatLevel - MemoryDecayRate * DeltaTime);
    }

    // Remove fully decayed memories
    MemoryBank.RemoveAll([](const FNPC_Memory& M) { return M.ThreatLevel <= 0.0f; });
}
