#include "NPCBehaviorManager.h"
#include "NPCBehaviorComponent.h"
#include "NPCBehaviorCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"

UNPC_BehaviorManager::UNPC_BehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for behavior updates
    
    // Initialize behavior parameters
    BehaviorUpdateInterval = 0.5f;
    MaxNPCsPerFrame = 5;
    LastUpdateTime = 0.0f;
    CurrentNPCIndex = 0;
    
    // Default behavior weights
    DefaultBehaviorWeights.Add(ENPC_BehaviorType::Idle, 0.3f);
    DefaultBehaviorWeights.Add(ENPC_BehaviorType::Wander, 0.25f);
    DefaultBehaviorWeights.Add(ENPC_BehaviorType::Gather, 0.2f);
    DefaultBehaviorWeights.Add(ENPC_BehaviorType::Social, 0.15f);
    DefaultBehaviorWeights.Add(ENPC_BehaviorType::Flee, 0.1f);
    
    bIsActive = true;
}

void UNPC_BehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all NPCs in the world
    RefreshNPCList();
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorManager initialized with %d NPCs"), RegisteredNPCs.Num());
}

void UNPC_BehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsActive || RegisteredNPCs.Num() == 0)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update behaviors at specified interval
    if (CurrentTime - LastUpdateTime >= BehaviorUpdateInterval)
    {
        UpdateNPCBehaviors(DeltaTime);
        LastUpdateTime = CurrentTime;
    }
    
    // Process social interactions
    ProcessSocialInteractions(DeltaTime);
}

void UNPC_BehaviorManager::RegisterNPC(UNPC_BehaviorComponent* NPCComponent)
{
    if (NPCComponent && !RegisteredNPCs.Contains(NPCComponent))
    {
        RegisteredNPCs.Add(NPCComponent);
        
        // Initialize NPC with default behavior weights
        NPCComponent->SetBehaviorWeights(DefaultBehaviorWeights);
        
        UE_LOG(LogTemp, Log, TEXT("Registered NPC: %s"), 
            NPCComponent->GetOwner() ? *NPCComponent->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UNPC_BehaviorManager::UnregisterNPC(UNPC_BehaviorComponent* NPCComponent)
{
    if (NPCComponent)
    {
        RegisteredNPCs.Remove(NPCComponent);
        
        UE_LOG(LogTemp, Log, TEXT("Unregistered NPC: %s"), 
            NPCComponent->GetOwner() ? *NPCComponent->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UNPC_BehaviorManager::RefreshNPCList()
{
    RegisteredNPCs.Empty();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all actors with NPC behavior components
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (UNPC_BehaviorComponent* NPCComponent = Actor->FindComponentByClass<UNPC_BehaviorComponent>())
        {
            RegisterNPC(NPCComponent);
        }
    }
}

void UNPC_BehaviorManager::UpdateNPCBehaviors(float DeltaTime)
{
    if (RegisteredNPCs.Num() == 0)
    {
        return;
    }
    
    // Update a subset of NPCs each frame to spread the load
    int32 NPCsToUpdate = FMath::Min(MaxNPCsPerFrame, RegisteredNPCs.Num());
    
    for (int32 i = 0; i < NPCsToUpdate; i++)
    {
        int32 NPCIndex = (CurrentNPCIndex + i) % RegisteredNPCs.Num();
        
        if (RegisteredNPCs.IsValidIndex(NPCIndex) && RegisteredNPCs[NPCIndex])
        {
            UNPC_BehaviorComponent* NPC = RegisteredNPCs[NPCIndex];
            
            // Update NPC behavior based on current state and environment
            UpdateIndividualNPC(NPC, DeltaTime);
        }
    }
    
    CurrentNPCIndex = (CurrentNPCIndex + NPCsToUpdate) % RegisteredNPCs.Num();
}

void UNPC_BehaviorManager::UpdateIndividualNPC(UNPC_BehaviorComponent* NPCComponent, float DeltaTime)
{
    if (!NPCComponent || !NPCComponent->GetOwner())
    {
        return;
    }
    
    APawn* NPCPawn = Cast<APawn>(NPCComponent->GetOwner());
    if (!NPCPawn)
    {
        return;
    }
    
    // Get current behavior state
    ENPC_BehaviorType CurrentBehavior = NPCComponent->GetCurrentBehavior();
    
    // Check for environmental threats (dinosaurs, etc.)
    bool bThreatNearby = CheckForThreats(NPCPawn);
    
    if (bThreatNearby && CurrentBehavior != ENPC_BehaviorType::Flee)
    {
        // Force flee behavior when threatened
        NPCComponent->SetCurrentBehavior(ENPC_BehaviorType::Flee);
        NPCComponent->SetBehaviorDuration(FMath::RandRange(5.0f, 15.0f));
        return;
    }
    
    // Normal behavior updates
    float BehaviorTime = NPCComponent->GetBehaviorTime();
    float BehaviorDuration = NPCComponent->GetBehaviorDuration();
    
    if (BehaviorTime >= BehaviorDuration)
    {
        // Choose new behavior based on weights and context
        ENPC_BehaviorType NewBehavior = ChooseNewBehavior(NPCComponent);
        NPCComponent->SetCurrentBehavior(NewBehavior);
        NPCComponent->SetBehaviorDuration(GetBehaviorDuration(NewBehavior));
        NPCComponent->ResetBehaviorTime();
    }
}

bool UNPC_BehaviorManager::CheckForThreats(APawn* NPCPawn)
{
    if (!NPCPawn)
    {
        return false;
    }
    
    UWorld* World = NPCPawn->GetWorld();
    if (!World)
    {
        return false;
    }
    
    FVector NPCLocation = NPCPawn->GetActorLocation();
    float ThreatDetectionRadius = 1500.0f; // 15 meters
    
    // Check for dinosaurs or other threats within detection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == NPCPawn)
        {
            continue;
        }
        
        float Distance = FVector::Dist(NPCLocation, Actor->GetActorLocation());
        if (Distance <= ThreatDetectionRadius)
        {
            // Check if it's a dinosaur or threat (simplified check by name)
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Dinosaur")) || 
                ActorName.Contains(TEXT("Rex")) || 
                ActorName.Contains(TEXT("Raptor")))
            {
                return true;
            }
        }
    }
    
    return false;
}

ENPC_BehaviorType UNPC_BehaviorManager::ChooseNewBehavior(UNPC_BehaviorComponent* NPCComponent)
{
    if (!NPCComponent)
    {
        return ENPC_BehaviorType::Idle;
    }
    
    // Get behavior weights
    TMap<ENPC_BehaviorType, float> Weights = NPCComponent->GetBehaviorWeights();
    
    // Calculate total weight
    float TotalWeight = 0.0f;
    for (auto& WeightPair : Weights)
    {
        TotalWeight += WeightPair.Value;
    }
    
    if (TotalWeight <= 0.0f)
    {
        return ENPC_BehaviorType::Idle;
    }
    
    // Weighted random selection
    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (auto& WeightPair : Weights)
    {
        CurrentWeight += WeightPair.Value;
        if (RandomValue <= CurrentWeight)
        {
            return WeightPair.Key;
        }
    }
    
    return ENPC_BehaviorType::Idle;
}

float UNPC_BehaviorManager::GetBehaviorDuration(ENPC_BehaviorType BehaviorType)
{
    switch (BehaviorType)
    {
        case ENPC_BehaviorType::Idle:
            return FMath::RandRange(3.0f, 8.0f);
        case ENPC_BehaviorType::Wander:
            return FMath::RandRange(10.0f, 20.0f);
        case ENPC_BehaviorType::Gather:
            return FMath::RandRange(15.0f, 30.0f);
        case ENPC_BehaviorType::Social:
            return FMath::RandRange(5.0f, 15.0f);
        case ENPC_BehaviorType::Flee:
            return FMath::RandRange(8.0f, 20.0f);
        default:
            return 5.0f;
    }
}

void UNPC_BehaviorManager::ProcessSocialInteractions(float DeltaTime)
{
    // Process social interactions between NPCs
    float SocialRadius = 500.0f; // 5 meters
    
    for (int32 i = 0; i < RegisteredNPCs.Num(); i++)
    {
        UNPC_BehaviorComponent* NPC1 = RegisteredNPCs[i];
        if (!NPC1 || !NPC1->GetOwner())
        {
            continue;
        }
        
        FVector NPC1Location = NPC1->GetOwner()->GetActorLocation();
        
        for (int32 j = i + 1; j < RegisteredNPCs.Num(); j++)
        {
            UNPC_BehaviorComponent* NPC2 = RegisteredNPCs[j];
            if (!NPC2 || !NPC2->GetOwner())
            {
                continue;
            }
            
            FVector NPC2Location = NPC2->GetOwner()->GetActorLocation();
            float Distance = FVector::Dist(NPC1Location, NPC2Location);
            
            if (Distance <= SocialRadius)
            {
                // NPCs are close enough to interact
                ProcessNPCInteraction(NPC1, NPC2, Distance, DeltaTime);
            }
        }
    }
}

void UNPC_BehaviorManager::ProcessNPCInteraction(UNPC_BehaviorComponent* NPC1, UNPC_BehaviorComponent* NPC2, float Distance, float DeltaTime)
{
    if (!NPC1 || !NPC2)
    {
        return;
    }
    
    // Simple social interaction - both NPCs might switch to social behavior
    float InteractionChance = 0.1f; // 10% chance per second when close
    
    if (FMath::RandRange(0.0f, 1.0f) < InteractionChance * DeltaTime)
    {
        // Both NPCs engage in social behavior
        if (NPC1->GetCurrentBehavior() != ENPC_BehaviorType::Flee)
        {
            NPC1->SetCurrentBehavior(ENPC_BehaviorType::Social);
            NPC1->SetBehaviorDuration(FMath::RandRange(5.0f, 10.0f));
        }
        
        if (NPC2->GetCurrentBehavior() != ENPC_BehaviorType::Flee)
        {
            NPC2->SetCurrentBehavior(ENPC_BehaviorType::Social);
            NPC2->SetBehaviorDuration(FMath::RandRange(5.0f, 10.0f));
        }
    }
}

TArray<UNPC_BehaviorComponent*> UNPC_BehaviorManager::GetNearbyNPCs(FVector Location, float Radius)
{
    TArray<UNPC_BehaviorComponent*> NearbyNPCs;
    
    for (UNPC_BehaviorComponent* NPC : RegisteredNPCs)
    {
        if (!NPC || !NPC->GetOwner())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Location, NPC->GetOwner()->GetActorLocation());
        if (Distance <= Radius)
        {
            NearbyNPCs.Add(NPC);
        }
    }
    
    return NearbyNPCs;
}

void UNPC_BehaviorManager::SetBehaviorManagerActive(bool bActive)
{
    bIsActive = bActive;
    
    UE_LOG(LogTemp, Log, TEXT("NPCBehaviorManager set to %s"), 
        bActive ? TEXT("Active") : TEXT("Inactive"));
}